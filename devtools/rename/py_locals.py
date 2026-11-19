#!/usr/bin/env python3
"""Rename local variables in the compiler (mpcl) by the naming rules.

Only names that are local to a function or comprehension are renamed:
not parameters (callers may pass them by keyword), not functions or
classes defined inside functions (their __name__ can end up in the
output), not names declared global or nonlocal. Functions that look at
their own namespace (locals(), vars(), exec, eval) are left alone.

A new name is used only if it does not occur anywhere in the enclosing
top-level function and is neither a global nor a builtin of the module.

Usage: py_locals.py [--table OUT.tsv] FILE...
"""

import argparse
import builtins
import os
import re
import sys

import libcst as cst
import libcst.metadata as meta

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from naming import new_name  # noqa: E402

# where the general dictionary guesses wrong
PYTHON_OVERRIDES = {
    'n_opt_loops_inst': 'n_optimized_loops_instruction',
    'n_opt_loops_reg': 'n_optimized_loops_register',
}

INTROSPECTION = {'locals', 'vars', 'exec', 'eval', 'globals'}


class NameCollector(cst.CSTVisitor):
    def __init__(self):
        self.names = set()

    def visit_Name(self, node):
        self.names.add(node.value)


def names_in(node):
    collector = NameCollector()
    node.visit(collector)
    return collector.names


def python_name(name):
    """The new spelling of a local, or None to keep it. Names that would
    only change case (N, X_sub, nYdf) keep their mathematical spelling;
    real camelCase (varargCount) becomes snake_case."""
    if name in PYTHON_OVERRIDES:
        return PYTHON_OVERRIDES[name]
    new = new_name(name, 'snake')
    if not new or new == name:
        return None
    new = re.sub(r'gf2_n(?![a-z0-9])', 'gf2n', new)
    if new.replace('_', '') == name.lower().replace('_', '') and \
            not re.search(r'[a-z]{2}[A-Z][a-z]{2}', name):
        return None
    return new


class DeclarationCollector(cst.CSTVisitor):
    def __init__(self):
        self.names = set()

    def visit_Global(self, node):
        self.names.update(item.name.value for item in node.names)

    def visit_Nonlocal(self, node):
        self.names.update(item.name.value for item in node.names)


def declared_names(node):
    collector = DeclarationCollector()
    node.visit(collector)
    return collector.names


def outermost_function(scope):
    """The top-level function (or the comprehension at module level) that
    contains a scope."""
    while not isinstance(scope.parent, (meta.GlobalScope, meta.ClassScope)):
        scope = scope.parent
    return scope


class Planner(cst.CSTVisitor):
    METADATA_DEPENDENCIES = (meta.ScopeProvider, meta.ParentNodeProvider)

    def __init__(self, module_names):
        self.module_names = module_names
        self.renames = {}   # Name node -> new spelling
        self.table = []
        self.taken = {}     # top-level function -> names in use there
        self.declared = {}  # top-level function -> global/nonlocal names

    def plan_scope(self, scope, owner):
        """``owner`` is the node whose subtree must not contain the new name."""
        if owner not in self.taken:
            used = names_in(owner)
            self.taken[owner] = None if used & INTROSPECTION else \
                used | self.module_names | set(dir(builtins))
            self.declared[owner] = declared_names(owner)
        taken = self.taken[owner]
        if taken is None:
            return
        declared_elsewhere = set()
        for assignment in scope.assignments:
            if not isinstance(assignment, meta.Assignment):
                declared_elsewhere.add(assignment.name)
        groups = {}
        for assignment in scope.assignments:
            groups.setdefault(assignment.name, []).append(assignment)
        for name, assignments in sorted(groups.items()):
            if name in declared_elsewhere or name.startswith('__') or \
                    name == '_':
                continue
            if any(not self.renamable(a) for a in assignments):
                continue
            if name in self.declared[owner]:
                continue
            new = python_name(name)
            if not new or new in taken or not new.isidentifier():
                continue
            taken.add(new)
            nodes = []
            for assignment in assignments:
                nodes.append(assignment.node)
                nodes.extend(ref.node for ref in assignment.references)
            for node in nodes:
                if not isinstance(node, cst.Name) or node.value != name:
                    break
            else:
                for node in nodes:
                    self.renames[node] = new
                self.table.append((name, new))

    def renamable(self, assignment):
        node = assignment.node
        parent = self.get_metadata(meta.ParentNodeProvider, node, None)
        # parameters, defs, classes and imports keep their names
        if isinstance(parent, (cst.Param, cst.FunctionDef, cst.ClassDef,
                               cst.ImportAlias, cst.AsName, cst.Global,
                               cst.Nonlocal, cst.Lambda)):
            return False
        if isinstance(parent, cst.AsName):
            return False
        return isinstance(node, cst.Name)

    def visit_FunctionDef(self, node):
        scope = self.get_metadata(meta.ScopeProvider, node.body, None)
        if scope is None:
            return
        self.plan_scope(scope, self.top_level(node))

    def top_level(self, node):
        top = node
        current = node
        while True:
            parent = self.get_metadata(meta.ParentNodeProvider, current, None)
            if parent is None or isinstance(parent, (cst.Module,
                                                     cst.ClassDef)):
                return top
            if isinstance(parent, (cst.FunctionDef, cst.Lambda)):
                top = parent
            current = parent


class Renamer(cst.CSTTransformer):
    def __init__(self, renames):
        self.renames = renames

    def leave_Name(self, original, updated):
        new = self.renames.get(original)
        if new:
            return updated.with_changes(value=new)
        return updated


def module_level_names(module):
    names = set()
    for statement in module.body:
        names |= names_in(statement) if not isinstance(
            statement, (cst.FunctionDef, cst.ClassDef)) else set()
        if isinstance(statement, (cst.FunctionDef, cst.ClassDef)):
            names.add(statement.name.value)
    return names


def process(path):
    with open(path) as f:
        source = f.read()
    module = cst.parse_module(source)
    wrapper = meta.MetadataWrapper(module)
    planner = Planner(module_level_names(module))
    wrapper.visit(planner)
    if not planner.renames:
        return []
    new_module = wrapper.module.visit(Renamer(planner.renames))
    with open(path, 'w') as f:
        f.write(new_module.code)
    return planner.table


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--table')
    parser.add_argument('files', nargs='+')
    options = parser.parse_args()
    rows = []
    for path in options.files:
        for old, new in process(path):
            rows.append((path, old, new))
    if options.table:
        with open(options.table, 'w') as out:
            out.write('# file\told\tnew  (written by devtools/rename/py_locals.py)\n')
            for row in rows:
                out.write('\t'.join(row) + '\n')
    print('%d local names renamed in %d files' %
          (len(rows), len({r[0] for r in rows})))


if __name__ == '__main__':
    main()

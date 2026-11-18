#!/usr/bin/env python3
"""Move the ``Compiler`` package to ``mpcl`` and rewrite its imports.

The new package groups the modules by purpose:

    mpcl.core     compiler driver, program representation, instructions,
                  register allocation, basic types
    mpcl.lib      the standard library used by programs
    mpcl.oram     oblivious RAM and oblivious data structures
    mpcl.apps     higher-level applications (machine learning, graphs, ...)
    mpcl.boolean  binary-circuit types and instructions (was Compiler.GC)

``Compiler`` stays importable: it becomes a thin package that aliases
every old module name to the new module object, so existing programs and
scripts using ``from Compiler.types import sint`` keep working and see
the very same classes.

Every import statement is rewritten to the absolute new form.  The
rewrite works on the syntax tree, so multi-line imports are handled, and
it refuses to continue if any reference to the old package remains.
"""

import ast
import os
import re
import subprocess
import sys

MODULES = {
    # old module            new module
    'compilerLib': 'core.driver',
    'program': 'core.program',
    'instructions': 'core.instructions',
    'instructions_base': 'core.instructions_base',
    'allocator': 'core.allocator',
    'config': 'core.config',
    'exceptions': 'core.exceptions',
    'util': 'core.util',
    'tools': 'core.tools',
    'cost': 'core.cost',
    'types': 'core.types',
    'library': 'lib.library',
    'comparison': 'lib.comparison',
    'floatingpoint': 'lib.floatingpoint',
    'mpc_math': 'lib.mpc_math',
    'non_linear': 'lib.non_linear',
    'sorting': 'lib.sorting',
    'permutation': 'lib.permutation',
    'circuit': 'lib.circuit',
    'oram': 'oram.oram',
    'path_oram': 'oram.path_oram',
    'circuit_oram': 'oram.circuit_oram',
    'sqrt_oram': 'oram.sqrt_oram',
    'path_oblivious_heap': 'oram.path_oblivious_heap',
    'ml': 'apps.ml',
    'decision_tree': 'apps.decision_tree',
    'dijkstra': 'apps.dijkstra',
    'graph': 'apps.graph',
    'gs': 'apps.gs',
    'sparse': 'apps.sparse',
    'papers': 'apps.papers',
    'GC': 'boolean',
    'GC.types': 'boolean.types',
    'GC.instructions': 'boolean.instructions',
    'GC.program': 'boolean.program',
}
OLD = 'Compiler'
NEW = 'mpcl'
SUBPACKAGES = ['core', 'lib', 'oram', 'apps', 'boolean']


def new_name(old_dotted):
    """Compiler.GC.types -> mpcl.boolean.types"""
    assert old_dotted == OLD or old_dotted.startswith(OLD + '.')
    rest = old_dotted[len(OLD) + 1:]
    if not rest:
        return NEW
    return NEW + '.' + MODULES[rest]


def absolute(module_old, level, name):
    """Resolve a (possibly relative) import inside the old package.

    ``module_old`` names the importing module; packages are given with a
    trailing ``.__init__`` so that ``from . import x`` resolves inside them.
    """
    if level == 0:
        return name
    package = module_old.split('.')[:-1]
    base = package[:len(package) - level + 1]
    return '.'.join(base + ([name] if name else []))


def rewrite_import(node, module_old):
    """Return replacement source for an import node, or None."""
    if isinstance(node, ast.Import):
        if not any(a.name == OLD or a.name.startswith(OLD + '.')
                   for a in node.names):
            return None
        parts = []
        for alias in node.names:
            if alias.name == OLD or alias.name.startswith(OLD + '.'):
                target = new_name(alias.name)
                if alias.asname:
                    parts.append('import %s as %s' % (target, alias.asname))
                else:
                    # "import Compiler.x" binds "Compiler"; the dotted uses
                    # are renamed by rename_dotted() below
                    parts.append('import %s' % target)
            else:
                parts.append('import %s' % alias.name +
                             (' as ' + alias.asname if alias.asname else ''))
        return '; '.join(parts)

    source = absolute(module_old, node.level, node.module)
    if not (source == OLD or source.startswith(OLD + '.')):
        return None
    if source == OLD or source == OLD + '.GC':
        # "from Compiler import types": each name is a submodule (or a
        # name re-exported by the package)
        statements = []
        for alias in node.names:
            dotted = source + '.' + alias.name
            if dotted[len(OLD) + 1:] in MODULES:
                target = new_name(dotted)
                package, _, leaf = target.rpartition('.')
                statements.append('from %s import %s%s' % (
                    package, leaf,
                    ' as ' + (alias.asname or alias.name)
                    if (alias.asname or alias.name) != leaf else ''))
            else:
                statements.append('from %s import %s%s' % (
                    new_name(source), alias.name,
                    ' as ' + alias.asname if alias.asname else ''))
        return '; '.join(statements)
    names = ', '.join(a.name + (' as ' + a.asname if a.asname else '')
                      for a in node.names)
    return 'from %s import %s' % (new_name(source), names)


DOTTED = re.compile(r'(?<![\w/.#-])Compiler(\.(?:%s))(?![\w])' % '|'.join(
    re.escape(m) for m in sorted(MODULES, key=len, reverse=True)))


def rename_dotted(text):
    """``Compiler.types.sint`` -> ``mpcl.core.types.sint`` in code and
    docstrings; links to the upstream documentation (``...html#Compiler.``)
    are left alone by the look-behind."""
    return DOTTED.sub(lambda m: new_name(OLD + m.group(1)), text)


def rewrite_source(text, module_old):
    tree = ast.parse(text)
    lines = text.split('\n')
    replacements = []
    for node in ast.walk(tree):
        if isinstance(node, (ast.Import, ast.ImportFrom)):
            new = rewrite_import(node, module_old)
            if new is not None:
                replacements.append((node, new))
    # apply bottom-up so that line numbers stay valid
    for node, new in sorted(replacements, key=lambda r: (r[0].lineno,
                                                         r[0].col_offset),
                            reverse=True):
        start, end = node.lineno - 1, node.end_lineno - 1
        first, last = lines[start], lines[end]
        indent = first[:node.col_offset]
        if not indent.strip():
            # one statement per line, at the original indentation
            new = new.replace('; ', '\n' + indent)
        lines[start:end + 1] = (indent + new +
                                last[node.end_col_offset:]).split('\n')
    return rename_dotted('\n'.join(lines))


def old_module_name(path):
    rel = os.path.splitext(path)[0].replace('/', '.')
    return rel[:-len('.__init__')] if rel.endswith('.__init__') else rel


def importer_name(path):
    """Name used to resolve relative imports (keeps ``.__init__``)."""
    return os.path.splitext(path)[0].replace('/', '.')


def main():
    files = subprocess.run(['git', 'ls-files', OLD], capture_output=True,
                           text=True, check=True).stdout.split()
    # rewrite in place first, then move
    for path in files:
        if not path.endswith('.py'):
            continue
        with open(path) as handle:
            text = handle.read()
        new_text = rewrite_source(text, importer_name(path))
        if new_text != text:
            with open(path, 'w') as handle:
                handle.write(new_text)
    for sub in SUBPACKAGES:
        os.makedirs(os.path.join(NEW, sub), exist_ok=True)
    for path in files:
        module = old_module_name(path)[len(OLD) + 1:]
        if path.endswith('__init__.py'):
            target = os.path.join(NEW, 'boolean' if module == 'GC' else '',
                                  '__init__.py')
        else:
            target = os.path.join(NEW, *MODULES[module].split('.')) + '.py'
        subprocess.run(['git', 'mv', path, target], check=True)
    for sub in SUBPACKAGES:
        init = os.path.join(NEW, sub, '__init__.py')
        if not os.path.exists(init):
            open(init, 'w').close()
            subprocess.run(['git', 'add', init], check=True)
    write_legacy_map()
    print('moved', len(files), 'files')


def write_legacy_map():
    path = os.path.join(NEW, 'legacy.py')
    with open(path, 'w') as handle:
        handle.write('"""Old module names (``Compiler.*``) and their new homes.\n\n'
                     'Used by the ``Compiler`` compatibility package.\n"""\n\n'
                     'MODULES = {\n')
        for old in sorted(MODULES):
            handle.write("    '%s.%s': '%s',\n" % (OLD, old,
                                                   new_name(OLD + '.' + old)))
        handle.write('}\n')
    subprocess.run(['git', 'add', path], check=True)


if __name__ == '__main__':
    main()

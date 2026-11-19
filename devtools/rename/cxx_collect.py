#!/usr/bin/env python3
"""Collect the C++ names declared in this tree and in everything it uses.

Every translation unit in compile_commands.json is parsed with libclang.
For declarations in engine/, parties/ and clients/ the script records the
spelling and the kind (type, function, variable, ...). For declarations
in any other file (the C++ library, Boost, OpenSSL, third_party/, ...)
it records the spelling of everything that can be named from outside:
namespaces, types, functions, members, enumerators and macros. Those
spellings are frozen by the renaming, since tokens are renamed by
spelling.

Usage: cxx_collect.py OUTPUT.json [--jobs N]
Needs a libclang that understands the system headers; set LIBCLANG to
its path (default: Homebrew LLVM).
"""

import argparse
import collections
import json
import multiprocessing
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))
OURS = tuple(os.path.join(ROOT, d) + os.sep
             for d in ('engine', 'parties', 'clients'))
LIBCLANG = os.environ.get('LIBCLANG',
                          '/opt/homebrew/opt/llvm/lib/libclang.dylib')
CLANG = os.path.join(os.path.dirname(os.path.dirname(LIBCLANG)),
                     'bin', 'clang')

ci = None
K = None
TYPE_KINDS = API_KINDS = OUR_KINDS = DESCEND = None


def setup():
    global ci, K, TYPE_KINDS, API_KINDS, OUR_KINDS, DESCEND
    import clang.cindex
    ci = clang.cindex
    if not ci.Config.loaded:
        ci.Config.set_library_file(LIBCLANG)
    K = ci.CursorKind
    TYPE_KINDS = {
        K.CLASS_DECL: 'type', K.STRUCT_DECL: 'type', K.UNION_DECL: 'type',
        K.ENUM_DECL: 'type', K.TYPEDEF_DECL: 'type',
        K.TYPE_ALIAS_DECL: 'type', K.TYPE_ALIAS_TEMPLATE_DECL: 'type',
        K.CLASS_TEMPLATE: 'type',
        K.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION: 'type',
        K.CONCEPT_DECL: 'type',
        K.TEMPLATE_TYPE_PARAMETER: 'tparam',
        K.TEMPLATE_TEMPLATE_PARAMETER: 'tparam',
        K.TEMPLATE_NON_TYPE_PARAMETER: 'tvalue',
        K.FUNCTION_DECL: 'function', K.CXX_METHOD: 'function',
        K.FUNCTION_TEMPLATE: 'function', K.CONVERSION_FUNCTION: 'function',
        K.FIELD_DECL: 'variable', K.VAR_DECL: 'variable',
        K.PARM_DECL: 'variable',
        K.ENUM_CONSTANT_DECL: 'constant', K.NAMESPACE: 'namespace',
        K.NAMESPACE_ALIAS: 'namespace', K.MACRO_DEFINITION: 'macro',
        K.LABEL_STMT: 'label',
    }
    OUR_KINDS = TYPE_KINDS
    API_KINDS = {k: v for k, v in TYPE_KINDS.items()
                 if v not in ('tparam', 'tvalue', 'label')}
    DESCEND = {K.NAMESPACE, K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL,
               K.CLASS_TEMPLATE, K.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION,
               K.ENUM_DECL, K.LINKAGE_SPEC, K.UNEXPOSED_DECL}


def is_ours(path):
    return os.path.realpath(path).startswith(OURS)


def collect(entry):
    setup()
    args = [a for a in entry['arguments'][1:]
            if a not in ('-c', '-MMD', '-MP', '-Werror')]
    if '-o' in args:
        i = args.index('-o')
        del args[i:i + 2]
    args.remove(entry['file'])
    resource = subprocess.run([CLANG, '-print-resource-dir'],
                              capture_output=True, text=True).stdout.strip()
    sdk = subprocess.run(['xcrun', '--show-sdk-path'], capture_output=True,
                         text=True).stdout.strip()
    args += ['-resource-dir', resource, '-isysroot', sdk, '-Wno-everything']
    os.chdir(entry['directory'])
    tu = ci.Index.create().parse(
        entry['file'], args=args,
        options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    errors = [str(d) for d in tu.diagnostics if d.severity >= 3]
    ours = collections.defaultdict(collections.Counter)
    foreign = set()
    file_cache = {}

    def ours_file(cursor):
        f = cursor.location.file
        if f is None:
            return None
        name = f.name
        if name not in file_cache:
            file_cache[name] = is_ours(name)
        return file_cache[name]

    def visit(cursor):
        for child in cursor.get_children():
            mine = ours_file(child)
            try:
                kind = child.kind
            except ValueError:
                # a cursor kind newer than these Python bindings
                kind = None
            spelling = child.spelling
            if mine is None:
                if kind == K.MACRO_DEFINITION:
                    foreign.add(spelling)
                continue
            if mine:
                what = OUR_KINDS.get(kind)
                if what and spelling:
                    ours[spelling][what] += 1
                visit(child)
            else:
                what = API_KINDS.get(kind)
                if what and spelling:
                    foreign.add(spelling)
                if kind in DESCEND:
                    visit(child)

    visit(tu.cursor)
    includes = sorted({inc.include.name for inc in tu.get_includes()})
    return {'file': entry['file'], 'errors': errors[:5],
            'ours': {k: dict(v) for k, v in ours.items()},
            'foreign': sorted(foreign), 'includes': includes}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('output')
    parser.add_argument('--jobs', type=int, default=os.cpu_count())
    parser.add_argument('--limit', type=int)
    options = parser.parse_args()
    with open(os.path.join(ROOT, 'compile_commands.json')) as f:
        entries = json.load(f)
    if options.limit:
        entries = entries[:options.limit]
    ours = collections.defaultdict(collections.Counter)
    foreign = set()
    includes = set()
    errors = {}
    with multiprocessing.Pool(options.jobs) as pool:
        for n, result in enumerate(pool.imap_unordered(collect, entries)):
            for name, kinds in result['ours'].items():
                ours[name].update(kinds)
            foreign.update(result['foreign'])
            includes.update(result['includes'])
            if result['errors']:
                errors[result['file']] = result['errors']
            print('%d/%d %s' % (n + 1, len(entries), result['file']),
                  file=sys.stderr)
    with open(options.output, 'w') as out:
        json.dump({'ours': {k: dict(v) for k, v in sorted(ours.items())},
                   'foreign': sorted(foreign),
                   'includes': sorted(includes), 'errors': errors},
                  out, indent=0)
    print('%d own names, %d foreign names, %d files with errors' %
          (len(ours), len(foreign), len(errors)))


if __name__ == '__main__':
    main()

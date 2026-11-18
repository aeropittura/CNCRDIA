#!/usr/bin/env python3
"""Move the C++ sources into the new layout and rewrite every include.

Steps:
  1. compute the mapping (layout.plan) and resolve every quoted/angle
     include of every tracked C++ file against the *old* tree, using the
     same search order as the old build (includer dir, local/include, .,
     deps);
  2. ``git mv`` the files and move the submodules from deps/ to third_party/;
  3. rewrite the includes to the canonical new spelling (paths below
     engine/ are written relative to engine/, everything else relative
     to the repository root);
  4. re-resolve every rewritten include against the *new* tree with the
     new search order and fail if any of them lands on a different file.

Run from the repository root on a clean work tree.
"""

import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import layout  # noqa: E402

INCLUDE = re.compile(r'^(\s*#\s*include\s*)([<"])([^>"]+)([>"])(.*)$')
CODE = ('.h', '.hpp', '.cpp')
OLD_SEARCH = ['local/include', '.', 'deps']
NEW_SEARCH = ['local/include', '.', 'engine', 'third_party']
SUBMODULE_MOVES = {
    'deps/SimpleOT': 'third_party/SimpleOT',
    'deps/SimplestOT_C': 'third_party/SimplestOT_C',
    'deps/libOTe': 'third_party/libOTe',
    'deps/libOTe2': 'third_party/libOTe2',
    'deps/secure-join': 'third_party/secure-join',
    'deps/simde': 'third_party/simde',
    'deps/sse2neon': 'third_party/sse2neon',
}


def git(*args, capture=True):
    return subprocess.run(['git'] + list(args), check=True, text=True,
                          capture_output=capture).stdout


def resolve(includer, spelled, angle, tracked, search):
    candidates = []
    if not angle:
        candidates.append(os.path.join(os.path.dirname(includer), spelled))
    candidates += [os.path.join(base, spelled) for base in search]
    for candidate in candidates:
        candidate = os.path.normpath(candidate)
        if candidate in tracked:
            return candidate
        if os.path.isfile(candidate):
            return 'external:' + candidate
    return None


def spelling(target):
    if target.startswith('engine/'):
        return target[len('engine/'):]
    return target


def external_spelling(old_external, spelled):
    """Third-party headers: keep the spelling unless it named deps/."""
    path = old_external[len('external:'):]
    if spelled.startswith('deps/'):
        for old, new in SUBMODULE_MOVES.items():
            if path.startswith(old + '/'):
                return new + path[len(old):]
    return spelled


def main():
    tracked_list = [f for f in git('ls-files').split('\n') if f]
    tracked = set(tracked_list)
    mapping = layout.plan(tracked_list)
    code_files = [f for f in tracked_list if f.endswith(CODE)
                  and not f.startswith(('deps/', 'Programs/Circuits'))]

    # 1. resolve against the old tree
    edits = {}  # old file -> list of (line number, new line, new target)
    for path in code_files:
        with open(path) as handle:
            lines = handle.read().split('\n')
        for number, line in enumerate(lines):
            match = INCLUDE.match(line)
            if not match:
                continue
            prefix, opening, spelled, _, rest = match.groups()
            target = resolve(path, spelled, opening == '<', tracked,
                             OLD_SEARCH)
            if target is None:
                continue  # system header
            if target.startswith('external:'):
                new_spelled = external_spelling(target, spelled)
                expected = target  # checked after the move by content
            else:
                expected = mapping.get(target, target)
                new_spelled = spelling(expected)
                opening = '"'
            closing = '"' if opening == '"' else '>'
            new_line = '%s%s%s%s%s' % (prefix, opening, new_spelled,
                                       closing, rest)
            edits.setdefault(path, []).append((number, new_line, expected))

    # 2. move
    for old, new in mapping.items():
        os.makedirs(os.path.dirname(new), exist_ok=True)
        git('mv', old, new)
    for old, new in SUBMODULE_MOVES.items():
        if os.path.exists(old):
            os.makedirs(os.path.dirname(new), exist_ok=True)
            git('mv', old, new)

    with open(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           'moved-files.txt'), 'w') as handle:
        for old, new in list(mapping.items()) + list(SUBMODULE_MOVES.items()):
            handle.write('%s %s\n' % (old, new))

    # 3. rewrite
    for old_path, changes in edits.items():
        path = mapping.get(old_path, old_path)
        with open(path) as handle:
            lines = handle.read().split('\n')
        for number, new_line, _ in changes:
            lines[number] = new_line
        with open(path, 'w') as handle:
            handle.write('\n'.join(lines))

    # 4. verify against the new tree
    new_tracked = set(mapping.get(f, f) for f in tracked_list)
    failures = 0
    for old_path, changes in edits.items():
        path = mapping.get(old_path, old_path)
        for number, new_line, expected in changes:
            match = INCLUDE.match(new_line)
            spelled, angle = match.group(3), match.group(2) == '<'
            found = resolve(path, spelled, angle, new_tracked, NEW_SEARCH)
            if expected.startswith('external:'):
                ok = found is not None and found.startswith('external:')
                if ok and os.path.realpath(found[9:]) != os.path.realpath(
                        external_new_path(expected[9:])):
                    ok = False
            else:
                ok = found == expected
            if not ok:
                failures += 1
                print('MISRESOLVED %s:%d %s -> %s (wanted %s)' %
                      (path, number + 1, spelled, found, expected))
    print('%d files moved, %d files with includes rewritten, %d failures'
          % (len(mapping), len(edits), failures))
    sys.exit(1 if failures else 0)


def external_new_path(path):
    for old, new in SUBMODULE_MOVES.items():
        if path.startswith(old + '/'):
            return new + path[len(old):]
    return path


if __name__ == '__main__':
    main()

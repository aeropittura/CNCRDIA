#!/usr/bin/env python3
"""Write compile_commands.json for the C++ sources (clangd, clang-tidy).

The commands are taken from a dry run of make, so they carry exactly the
flags of the real build, including CONFIG.mine.

Usage: devtools/compile_commands.py [TARGET ...]   (default: every
executable in tests/baseline/binaries.txt)
"""

import json
import os
import shlex
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    targets = sys.argv[1:]
    if not targets:
        with open(os.path.join(ROOT, 'tests', 'baseline', 'binaries.txt')) as f:
            targets = f.read().split()
    dry_run = subprocess.run(['make', '-n', '-B'] + targets, cwd=ROOT,
                             capture_output=True, text=True).stdout
    entries = {}
    for line in dry_run.splitlines():
        words = shlex.split(line) if ' -c' in line else []
        if not words or '-o' not in words:
            continue
        sources = [w for w in words if w.endswith('.cpp')]
        if len(sources) != 1 or sources[0].startswith(('/', 'third_party')):
            continue
        entries[sources[0]] = {'directory': ROOT, 'file': sources[0],
                               'arguments': words}
    with open(os.path.join(ROOT, 'compile_commands.json'), 'w') as out:
        json.dump(sorted(entries.values(), key=lambda e: e['file']), out,
                  indent=1)
    print('%d entries in compile_commands.json' % len(entries))


if __name__ == '__main__':
    main()

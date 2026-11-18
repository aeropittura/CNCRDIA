#!/usr/bin/env python3
"""Rewrite textual references to moved files (docs, comments, scripts).

Usage: rewrite_references.py MAPPING_FILE [PATH ...]

MAPPING_FILE has one ``old new`` pair per line (as written by apply.py).
Every tracked text file outside third_party/ is scanned, or only the
given paths.  An occurrence is replaced only when it is not part of a
longer path, so ``GC/Secret.h`` does not match ``XGC/Secret.h``.
"""

import re
import subprocess
import sys

SKIP_PREFIXES = ('third_party/', 'deps/', 'Programs/Circuits', 'tests/baseline/')
TEXT_SUFFIXES = ('.h', '.hpp', '.cpp', '.py', '.sh', '.rst', '.md', '.txt',
                 '.mpc', '.yml', '.yaml', 'Dockerfile', 'Doxyfile', '.mk',
                 'Makefile', '.cfg', '.toml')


def main():
    with open(sys.argv[1]) as handle:
        pairs = [line.split() for line in handle if line.strip()]
    # longest first so that BMR/network/x wins over BMR/x
    pairs.sort(key=lambda pair: -len(pair[0]))
    pattern = re.compile(r'(?<![\w/.-])(\.\./)?(' +
                         '|'.join(re.escape(old) for old, _ in pairs) +
                         r')(?![\w])')
    lookup = dict(pairs)
    if len(sys.argv) > 2:
        files = sys.argv[2:]
    else:
        files = subprocess.run(['git', 'ls-files'], capture_output=True,
                               text=True, check=True).stdout.split('\n')
    changed = 0
    for path in files:
        if not path or path.startswith(SKIP_PREFIXES) or \
                not path.endswith(TEXT_SUFFIXES):
            continue
        try:
            with open(path) as handle:
                text = handle.read()
        except (UnicodeDecodeError, FileNotFoundError, IsADirectoryError):
            continue
        new_text = pattern.sub(
            lambda m: (m.group(1) or '') + lookup[m.group(2)], text)
        if new_text != text:
            changed += 1
            with open(path, 'w') as handle:
                handle.write(new_text)
            print('updated', path)
    print(changed, 'files updated')


if __name__ == '__main__':
    main()

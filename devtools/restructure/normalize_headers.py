#!/usr/bin/env python3
"""Normalise C++ file preambles after the move.

* Include guards named after the old paths (``GC_ACCESS_H_``,
  ``PROTOCOLS_SHARE_H_``, ``_Share``...) become ``#pragma once``; headers
  without any guard get one too.  The script verifies first that no guard
  macro is tested anywhere else, which would make the change observable.
* The banner comment that repeated the old file name (``/* * Access.h */``)
  now names the file by its path in the new layout.

Usage: normalize_headers.py [--check]
"""

import os
import re
import subprocess
import sys

GUARD = re.compile(r'^[ \t]*#[ \t]*ifndef[ \t]+(\w+)[ \t]*\n[ \t]*#[ \t]*define[ \t]+\1\b[^\n]*\n',
                   re.M)
TRAILING_ENDIF = re.compile(r'\n[ \t]*#[ \t]*endif\b[^\n]*\s*$')
BANNER = re.compile(r'\A/\*\n \* ([\w.+^-]+\.(?:h|hpp|cpp))\n')
ROOTS = ('engine', 'parties', 'clients')


def tracked(suffixes):
    files = subprocess.run(['git', 'ls-files'] + list(ROOTS),
                           capture_output=True, text=True,
                           check=True).stdout.split()
    return [f for f in files if f.endswith(suffixes)]


def display_name(path):
    return path[len('engine/'):] if path.startswith('engine/') else path


def convert_guard(path, text, foreign_tests):
    match = GUARD.search(text)
    preamble = text[:match.start()] if match else ''
    # the guard must be the first directive of the file
    if match and not re.search(r'^\s*#', preamble, re.M):
        name = match.group(1)
        if name in foreign_tests:
            raise SystemExit('%s: guard %s is tested elsewhere' % (path, name))
        tail = TRAILING_ENDIF.search(text)
        if not tail:
            raise SystemExit('%s: no closing #endif for %s' % (path, name))
        body = text[match.end():tail.start()]
        return preamble + '#pragma once\n' + body.rstrip('\n') + '\n'
    if re.search(r'^\s*#\s*pragma\s+once', text, re.M):
        return text
    # unguarded header: put the pragma after the leading comment block
    banner_end = 0
    banner = re.match(r'\A(/\*.*?\*/\s*\n)', text, re.S)
    if banner:
        banner_end = banner.end()
    return text[:banner_end] + '#pragma once\n\n' + text[banner_end:]


def main():
    check_only = '--check' in sys.argv
    headers = tracked(('.h', '.hpp'))
    sources = tracked(('.h', '.hpp', '.cpp'))

    # every macro that appears in a conditional outside its own guard line
    conditionals = {}
    for path in sources:
        with open(path) as handle:
            for line in handle:
                if re.match(r'\s*#\s*(if|ifdef|ifndef|elif)\b', line):
                    for word in re.findall(r'\b[A-Za-z_]\w*\b', line):
                        conditionals[word] = conditionals.get(word, 0) + 1
    guards = {}
    for path in headers:
        with open(path) as handle:
            match = GUARD.search(handle.read())
        if match:
            guards[match.group(1)] = path
    foreign = {g for g in guards if conditionals.get(g, 0) > 1}

    changed = 0
    for path in sources:
        with open(path) as handle:
            text = handle.read()
        new = text
        if path.endswith(('.h', '.hpp')):
            new = convert_guard(path, new, foreign)
        new = BANNER.sub('/*\n * %s\n' % display_name(path), new, count=1)
        if new != text:
            changed += 1
            if not check_only:
                with open(path, 'w') as handle:
                    handle.write(new)
    print('%d files %s' % (changed, 'would change' if check_only
                           else 'changed'))


if __name__ == '__main__':
    main()

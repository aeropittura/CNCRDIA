#!/usr/bin/env python3
"""Apply a rename table (from cxx_plan.py) to the C++ sources and docs.

In code, every identifier token with an old spelling gets the new one,
except on directive lines and in attributes (see cxx_lex.py). In
comments, only spellings that look like code (an underscore, a digit or
an inner capital) are replaced, plus the name after @param or \\param,
so that ordinary words stay. In documentation, names in code spans and
in doxygen directives are replaced.

Usage: cxx_apply.py TABLE.tsv
"""

import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cxx_lex import tokens  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))
CODE_LIKE = re.compile(r'_|\d|[a-z][A-Z]')
WORD = re.compile(r'[A-Za-z_]\w*')


def load(table):
    mapping = {}
    with open(table) as f:
        for line in f:
            if line.startswith('#') or not line.strip():
                continue
            old, new = line.split('\t')[:2]
            mapping[old] = new
    return mapping


def rename_comment(text, mapping):
    def word(match):
        name = match.group()
        new = mapping.get(name)
        if new and CODE_LIKE.search(name):
            return new
        return name
    text = re.sub(r'([@\\]param(?:\[\w+\])?\s+)(\w+)',
                  lambda m: m.group(1) + mapping.get(m.group(2), m.group(2)),
                  text)
    return WORD.sub(word, text)


def rename_code(text, mapping):
    out = []
    last = 0
    for kind, start, end, frozen in tokens(text):
        if kind == 'ident' and not frozen:
            new = mapping.get(text[start:end])
            if new:
                out.append(text[last:start])
                out.append(new)
                last = end
        elif kind == 'comment':
            new = rename_comment(text[start:end], mapping)
            if new != text[start:end]:
                out.append(text[last:start])
                out.append(new)
                last = end
    out.append(text[last:])
    return ''.join(out)


def python_names():
    names = set()
    for path in tracked('mpcl', 'Compiler', 'examples'):
        if path.endswith(('.py', '.mpc')):
            with open(os.path.join(ROOT, path), encoding='utf-8') as f:
                names.update(WORD.findall(f.read()))
    return names


def rename_doc(text, mapping):
    """Documentation mixes C++ and the Python front end (cint, sint, ...),
    so only names that look like code and are not Python names change."""
    def one(match):
        name = match.group()
        new = mapping.get(name)
        if new and CODE_LIKE.search(name) and name not in PYTHON_NAMES:
            return new
        return name

    def names(fragment):
        return WORD.sub(one, fragment)
    # ``code`` (rst) and `code` (markdown)
    text = re.sub(r'``[^`\n]+``|(?<!`)`[^`\n]+`(?!`)',
                  lambda m: names(m.group()), text)
    # .. doxygenclass:: Name   and its :members: line
    text = re.sub(r'(?m)^(\s*\.\. doxygen\w+::.*)$',
                  lambda m: names(m.group(1)), text)
    text = re.sub(r'(?m)^(\s*:members:.*)$', lambda m: names(m.group(1)),
                  text)
    return text


def tracked(*paths):
    return subprocess.run(['git', 'ls-files'] + list(paths), cwd=ROOT,
                          capture_output=True, text=True,
                          check=True).stdout.split()


PYTHON_NAMES = set()


def main():
    mapping = load(sys.argv[1])
    PYTHON_NAMES.update(python_names())
    changed = 0
    for path in tracked('engine', 'parties', 'clients'):
        if path.endswith(('.h', '.hpp', '.cpp')):
            convert = rename_code
        elif path.endswith(('.md', '.rst')):
            convert = rename_doc
        else:
            continue
        full = os.path.join(ROOT, path)
        if os.path.islink(full):
            continue
        with open(full, encoding='latin-1') as f:
            text = f.read()
        new = convert(text, mapping)
        if new != text:
            changed += 1
            with open(full, 'w', encoding='latin-1') as f:
                f.write(new)
    for path in tracked('docs', 'README.md', 'ARCHITECTURE.md'):
        if not path.endswith(('.md', '.rst')) or \
                os.path.islink(os.path.join(ROOT, path)):
            continue
        full = os.path.join(ROOT, path)
        with open(full, encoding='utf-8') as f:
            text = f.read()
        new = rename_doc(text, mapping)
        if new != text:
            changed += 1
            with open(full, 'w', encoding='utf-8') as f:
                f.write(new)
    print('%d files changed' % changed)


if __name__ == '__main__':
    main()

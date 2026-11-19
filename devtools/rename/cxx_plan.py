#!/usr/bin/env python3
"""Decide the new spelling of every C++ name declared in this tree.

Input: the output of cxx_collect.py. Output: a table ``old new kind``
(cxx-names.tsv), sorted by name. A name is renamed only if

* it is declared in engine/, parties/ or clients/,
* nothing outside the tree (C++ library, Boost, OpenSSL, NTL,
  third_party/, compiler intrinsics) uses the same spelling,
* it is not a macro, a keyword, a reserved name or an attribute,
* the new spelling is not used anywhere yet and no other name gets it.

Usage: cxx_plan.py NAMES.json OUTPUT.tsv
"""

import collections
import json
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cxx_lex import identifiers  # noqa: E402
from naming import new_name  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))

KEYWORDS = set('''alignas alignof and and_eq asm auto bitand bitor bool break
case catch char char8_t char16_t char32_t class compl concept const consteval
constexpr constinit const_cast continue co_await co_return co_yield decltype
default delete do double dynamic_cast else enum explicit export extern false
float for friend goto if inline int long mutable namespace new noexcept not
not_eq nullptr operator or or_eq private protected public register
reinterpret_cast requires return short signed sizeof static static_assert
static_cast struct switch template this thread_local throw true try typedef
typeid typename union unsigned using virtual void volatile wchar_t while xor
xor_eq final override import module defined main restrict'''.split())

# headers that no translation unit includes in this configuration but
# other configurations do (x86, NTL, libOTe), plus third_party/ as a whole
TEXT_ONLY_HEADERS = [os.path.join(ROOT, 'third_party'),
                     '/opt/homebrew/include/NTL',
                     '/usr/local/include/NTL',
                     '/usr/include/NTL']


def clang_headers():
    clang = os.environ.get('CLANG', '/opt/homebrew/opt/llvm/bin/clang')
    resource = subprocess.run([clang, '-print-resource-dir'],
                              capture_output=True, text=True).stdout.strip()
    return [os.path.join(resource, 'include')]


def header_text_names(roots):
    code = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|'
                      r"'(?:\\.|[^'\\\n])*'|([A-Za-z_]\w*)", re.S)
    names = set()
    for root in roots:
        for path, _, files in os.walk(root):
            for name in files:
                if name.endswith(('.h', '.hpp', '.hh', '.ipp', '.inl',
                                  '.hxx', '.tcc')):
                    with open(os.path.join(path, name), 'rb') as f:
                        text = f.read().decode('latin-1')
                    names.update(m.group(1) for m in code.finditer(text)
                                 if m.group(1))
    return names


def own_files():
    files = subprocess.run(['git', 'ls-files', 'engine', 'parties',
                            'clients'], cwd=ROOT, capture_output=True,
                           text=True, check=True).stdout.split()
    return [f for f in files if f.endswith(('.h', '.hpp', '.cpp'))]


def macro_names(texts):
    names = set()
    for text in texts:
        names.update(re.findall(r'^\s*#\s*define\s+(\w+)', text, re.M))
        for line in re.findall(r'^\s*#\s*(?:if|ifdef|ifndef|elif|undef)\b(.*)',
                               text, re.M):
            names.update(re.findall(r'[A-Za-z_]\w*', line))
    with open(os.path.join(ROOT, 'build', 'config.mk')) as f:
        names.update(re.findall(r'-D(\w+)', f.read()))
    return names


def pasted_prefixes(texts):
    """Identifiers glued to ``##`` produce names nobody spells in full."""
    prefixes = set()
    for text in texts:
        prefixes.update(re.findall(r'(\w+)\s*##', text))
    return prefixes


def style_of(kinds):
    if kinds & {'type', 'tparam'}:
        return 'type'
    if 'function' in kinds and not kinds & {'variable'}:
        return 'function'
    if kinds & {'function', 'variable', 'label'}:
        return 'snake'
    return None


def main():
    names_file, output = sys.argv[1:3]
    with open(names_file) as f:
        collected = json.load(f)
    texts = []
    for path in own_files():
        with open(os.path.join(ROOT, path), encoding='latin-1') as f:
            texts.append(f.read())
    in_code = set()
    for text in texts:
        in_code |= identifiers(text)

    frozen = set(collected['foreign']) | KEYWORDS
    frozen |= header_text_names(TEXT_ONLY_HEADERS + clang_headers())
    frozen |= macro_names(texts)
    prefixes = pasted_prefixes(texts)

    taken = frozen | in_code | set(collected['ours'])
    wanted = {}
    reasons = collections.Counter()
    for name, kinds in sorted(collected['ours'].items()):
        if not re.fullmatch(r'[A-Za-z_]\w*', name):
            continue
        kinds = set(kinds)
        if name in frozen or name.startswith('__') or \
                re.match(r'_[A-Z]', name):
            reasons['frozen'] += 1
            continue
        if any(name.startswith(p) for p in prefixes):
            reasons['pasted'] += 1
            continue
        if kinds & {'namespace', 'macro', 'constant', 'tvalue'}:
            reasons['kind kept'] += 1
            continue
        style = style_of(kinds)
        new = new_name(name, style) if style else None
        if new is None or new == name:
            reasons['already fine'] += 1
            continue
        wanted[name] = (new, '/'.join(sorted(kinds)))

    counts = collections.Counter(new for new, _ in wanted.values())
    table = []
    for name, (new, kind) in sorted(wanted.items()):
        if new in taken or counts[new] > 1 or new in KEYWORDS:
            reasons['new name taken'] += 1
            continue
        table.append((name, new, kind))
    with open(output, 'w') as out:
        out.write('# old\tnew\tkind  (written by devtools/rename/cxx_plan.py)\n')
        for row in table:
            out.write('\t'.join(row) + '\n')
    print('%d names renamed; skipped: %s' % (len(table), dict(reasons)))


if __name__ == '__main__':
    main()

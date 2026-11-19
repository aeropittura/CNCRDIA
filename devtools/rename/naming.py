"""Naming rules shared by the C++ and Python renaming.

Names are split into words, abbreviations are spelled out, and the words
are joined in the style for the kind of name:

    types and namespaces of types   PascalCase, acronyms as words (MacCheck)
    functions, variables            snake_case
    constants, macros               unchanged

Leading and trailing underscores and digits stay where they are.
"""

import re

WORD = re.compile(r'[A-Z]{2,}s(?![a-z])|[A-Z]+(?![a-z])(?:[0-9]+[a-z]*)?|[A-Z]?[a-z][a-z0-9]*|[0-9]+[a-z0-9]*')

# abbreviations that are spelled out wherever they are a word of a name
SPELLED_OUT = {
    'mc': 'opener',
    'proc': 'processor',
    'procs': 'processors',
    'msg': 'message',
    'msgs': 'messages',
    'buf': 'buffer',
    'bufs': 'buffers',
    'len': 'length',
    'lens': 'lengths',
    'idx': 'index',
    'ctx': 'context',
    'cnt': 'count',
    'res': 'result',
    'ret': 'result',
    'val': 'value',
    'vals': 'values',
    'tmp': 'temp',
    'elem': 'element',
    'elems': 'elements',
    'col': 'column',
    'cols': 'columns',
    'pos': 'position',
    'prev': 'previous',
    'curr': 'current',
    'cur': 'current',
    'obj': 'object',
    'dir': 'directory',
    'cfg': 'config',
    'conf': 'config',
    'arr': 'array',
    'mul': 'mult',
    'inv': 'inverse',
    'rand': 'random',
    'sq': 'square',
    'sqr': 'square',
    'ptr': 'pointer',
    'addr': 'address',
    'src': 'source',
    'dst': 'destination',
    'dest': 'destination',
    'opt': 'option',
    'opts': 'options',
    'param': 'parameter',
    'params': 'parameters',
    'stats': 'statistics',
    'perm': 'permutation',
    'perms': 'permutations',
    'comm': 'communication',
    'mem': 'memory',
    'regs': 'registers',
    'dim': 'dimension',
    'dims': 'dimensions',
    'coeff': 'coefficient',
    'coeffs': 'coefficients',
    'poly': 'polynomial',
    'polys': 'polynomials',
    'mat': 'matrix',
    'vec': 'vector',
    'vecs': 'vectors',
    'sz': 'size',
    'prot': 'protocol',
    'proto': 'protocol',
    'recv': 'receive',
    'recvd': 'received',
    'cmd': 'command',
    'fname': 'filename',
    'diag': 'diagonal',
}

# different words for type names, which are nouns
TYPE_SPELLED_OUT = {
    'gen': 'generator',
}

# verbs only make sense in function names
FUNCTION_SPELLED_OUT = {
    'init': 'initialize',
    'calc': 'compute',
}

# names whose words cannot be told apart by capitalisation
OVERRIDES = {
    'NIZKPoK': 'nizk_pok',
}

# whole names that must keep their exact meaning even though one of their
# words is in SPELLED_OUT
KEEP_WORDS = {'my_num'}


def split(name):
    """Split a name into (leading underscores, words, trailing underscores)."""
    core = name.strip('_')
    lead = name[:len(name) - len(name.lstrip('_'))]
    trail = name[len(name.rstrip('_')):] if core else ''
    words = []
    for part in core.split('_'):
        found = WORD.findall(part)
        if ''.join(found) != part:
            return None
        for first, second in zip(found, found[1:]):
            # MCp or baseOTstart: where the acronym ends is a guess
            if len(first) == 1 and first.isupper() and second[0].isupper():
                return None
        words.extend(found)
    return lead, words, trail


def spell_out(words, style):
    result = []
    for word in words:
        replacement = None
        if style == 'type':
            replacement = TYPE_SPELLED_OUT.get(word.lower())
        elif style == 'function':
            replacement = FUNCTION_SPELLED_OUT.get(word.lower())
        replacement = replacement or SPELLED_OUT.get(word.lower())
        if replacement is None:
            result.append(word)
        elif word.isupper() and len(word) > 1:
            result.append(replacement.upper())
        elif word[0].isupper():
            result.append(replacement.capitalize())
        else:
            result.append(replacement)
    return result


def pascal(words):
    out = ''
    for word in words:
        piece = word[0].upper() + word[1:].lower()
        # keep digits readable: Rep3, Gf2n, Spdz2k
        if out and out[-1].isdigit() and piece[0].isdigit():
            out += '_'
        out += piece
    return out


def snake(words):
    return '_'.join(word.lower() for word in words)


def new_name(name, style, expand=True):
    """The name in the given style, or None if it cannot be converted."""
    if name in OVERRIDES:
        return OVERRIDES[name]
    if name in KEEP_WORDS or re.fullmatch(r'_*[A-Z][A-Z0-9_]+', name):
        # constants and acronyms keep their spelling
        return name
    parts = split(name)
    if parts is None or not parts[1]:
        return None
    lead, words, trail = parts
    if expand:
        words = spell_out(words, style)
    if style == 'type':
        if len(words) > 1 and words[-1] == 't':
            words = words[:-1]  # endpoint_t is a type anyway
        body = pascal(words)
    elif style in ('snake', 'function'):
        body = snake(words)
    else:
        return None
    return lead + body + trail

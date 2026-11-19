"""A small C++ lexer, just good enough to rename identifiers safely.

It separates comments, string and character literals (including raw
strings and digit separators in numbers) from code, and marks the
preprocessor lines whose identifiers must not change.
"""

import re

TOKEN = re.compile(r'''
    (?P<comment>//[^\n]*|/\*.*?\*/)
  | (?P<string>(?:u8|u|U|L)?R"(?P<delim>[^()\s\\]{0,16})\(.*?\)(?P=delim)"
             | (?:u8|u|U|L)?"(?:\\.|[^"\\\n])*"
             | (?:u8|u|U|L)?'(?:\\.|[^'\\\n])*')
  | (?P<number>\.?[0-9](?:[eEpP][+-]|[\w.']|'(?=\w))*)
  | (?P<ident>[A-Za-z_]\w*)
  | (?P<newline>\n)
  | (?P<other>.)
''', re.S | re.X)

# directives whose identifiers are left alone: file names, macro names in
# conditions, and free text
FROZEN_DIRECTIVES = {'include', 'include_next', 'import', 'pragma', 'if',
                     'ifdef', 'ifndef', 'elif', 'elifdef', 'elifndef',
                     'undef', 'error', 'warning', 'line'}

# contents of __attribute__((...)), [[...]] and __declspec(...) are left
# alone as well
ATTRIBUTE_OPENERS = {'__attribute__', '__attribute', '__declspec', 'alignas',
                     '_Pragma'}


def tokens(text):
    """Yield (kind, start, end, frozen) for every token.

    ``frozen`` is true for identifiers that must keep their spelling
    because of where they are (directive lines, attributes)."""
    at_line_start = True
    directive = None        # the directive of the current logical line
    expect_directive = False
    attribute_depth = None  # parenthesis depth inside an attribute
    paren_depth = 0
    bracket_attr = 0
    previous = None
    for match in TOKEN.finditer(text):
        kind = match.lastgroup
        if kind == 'delim':
            kind = 'string'
        start, end = match.span()
        value = match.group()
        if kind == 'newline':
            if directive is not None and text[max(0, start - 1)] != '\\':
                directive = None
            at_line_start = True
            yield kind, start, end, False
            continue
        if kind == 'other' and value in ' \t\r\f\v':
            yield kind, start, end, False
            continue
        if kind == 'other' and value == '#' and at_line_start:
            expect_directive = True
            at_line_start = False
            yield kind, start, end, False
            continue
        at_line_start = False
        frozen = False
        if kind == 'ident':
            if expect_directive:
                directive = value
                expect_directive = False
                frozen = True
            elif directive in FROZEN_DIRECTIVES:
                frozen = True
            elif directive == 'define' and previous == 'define':
                frozen = True  # the macro name
            elif attribute_depth is not None or bracket_attr:
                frozen = True
            elif value in ATTRIBUTE_OPENERS:
                attribute_depth = paren_depth
                frozen = True
            previous = value
        else:
            expect_directive = False
            if kind == 'other':
                if value == '(':
                    paren_depth += 1
                elif value == ')':
                    paren_depth -= 1
                    if attribute_depth is not None and \
                            paren_depth <= attribute_depth:
                        attribute_depth = None
                elif value == '[' and text.startswith('[[', start) and \
                        not bracket_attr:
                    bracket_attr = 2
                elif value == ']' and bracket_attr:
                    bracket_attr -= 1
            previous = value
        yield kind, start, end, frozen


def identifiers(text):
    """All identifier spellings in code (not comments or strings)."""
    return {text[s:e] for kind, s, e, _ in tokens(text) if kind == 'ident'}

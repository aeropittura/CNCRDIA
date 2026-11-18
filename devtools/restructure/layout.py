"""Path mapping from the inherited MP-SPDZ layout to the CNCRDIA layout.

``plan(files)`` takes the list of tracked paths and returns an ordered
``{old_path: new_path}`` dictionary for every file that moves.  The rules
are deliberately data-driven so that the mapping can be reviewed in one
place and re-applied deterministically.
"""

import os
import re

# words that stay glued together when converting CamelCase to snake_case
GLUED = {
    'CowGear': 'cowgear', 'ChaiGear': 'chaigear', 'LowGear': 'lowgear',
    'HighGear': 'highgear', 'SpdzWise': 'spdzwise', 'SPDZ2k': 'spdz2k',
    'Spdz2k': 'spdz2k', 'MalRep': 'malrep', 'PostSacri': 'postsacri',
    'octetStream': 'octet_stream', 'ezOptionParser': 'ez_option_parser',
    'gf2n': 'gf2n', 'gfp': 'gfp', 'gfpvar': 'gfpvar', 'Z2k': 'z2k',
    'FHE': 'fhe', 'BMR': 'bmr', 'OT': 'ot', 'MAC': 'mac', 'P256': 'p256',
    'NPC': 'npc', 'DPC': 'dpc',
}

# top-level C++ directories moving into engine/
ENGINE_DIRS = {
    'Math': 'engine/numeric',
    'Tools': 'engine/support',
    'Networking': 'engine/net',
    'Processor': 'engine/vm',
    'GC': 'engine/boolean',
    'Yao': 'engine/garbled/yao',
    'BMR/network': 'engine/garbled/bmr/transport',
    'BMR': 'engine/garbled/bmr',
    'FHE': 'engine/he/core',
    'FHEOffline': 'engine/he/offline',
    'OT': 'engine/ot',
    'ECDSA': 'engine/ecdsa',
}

# protocol families, first match wins
PROTOCOL_FAMILIES = [
    ('replicated', r'^(SemiRep3|Rep3|Replicated(?!Prep)|MaliciousRep|MalRepRing'
                   r'|MaliciousRep3|PostSacri|RepRingOnly)'),
    ('astra_trio', r'^(Astra|Trio)'),
    ('atlas', r'^Atlas'),
    ('brain', r'^Brain'),
    ('dealer', r'^Dealer'),
    ('spdz2k', r'^(SPDZ2k|Spdz2k)'),
    ('spdzwise', r'^SpdzWise'),
    ('rep4', r'^Rep4'),
    ('shamir', r'^(Shamir|MaliciousShamir)'),
    ('semi', r'^(Semi|Hemi|Temi|Soho)'),
    ('spdz', r'^(SPDZ|Mascot|Mama|LowGear|HighGear|CowGear|ChaiGear)'),
    ('fake', r'^(Fake|fake-stuff)'),
    ('common', r''),
]

CODE_EXTENSIONS = ('.h', '.hpp', '.cpp')


def snake(stem):
    """``MaliciousRep3Share`` -> ``malicious_rep3_share``."""
    for word, replacement in sorted(GLUED.items(), key=lambda x: -len(x[0])):
        stem = re.sub(r'(?<![A-Za-z])' + re.escape(word) +
                      r'(?=[A-Z_\-\d]|$)',
                      '\0' + replacement + '\0', stem)
    pieces = []
    for chunk in stem.split('\0'):
        if chunk in GLUED.values():
            pieces.append(chunk)
            continue
        chunk = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', chunk)
        chunk = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', chunk)
        pieces.append(chunk.lower())
    joined = '_'.join(p for p in pieces if p)
    joined = re.sub(r'[\-\^\+]', '_', joined)
    joined = re.sub(r'_+', '_', joined).strip('_')
    return joined


def snake_file(name):
    stem, ext = os.path.splitext(name)
    if ext not in CODE_EXTENSIONS:
        return name
    # SPDZ2^64+48.cpp and friends instantiate SPDZ2k for given parameters
    stem = re.sub(r'^SPDZ2\^(\d+)\+(\d+)$', r'SPDZ2k_\1_\2', stem)
    return snake(stem) + ext


def has_main(path):
    try:
        with open(path, errors='replace') as handle:
            return re.search(r'\bint\s+main\s*\(', handle.read()) is not None
    except OSError:
        return False


def protocol_family(name):
    for family, pattern in PROTOCOL_FAMILIES:
        if re.match(pattern, name):
            return family
    raise AssertionError(name)


def party_group(name):
    """Group for an executable entry point (kept under its binary name)."""
    stem = os.path.splitext(name)[0]
    if stem.startswith('export-'):
        return 'export'
    if stem.endswith('-example') or stem == 'protocol-tutorial':
        return 'examples'
    if 'offline' in stem.lower() or stem in (
            'OText_main', 'TripleMachine', 'stream-fake-mascot-triples',
            'check-passive', 'Check-Offline', 'Check-Offline-Z2k'):
        return 'offline'
    if stem.endswith('-ecdsa-party') or stem == 'Fake-ECDSA':
        return 'ecdsa'
    if stem.endswith('-bmr-party') or stem.startswith('bmr-'):
        return 'bmr'
    if re.search(r'(-bin-party|^tiny|^tinier|ccd-party|^yao-party)', stem):
        return 'boolean'
    if stem.endswith('-party') or stem in ('emulate', 'Player-Online'):
        return 'arith'
    return 'tools'


def plan(files):
    mapping = {}
    for path in files:
        parts = path.split('/')
        top, name = parts[0], parts[-1]
        directory = '/'.join(parts[:-1])
        new = None

        if top == 'Protocols':
            new = 'engine/protocols/%s/%s' % (protocol_family(name),
                                               snake_file(name))
        elif top in ('Machines', 'Utils') or (
                top == 'ECDSA' and has_main(path)):
            if name.endswith('.cpp') and has_main(path):
                stem = os.path.splitext(name)[0]
                new = 'parties/%s/%s' % (party_group(name), name)
                if stem in ('OText_main', 'TripleMachine'):
                    new = 'parties/offline/%s.cpp' % {
                        'OText_main': 'ot', 'TripleMachine': 'ot-offline'
                    }[stem]
            elif top in ('Machines', 'Utils'):
                if name.startswith('export-'):
                    new = 'parties/export/backends/' + snake_file(name)
                else:
                    sub = 'bmr/' if directory == 'Machines/BMR' else ''
                    new = 'parties/shared/' + sub + snake_file(name)
        elif top == 'ExternalIO':
            new = 'clients/' + (name if name.endswith(('.py', '.md', '.cpp'))
                                else snake_file(name))
        elif top == 'deps':
            continue  # submodules are moved separately
        elif top == 'Programs' and len(parts) > 2 and parts[1] == 'Source':
            new = 'examples/' + '/'.join(parts[2:])
        else:
            for old_dir in sorted(ENGINE_DIRS, key=len, reverse=True):
                if directory == old_dir:
                    new = '%s/%s' % (ENGINE_DIRS[old_dir], snake_file(name))
                    break
        if new and new != path:
            mapping[path] = new
    check(mapping, files)
    return mapping


def check(mapping, files):
    """No two final paths may coincide, even case-insensitively."""
    final = {}
    for path in files:
        target = mapping.get(path, path)
        key = target.lower()
        if key in final:
            raise SystemExit('collision: %s and %s -> %s' %
                             (final[key], path, target))
        final[key] = path
    # directories must not differ only in case either
    dirs = {}
    for target in (mapping.get(p, p) for p in files):
        for i in range(1, target.count('/') + 1):
            d = target.rsplit('/', i)[0]
            if dirs.setdefault(d.lower(), d) != d:
                raise SystemExit('directory case clash: %s vs %s' %
                                 (dirs[d.lower()], d))

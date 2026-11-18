#!/usr/bin/env python3
"""Compile every example program under several flag sets and fingerprint
the compiler output.

Each compilation runs in its own scratch directory so jobs can run in
parallel without clobbering ``Programs/Bytecode``.  The result is a JSON
document mapping ``"<program> <flags>"`` to either the SHA-256 of every
produced file or the last line of the error, so a refactor that must not
change behaviour can be checked with a plain diff.

Usage:
    compile_all.py [--jobs N] [--only PATTERN] OUTPUT.json
    compile_all.py --compare BASELINE.json CURRENT.json
"""

import argparse
import concurrent.futures
import fnmatch
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
FLAG_SETS = ['', '-R 64', '-B 32', '-R 64 -Y', '-F 64 -X']
TIMEOUT = 600


def find_sources():
    for candidate in ('examples', os.path.join('Programs', 'Source')):
        path = os.path.join(REPO, candidate)
        if os.path.isdir(path) and not os.path.islink(path):
            return path
    sys.exit('no example program directory found')


def fingerprint_tree(root):
    digests = {}
    for sub in ('Bytecode', 'Schedules', 'Functions'):
        base = os.path.join(root, 'Programs', sub)
        if not os.path.isdir(base):
            continue
        for name in sorted(os.listdir(base)):
            with open(os.path.join(base, name), 'rb') as handle:
                content = handle.read()
            if sub == 'Schedules':
                # the schedule records the command line, including the
                # location of compile.py; make it independent of the checkout
                content = content.replace(
                    os.path.join(REPO, 'compile.py').encode(), b'compile.py')
            digests[sub + '/' + name] = hashlib.sha256(content).hexdigest()
    return digests


def compile_one(job):
    program, flags, source_dir, python = job
    scratch = tempfile.mkdtemp(prefix='cmp-')
    try:
        # compile.py finds the program next to itself, so the scratch
        # directory only receives the output
        env = dict(os.environ, PYTHONHASHSEED='0')
        command = [python, os.path.join(REPO, 'compile.py')] + \
            flags.split() + [program]
        try:
            done = subprocess.run(command, cwd=scratch, env=env,
                                  capture_output=True, text=True,
                                  timeout=TIMEOUT)
        except subprocess.TimeoutExpired:
            return program, flags, {'status': 'timeout'}
        if done.returncode != 0:
            lines = [l for l in done.stderr.strip().splitlines() if l]
            return program, flags, {'status': 'error',
                                    'message': lines[-1] if lines else ''}
        return program, flags, {'status': 'ok',
                                'files': fingerprint_tree(scratch)}
    finally:
        shutil.rmtree(scratch, ignore_errors=True)


def run(args):
    source_dir = find_sources()
    programs = sorted(name[:-4] for name in os.listdir(source_dir)
                      if name.endswith('.mpc'))
    if args.only:
        programs = [p for p in programs if fnmatch.fnmatch(p, args.only)]
    jobs = [(p, f, source_dir, sys.executable)
            for p in programs for f in FLAG_SETS]
    results = {}
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        for program, flags, outcome in pool.map(compile_one, jobs):
            key = '%s [%s]' % (program, flags)
            results[key] = outcome
            print('%-50s %s' % (key, outcome['status']), flush=True)
    with open(args.output, 'w') as handle:
        json.dump(results, handle, indent=1, sort_keys=True)


# outcomes that depend on the machine rather than on the code: the
# compiler's own wall-clock limit (five minutes) and network downloads
UNSTABLE = ('Compilation timeout reached', 'URLError', 'HTTPError')


def legacy_modules():
    """Old compiler module names and their new ones (mpcl/legacy.py)."""
    path = os.path.join(REPO, 'mpcl', 'legacy.py')
    if not os.path.exists(path):
        return {}
    namespace = {}
    with open(path) as handle:
        exec(handle.read(), namespace)
    return namespace['MODULES']


def normalize(results):
    """Error messages are compared without the module path of the
    exception class, which changes when modules move.  Unstable outcomes
    are reported as timeouts."""
    renames = sorted(legacy_modules().items(), key=lambda x: -len(x[0]))
    for outcome in results.values():
        if 'message' in outcome:
            for old, new in renames:
                outcome['message'] = re.sub(
                    r'(?<![\w.])' + re.escape(old) + r'(?=\.|\b)', new,
                    outcome['message'])
            outcome['message'] = re.sub(r'^([A-Za-z_][\w]*\.)+(?=\w+:)', '',
                                        outcome['message'])
            if any(marker in outcome['message'] for marker in UNSTABLE):
                outcome['status'] = 'timeout'
    return results


def compare(baseline_path, current_path):
    with open(baseline_path) as handle:
        baseline = normalize(json.load(handle))
    with open(current_path) as handle:
        current = normalize(json.load(handle))
    problems = 0
    for key in sorted(set(baseline) | set(current)):
        old, new = baseline.get(key), current.get(key)
        if 'timeout' in ((old or {}).get('status'),
                         (new or {}).get('status')):
            print('SKIPPED (timeout)', key)
            continue
        if old != new:
            problems += 1
            print('MISMATCH', key)
            print('  before:', json.dumps(old)[:300])
            print('  after: ', json.dumps(new)[:300])
    print('%d mismatches out of %d entries' % (problems, len(baseline)))
    return 1 if problems else 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('output', nargs='?')
    parser.add_argument('--jobs', type=int, default=os.cpu_count())
    parser.add_argument('--only')
    parser.add_argument('--compare', nargs=2, metavar=('BASELINE', 'CURRENT'))
    args = parser.parse_args()
    if args.compare:
        sys.exit(compare(*args.compare))
    if not args.output:
        parser.error('output file required')
    run(args)


if __name__ == '__main__':
    main()

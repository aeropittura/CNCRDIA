#!/usr/bin/env python3

import sys, os
sys.path.append(os.curdir)

from mpcl.core.program import Program, defaults

opts = defaults()
opts.ring = 64

prog = Program(['direct_compilation'], opts)

from mpcl.lib.library import print_ln
from mpcl.core.types import sint

print_ln('%s', (sint(0) < sint(1)).reveal())

prog.finalize()

import subprocess
subprocess.run(['./emulate.x', 'direct_compilation'])

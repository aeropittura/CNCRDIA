import subprocess, sys, os
sys.path.insert(0, os.path.dirname(__file__))
import layout
files = subprocess.run(['git', 'ls-files'], capture_output=True, text=True, check=True).stdout.split()
for old, new in layout.plan(files).items():
    print(old, '->', new)

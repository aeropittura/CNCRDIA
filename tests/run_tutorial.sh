#!/usr/bin/env bash
# Run the tutorial program on every protocol through the shipped test
# script and require the number of successful runs recorded in the
# baseline, with no failure.

cd "$(dirname "$0")/.."
expected=$(cat tests/baseline/tutorial-runs.txt)

for launcher_dir in launch Scripts; do
    test -x $launcher_dir/test_tutorial.sh && break
done

log=$(mktemp)
$launcher_dir/test_tutorial.sh -C > "$log" 2>&1
passed=$(grep -c "weighted average: 2.333" "$log")
failed=$(grep -c "^== Party 0" "$log")
echo "tutorial runs: $passed passed, $failed failed (expected $expected)"
rm -f "$log"
test "$passed" = "$expected" -a "$failed" = 0

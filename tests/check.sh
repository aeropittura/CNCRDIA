#!/usr/bin/env bash
# Regression gate: every restructuring step has to pass this unchanged.
#
#   tests/check.sh [build|compile|run|all]   (default: all)
#
# build    builds every executable listed in tests/baseline/binaries.txt
# compile  recompiles all example programs and compares the output hashes
#          with tests/baseline/compiler-output.json
# run      runs the tutorial program on every protocol (tests/run_tutorial.sh)

set -u
cd "$(dirname "$0")/.."
root=$PWD
baseline=tests/baseline
scratch=${TMPDIR:-/tmp}/cncrdia-check.$$
mkdir -p "$scratch"
python=${PYTHON:-python3}
jobs=${JOBS:-$(getconf _NPROCESSORS_ONLN)}
status=0

step_build() {
    echo "== building $(wc -l < $baseline/binaries.txt) executables"
    if ! make -j"$jobs" -k $(cat $baseline/binaries.txt) > "$scratch/build.log" 2>&1; then
        grep -E "error|Error" "$scratch/build.log" | head -20
        echo "build FAILED (log: $scratch/build.log)"
        status=1
    fi
    for binary in $(cat $baseline/binaries.txt); do
        test -x "$binary" || { echo "missing $binary"; status=1; }
    done
}

step_compile() {
    echo "== compiling example programs"
    "$python" tests/baseline/compile_all.py --jobs "$jobs" \
        "$scratch/compiler-output.json" > "$scratch/compile.log" 2>&1
    "$python" tests/baseline/compile_all.py --compare \
        $baseline/compiler-output.json "$scratch/compiler-output.json" \
        > "$scratch/compare.log"
    test $? = 0 || status=1
    tail -20 "$scratch/compare.log"
}

step_run() {
    echo "== running the tutorial on every protocol"
    if ! tests/run_tutorial.sh > "$scratch/run.log" 2>&1; then
        tail -20 "$scratch/run.log"
        echo "protocol run FAILED (log: $scratch/run.log)"
        status=1
    fi
}

case ${1:-all} in
    build) step_build ;;
    compile) step_compile ;;
    run) step_run ;;
    all) step_build; step_compile; step_run ;;
    *) echo "usage: $0 [build|compile|run|all]"; exit 2 ;;
esac

if [ $status = 0 ]; then
    echo "== all checks passed"
    rm -rf "$scratch"
else
    echo "== CHECKS FAILED, logs in $scratch"
fi
exit $status

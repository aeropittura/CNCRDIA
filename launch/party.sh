#!/usr/bin/env bash
# Runs every party of one protocol on this machine.
#
# The launchers <protocol>.sh are symbolic links to this script. The name
# it is called by selects a row of protocols.tsv, which gives the
# executable, the number of parties and any extra handling. Arguments are
# passed on to the executable, see run-common.sh.

HERE=$(cd `dirname $0`; pwd)
SPDZROOT=$HERE/..

script=$(basename "$0" .sh)
read -r _ binary parties _ options <<< \
     "$(awk -v script="$script" '$1 == script' "$HERE/protocols.tsv")"

if test -z "$binary"; then
    echo "$0: $script is not in $HERE/protocols.tsv" >&2
    exit 1
fi

case $parties in
    fixed:*) export PLAYERS=${parties#fixed:} ;;
    default:*) export PLAYERS=${PLAYERS:-${parties#default:}} ;;
esac

extra=
for option in ${options//,/ }; do
    case $option in
        threshold) test "$THRESHOLD" && extra="$extra -T $THRESHOLD" ;;
        args=*) extra="$extra ${option#args=}" ;;
        clean=*) rm ${option#clean=} ;;
    esac
done

. $HERE/run-common.sh

run_player $binary $* $extra || exit 1

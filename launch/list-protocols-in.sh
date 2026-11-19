#!/usr/bin/env bash
# Prints the protocols that protocols.tsv puts in the given lists, in order.

dir="$(dirname $0)"
echo $(awk -v lists=" $* " 'index(lists, " " $4 " ") && !/^#/ { print $1 }' \
           "$dir/protocols.tsv")

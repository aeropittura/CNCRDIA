#!/bin/bash

./compile.py -l test_flow_optimization || exit 1
launch/rep-field.sh test_flow_optimization || exit 1

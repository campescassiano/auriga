#!/bin/bash

set -eu
set -o pipefail

output="test_is"

gcc -Wall main.c -o $output

if [[ "$#" -eq 0 ]]; then
    echo "Build completed, output=$output"
else
    echo "Error during build"
fi

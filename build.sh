#!/usr/bin/env bash

set -eu
set -o pipefail

output="test_is"

gcc -Wall main.c errors.c crc32.c utils.c file_ops.c message.c -o $output

echo "Build completed, output=$output"

#!/bin/bash

set -eu
set -o pipefail

gcc -Wall main.c -o test_is

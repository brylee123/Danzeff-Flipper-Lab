#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -o /tmp/danzeff-core-test tests/core_test.c danzeff_core.c
/tmp/danzeff-core-test

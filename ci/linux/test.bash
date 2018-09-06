#!/bin/bash

set -e
source "$(dirname "${BASH_SOURCE[0]}")/common.bashrc"
set -x

echo "Testing..."


docker_exec bash -c '
    set -xe
    export CTEST_OUTPUT_ON_FAILURE=1

    cd /foreigncc-gcc
    make test -j "$NCPU"

    cd /foreigncc-clang
    make test -j "$NCPU"
    '

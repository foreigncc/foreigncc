#!/usr/bin/env bash

set -e


if hash python; then
    PYTHON=python
elif hash python3; then
    PYTHON=python3
elif hash python2; then
    PYTHON=python2
else
    echo "ERROR: python/python3/python2 not found."
    exit 1
fi


exec "$PYTHON" "$(dirname "$(readlink -f "$0")")/run.py" "$@"

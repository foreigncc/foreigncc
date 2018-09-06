#!/bin/bash

set -e
source "$(dirname "${BASH_SOURCE[0]}")/common.bashrc"
set -x

echo "Before Building..."


# Show distribution
if hash lsb_release 2>/dev/null; then
    lsb_release -a
fi
if hash uname 2>/dev/null; then
    uname -a
fi

# Show CPU and memory information
if hash lscpu 2>/dev/null; then
    lscpu
elif [ -f "/proc/cpuinfo" ]; then
    cat "/proc/cpuinfo"
fi

free

# Show software information
docker --version
git --version

# Show environment variables
export

# Show git information
cd "$BUILD_ROOT"
git status
git log -5

# Enable TIPC
sudo modprobe tipc


# Prepare docker container
echo "Container name: $CONTAINER_NAME"
(sudo docker stop "$CONTAINER_NAME"; sudo docker rm "$CONTAINER_NAME"; sleep 1) || true
sudo docker run --privileged --tty --rm --detach --name "$CONTAINER_NAME" "foreigncc/$OS_NAME-for-ci:$OS_VER" bash
sudo docker cp "$BUILD_ROOT" "$CONTAINER_NAME":"$CONTAINER_BUILD_ROOT"

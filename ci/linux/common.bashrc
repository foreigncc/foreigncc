#!/bin/echo "Error: This file must be sourced but not invoked"

#------------------------------------------------------------------------------
# Expect input environment variables:
#   OS_NAME             | docker image name
#   OS_VER              | docker image tag
#                       |
#------------------------------------------------------------------------------
# Set new environment variables:
#   BUILD_ROOT          | repo root directory (cloned by CI)
#   CONTAINER_BUILD_ROOT| repo root directory inside docker (copied to docker)
#   CONTAINER_NAME      | name of container
#   NCPU                | number of CPU cores
#                       |
#------------------------------------------------------------------------------
# Provide new functions:
#   docker_exec         | `docker exec ...` with above environment variables
#                          except: BUILD_ROOT
#


# Check OS_NAME, OS_VER
if [ -z "$OS_NAME" ] || [ -z "$OS_VER" ]; then
    echo "Error: Missing OS_NAME($OS_NAME) or OS_VER($OS_VER)"
    exit 1
else
    echo "Info: Build on $OS_NAME $OS_VER"
fi


# Number of CPUs
export NCPU="$(cat /proc/cpuinfo | grep processor | wc -l)"


# Get BUILD_ROOT
if [ -n "$TRAVIS_BUILD_DIR" ]; then
    echo "Info: Build on Travis-CI, set BUILD_ROOT to TRAVIS_BUILD_DIR: $TRAVIS_BUILD_DIR"
    export BUILD_ROOT="$TRAVIS_BUILD_DIR"
elif [ -n "$APPVEYOR_BUILD_FOLDER" ]; then
    echo "Info: Build on AppVeyor, set BUILD_ROOT to APPVEYOR_BUILD_FOLDER: $APPVEYOR_BUILD_FOLDER"
    export BUILD_ROOT="$APPVEYOR_BUILD_FOLDER"
else
    echo "Warn: Can't determine BUILD_ROOT. Use pwd: $(pwd)"
    export BUILD_ROOT="$(pwd)"
fi


# Decide container name
export CONTAINER_NAME="foreigncc_${OS_NAME}_${OS_VER}"
export CONTAINER_BUILD_ROOT="/foreigncc"
docker_exec() {
    sudo docker exec \
        --privileged \
        --tty \
        --env OS_NAME="$OS_NAME" \
        --env OS_VER="$OS_VER" \
        --env CONTAINER_NAME="$CONTAINER_NAME" \
        --env CONTAINER_BUILD_ROOT="$CONTAINER_BUILD_ROOT" \
        --env NCPU="$NCPU" \
        `#--workdir "$CONTAINER_BUILD_ROOT" # --workdir is not supported in earlier docker version` \
        "$CONTAINER_NAME" \
        "$@"
}

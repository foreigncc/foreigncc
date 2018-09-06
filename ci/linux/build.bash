#!/bin/bash

set -e
source "$(dirname "${BASH_SOURCE[0]}")/common.bashrc"
set -x

echo "Building..."


build() {

    # Make with GCC
    docker_exec bash -c '
        set -xe

        # Enable newer version of gcc on CentOS 6.x
        if [ "$OS_NAME" = "centos" ]; then
            if [ -f "/opt/rh/devtoolset-2/enable" ]; then
                . /opt/rh/devtoolset-2/enable
            fi
        fi

        gcc --version
        g++ --version
        export CC="$(which gcc)"
        export CXX="$(which g++)"
        mkdir /foreigncc-gcc
        cd /foreigncc-gcc
        cmake -DCMAKE_BUILD_TYPE=Release /foreigncc
        make -j "$NCPU"
        '

    # Make with Clang
    docker_exec bash -c '
        set -xe

        # TODO: This is a workaround
        # TODO: Workaround in CMakeFile.txt, or give this a documentation
        # Use clang option --gcc-toolchain to search for new include path
        # See: https://stackoverflow.com/questions/27319561/clang-is-using-the-wrong-system-include-directory
        if [ -d "/opt/rh/devtoolset-2/root/usr" ]; then
            export GCC_TOOLCHAIN="/opt/rh/devtoolset-2/root/usr"
        fi
        echo "#include <atomic>" >/tmp/test.cpp
        echo "int main() { return 0; }" >>/tmp/test.cpp
        if ! clang++ -std=c++11 -o /dev/null /tmp/test.cpp; then
            if clang++ -std=c++11 -o /dev/null --gcc-toolchain="$GCC_TOOLCHAIN" /tmp/test.cpp; then
                # TODO: What if, in any case, there are spaces in $GCC_TOOLCHAIN?
                export CXXFLAGS="--gcc-toolchain=$GCC_TOOLCHAIN"
                export CFLAGS="--gcc-toolchain=$GCC_TOOLCHAIN"
            elif clang++ -std=c++11 -o /dev/null -gcc-toolchain="$GCC_TOOLCHAIN" /tmp/test.cpp; then
                export CXXFLAGS="-gcc-toolchain=$GCC_TOOLCHAIN"
                export CFLAGS="-gcc-toolchain=$GCC_TOOLCHAIN"
            else
                echo "Error: clang++ seems not working. Compiling failure is likely to occur."
            fi
        fi
        rm -f /tmp/test.cpp

        clang --version
        clang++ --version
        export CC="$(which clang)"
        export CXX="$(which clang++)"
        mkdir /foreigncc-clang
        cd /foreigncc-clang
        cmake -DCMAKE_BUILD_TYPE=Release /foreigncc
        make -j "$NCPU"
        '
}


#build_on_apt_based() {
#    docker_exec bash -c '
#        set -xe
#
#        export DEBIAN_FRONTEND=noninteractive
#        apt-get update -yqq
#        apt-get install -yqq cmake build-essential clang #libzmq3-dev
#        '
#    build
#}
#
#build_on_yum_based() {
#    docker_exec bash -c '
#        set -xe
#
#        # CentOS systemd issue (on CentOS 7.0 7.1)
#        # See: https://stackoverflow.com/questions/36630718/docker-as-a-builder-cant-install-systemd-header-files
#        if [ "$OS_NAME" = "centos" ]; then
#            yum swap -y fakesystemd systemd || true
#        fi
#
#        # Enable epel source for CentOS
#        if [ "$OS_NAME" = "centos" ]; then
#            yum install -y epel-release
#        fi
#
#        yum makecache
#        yum groupinstall -y "Development tools"
#        yum install -y wget which cmake clang #zeromq3-devel
#
#        if [ "$OS_NAME" = "centos" ]; then
#            # Install newer gcc/g++ if c++11 not available
#            echo "int main() { return 0; }" >/tmp/test.cpp
#            if g++ -o /dev/null /tmp/test.cpp && ! g++ -o /dev/null -std=c++11 /tmp/test.cpp; then
#                wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo
#                yum install -y devtoolset-2-gcc devtoolset-2-binutils devtoolset-2-gcc-c++
#            fi
#            rm -f /tmp/test.cpp
#        fi
#        '
#    build
#}



#case "$OS_NAME" in
#"ubuntu"|"debian"):
#    build_on_apt_based
#    ;;
#"centos"|"fedora"):
#    build_on_yum_based
#    ;;
#*):
#    echo "Error: Unknown OS_NAME: $OS_NAME"
#    exit 1
#esac

case "$OS_NAME" in
"ubuntu"|"debian"|"centos"|"fedora"):
    build
    ;;
*):
    echo "Error: Unknown OS_NAME: $OS_NAME"
    exit 1
esac

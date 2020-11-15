#!/bin/sh

ENABLE_CLANG=0

build() {
    echo build type: Debug

    if [ $ENABLE_CLANG -ne 0 ] && command -v which clang >/dev/null 2>&1; then
        export CC="clang"
        export CXX="clang++"
    fi

    if cmake -B build \
        -DCMAKE_BUILD_TYPE=Debug; then
        make -C build -j 3
    fi
}
clean() {
    rm -rf build/*
}

while getopts "cr" arg; do
    case $arg in
    c) echo clean ;;
    r)
        clean
        build
        ;;
    *) echo Usage: build.sh [clean] ;;
    esac
done
if [ $OPTIND -eq 1 ]; then
    build
fi

#!/bin/bash

set -e

mkdir -p build
cd build
../configure --prefix "$glibc_install"
make -j4
make install


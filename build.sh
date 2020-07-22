#export glibc_install="$(pwd)/build/install"

mkdir build
cd build
../configure --prefix "$glibc_install"
make
make install
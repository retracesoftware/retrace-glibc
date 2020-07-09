mkdir build
cd build
../configure --prefix "$glibc_install"
make
make install
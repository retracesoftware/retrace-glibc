#export glibc_install="$(pwd)/build/install"

mkdir build
cd build
../configure --prefix "$glibc_install" --disable-werror
#make -k 
make -j8
make install

gcc test.c -o test -Wl,--rpath="${glibc_install}/lib" -Wl,--dynamic-linker="${glibc_install}/lib/ld-linux-x86-64.so.2" -I "${glibc_install}/include" -L "${glibc_install}/lib"

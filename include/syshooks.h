#ifndef _LIBC_SYSHOOKS_H_
#define _LIBC_SYSHOOKS_H_ 1

typedef int (* syscall_socket)(int, int, int);
typedef int (* syscall_socket_hook) (syscall_socket, int, int, int);

void syscall_set_socket_hook(syscall_socket_hook);

#endif

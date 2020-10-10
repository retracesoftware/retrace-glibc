#include <stdio.h>
#include <syshooks.h>

extern int __libc_socket(int, int, int);

__thread syscall_socket_hook hook = NULL;

void syscall_set_socket_hook(syscall_socket_hook f) {
   hook = f;
}

int __socket(int domain, int type, int protocol)
{
	if (hook == NULL) {
		return __libc_socket(domain, type, protocol);
	} else {
		return hook(__libc_socket, domain, type, protocol);
	}
}

	int
socket (int domain, int type, int protocol)
{
	return __socket(domain, type, protocol);
}


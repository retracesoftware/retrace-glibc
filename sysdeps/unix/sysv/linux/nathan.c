#include <stdio.h>

extern int __libc_socket(int, int, int);

int __socket(int domain, int type, int protocol)
{
        printf("in nathan's hacked version");

        return __libc_socket(domain, type, protocol);
}

	int
socket (int domain, int type, int protocol)
{
	return __socket(domain, type, protocol);
}


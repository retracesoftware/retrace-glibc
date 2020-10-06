#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


int (* syshook_open)(const char *,int, int);
int (* syshook_close)(int);

extern void *(* __malloc_hook)(size_t __size, const void *);

int patched(const char *file, int oflag, int mode) {

	printf("In patched open()\n");

    return syshook_open(file, oflag, mode);
}

int patched_close(int fd) {

    printf("In patched close()\n");

    return syshook_close(fd);
}

int main() {

	void * (* test)(size_t, const void *) = __malloc_hook;

    syshook_open = syscall_open;

	syscall_open = patched;

	int f = open("/dev/null", O_RDONLY);

	printf ("%i\n", f);

	FILE * fp = fopen("/dev/null", "rb");

    syshook_close = syscall_close;
    syscall_close = patched_close;

    close(f);

	return 0;
}


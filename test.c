#include <fcntl.h>
#include <stdio.h>

int (* current)(const char *,int, ...);

extern void *(* __malloc_hook)(size_t __size, const void *);

int patched(const char *file, int oflag, ...) {

	printf("In patched open()\n");

	return current(file, oflag);
}

int main() {

	void * (* test)(size_t, const void *) = __malloc_hook;

	current = syscall_open;

	syscall_open = patched;

	int f = open("/dev/null", O_RDONLY);

	printf ("%i\n", f);

	FILE * fp = fopen("/dev/null", "rb");

	return 0;
}


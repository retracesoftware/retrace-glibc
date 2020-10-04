#include <fcntl.h>
#include <stdio.h>

int (* current)(const char *,int, int);

extern void *(* __malloc_hook)(size_t __size, const void *);

/*
 * 
 *
 *
 */
int patched(const char *file, int oflag, int mode) {

	// check the mode
	// make the call
	// write the result to a buffer
	// api call adds it to queue, memcpy it
	//
	// grab a lock on a shared buffer, write 
	printf("In patched open()\n");

	return current(file, oflag, mode);
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


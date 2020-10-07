#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>


int (* syshook_open)(const char *,int, int);
int (* syshook_close)(int);
ssize_t (* syshook_read)(int,void *,size_t);
ssize_t (* syshook_write)(int,const void *,size_t);
int (* syshook_accept)(int, __SOCKADDR_ARG, socklen_t* );

extern void *(* __malloc_hook)(size_t __size, const void *);

int patched_open(const char *file, int oflag, int mode) {

	printf("In patched open()\n");

    return syshook_open(file, oflag, mode);
}

int patched_close(int fd) {

    printf("In patched close()\n");

    return syshook_close(fd);
}
ssize_t patched_read(int fd, void *buf, size_t nbytes) {

    printf("In patched read()\n");

    return syshook_read(fd,buf,nbytes);
}
ssize_t patched_write(int fd, const void *buf, size_t nbytes) {

    //printf("In patched write()\n");  #TODO find the reason of Segmentation fault(SIGSEGV)

    return syshook_write(fd,buf,nbytes);
}

int patched_accept(int fd, __SOCKADDR_ARG addr, socklen_t *len) {

    printf("In patched accept()\n");

    return syshook_accept(fd, addr, len);
}

void init_hooks(){
    syshook_open = syscall_open;
    syscall_open = patched_open;

    syshook_read = syscall_read;
    syscall_read = patched_read;

    syshook_write = syscall_write;
    syscall_write = patched_write;

    syshook_close = syscall_close;
    syscall_close = patched_close;

    syshook_accept = syscall_accept;
    syscall_accept = patched_accept;
}
#define TEST_FILE "file_for_test"
int main() {

	void * (* test)(size_t, const void *) = __malloc_hook;

    init_hooks();


    int f = open(TEST_FILE, O_RDONLY);

    printf ("%i\n", f);

    FILE * fp = fopen(TEST_FILE, "rb");



    close(f);

	return 0;
}


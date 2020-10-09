#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>


int (* syshook_open)(const char *,int, int);
int (* syshook_close)(int);
ssize_t (* syshook_read)(int,void *,size_t);
ssize_t (* syshook_write)(int,const void *,size_t);
int (* syshook_accept)(int, __SOCKADDR_ARG, socklen_t* );
int (* syshook_socket)(int, int, int );
int (* syshook_listen)(int, int);
int (* syshook_bind)(int, __CONST_SOCKADDR_ARG, socklen_t );
ssize_t (* syshook_sent)(int, const void *, size_t, int);
ssize_t (* syshook_recv)(int, void *, size_t, int);
int (* syshook_connect)(int,__CONST_SOCKADDR_ARG,socklen_t);
int (* syshook_pause)(void);
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

int patched_socket(int fd, int type, int domain) {

    printf("In patched socket()\n");

    return syshook_socket(fd, type, domain);
}
int patched_bind(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) {

    printf("In patched bind()\n");

    return syshook_bind(fd, addr, len);
}
int patched_listen(int fd, int backlog) {

    printf("In patched listen()\n");

    return syshook_listen(fd, backlog);
}
ssize_t patched_sent(int fd, const void *buf, size_t len, int flags) {

    printf("In patched sent()\n");

    return syshook_sent(fd, buf, len, flags);
}
ssize_t patched_recv(int fd, void *buf, size_t len, int flags) {

    printf("In patched recv()\n");

    return syshook_recv(fd, buf, len, flags);
}
int patched_connect(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) {

    printf("In patched connect()\n");

    return syshook_connect(fd, addr, len);
}

int patched_pause(void) {

    printf("In patched pause()\n");

    return syshook_pause();
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

    syshook_socket = syscall_socket;
    syscall_socket = patched_socket;

    syshook_listen = syscall_listen;
    syscall_listen = patched_listen;

    syshook_bind = syscall_bind;
    syscall_bind= patched_bind;

    syshook_sent = syscall_sent;
    syscall_sent= patched_sent;

    syshook_recv = syscall_recv;
    syscall_recv= patched_recv;

    syshook_connect = syscall_connect;
    syscall_connect = patched_connect;

    syshook_pause = syscall_pause;
    syscall_pause = patched_pause;
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


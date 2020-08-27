#ifndef RETRACE_LIB_H
#define RETRACE_LIB_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sys/uio.h>
#include <time.h>
#include <stdarg.h>
#include <sys/socket.h>
#define LOG_FILE "log.dat"
#define RETRACE_DIR ".retrace/"

typedef enum
{
    Retrace_Disabled_Mode,
    Retrace_Record_Mode,
    Retrace_Replay_Mode
    
} Retrace_Mode;

typedef struct
{
    char* path;
    FILE* fileHandler;
    Retrace_Mode mode;

} Retrace_Log;

typedef struct IntPair
{
    int key;
    int value;
    struct IntPair* next;

} IntPair;

pthread_mutex_t rmutex;
extern  __thread Retrace_Log rlog;
extern __thread IntPair* sock_pair;

extern int (*ptrRetraceSocket) (int fd, int type, int domain);
extern int (*ptrRetraceBind) (int fd, __CONST_SOCKADDR_ARG addr, socklen_t len);
extern int (*ptrRetraceListen) (int fd, int backlog);
static int (*ptrRetraceAccept) (int fd, __SOCKADDR_ARG addr, socklen_t *len) = NULL;
static int (*ptrRetraceConnect) (int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) = NULL;

void Insert_IntPair(IntPair** root, int key, int value);
IntPair* Find_IntPair(IntPair* tail, int key);
void Deallocate_IntPairs(IntPair** root);

extern __thread Retrace_Log rlog;
extern __thread IntPair* fd_pair;

extern void RLog_Init(Retrace_Log* log, char* log_path, Retrace_Mode mode);
extern void RLog_Displose(Retrace_Log* log);

extern void RLog_Push(Retrace_Log* rlog, const void* buffer, size_t buffer_size);
extern void* RLog_Fetch(Retrace_Log* rlog, void* buffer, size_t buffer_size);
extern size_t RLog_Fetch_Length(Retrace_Log* rlog);

extern size_t Record_Args(Retrace_Log* rlog, int arg_num, ...);

extern int Retrace_Read(int fd, void* buffer, size_t len);
extern int Retrace_Write(int fd, const void* buffer, size_t len);
extern int Retrace_Open64(const char *file, int oflag, int mode);
extern int Retrace_Close(int fd);



#endif

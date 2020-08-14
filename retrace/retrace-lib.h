
#if !defined(RETRACE_LIB_H)
#define RETRACE_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

void Insert_IntPair(IntPair** root, int key, int value);
IntPair* Find_IntPair(IntPair* tail, int key);
void Deallocate_IntPairs(IntPair** root);

extern __thread Retrace_Log rlog;
extern __thread IntPair* fd_pair;
extern __thread IntPair* sock_pair;

void RLog_Init(Retrace_Log* log, char* log_path, Retrace_Mode mode);
void RLog_Displose(Retrace_Log* log);

void RLog_Push(Retrace_Log* rlog, const void* buffer, size_t buffer_size);
void* RLog_Fetch(Retrace_Log* rlog, void* buffer, size_t buffer_size);
size_t RLog_Fetch_Length(Retrace_Log* rlog);

void Check_Dir(const char* dir_name);

size_t Record_Args(Retrace_Log* rlog, int arg_num, ...);

extern int Retrace_Read(int fd, void* buffer, size_t len);
extern int Retrace_Write(int fd, const void* buffer, size_t len);
extern int Retrace_Open64(const char *file, int oflag, int mode);
extern int Retrace_Close(int fd);

extern int Retrace_Socket(int fd, int type, int domain);
extern int Retrace_Bind(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len);
extern int Retrace_Listen(int fd, int backlog);
extern int Retrace_Accept(int fd, __SOCKADDR_ARG addr, socklen_t *len);
extern int Retrace_Connect(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len);
extern int Retrace_Send(int fd, const void *buf, size_t len, int flags);
extern int Retrace_Recv(int fd, void *buf, size_t len, int flags);

extern time_t Retrace_Time(time_t *timer);
extern int Retrace_Nanosleep(const struct timespec *requested_time, struct timespec *remaining);
extern int Retrace_Pause(void);

#endif
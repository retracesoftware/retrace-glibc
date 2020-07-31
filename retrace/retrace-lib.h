
#if !defined(RETRACE_LIB_H)
#define RETRACE_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

extern void Record_Read(int fd, void* buffer, size_t len);
extern int Replay_Read(int fd, void* buffer, size_t len);

extern void Record_Open64(const char *file, int oflag, int mode);
extern int Replay_Open64(const char *file, int oflag, int mode);

extern void Record_Close(int fd);
extern int Replay_Close(int fd);

#endif

#if !defined(RETRACE_LIB_H)
#define RETRACE_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

extern __thread Retrace_Log rlog;

extern void RLog_Init(Retrace_Log* log, char* log_path, Retrace_Mode mode);
extern void RLog_Displose(Retrace_Log* log);

extern void Record_Read(int fd, char* buffer, size_t len);
extern void Replay_Read(int fd, char* buffer, size_t len);

#endif
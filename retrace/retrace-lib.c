
#include "retrace-lib.h"

//__thread Retrace_Log rlog;

void RLog_Init(Retrace_Log* log, char* path, Retrace_Mode mode)
{
    if(log == NULL) return;

    log->path = path;
      
    log->fileHandler = fopen(log->path, "a+b");

    assert(log->fileHandler != NULL);

    log->mode = mode; 

    printf("%s() success\n\n", __func__);
}

void RLog_Displose(Retrace_Log* log)
{
    if (log == NULL) return;
    if(log->fileHandler == NULL) return;

    log->mode = Retrace_Disabled_Mode;

    if(EOF == fclose(log->fileHandler))
    {
        printf("File close failed!\n");
        return;
    }

    log->fileHandler = NULL;
    log->path = NULL;

    printf("%s() success\n\n", __func__);
}

void Record_Read(int fd, char* buffer, size_t len)
{
    printf("Hello %s\n", __func__);

    fwrite(buffer, len, 1, rlog.fileHandler);

}

void Replay_Read(int fd, char* buffer, size_t len)
{
    printf("Hello %s\n", __func__);



}
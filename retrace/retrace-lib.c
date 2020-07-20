
#include "retrace-lib.h"
#include <assert.h>

void RLog_Init(Retrace_Log* log, char* path, Retrace_Mode mode)
{
    if(log == NULL) return;

    log->path = path;
    log->mode = mode;   
    log->fileHandler = fopen(log->path, "a+b");

    assert(log->fileHandler != NULL);

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

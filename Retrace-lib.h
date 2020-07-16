
#if !defined(VAULT_H)
#define VAULT_H



typedef enum
{
    Retrace_Disabled_Mode,
    Retrace_Write_Mode,
    Retrace_Read_Mode
    
} Retrace_Mode;

typedef struct
{
    char* _path;
    FILE* _fileHandler;
    Retrace_Mode mode;

} Retrace_Log;

extern __thread int retrace_mode;

void RLog_Init(Retrace_Log* log, char* vlt_path, Retrace_Mode vmode);



#endif

#include "retrace-lib.h"

void Insert_IntPair(IntPair** root, int key, int value)
{
    IntPair* new_pair = malloc(sizeof(IntPair));
    
    if (NULL == new_pair)
    {
        fprintf(stderr, "%s(), memory allocation error!\n", __func__);
        abort();
    }
    
    new_pair->next = NULL;
    new_pair->key = key;
    new_pair->value = value;

    if (NULL == *root)
    {
        *root = new_pair;
        return;
    }
    
    IntPair* curr = *root;

    while (curr->next != NULL)
        curr = curr->next;

    curr->next = new_pair;    
}

IntPair* Find_IntPair(IntPair* tail, int key)
{
    for (IntPair* curr = tail; curr != NULL; curr = curr->next)
        if (curr->key == key) return curr;
    
    return NULL;
}

void Deallocate_IntPairs(IntPair** root)
{
    IntPair* curr = *root;
    IntPair* aux = NULL;

    while (curr != NULL)
    {
        aux = curr;
        curr = curr->next;
        free(aux);
    }
    
    *root = NULL;
}


void RLog_Init(Retrace_Log* log, char* path, Retrace_Mode mode)
{
    if(log == NULL) return;

    log->path = path;

    log->mode = Retrace_Disabled_Mode;
      
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

void RLog_Push(Retrace_Log* rlog, const void* buffer, size_t buffer_size)
{
    if (rlog->mode != Retrace_Record_Mode) return;
      
    if (NULL == buffer) return;
    
    fwrite(&buffer_size, sizeof(buffer_size), 1, rlog->fileHandler);
    fwrite(buffer, buffer_size, 1, rlog->fileHandler);
}

void* RLog_Fetch(Retrace_Log* rlog, void* buffer, size_t buffer_size)
{
    if (rlog->mode != Retrace_Replay_Mode)
    {
        fprintf(stderr, "Vault is not in read mode!\n");
        abort();
    }

    if(NULL == buffer) return NULL;
    
    size_t read_len = 0;
    fread(&(read_len), sizeof(read_len), 1, rlog->fileHandler);

    if (read_len < 0)
    {
        fprintf(stderr, "%s(): Length is less zero (%ld)!\n", __func__, read_len);
        abort();
    }
    
    if (buffer_size != read_len)
    {
        fprintf(stderr, "Length of buffer (%ld) is not equal to read one: (%ld)!\n", buffer_size, read_len);

        #ifdef DEBUG
            fread(buffer, 1, read_len, rlog->fileHandler);

            printf("Read data: ");
            for(int i = 0; i < read_len; i++)
                printf("%c", ((char*)buffer)[i]);
            printf("\n");
        #endif

        abort();
    }

    fread(buffer, 1, buffer_size, rlog->fileHandler);

    return buffer;
}

size_t RLog_Fetch_Length(Retrace_Log* rlog)
{
    if (rlog->mode != Retrace_Replay_Mode)
    {
        fprintf(stderr, "Vault is not in read mode!\n");
        abort();
    }

    size_t read_len = 0;

    fread(&read_len, sizeof(read_len), 1, rlog->fileHandler);
    
    if (read_len < 0)
    {   
        fprintf(stderr, "Length is less zero!\n");
        abort();               
    }
    
    fseek(rlog->fileHandler, -sizeof(read_len), SEEK_CUR);

    return read_len;
}


void Record_Read(int fd, void* buffer, size_t len)
{
    fwrite(buffer, len, 1, rlog.fileHandler);
}

int Replay_Read(int fd, void* buffer, size_t len)
{
    fread(buffer, len, 1, rlog.fileHandler);

    return len;
}

void Record_Open64(const char *file, int oflag, int mode)
{
    
    
    
}

int Replay_Open64(const char *file, int oflag, int mode)
{
    return fileno(rlog.fileHandler);
}

void Record_Close(int fd)
{


}

 int Replay_Close(int fd)
 {

     return 0;
 }
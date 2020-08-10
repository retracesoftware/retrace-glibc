
#include "retrace-lib.h"

#include <sysdep-cancel.h>
#include <socketcall.h>
#include <kernel-features.h>
#include <sys/syscall.h>

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

    //pthread_mutex_init(&rmutex, NULL);

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

    Deallocate_IntPairs(&fd_pair);

    //pthread_mutex_destroy(&rmutex);

    printf("%s() success\n\n", __func__);
}

void RLog_Push(Retrace_Log* rlog, const void* buffer, size_t buffer_size)
{
    //pthread_mutex_lock(&rmutex);

    if (rlog->mode == Retrace_Replay_Mode) return;
      
    if (NULL == buffer) return;
      
    if(fwrite(&buffer_size, sizeof(buffer_size), 1, rlog->fileHandler) != 1)
    {
        fprintf(stderr, "%s(), fwrite error!\n", __func__);
        abort();
    }

    if(fwrite(buffer, buffer_size, 1, rlog->fileHandler) != 1)
    {
        fprintf(stderr, "%s(), fwrite error!\n", __func__);
        abort();
    }

    //pthread_mutex_unlock(&rmutex);
}

void* RLog_Fetch(Retrace_Log* rlog, void* buffer, size_t buffer_size)
{
    //pthread_mutex_lock(&rmutex);

    if ((rlog->mode != Retrace_Replay_Mode) && (rlog->mode != Retrace_Disabled_Mode))
    {
        fprintf(stderr, "RLog is not in read/disabled mode!\n");
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
    
    //pthread_mutex_unlock(&rmutex);

    return buffer;
}

size_t RLog_Fetch_Length(Retrace_Log* rlog)
{
    //pthread_mutex_lock(&rmutex);
    
    if ((rlog->mode != Retrace_Replay_Mode) && (rlog->mode != Retrace_Disabled_Mode))
    {
        fprintf(stderr, "RLog is not in read/disabled mode!\n");
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
    
    //pthread_mutex_unlock(&rmutex);

    return read_len;
}

size_t Record_Args(Retrace_Log* rlog, int arg_num, ...)
{
    if (rlog->mode == Retrace_Replay_Mode) return EXIT_SUCCESS;

    if (arg_num % 2 != 0)
    {
        fprintf(stderr, "Error! Usage: first arg - buffer size, second one - pointer to buffer\n");
        return EXIT_FAILURE;
    }
    
    // Sequence: first arg is buffer size, next - void* buffer  
    va_list arg_ptr;
    va_start(arg_ptr, arg_num);

    size_t buffer_size;
    void* buffer = NULL;

    while(arg_num)
    {
        buffer_size = va_arg(arg_ptr, size_t);
        arg_num--;
        buffer = va_arg(arg_ptr, void*);
        arg_num--;

        if(fwrite(&buffer_size, sizeof(size_t), 1, rlog->fileHandler) != 1)
        {
            fprintf(stderr, "%s(), fwrite error!\n", __func__);
            return EXIT_FAILURE;
        }

        if(fwrite(buffer, 1, buffer_size, rlog->fileHandler) != buffer_size)
        {
            fprintf(stderr, "%s(), fwrite error!\n", __func__);
            return EXIT_FAILURE;
        }
    }

    va_end(arg_ptr);

    return EXIT_SUCCESS;
}

void Check_Dir(const char* dir_name)
{
    struct stat stats;
    stat(dir_name, &stats);

    if (!S_ISDIR(stats.st_mode))
    {
        if(mkdir(dir_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
        {
            fprintf(stderr, "Cannot create %s dir\n", dir_name);
            abort();
        }
    }
}

int Retrace_Read(int fd, void* buffer, size_t len)
{
    int ret_val = -1;
    size_t syscall_num = __NR_read;
    time_t cur_time = 0;
    pthread_t thread_id = 0;

    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        ret_val = SYSCALL_CANCEL (read, fd, buffer, len);
        
        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        IntPair* pair = Find_IntPair(fd_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "%s(), IntPair doesn't found!\n", __func__);
            abort();
        }
        
        if(len != write(pair->value, buffer, len))
        {
            fprintf(stderr, "Write error!\n");
            abort();
        }
        
        rlog.mode = Retrace_Record_Mode;
    } 
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));

        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        ret_val = SYSCALL_CANCEL (read, fd, buffer, len);

        rlog.mode = Retrace_Replay_Mode;
    }
    else return SYSCALL_CANCEL (read, fd, buffer, len);
    
    return ret_val;
}

int Retrace_Write(int fd, const void* buffer, size_t len)
{
    int ret_val = -1;
    size_t syscall_num = __NR_write;
    time_t cur_time = 0;
    pthread_t thread_id = 0;

    if(fd < 3)
        return SYSCALL_CANCEL (write, fd, buffer, len);

    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        ret_val = SYSCALL_CANCEL (write, fd, buffer, len);

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        IntPair* pair = Find_IntPair(fd_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "%s(), IntPair doesn't found!\n", __func__);
            abort();
        }
        
        if(len != write(pair->value, buffer, len))
        {
            fprintf(stderr, "Write error!\n");
            abort();
        }
        
        rlog.mode = Retrace_Record_Mode;
    } 
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));

        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        ret_val = SYSCALL_CANCEL (write, fd, buffer, len);

        rlog.mode = Retrace_Replay_Mode;
    }
    else return SYSCALL_CANCEL (write, fd, buffer, len);

    return ret_val;
}

int Retrace_Open64(const char *file, int oflag, int mode)
{
    #ifdef __OFF_T_MATCHES_OFF64_T
    # define EXTRA_OPEN_FLAGS 0
    #else
    # define EXTRA_OPEN_FLAGS O_LARGEFILE
    #endif
    
    int ret_val = -1;
    size_t syscall_num = __NR_open;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        ret_val = SYSCALL_CANCEL (openat, AT_FDCWD, file, oflag | EXTRA_OPEN_FLAGS, mode);

        thread_id = pthread_self();
        cur_time = time(NULL);

        char recorded_file_path[FILENAME_MAX];
        sprintf(recorded_file_path, "%s%d", RETRACE_DIR, ret_val);
        
        Check_Dir(RETRACE_DIR);

        int record_fd = open(recorded_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);

        if (record_fd < 0)
        {
            fprintf(stderr, "Record file open error!\n");
            abort();
        }      

        Insert_IntPair(&fd_pair, ret_val, record_fd);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));
        RLog_Push(&rlog, file, strlen(file) + 1);
        RLog_Push(&rlog, recorded_file_path, strlen(recorded_file_path) + 1);

        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
    
        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }
        
        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        char* file_name = malloc(strlen(file) + 1);
        
        RLog_Fetch(&rlog, file_name, strlen(file) + 1);

        if (strcmp(file, file_name))
        {
            fprintf(stderr, "Fetched filename isn't equal to current one!\n");
            abort();
        }
        
        size_t filename_len = RLog_Fetch_Length(&rlog);

        char* recorded_file_path = malloc(filename_len);

        RLog_Fetch(&rlog, recorded_file_path, filename_len);

        ret_val = open(recorded_file_path, O_RDWR);

        free(file_name);
        free(recorded_file_path);

        rlog.mode = Retrace_Replay_Mode;
    }
    else return SYSCALL_CANCEL (openat, AT_FDCWD, file, oflag | EXTRA_OPEN_FLAGS, mode);

    return ret_val;
}

int Retrace_Close(int fd)
{
    pthread_t thread_id = 0;
    int ret_val = -1;
    size_t syscall_num = __NR_close;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        ret_val = SYSCALL_CANCEL (close, fd);
        
        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        IntPair* pair = Find_IntPair(fd_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "IntPair doesn't found!\n");
            abort();
        }

        if(-1 == close(pair->value))
        {
            fprintf(stderr, "Closing error!\n");
            abort();
        }
            
        rlog.mode = Retrace_Record_Mode;
    } 
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
        
        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }
        
        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        ret_val = SYSCALL_CANCEL (close, fd);

        rlog.mode = Retrace_Replay_Mode;
    }
    else return SYSCALL_CANCEL (close, fd);

    return ret_val;
}

int Retrace_Socket(int fd, int type, int domain)
{
    int ret_val = -1;
    size_t syscall_num = __NR_socket;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_SOCKET_SYSCALL
            ret_val = INLINE_SYSCALL(socket, 3, fd, type, domain);
        #else
            ret_val = SOCKETCALL(socket, fd, type, domain);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        char recorded_file_path[FILENAME_MAX];
        sprintf(recorded_file_path, "%s%d", RETRACE_DIR, ret_val);
        
        Check_Dir(RETRACE_DIR);

        int record_fd = open(recorded_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);

        if (record_fd < 0)
        {
            fprintf(stderr, "Record file open error!\n");
            abort();
        }      

        Insert_IntPair(&sock_pair, ret_val, record_fd);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));
        RLog_Push(&rlog, recorded_file_path, strlen(recorded_file_path) + 1);

        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
    
        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }
        
        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        size_t filename_len = RLog_Fetch_Length(&rlog);

        char* recorded_file_path = malloc(filename_len);

        if(NULL == recorded_file_path)
        {
            fprintf(stderr, "Allocation memory error!\n");
            abort();
        }

        RLog_Fetch(&rlog, recorded_file_path, filename_len);

        ret_val = open(recorded_file_path, O_RDWR);

        free(recorded_file_path);

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_SOCKET_SYSCALL
            return INLINE_SYSCALL (socket, 3, fd, type, domain);
        #else
            return SOCKETCALL (socket, fd, type, domain);
        #endif
    }

    return ret_val;
}

int Retrace_Bind(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
    int ret_val = -1;
    size_t syscall_num = __NR_bind;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_BIND_SYSCALL
            ret_val = INLINE_SYSCALL (bind, 3, fd, addr.__sockaddr__, len);
        #else
            ret_val = SOCKETCALL (bind, fd, addr.__sockaddr__, len, 0, 0, 0);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));
        
        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
    
        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));
        
        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_BIND_SYSCALL
            return INLINE_SYSCALL (bind, 3, fd, addr.__sockaddr__, len);
        #else
            return SOCKETCALL (bind, fd, addr.__sockaddr__, len, 0, 0, 0);
        #endif
    }
        
    return ret_val;
}

int Retrace_Listen(int fd, int backlog)
{
    int ret_val = -1;
    size_t syscall_num = __NR_listen;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_LISTEN_SYSCALL
            ret_val = INLINE_SYSCALL (listen, 2, fd, backlog);
        #else
            ret_val = SOCKETCALL (listen, fd, backlog);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));
        
        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
    
        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_LISTEN_SYSCALL
            return INLINE_SYSCALL (listen, 2, fd, backlog);
        #else
            return SOCKETCALL (listen, fd, backlog);
        #endif
    }

    return ret_val;
}

int Retrace_Accept(int fd, __SOCKADDR_ARG addr, socklen_t *len)
{
    int ret_val = -1;
    size_t syscall_num = __NR_accept;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_ACCEPT_SYSCALL
            ret_val = SYSCALL_CANCEL (accept, fd, addr.__sockaddr__, len);
        #elif defined __ASSUME_ACCEPT4_SYSCALL
            ret_val = SYSCALL_CANCEL (accept4, fd, addr.__sockaddr__, len, 0);
        #else
            ret_val = SOCKETCALL_CANCEL (accept, fd, addr.__sockaddr__, len);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));
        
        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
    
        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));
        
        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_ACCEPT_SYSCALL
            return SYSCALL_CANCEL (accept, fd, addr.__sockaddr__, len);
        #elif defined __ASSUME_ACCEPT4_SYSCALL
            return SYSCALL_CANCEL (accept4, fd, addr.__sockaddr__, len, 0);
        #else
            return SOCKETCALL_CANCEL (accept, fd, addr.__sockaddr__, len);
        #endif
    }

    return ret_val;
}

int Retrace_Connect(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
    int ret_val = -1;
    size_t syscall_num = __NR_connect;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_CONNECT_SYSCALL
            ret_val = SYSCALL_CANCEL (connect, fd, addr.__sockaddr__, len);
        #else
            ret_val = SOCKETCALL_CANCEL (connect, fd, addr.__sockaddr__, len);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
    
        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_CONNECT_SYSCALL
            return SYSCALL_CANCEL (connect, fd, addr.__sockaddr__, len);
        #else
            return SOCKETCALL_CANCEL (connect, fd, addr.__sockaddr__, len);
        #endif
    }

    return ret_val;
}

int Retrace_Send(int fd, const void *buf, size_t len, int flags)
{
    int ret_val = -1;
    size_t syscall_num = __NR_sendto;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_SEND_SYSCALL
            ret_val = SYSCALL_CANCEL (send, fd, buf, len, flags);
        #elif defined __ASSUME_SENDTO_SYSCALL
            ret_val = SYSCALL_CANCEL (sendto, fd, buf, len, flags, NULL, 0);
        #else
            ret_val = SOCKETCALL_CANCEL (send, fd, buf, len, flags);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        IntPair* pair = Find_IntPair(sock_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "%s(), IntPair doesn't found!\n", __func__);
            abort();
        }
        
        if(len != write(pair->value, buf, len))
        {
            fprintf(stderr, "Write error!\n");
            abort();
        }

        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));

        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        ret_val = SYSCALL_CANCEL (write, fd, buf, len);

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_SEND_SYSCALL
            return SYSCALL_CANCEL (send, fd, buf, len, flags);
        #elif defined __ASSUME_SENDTO_SYSCALL
            return SYSCALL_CANCEL (sendto, fd, buf, len, flags, NULL, 0);
        #else
            return SOCKETCALL_CANCEL (send, fd, buf, len, flags);
        #endif
    }

    return ret_val;
}


int Retrace_Recv(int fd, void *buf, size_t len, int flags)
{
    int ret_val = -1;
    size_t syscall_num = __NR_recvfrom;
    pthread_t thread_id;
    time_t cur_time;
    
    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_RECV_SYSCALL
            ret_val = SYSCALL_CANCEL (recv, fd, buf, len, flags);
        #elif defined __ASSUME_RECVFROM_SYSCALL
            ret_val = SYSCALL_CANCEL (recvfrom, fd, buf, len, flags, NULL, NULL);
        #else
            return SOCKETCALL_CANCEL (recv, fd, buf, len, flags);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        IntPair* pair = Find_IntPair(sock_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "%s(), IntPair doesn't found!\n", __func__);
            abort();
        }

        if(len != write(pair->value, buf, len))
        {
            fprintf(stderr, "Write error!\n");
            abort();
        }

        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));

        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        ret_val = SYSCALL_CANCEL (read, fd, buf, len);

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_RECV_SYSCALL
            return SYSCALL_CANCEL (recv, fd, buf, len, flags);
        #elif defined __ASSUME_RECVFROM_SYSCALL
            return SYSCALL_CANCEL (recvfrom, fd, buf, len, flags, NULL, NULL);
        #else
            return SOCKETCALL_CANCEL (recv, fd, buf, len, flags);
        #endif
    }

    return ret_val;
}
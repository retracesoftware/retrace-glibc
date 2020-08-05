#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>

#include "retrace/retrace-lib.h"

int check_open(const char *filename) 
{
    RLog_Init(&rlog, LOG_FILE, Retrace_Record_Mode);

    register int i;
    FILE *fp;
    float data[100];

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0)
    {
        printf("cannot open file");
        exit(1);
    }

    for(i=0; i<100; i++) 
        data[i] = (float) i;

    write(fd, data, sizeof(data));
    close(fd);

    for(i=0; i<100; i++) 
        data[i] = 0.0;

    if((fp=fopen(filename,"rb")) == NULL ) 
    {
        printf("cannot open file");
        return 1;
    }

    fread(data, sizeof data, 1, fp);

    for(i=0; i<100; i++)
        printf("%f  \n", data [i]);

    fflush(fp);
    fclose(fp);
    

    RLog_Displose(&rlog);
}

int check_socket()
{
    int pid = fork();

    if(pid == -1)
    {
        fprintf(stderr, "fork error!\n");
        return EXIT_FAILURE;
    }

    // if child
    if(pid == 0) 
    {
        char message[] = "Hello there!\n";
        char buf[sizeof(message)];

        // Echo client
        struct sockaddr_in addr;
        int sock = socket(AF_INET, SOCK_STREAM, 0);

        if(sock < 0)
        {
            perror("socket");
            exit(1);
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(3425); 
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        
        if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("connect");
            exit(2);
        }

        send(sock, message, sizeof(message), 0);
        recv(sock, buf, sizeof(message), 0);
        
        printf("buffer: %s\n", buf);
        close(sock);

        return 0;
    }
    else
    {
        // Echo server
        int sock, listener;
        struct sockaddr_in addr;
        char buf[1024];
        int bytes_read;

        listener = socket(AF_INET, SOCK_STREAM, 0);
        
        if(listener < 0)
        {
            perror("socket");
            exit(1);
        }
        
        addr.sin_family = AF_INET;
        addr.sin_port = htons(3425);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            exit(2);
        }

        listen(listener, 1);
        
        sock = accept(listener, NULL, NULL);
        
        if(sock < 0)
        {
            perror("accept");
            exit(3);
        }

        bytes_read = recv(sock, buf, 1024, 0);
        
        send(sock, buf, bytes_read, 0);
        
        close(sock);

        return 0;
    }
    

    
}

int main(void)
{
    check_socket();

    return 0;
}
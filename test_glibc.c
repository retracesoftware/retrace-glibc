#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "retrace/retrace-lib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void init_ptrGlobal(){
    //ptrRetraceSocket = Retrace_Socket;
    ptrRetraceConnect = Retrace_Connect;
    //ptrRetraceBind = Retrace_Bind;
    //ptrRetraceListen = Retrace_Listen;
    ptrRetraceAccept = Retrace_Accept;
    ptrRetraceSend = Retrace_Send;
    ptrRetraceRecv = Retrace_Recv;

}

int check_sleep()
{
    printf("Time: %d\n", time(NULL));
    sleep(1);

    printf("Time: %d\n", time(NULL));
    sleep(1);

    printf("Time: %d\n", time(NULL));
    sleep(1);
}

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
void echo_client(){

    char message[] = "Hello there!\n";
    char buf[sizeof(message)];

    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425); // или любой другой порт...
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }

    send(sock, message, sizeof(message), 0);
    recv(sock, buf, sizeof(message), 0);

    printf(buf);
    close(sock);


}
void echo_server()
{
    int sock, listener;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;
    printf("socket\n");
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

    while(1)
    {
        printf(" echoSOkcketServ accept\n");
        sock = accept(listener, NULL, NULL);
        if(sock < 0)
        {
            perror("accept");
            exit(3);
        }

        while(1)
        {
            bytes_read = recv(sock, buf, 1024, 0);
            if(bytes_read <= 0) break;
            send(sock, buf, bytes_read, 0);
        }

        close(sock);
    }
}
void sender()
{
    char msg1[] = "Hello there!\n";
    char msg2[] = "Bye bye!\n";
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sendto(sock, msg1, sizeof(msg1), 0,
           (struct sockaddr *)&addr, sizeof(addr));

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    send(sock, msg2, sizeof(msg2), 0);

    close(sock);

}
void receiver()
{
    int sock;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    while(1)
    {
        bytes_read = recvfrom(sock, buf, 1024, 0, NULL, NULL);
        buf[bytes_read] = '\0';
        printf(buf);
    }

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
int test_soceket(){
    sender();
    receiver();
}
//extern int (*ptrRetraceSocket) (int, int, int) = Retrace_Socket;
//extern int (*ptrRetraceAccept) (int fd, __SOCKADDR_ARG addr, socklen_t *len) = NULL;// = Retrace_Accept;
int main(void)
{
    init_ptrGlobal();
    printf("test\n");
    ptrRetraceAccept = Retrace_Accept;
    check_open("test.txt");
    //check_socket();
    test_soceket();
    //echoSOkcketServ();
    check_sleep();
    return 0;
}

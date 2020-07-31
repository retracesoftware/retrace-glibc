#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void)
{
    check_open("balance");

    return 0;
}
#include <stdio.h>
#include <stdlib.h>

#include "retrace/retrace-lib.h"

int main(void)
{
    RLog_Init(&rlog, "log.dat", Retrace_Replay_Mode);   

    register int i;
    FILE *fp;
    float balance[100];
     
    
    if((fp=fopen("balance", "wb"))==NULL) {
    printf("Cannot open file.");
    return 1;
    }
    for(i=0; i<100; i++) balance[i] = (float) i;

    fwrite(balance, sizeof balance, 1, fp) ;
    fclose(fp);

    for(i=0; i<100; i++) balance[i] = 0.0;

    if((fp=fopen("balance","rb"))==NULL) {
    printf("cannot open file");
    return 1;
    }
  
    fread(balance, sizeof balance, 1, fp);


    for(i=0; i<100; i++) 
        printf("%f  \n", balance [i]);
        
    fclose(fp);
    
    //RLog_Displose(&rlog);


    return 0;
}
#include <stdio.h>
#include <stdlib.h>

/* #define _GNU_SOURCE
#include <dlfcn.h> */

#include "Retrace-lib.h"

int main(void)
{
    /* void *imglib;
    
 
    imglib = dlopen("./build/install/lib/ld-linux-x86-64.so.2", RTLD_NOW);

    if ( imglib != NULL ) 
    {
        void* obj = dlsym(imglib, "retrace_mode");

        int* ptr = (int*)obj;    
        printf("%d\n", *ptr);

    }
    else return EXIT_FAILURE;

    if (imglib != NULL ) dlclose(imglib);
    return EXIT_SUCCESS;
 */
    register int i;
    FILE *fp;
    float balance[100];
     
    retrace_mode = Retrace_Read_Mode;

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


    for(i=0; i<100; i++) printf("%f  ", balance [i]);
    fclose(fp);
    
    return 0;
}
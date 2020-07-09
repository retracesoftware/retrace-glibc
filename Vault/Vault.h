
#if !defined(VAULT_H)
#define VAULT_H

#define DEBUG
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>



typedef enum
{
    Vault_Disabled_Mode,
    Vault_Write_Mode,
    Vault_Read_Mode
    
} Vault_Mode;

typedef struct
{
    char* _path;
    FILE* _fileHandler;
    Vault_Mode mode;
    
} Vault;

Vault main_vlt;

void __attribute__((constructor)) Load(); 
void __attribute__((destructor)) Unload();

void Load();   
void Unload(); 

void Vault_Init(Vault* vlt, char* vlt_path, Vault_Mode vmode);

bool Is_Vault_OK(Vault* vlt);

bool Is_File_Empty(Vault* vlt);

bool Is_NULL(Vault* vlt);

void Vault_SetMode(Vault* vlt, Vault_Mode vmode);


// Vault push-function
void Vault_Push(Vault* vlt, const void* buffer, size_t buffer_size); 

// Vault fetch-function
void* Vault_Fetch(Vault* vlt, void* buffer, size_t buffer_size);

size_t Vault_Fetch_Length(Vault* vlt);

void Vault_Dispose(Vault* vlt);

void CheckFuncName(Vault* vlt, const char* fname, size_t fname_len);

#endif
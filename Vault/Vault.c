
#include "Vault.h"

void Load()
{
    printf("Loaded!\n");

    /* const char* env_name = "VAULT_MODE";
    char* env_value = getenv(env_name);

    if (NULL == env_value)
    {
        fprintf(stderr, "Environment value is NULL!\n");
        abort();
    }
    
    if (!strcmp(env_value, "write"))
        Vault_Init(&main_vlt, "log.dat", Vault_Write_Mode);
    else if (!strcmp(env_value, "read"))
        Vault_Init(&main_vlt, "log.dat", Vault_Read_Mode);
    else if (!strcmp(env_value, "disabled"))
        Vault_Init(&main_vlt, "log.dat", Vault_Disabled_Mode);
    else
    {
        fprintf(stderr, "Environment value isn't set correctly!\n");
        abort();
    } */
}

void Unload()
{
    printf("Unloaded!\n");
    //Vault_Dispose(&main_vlt);
}

void Vault_Init(Vault* vlt, char* vlt_path, Vault_Mode vmode)
{
    if(vlt == NULL) return;

    vlt->_path = vlt_path;
    vlt->mode = vmode;

    vlt->_fileHandler = fopen(vlt->_path, "a+b");

    assert(vlt->_fileHandler != NULL);

    printf("%s() success\n\n", __func__);
}

bool Is_Vault_OK(Vault* vlt)
{
    if(vlt == NULL) return false;
    else if(vlt->_fileHandler == NULL || vlt->_path == NULL) return false;
    else return true;
}

void Vault_SetMode(Vault* vlt, Vault_Mode vmode)
{
    if (vlt->_fileHandler == NULL)
    {
        fprintf(stderr, "%s: error! File handler is NULL\n", __func__);
        exit(EXIT_FAILURE);
    }

    vlt->mode = vmode;

    switch (vmode)
    {
        case Vault_Write_Mode:
            assert(0 == fseek(vlt->_fileHandler, 0, SEEK_END));
            break;
        case Vault_Read_Mode:
            assert(0 == fseek(vlt->_fileHandler, 0, SEEK_SET));
            break;

        default: break;
    }
}

bool Is_File_Empty(Vault* vlt)
{
    if (vlt->_fileHandler == NULL)
    {
        fprintf(stderr, "%s: error! File handler is NULL\n", __func__);
        exit(EXIT_FAILURE);
    }

    fseek(vlt->_fileHandler, 0, SEEK_END);
    long size = ftell(vlt->_fileHandler);
    fseek(vlt->_fileHandler, 0, SEEK_SET);

    if (0 == size) return true;
    else return false;
}

bool Is_NULL(Vault* vlt)
{
    const size_t nullstr_size = sizeof("NULL");
    char temp_buf[nullstr_size];

    fread(temp_buf, 1, nullstr_size, vlt->_fileHandler);
    
    if (!strcmp(temp_buf, "NULL")) return true;
    else return false;
}

void Vault_Push(Vault* vlt, const void* buffer, size_t buffer_size)
{
    if (vlt->mode != Vault_Write_Mode) return;

    if(!Is_Vault_OK(vlt))
    {
        fprintf(stderr, "Vault is not initialized correctly!\n");
        return;
    }
    
    if (NULL == buffer)
    {
        char temp_buf[] = "NULL";
        size_t nullstr_size = sizeof(temp_buf);

        fwrite(&nullstr_size, sizeof(nullstr_size), 1, vlt->_fileHandler);
        fwrite(temp_buf, nullstr_size, 1, vlt->_fileHandler);

        return;
    }    
	
    fwrite(&buffer_size, sizeof(buffer_size), 1, vlt->_fileHandler);
    fwrite(buffer, buffer_size, 1, vlt->_fileHandler);
}

void* Vault_Fetch(Vault* vlt, void* buffer, size_t buffer_size)
{
    if (vlt->mode != Vault_Read_Mode)
    {
        fprintf(stderr, "Vault is not in read mode!\n");
        abort();
    }

    if(!Is_Vault_OK(vlt))
    {
        fprintf(stderr, "Vault is not initialized correctly!\n");
        abort();
    }

    size_t read_len = 0;
    fread(&read_len, sizeof(read_len), 1, vlt->_fileHandler);
    
    if (read_len < 0)
    {
        fprintf(stderr, "%s(): Length is less zero (%ld)!\n", __func__, read_len);
        abort();
    }
    
    if (buffer_size != read_len)
    {
        if(sizeof("NULL") == read_len)
        {
            if(Is_NULL(vlt)) return NULL;
        }

        fprintf(stderr, "Length of buffer (%ld) is not equal to read one: (%ld)!\n", buffer_size, read_len);

        #ifdef DEBUG
            fread(buffer, 1, read_len, vlt->_fileHandler);

            printf("Read data: ");
            for(int i = 0; i < read_len; i++)
                printf("%c", ((char*)buffer)[i]);
            printf("\n");
        #endif

        abort();
    }

    if(sizeof("NULL") == read_len)
    {
        if(Is_NULL(vlt)) return NULL;
        else fseek(vlt->_fileHandler, -sizeof("NULL"), SEEK_CUR);
    }
    
    fread(buffer, 1, buffer_size, vlt->_fileHandler);
    
    return buffer;
}

size_t Vault_Fetch_Length(Vault* vlt)
{
    if (vlt->mode != Vault_Read_Mode)
    {
        fprintf(stderr, "Vault is not in read mode!\n");
        abort();
    }

    if(!Is_Vault_OK(vlt))
    {
        fprintf(stderr, "Vault is not initialized correctly!\n");
        abort();
    }

    size_t read_len = 0;

    fread(&read_len, sizeof(read_len), 1, vlt->_fileHandler);
    
    if (read_len < 0)
    {   
        fprintf(stderr, "Length is less zero!\n");
        abort();               
    }
    
    fseek(vlt->_fileHandler, -sizeof(read_len), SEEK_CUR);

    return read_len;
}

void Vault_Dispose(Vault* vlt)
{
    if (vlt == NULL) return;
    if(vlt->_fileHandler == NULL) return;

    vlt->mode = Vault_Disabled_Mode;

    if(EOF == fclose(vlt->_fileHandler))
    {
        printf("File close failed!\nError desc: %s", strerror(errno));
        return;
    }

    vlt->_fileHandler = NULL;
    vlt->_path = NULL;

    printf("%s() success\n\n", __func__);
}


void CheckFuncName(Vault* vlt, const char* fname, size_t fname_len)
{
    #ifdef DEBUG
        printf("Fetching of function: %s()\n", fname);
    #endif

    char* func_name = malloc(fname_len);
    
    if (NULL == func_name)
    {
        fprintf(stderr, "%s(): Error dynamic allocation!\n", __func__);
        abort();
    }

    Vault_Fetch(vlt, func_name, fname_len);
    
    if (strcmp(func_name, fname))
    {
        fprintf(stderr, "Name of this function (%s()) is not equal to recorded one! (%s())\n", fname, func_name);
        free(func_name);
        abort();
    }
}


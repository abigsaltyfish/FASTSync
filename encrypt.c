#include <openssl/aes.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

unsigned char *str2hex(char *str)
{
    unsigned char *ret = NULL;
    int str_len = strlen(str);
    int i = 0;
    if((str_len % 2) != 0){
        str[str_len-1] = '\0';
        str_len -= 1;
    }
    ret = (char *)malloc(str_len / 2);
    for (i = 0; i < str_len; i = i + 2)
    {
        sscanf(str + i, "%2hhx", &ret[i / 2]);
    }
    return ret;
}

char *padding_buf(char *buf, int size, int *final_size)
{
    char *ret = NULL;
    int pidding_size = AES_BLOCK_SIZE - (size % AES_BLOCK_SIZE);
    int i;
    *final_size = size + pidding_size;
    ret = (char *)malloc(size + pidding_size);
    memcpy(ret, buf, size);
    if (pidding_size != 0)
    {
        for (i = size; i < (size + pidding_size); i++)
        {
            ret[i] = 0;
        }
    }
    return ret;
}

void encrypt_chunk(char *chunk, int len, int key)
{
    int pad_len = (len % 16) == 0 ? 0 : 16 - (len % 16);
    char *tem_buf = NULL;//暂存原数据
    char *encrypt_buf = NULL;//暂存加密数据
    char string_key[16] = {0};
    AES_KEY aes;
    unsigned char *iv = str2hex("667b02a85c61c786def4521b060265e8");
    sprintf(string_key, "%d", key);//将int型的密钥转换成字符串
    AES_set_encrypt_key(string_key, 128, &aes);
    encrypt_buf = malloc(len + pad_len);
    tem_buf = malloc(len + pad_len);
    memmove(tem_buf,chunk,len);
    AES_cbc_encrypt(tem_buf, encrypt_buf, len + pad_len, &aes, iv, AES_ENCRYPT);
    memmove(chunk,encrypt_buf,len);
    free(encrypt_buf);
    free(tem_buf);
}

// void decrypt(char *raw_buf,int int_key, char **encrpy_buf, int len)
// {
//     AES_KEY aes;
//     char string_key[16] = {0};
// 	sprintf(string_key, "%d", int_key);
//     unsigned char *key = str2hex(string_key);
//     unsigned char *iv = str2hex("667b02a85c61c786def4521b060265e8");
//     AES_set_decrypt_key(key, 128, &aes);
//     AES_cbc_encrypt(raw_buf, *encrpy_buf, len, &aes, iv, AES_DECRYPT);
//     free(key);
//     free(iv);
// }

// char *decrypt_chunk(char *chunk, int len, int key)
// {
//     char *decrypt_buf = NULL;

//     decrypt_buf = (char *)malloc(len);
//     decrypt(chunk, key, &decrypt_buf, len);

//     return decrypt_buf;
// }

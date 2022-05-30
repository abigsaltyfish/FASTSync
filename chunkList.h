#include "uthash.h"
#include <stdio.h>
#include <openssl/md5.h>

typedef struct chunk{
    int id;
    int offset;
    int length;
    uint64_t weakhash;
    unsigned char stronghash[17];
}chunk;

typedef struct chunkList{
    int size;
    int capcity;
    chunk *chunks;
}chunkList;

void Hex2Str(const char *sSrc,char *sDest,int nSrcLen);

void printChunkMsg(chunkList list);

void printchunki(chunkList list,int i);

chunkList newchunkList(int initsize);

void addchunk(chunkList *list,int id,int offset,int length,uint64_t weakhash,unsigned char* stronghash);

void getHash(MD5_CTX ctx,unsigned char outmd[17],unsigned char *content,int len);
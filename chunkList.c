#include "chunkList.h"

void Hex2Str( const char *sSrc,  char *sDest, int nSrcLen )
{
    int  i;
    char szTmp[3];
    for( i = 0; i < nSrcLen; i++ )
    {
        sprintf( szTmp, "%02X", (unsigned char) sSrc[i] );
        memcpy( &sDest[i * 2], szTmp, 2 );
    }    //memcpy(list->chunks + list->size * sizeof(chunk),&c,sizeof(chunk));
    return;
}

void printChunkMsg(chunkList list){
    for(int i=0;i<list.size;i++){
        chunk c = list.chunks[i];
        char hash[33];
        Hex2Str(c.stronghash,hash,16);
        hash[32] = 0;
        printf("chunk id:%6d offset = %10d length = %6d hash = %20lu md5 = %s\n",c.id,c.offset,c.length,c.weakhash,hash);
        if(i % 512 == 0){
            printf("\n");
        }
    }
}

void printchunki(chunkList list,int i){
    chunk c = list.chunks[i];
    char hash[33];
    Hex2Str(c.stronghash,hash,16);
    hash[32] = 0;
    printf("chunk id:%6d offset = %10d length = %6d hash = %20lu md5 = %s\n",c.id,c.offset,c.length,c.weakhash,hash);
    if(i % 512 == 0){
        printf("\n");
    }
}

chunkList newchunkList(int initsize){
    chunkList list;
    list.size = 0;
    list.capcity = initsize;
    list.chunks = (chunk*)malloc(initsize * sizeof(chunk));
    return list;
}

void addchunk(chunkList *list,int id,int offset,int length,uint64_t weakhash,unsigned char* stronghash){
    if(list->size == list->capcity){
        //ensurecapcity
        list->capcity *= 2;
        list->chunks = (chunk*)realloc(list->chunks,list->capcity * sizeof(chunk));
    }
    chunk c;
    c.id = id;
    c.offset = offset;
    c.length = length;
    c.weakhash = weakhash;
    strcpy(c.stronghash,stronghash);
    list->chunks[list->size] = c;
    //memcpy(list->chunks + list->size * sizeof(chunk),&c,sizeof(chunk));
    list->size++;
}

void getHash(MD5_CTX ctx,unsigned char outmd[17],unsigned char *content,int len){
    MD5_Init(&ctx);
    MD5_Update(&ctx,content,len);
    MD5_Final(outmd,&ctx);
    outmd[16] = 0;
}

//test
// void main(){
//     chunkList list = newchunkList(8);
//     for(int i=0;i<10000;i++){
//         addchunk(&list,i,i*100,100,1,"1");
//     }
//     printChunkMsg(list);
// }
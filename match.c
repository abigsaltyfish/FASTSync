#include "match.h"

void send_token(int fd,int id);

void send_delta_data(int fd,unsigned char *fileCache,int offset,int length);

struct hashmap* buildhashtable(chunkList list){
    struct hashmap *map = hashmap_new(sizeof(hash2id), 0, 0, 0, 
                                     hash_func, compare_func, NULL, NULL);

    for(int i=0;i<list.size;i++){
        hashmap_set(map, &(hash2id){ .weakhash=list.chunks[i].weakhash, .id=list.chunks[i].id });
    }

    // hashmap_scan(map, iter_func, NULL);

    return map;
}

int ismatch(uint64_t weakhash,struct hashmap *map){
    struct hash2id *hash;
    hash = hashmap_get(map, &(hash2id){ .weakhash=weakhash });
    if(hash){
        //printf("hash=%ld probably match in chunk %d\n", weakhash, hash->id);
        return hash->id;
    }
    return -1;
}

void match(chunkList list,chunkList farlist,unsigned char *fileCache,int fd){
    int num = 0;
    int unmatch = 0;
    int weak_match_time = 0;
    int strong_match_time = 0;
    struct hashmap *map = buildhashtable(farlist);
    struct timeval matchStartTime,matchEndTime;
    gettimeofday(&matchStartTime, NULL);
    for(int i=0;i<list.size;i++){
        int id = ismatch(list.chunks[i].weakhash,map);
        weak_match_time++;
        if(id != -1){
            strong_match_time++;
        }
        if(id != -1 && !strcmp(list.chunks[i].stronghash,farlist.chunks[id].stronghash)){
            //printf("match in chunk %d\n",id);
            num++;
            //send token
            send_token(fd,id);
        }else{
            //send data
            send_delta_data(fd,fileCache,list.chunks[i].offset,list.chunks[i].length);
            unmatch++;
        }
    }
    gettimeofday(&matchEndTime, NULL);
    float totalTm = ((matchEndTime.tv_sec - matchStartTime.tv_sec) * 1000000 +
                    (matchEndTime.tv_usec - matchStartTime.tv_usec));
    printf("total weak match %d\n",weak_match_time);
    printf("total strong match %d\n",strong_match_time);
    printf("match spend time %f s\n",totalTm / 1000000);
}

//test
// int main(void)
// {
//     uint64_t feature = 0;
//     FILE *fp = fopen("/home/aaa/文档/Ctest/1.mp4","r");
//     unsigned char *fileCache;
//     chunkList list = cdchunking(fp,&feature,fileCache);
//     match(list,list,fileCache);
//     free(fileCache);
//     return 0;
// }
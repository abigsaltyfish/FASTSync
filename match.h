#include <openssl/md5.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <zlib.h>
#include "chunkList.h"
#include "hashmap.h"

typedef struct hash2id
{
    uint64_t weakhash;
    int id;
}hash2id;

int compare_func(const void *a, const void *b, void *udata) {
    const struct hash2id *ua = a;
    const struct hash2id *ub = b;
    return ua->weakhash != ub->weakhash;
}

uint64_t hash_func(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct hash2id *hash = item;
    return hash->weakhash;
}

bool iter_func(const void *item, void *udata) {
    const struct hash2id *hash = item;
    printf("hash=%ld chunkid=%d\n", hash->weakhash, hash->id);
    return true;
}

//getchunklist
chunkList cdchunking(FILE *fp,uint64_t *feature,unsigned char *fileCache);

void match(chunkList list,chunkList farlist,unsigned char *fileCache,int fd);
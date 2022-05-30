#include "fastcdc.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "encrypt.h"
#include "utils.h"

// USE_CHUNKING_METHOD
#define USE_CHUNKING_METHOD 1
#define STEP 32

int feature_search(unsigned char *p, int n,uint64_t *feature);

int feature_based_encrypt(int fd_src,unsigned char *fileCache){
    uint64_t weakhash = 0;
    uint8_t SHA1_digest[20];
    size_t readStatus = 0;
    uint64_t feature = 0;
 
    int chunk_num = 0;

    memset(fileCache, 0, sizeof(CacheSize));

    int offset = 0, chunkLength = 0, readFlag = 0;
    fastCDC_init();

    struct stat st;
    if (fstat (fd_src, &st) < 0) {
        perror("Fail to open file");
        exit(-1);
    }
    uint64_t expected_size = st.st_size;
    

    readStatus = readn(fd_src, fileCache, expected_size);
    int end = readStatus;
    int maxchunksum = (readStatus / MinSize) + 1;
    int boundary[maxchunksum];
         
    //fast search feature
    for (;;) {
        // original fastcdc
        chunkLength = feature_search(fileCache + offset, end - offset, &feature);
        boundary[chunk_num] = chunkLength;
        offset += chunkLength;
        chunk_num += 1;
        if (offset >= end) 
            break;
    }

    //encrypt content based on feature
    offset = 0;
    for(int i=0;i<chunk_num;i++){
        encrypt_chunk(fileCache + offset,boundary[i],feature);
        offset += boundary[i];
    }

    return offset;
}

// functions
void fastCDC_init(void) {
    unsigned char md5_digest[16];
    uint8_t seed[SeedLength];
    for (int i = 0; i < SymbolCount; i++) {

        for (int j = 0; j < SeedLength; j++) {
            seed[j] = i;
        }

        g_global_matrix[i] = 0;
        MD5(seed, SeedLength, md5_digest);
        memcpy(&(g_global_matrix[i]), md5_digest, 4);
        g_global_matrix_left[i] = g_global_matrix[i] << 1;
    }

    // 64 bit init
    for (int i = 0; i < SymbolCount; i++) {
        LEARv2[i] = GEARv2[i] << 1;
    }

    MinSize = 8192 / 4;
    MaxSize = 8192 * 4;    // 32768;
    Mask_15 = 0xf9070353;  //  15个1
    Mask_11 = 0xd9000353;  //  11个1
    Mask_11_64 = 0x0000d90003530000;
    Mask_15_64 = 0x0000f90703530000;
    MinSize_divide_by_2 = MinSize / 2;
}

int feature_search(unsigned char *p, int n,uint64_t *feature){
    uint64_t fingerprint = 0, digest;
    int i = 0,ptr = 0;

    // The chunk size cannot exceed remaining length of file and MaxSize.
    n = n < MaxSize ? n : MaxSize;

    while (i <= n) {
        fingerprint = (fingerprint << 1) + (GEARv2[p[i]]);
        *feature = *feature > fingerprint ? *feature : fingerprint;
        if (i > MinSize && (!(fingerprint & FING_GEAR_08KB_64))) {
            return i;
        }
        i++;
        ptr++;
    }

    return n;
}
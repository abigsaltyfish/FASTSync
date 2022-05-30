#include "fastcdc.h"

// USE_CHUNKING_METHOD
#define USE_CHUNKING_METHOD 1
#define STEP 32

chunkList cdchunking(FILE *fp,uint64_t *feature,unsigned char *fileCache){
    chunkList list = newchunkList(8);
    uint64_t weakhash = 0;
    MD5_CTX ctx;
    unsigned char outmd[17];
    char hash[33];

    uint8_t SHA1_digest[20];
    uLong weakHash;
    size_t readStatus = 0;
 
    int chunk_num = 0;

    memset(fileCache, 0, sizeof(CacheSize));

    int offset = 0, chunkLength = 0, readFlag = 0;
    fastCDC_init();

    if (fp == NULL) {
        perror("Fail to open file");
        exit(-1);
    }

    readStatus = fread(fileCache, 1, CacheSize, fp);
    int end = readStatus;

    switch (USE_CHUNKING_METHOD)
    {
        case ORIGIN_CDC:
            chunking = cdc_origin_64;
            break;

        case ROLLING_2Bytes:
            chunking = rolling_data_2byes_64;
            break;
        
        case NORMALIZED_CDC:
            chunking = normalized_chunking_64;
            break;

        case NORMALIZED_2Bytes:
            chunking = normalized_chunking_2byes_64;
            break;
        
        default:
            fprintf(stderr, "No implement other chunking method");
    }
         
    //for debug
    // char headbuf[32];
    // char tailbuf[32];
    for (;;) {
        chunk_num += 1;

        gettimeofday(&tmStart, NULL);

        // original fastcdc
        chunkLength = chunking(fileCache + offset, end - offset, feature, &weakhash);
        
        MD5_Init(&ctx);
        MD5_Update(&ctx,fileCache + offset,chunkLength);
        MD5_Final(outmd,&ctx);
        outmd[16] = 0;

        gettimeofday(&tmEnd, NULL);
        totalTm += ((tmEnd.tv_sec - tmStart.tv_sec) * 1000000 + (tmEnd.tv_usec - tmStart.tv_usec));

        addchunk(&list,chunk_num-1,offset,chunkLength,weakhash,outmd);
        offset += chunkLength;

        if (CacheSize - offset < MaxSize) {
            memcpy(fileCache, fileCache + offset + 1, CacheSize - offset);
            readStatus = fread(fileCache + CacheSize - offset, 1, offset, fp);
            end = CacheSize - 1 - offset + readStatus;

            if (readStatus < offset + 1) {
                // all the files are read
                readFlag = 1;
            }

            offset = 0;
        }

        if (offset >= end) 
            break;
    }

    // printChunkMsg(list);
    // printf("totalTime is %f s\n", totalTm / 1000000);
    // printf("chunknum is %d\n", chunk_num);
    // printf("feature is %lu\n", feature);

    return list;
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

int normalized_chunking_64(unsigned char *p, int n,uint64_t *feature,uint64_t *weakhash) {
    uint64_t fingerprint = 0, digest;
    MinSize = 6 * 1024;
    int i = MinSize, Mid = 8 * 1024;

    // the minimal subChunk Size.
    if (n <= MinSize)  
        return n;

    if (n > MaxSize)
        n = MaxSize;
    else if (n < Mid)
        Mid = n;

    while (i < Mid) {
        fingerprint = (fingerprint << 1) + (GEARv2[p[i]]);

        if ((!(fingerprint & FING_GEAR_32KB_64))) {
            return i;
        }

        i++;
    }

    while (i < n) {
        fingerprint = (fingerprint << 1) + (GEARv2[p[i]]);

        if ((!(fingerprint & FING_GEAR_02KB_64))) {
            return i;
        }

        i++;
    }

    return n;
}

int normalized_chunking_2byes_64(unsigned char *p, int n,uint64_t *feature,uint64_t *weakhash) {
    uint64_t fingerprint = 0, digest;
    MinSize = 6 * 1024;
    int i = MinSize / 2, Mid = 8 * 1024;

    // the minimal subChunk Size.
    if (n <= MinSize) 
        return n;

    if (n > MaxSize)
        n = MaxSize;
    else if (n < Mid)
        Mid = n;

    while (i < Mid / 2) {
        int a = i * 2;
        fingerprint = (fingerprint << 2) + (LEARv2[p[a]]);

        if ((!(fingerprint & FING_GEAR_32KB_ls_64))) {
            return a;
        }

        fingerprint += GEARv2 [p[a + 1]];  

        if ((!(fingerprint & FING_GEAR_32KB_64))) {
            return a + 1;
        }

        i++;
    }

    while (i < n / 2) {
        int a = i * 2;
        fingerprint = (fingerprint << 2) + (LEARv2[p[a]]);

        if ((!(fingerprint & FING_GEAR_02KB_ls_64))) {
            return a;
        }

        fingerprint += GEARv2[p[a + 1]];

        if ((!(fingerprint & FING_GEAR_02KB_64))) {
            return a + 1;
        }

        i++;
    }

    return n;
}

int rolling_data_2byes_64(unsigned char *p, int n,uint64_t *feature,uint64_t *weakhash) {
    uint64_t fingerprint = 0, digest;
    int i = MinSize_divide_by_2;

    // the minimal subChunk Size.
    if (n <= MinSize) 
        return n;

    if (n > MaxSize) n = MaxSize;

    while (i < n / 2) {
        int a = i * 2;
        fingerprint = (fingerprint << 2) + (LEARv2[p[a]]);

        if ((!(fingerprint & FING_GEAR_08KB_ls_64))) {
            return a;
        }

        fingerprint += GEARv2[p[a + 1]];

        if ((!(fingerprint & FING_GEAR_08KB_64))) {
            return a + 1;
        }

        i++;
    }

    return n;
}

int cdc_origin_64(unsigned char *p, int n,uint64_t *feature,uint64_t *weakhash){
    uint64_t fingerprint = 0, digest;
    *weakhash = 0;
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
        if(ptr % STEP == 0){
            *weakhash += fingerprint;
        }
    }

    return n;
}

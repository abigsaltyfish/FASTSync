#include "io.h"
#include "socket.h"
#include "String.h"
#define TEMPBUFSIZE 1024 * 32

void buildFile(int connfd,FILE *fp,int filelen){
    char tempbuf[TEMPBUFSIZE];
    int readsize;
    int ptr = 0;
    while(ptr < filelen){
        readsize = read(connfd, tempbuf, TEMPBUFSIZE);
        fwrite(tempbuf,readsize,1,fp);
        ptr += readsize;
    }
}

void buildDeltaFile(int connfd,FILE *fp,int filelen,chunkList list,unsigned char* filecache){
    int ptr = 0;
    int chunkID;
    int length;
    int readsize;
    int realreadlen;
    char tempbuf[TEMPBUFSIZE];
    int num = 0;
    int check_error_num = 0;
    int chunknum = 0;
    int totalchunknum = read_int(connfd);
    for(;;){
        if(ptr >= filelen || chunknum == totalchunknum){
            fflush(fp);
            break;
        }
        printf("ptr = %d ,total = %d\n",ptr,filelen);
        if(read_head(connfd) == 0){
            //read token id
            chunkID = read_int(connfd);
            int check_id = read_int(connfd);
            if(check_id != chunkID){
                printf_int(check_id);
                printf("check error\n");
                if(chunkID < -1 || chunkID >= list.size){
                    chunkID = check_id;
                    check_error_num++;
                }
            }
            if(chunkID < -1 || chunkID >= list.size){
                printf("chunk read error\n");
                printf("id = %d,offset = %d\n",chunkID,ptr);
                break;
            }
            length = list.chunks[chunkID].length;
            printf("match in chunk id = %d,offset = %d,len = %d\n",chunkID,ptr,length);
            int offset = list.chunks[chunkID].offset;
            fwrite(filecache + offset,length,1,fp);
            ptr += length;
        }else{
            //read data length
            length = read_int(connfd);
            int check_length = read_int(connfd);
            if(check_length != length){
                printf_int(check_length);
                printf("check error\n");
                if(length <= 0 || length > 65536){
                    length = check_length;
                    check_error_num++;
                }
            }
            printf_int(length);
            if(length == 0){
                printf("length read error\n");
                break;
            }
            num++;
            printf("%d %d\n",num,length);
            int ret = read_n(connfd,tempbuf,length);
            if(ret != length){
                printf("read error %d byte\n",length - ret);
                exit(0);
            }
            read_until_eof(connfd);
            fwrite(tempbuf,ret,1,fp);
            ptr += ret;
            fflush(fp);
        }
        chunknum++;
    }
    printf("check num: %d\n",check_error_num);
}

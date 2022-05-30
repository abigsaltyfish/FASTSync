#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include "String.h"
#include "fileinfo.h"
#include "chunkList.h"
#include <signal.h>
#include <errno.h>
#include <fcntl.h> 
#include "socket.h"

#define PORT 7788
#define TEMPBUFSIZE 1024 * 33
#define CacheSize 1024 * 1024 * 1024

struct timeval time1,time2,time3,time4,time5;
int send_volume,recv_volume;

chunkList cdchunking(FILE *fp,uint64_t *feature,unsigned char *fileCache);

void match(chunkList list,chunkList farlist,unsigned char *fileCache,int fd);

void handle(int sig){
  printf("PIPE ERROR %d!\n",sig);
  exit(0);
}

void send_info(FileInfo *info,int fd){
  char buffer[512];
  memset(buffer,0,512);
  memcpy(buffer,info,sizeof(FileInfo));
  write(fd,buffer,sizeof(FileInfo));
  send_volume += sizeof(FileInfo);
}

void recv_ChunkList(int fd,chunkList* farlist){
  int readbyte;
  int onceread = 512 * sizeof(chunk);
  int len = 0;
  struct timeval timeout;
  int nret;
  fd_set fds;
  char emptybuf[4096];

  read(fd,&farlist->size,4);
  recv_volume += 4;
  //Timing : Received chunk list returned by the server
  gettimeofday(&time3, NULL);
  int readsize = farlist->size * sizeof(chunk);
  printf("readsize:%d\n",readsize);
  unsigned char* ptr = malloc(readsize);
  farlist->chunks = (chunk*)ptr;
  recv_volume += readsize;
  
  while(len < readsize){
    onceread = onceread > readsize - len ? readsize - len : onceread;
    readbyte = read(fd,ptr,onceread);
    ptr += readbyte;
    len += readbyte;
    
    //debug
    // printchunki(*farlist,(int)(len/sizeof(chunk)-1));
    // printchunki(*farlist,(int)(len/sizeof(chunk)));
    // if(len/sizeof(chunk) > 40000){
    //   printf("\n");
    // }
  }

  //clean socket buffer
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  while(readbyte != 0){
    nret = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);
    if(nret == 0){
      break;
    }
    readbyte = read(fd,emptybuf,4096);
  }
}

void recv_flag(int fd,char* path){
  int flag,readsize;
  uint64_t feature;
  unsigned char *fileCache;
  char tempbuf[TEMPBUFSIZE];
  FILE *fp = fopen(path,"r");

  read(fd,&flag,4);
  recv_volume += 4;
  //Timing : First received signal from server
  gettimeofday(&time1, NULL);

  if(flag == FLAG_NOEXSIST){
    printf("=== full sync ===\n");
    while((readsize = fread(tempbuf,1,TEMPBUFSIZE,fp)) > 0){
        int ret = write_n(fd, tempbuf, readsize);
        send_volume += ret;
    }
  }else if(flag == FLAG_PARTEXSIST){
    printf("=== delta sync ===\n");
    //chunking
    fileCache = (char *)malloc(CacheSize);
    chunkList list = cdchunking(fp,&feature,fileCache);
    //Timing : finish chunking
    gettimeofday(&time2, NULL);
    //recv chunk list
    chunkList farlist;
    recv_ChunkList(fd,&farlist);
    printf("success recv chunk list\n");
    //Timing : start match
    gettimeofday(&time4, NULL);
    //match,send delta data to server
    send_int(fd,list.size);
    match(list,farlist,fileCache,fd);
    //Timing : finish match
    gettimeofday(&time5, NULL);
    //free cache
    free(fileCache);
  }else if(flag == FLAG_EXSIST){
    printf("file exeist\n");
  }else{
    printf("recv signal error!\n");
  }

  //recv ack
  printf("wait ack\n");
  read(fd,&flag,4);
  printf("endflag = %d\n",flag);
  recv_volume += 4;

  fclose(fp);
}

void send_token(int fd,int id){
  int nwrite = send_message(fd,NULL,id,TOKEN);
  send_volume += nwrite;
}
   
int issocketclosed(int fd){  
  int i = 0;
  int recvBytes = send(fd, &i, sizeof(int), MSG_PEEK);
  int sockErr = errno;
  if(recvBytes > 0){
    return 0;
  }
  if((recvBytes == -1) && (sockErr == EWOULDBLOCK)){
    return 0;
  } 
  return 1;
}

static void sleep_us(unsigned int secs){
  struct timeval tval;
  tval.tv_sec=0;
  tval.tv_usec=secs;
  select(0,NULL,NULL,NULL,&tval);
}

void send_delta_data(int fd,unsigned char *fileCache,int offset,int length){
  //printf("start write\n");
  int nwrite = send_message(fd,fileCache + offset,length,MSG);
  //printf("send : %d byte\n",nwrite-6);
  send_volume += nwrite;
  //printf("total send : %d byte\n",send_volume);
  //printf("\n");
}

void blocksig(int sig){
  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs,sig);
  sigprocmask(SIG_BLOCK,&sigs,NULL);
}

int main(int argc, char const *argv[]) {
  int fd, readsize;
  struct hostent *he;
  struct sockaddr_in server;
  struct timeval startTime, endTime;
  float totalTime,t1,t2,t3,t4,t5;

  signal(SIGPIPE,handle);

  gettimeofday(&startTime, NULL);
  if(argc < 3){
    printf("Usage:%s<IP><DIR><FILE>\n", argv[0]);
    exit(1);
  }

  if((he = gethostbyname(argv[1])) == NULL){
  	printf("argv[1]=%s\n", argv[1]);
    perror("gethostbyname error.\n");
    exit(1);
  }

  if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("socket error.\n");
    exit(1);
  }

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr = *((struct in_addr *)he->h_addr);

  if(connect(fd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1){
    perror("connect error.\n");
    exit(1);
  }

  //set_no_delay(fd);

  const char *dir = argv[2];
  const char *file = argv[3];
  send_volume = 0;
  recv_volume = 0;

  // struct stat st;
  // stat(path,&st);
  // printf("%ld\n",st.st_size);
  // char *statbuf = malloc(sizeof(struct stat));
  // memcpy(statbuf,&st,sizeof(struct stat));
  FileInfo info = getFileInfo(dir,file);
  send_info(&info,fd);

  char path[1000];
  path[0] = 0;
  strcat(path,dir);
  strcat(path,"/");
  strcat(path,file);

  recv_flag(fd,path);

  close(fd);

  gettimeofday(&endTime, NULL);

  //output time data
  t1 = ((time1.tv_sec - startTime.tv_sec) * 1000000 + (time1.tv_usec - startTime.tv_usec));
  t2 = ((time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_usec - time1.tv_usec));
  t3 = ((time3.tv_sec - time1.tv_sec) * 1000000 + (time3.tv_usec - time1.tv_usec));
  t4 = ((time4.tv_sec - time3.tv_sec) * 1000000 + (time4.tv_usec - time3.tv_usec));
  t5 = ((time5.tv_sec - time4.tv_sec) * 1000000 + (time5.tv_usec - time4.tv_usec));
  totalTime = ((endTime.tv_sec - startTime.tv_sec) * 1000000 + (endTime.tv_usec - startTime.tv_usec));
  
  // printf("wait server signal time is %f s\n", t1 / 1000000);
  // printf("client chunking time is %f s\n", t2 / 1000000);
  // printf("wait server chunk list time is %f s\n", t3 / 1000000);
  // printf("receive server chunk list time is %f s\n", t4 / 1000000);
  // printf("match time is %f s\n", t5 / 1000000);
  // printf("totalTime is %f s\n", totalTime / 1000000);

  printf("%f\n", t1 / 1000000);
  printf("%f\n", t2 / 1000000);
  printf("%f\n", t3 / 1000000);
  printf("%f\n", t4 / 1000000);
  printf("%f\n", t5 / 1000000);
  printf("totalTime is %f s\n", totalTime / 1000000);

  printf("total send %d byte\n",send_volume);
  printf("total recv %d byte\n",recv_volume);
  return 0;
}

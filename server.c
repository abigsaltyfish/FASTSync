#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "String.h"
#include "io.h"
#include "socket.h"

#define MAXFD 64
#define PORT 7788
#define BACKLOG 5
#define CacheSize 1024 * 1024 * 1024
#define END 114514

chunkList cdchunking(FILE *fp,uint64_t *feature,unsigned char *fileCache);

void match(chunkList list,chunkList farlist,unsigned char *fileCache);

void daemon_init(const char *pname, int facility)
{
  int i;
  pid_t pid;
  if((pid = fork()) !=0 ){
    exit(0);
  }

  setsid();
  signal(SIGHUP, SIG_IGN);

  if((pid = fork()) != 0){
    exit(0);
  }

  chdir("/");
  sigmask(0);

  for(i = 0; i < MAXFD; i++){
    close(i);
  }

  openlog(pname, LOG_PID, facility);
}

void recv_info(FileInfo *info,int connfd){
  char buffer[512];
  memset(buffer,0,512);
  read(connfd,buffer,sizeof(FileInfo));
  memcpy(info,buffer,sizeof(FileInfo));
}

void notifyClient(int connfd,int flag){
  write(connfd,&flag,4);
}

void send_ChunkList(int connfd,chunkList list){
  int total = list.size * sizeof(chunk);
  int oncewrite = 256 * sizeof(chunk);
  int ptr = 0;
  int len;
  int remain = total;
  char* chunks = (char*)list.chunks;

  write(connfd,&list.size,4);

  while(ptr < total){
    len = oncewrite > remain ? remain : oncewrite;
    int re = write(connfd,chunks + ptr,len);
    ptr += re;
    remain -= re;
  }
}

int main(int argc, char const *argv[]) {
  int listenfd, connfd;
  struct sockaddr_in server;

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  //daemon_init(argv[0], 0);

  if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    syslog(LOG_NOTICE | LOG_LOCAL0, "socket error.");
    exit(-1);
  }

  if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1){
    syslog(LOG_NOTICE | LOG_LOCAL0, "bind error.");
    exit(-1);
  }

  if(listen(listenfd, BACKLOG) == -1){
    syslog(LOG_NOTICE | LOG_LOCAL0, "listen error.");
    exit(-1);
  }

  set_reuse_addr(listenfd);

  for(;;){
    if((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1)
    {
      printf("accept() error\n");
      exit(1);
    }

    FileInfo *info = malloc(sizeof(FileInfo));
    recv_info(info,connfd);
    
    FILE *fp;
    int fileflag = openFile(info,&fp);
    uint64_t nonfeature;
    unsigned char* filecache;
    notifyClient(connfd,fileflag);
    
    if(fileflag == FLAG_NOEXSIST){
      buildFile(connfd,fp,info->size);
    }else if(fileflag == FLAG_PARTEXSIST){
      //chunking
      filecache = (char *)malloc(CacheSize);
      chunkList list = cdchunking(fp,&nonfeature,filecache);
      //printChunkMsg(list);
      //send chunking message
      send_ChunkList(connfd,list);
      //use temp file
      FILE *tempfile = getTempFd(info);
      //build new File
      buildDeltaFile(connfd,tempfile,info->size,list,filecache);
      //free cache
      free(filecache);
      //replace old file
      replace(info);
      //close fp
      fclose(tempfile);
    }

    //send ack
    int endflag = END;
    notifyClient(connfd,endflag);

    close(connfd);
    fclose(fp);
  }

  close(listenfd);
  
  return 0;
}

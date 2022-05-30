#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "fileinfo.h"
#include "chunkList.h"

void buildFile(int connfd,FILE *fp,int filelen);

void buildDeltaFile(int connfd,FILE *fp,int filelen,chunkList list,unsigned char* filecache);
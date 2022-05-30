#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define FLAG_NOEXSIST 0
#define FLAG_PARTEXSIST 1
#define FLAG_EXSIST 2

typedef struct FileInfo
{
    //设定目录长度小于256，文件名长度小于128
    char filename[128];
    char dirname[256];
    off_t size;
    struct timespec lastmodifytime;
}FileInfo;

FileInfo getFileInfo(const char *basename,const char *filename);

void printFileInfo(FileInfo info);

void getBasePath(char *basePath);

int openFile(FileInfo *info,FILE **fd);

FILE* getTempFd(FileInfo *info);

void replace(FileInfo *info);
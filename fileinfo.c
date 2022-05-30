#include "fileinfo.h"

char *storepath = "/home/aaa/store/";

 int readFileList(char *basePath)
 {
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    printf("the current dir is : %s\n",basePath);

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8){
            FileInfo info = getFileInfo(basePath,ptr->d_name);
            printFileInfo(info);
            FILE* fd;
            int flag = openFile(&info,&fd);
            printf("%d\n",flag);
        }
        else if(ptr->d_type == 10)    ///link file
            printf("d_name:%s/%s\n",basePath,ptr->d_name);
        else if(ptr->d_type == 4)    ///dir
        {
            memset(base,'\0',sizeof(base));
            strcpy(base,basePath);
            strcat(base,"/");
            strcat(base,ptr->d_name);
            readFileList(base);
        }
    }
    closedir(dir);
    return 1;
 }

 FileInfo getFileInfo(const char *basename,const char *filename){
    char path[1000];
    path[0] = 0;
    strcat(path,basename);
    strcat(path,"/");
    strcat(path,filename);
    struct stat st;
    stat(path,&st);
    FileInfo info;
    info.size = st.st_size;
    info.lastmodifytime = st.st_mtim;
    memset(info.dirname,0,256);
    memset(info.filename,0,128);
    memcpy(info.dirname,basename,strlen(basename));
    memcpy(info.filename,filename,strlen(filename));
    return info;
 }

 int openFile(FileInfo *info,FILE **fd){
    char path[128];
    path[0] = 0;
    strcat(path,storepath);
    strcat(path,info->filename);
    if((*fd = fopen(path,"r")) == NULL){
        *fd = fopen(path,"w");
        return FLAG_NOEXSIST;
    }
    return FLAG_PARTEXSIST;
 }

 FILE* getTempFd(FileInfo *info){
    char path[128];
    path[0] = 0;
    strcat(path,storepath);
    strcat(path,info->filename);
    strcat(path,".tmp");
    FILE *fd = fopen(path,"w");
    return fd;
 }

 void replace(FileInfo *info){
    char oldpath[128];
    char newpath[128];
    memset(oldpath,0,128);
    memset(newpath,0,128);
    strcat(oldpath,storepath);
    strcat(newpath,storepath);
    strcat(oldpath,info->filename);
    strcat(newpath,info->filename);
    strcat(newpath,".tmp");
    rename(newpath,oldpath);
 }

 void printFileInfo(FileInfo info){
    printf("File:%s/%s size:%ld\n",info.dirname,info.filename,info.size);
 }

 void getBasePath(char *basePath){
    ///get the current absoulte path
    memset(basePath,'\0',sizeof(basePath));
    getcwd(basePath, 999);
 }
 
//  int main(void)
//  {
//     DIR *dir;
//     char basePath[1000];

//     ///get the current absoulte path
//     getBasePath(basePath);
//     ///get the file list
//     readFileList(basePath);
//     return 0;
//  }

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

int get_file_count(char *root)
{
DIR *dir;
struct dirent * ptr;
int total = 0;
dir = opendir(root); /* 打开目录*/
if(dir == NULL)
{
perror("fail to open dir");
exit(1);
}
errno = 0;
while((ptr = readdir(dir)) != NULL)
{
if(ptr->d_name[0] == '.' && ptr->d_type == DT_DIR)
{
continue;
}
if(ptr->d_type == DT_DIR)
{
char slash[1] = "/";
char *path = malloc(1024);
memcpy(path,root,1024);
strcat(path,slash);
strcat(path,ptr->d_name);
total += get_file_count(path);
}

if(ptr->d_type == DT_REG)
{
total++;
}
}
if(errno != 0)
{
printf("fail to read dir"); //失败则输出提示信息
exit(1);
}
closedir(dir);
return total;
}

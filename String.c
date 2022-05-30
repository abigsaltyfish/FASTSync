#include "String.h"
#include <string.h>
#include <time.h>

int min(int a,int b){
    if(a > b){
        return b;
    }
    return a;
}

void printf_int(int i){
    unsigned char c[4];
    memcpy(c,&i,sizeof(int));
    printf("%hhu %hhu %hhu %hhu\n",c[0],c[1],c[2],c[3]);
}

void printf_short(short i){
    unsigned char c[2];
    memcpy(c,&i,sizeof(short));
    printf("%hhu %hhu\n",c[0],c[1]);
}

int get_random_num(int max){
    srand(time(NULL));
    int a = rand() % max;
    return a;
}
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include "socket.h"
#include <string.h>

void set_no_delay(int fd){
    int off = 1;
    setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&off,sizeof(off));
}

void set_reuse_addr(int fd){
    int on = 1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR, &on, sizeof(on));
}

int write_n(int fd,const void* buf,int len){
    int write_byte = 0;
    while (write_byte < len){
        int ret = send(fd,buf,len - write_byte,0);
        if(ret == 0){
            break;
        }else if(ret == -1){
            printf("write error\n");
            exit(0);
        }
        write_byte += ret;
    }
    return write_byte;
}

int read_n(int fd,void* buf,int len){
    int read_byte = 0;
    while(read_byte < len){
        int ret = recv(fd, buf, len - read_byte,0);
        if(ret == 0){
            break;
        }else if(ret == -1){
            printf("read error\n");
            exit(0);
        }
        read_byte += ret;
    }
    return read_byte;
}

int read_eof(int fd){
    char c;
    read(fd, &c, 1);
    if(c != EOF){
        printf("read message end error\n");
        return 0;
    }
    return 1;
}

void read_until_eof(int fd){
    int cnt = 0;
    while(read_eof(fd) == 0){
        cnt++;
    }
    if(cnt > 0){
        printf("read %d byte to read eof\n",cnt);
    }
}

int read_head(int fd){
    char c;
    read(fd, &c, 1);
    if(c == ID){
        return 0;
    }else if(c == LENGTH){
        return 1;
    }else{
        printf("read head error\n");
        exit(0);
    }
}

int read_int(int fd){
    uint32_t net_num = 0;
    read_n(fd,(char*)&net_num,sizeof(uint32_t));
    return ntohl(net_num);
}

int send_int(int fd,int num){
    uint32_t net_num = htonl(num);
    int nwrite = write_n(fd,&net_num,sizeof(uint32_t));
    return nwrite;
}

int send_message(int fd,void* buf,int len,int flag){
    if(flag == MSG){
        char tempbuf[len + 6];
        tempbuf[0] = LENGTH;
        uint32_t net_len = htonl(len);
        memcpy(tempbuf+1,&net_len,sizeof(uint32_t));
        memcpy(tempbuf+5,&net_len,sizeof(uint32_t));
        memcpy(tempbuf+9,buf,len);
        tempbuf[len+9] = EOF;
        int nwrite = write_n(fd,tempbuf,len+10);
        return nwrite;
    }else if(flag == TOKEN){
        char tokenbuf[9];
        tokenbuf[0] = ID;
        int net_id = htonl(len);
        memcpy(tokenbuf+1,&net_id,sizeof(int));
        memcpy(tokenbuf+5,&net_id,sizeof(int));
        int nwrite = write_n(fd,tokenbuf,9);
        return nwrite;
    }
    return -1;
}

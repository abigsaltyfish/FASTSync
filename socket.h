#define ID '\002'
#define LENGTH '\003'
#define TOKEN 0
#define MSG 1

void set_no_delay(int fd);

void set_reuse_addr(int fd);

int write_n(int fd,const void* buf,int len);

int read_n(int fd,void* buf,int len);

int read_eof(int fd);

void read_until_eof(int fd);

int read_head(int fd);

int read_int(int fd);

int send_int(int fd,int num);

int send_message(int fd,void* buf,int len,int flag);
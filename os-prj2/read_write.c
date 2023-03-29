
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
void write_safe(int fd,  void* buf, int len)
{
    int ret;
    while(len != 0 && (ret = write(fd, buf, len)))
    {
        if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            perror("write");
            exit(EXIT_FAILURE);
        }

        len -= ret;
        buf += ret;
    }   
}

void read_safe(int fd,  void* buf, int len)
{
    int ret;
    while(len != 0 && (ret = read(fd, buf, len)) != 0)
    {
        if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            perror("read");
            exit(EXIT_FAILURE);
        }

        len -= ret;
        buf += ret;
    }
}

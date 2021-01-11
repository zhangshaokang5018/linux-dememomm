#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include "demomem.h"
#include <fcntl.h>

#define DEVICE_PATH            "/dev/demomem"

int main()
{
    int fd;
    int ret;
    int n;
    int val;
    char buf[10] = "hello\n";
    fd = open(DEVICE_PATH,O_RDWR);
    if(fd < 0)
    {
        printf("failed to open\n");
        return -1;
    }

    n = write(fd, buf, strlen(buf));
    if(n < 0)
    {
        printf("faile to write\n");
        close(fd);
        return -1;
    }
    printf("write to demo %d bytes\n",n);

    ret = ioctl(fd, DEMO_MEM_GETVAL, &val);
    if(ret < 0)
    {
        printf("faile to ioctl\n");
        close(fd);
        return -1;
    }
    printf("val = %d\n", val);

    ret = ioctl(fd, DEMO_MEM_SETVAL, 100);
    if(ret < 0)
    {
        printf("faile to ioctl\n");
        close(fd);
        return -1;
    }

    ret = ioctl(fd, DEMO_MEM_GETVAL, &val);
    if(ret < 0)
    {
        printf("faile to ioctl\n");
        close(fd);
        return -1;
    }
    printf("val = %d\n", val);

    close(fd);

    return 0;
}
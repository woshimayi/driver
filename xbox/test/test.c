#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

void interrupt_handler(int num);
int fd;

int main(int argc, char **argv)
{
    char buf[2];
    int oflags;
    fd = open("/dev/joystick0", O_RDWR);
    if (fd != -1)
    {
        printf("fd: %d/n", fd);
        signal(SIGIO, interrupt_handler);
        fcntl(fd, F_SETOWN, getpid());
        oflags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, oflags | FASYNC);
        while (1) {};
    }
    else
        printf("Could not open device!\n");
    return 0;
}


void interrupt_handler(int num)
{
    char data[8];
    read(fd, data, 8);
    printf("%02X %02X %02X %02X %02X %02X %02X %02X/n",
           (char)data[0], (char)data[1], (char)data[2], (char)data[3],
           (char)data[4], (char)data[5], (char)data[6], (char)data[7]);
}



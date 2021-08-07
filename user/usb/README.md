## Ubuntu下重置USB端口（即断电后重新上电）

1.把下面的代码保存为usbreset.c文件，保存在home路径下

```c
/*重启usb硬件端口*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>

int main(int argc, char **argv)
{
    const char *filename;
    int fd;
    int rc;

    if (argc != 2) {
        fprintf(stderr, "Usage: usbreset device-filename\n");
        return 1;
    }
    filename = argv[1];//表示usb的ID

    fd = open(filename, O_WRONLY);
    if (fd < 0) {
        perror("Error opening output file");
        return 1;
    }

    printf("Resetting USB device %s\n", filename);
    rc = ioctl(fd, USBDEVFS_RESET, 0);//ioctl是设备驱动中，对I/O设备进行管理的函数
    if (rc < 0) {
        perror("Error in ioctl");
        return 1;
    }
    printf("Reset successful\n");

    close(fd);
    return 0;
}
```

2.在home路径下打开终端，编译上面的文件，如下输入命令：

>$ cc usbreset.c -o usbreset

3.在命令行输入命令，获取你想要的重置的BUS（总线）和Device（设备）的ID，输入如下命令：

> $ lsusb  

下面是我获取的ID号：

>Bus 002 Device 003: ID 0fe9:9010 DVICO 

4.使编译后的程序可执行，输入如下命令：

> $ chmod +x usbreset

5.在终端打开程序，并输入通过上面的lsusb命令获得的ID号，输入如下命令：

>  $ sudo ./usbreset /dev/bus/usb/002/003 

我在这个usb口连接的鼠标，当我执行第5步的时候，可以看到鼠标的红灯灭了一下，然后重新亮起。

在这顺便说一下，上述代码中main函数的两个输入参数(int argc, char **argv)的意义如下：
第一个参数argc是int型的，它用来存放命令行参数的个数。实际上argc所存放的数值比命令行参数的个数多1，这是因为系统默认将命令字(可执行文件名)作为第一个参数，存放在argv[0]的位置处。
第二个参数argv是一个一维的一级指针数组，它是用来存放命令行中各个参数和命令字的字符串的，并且规定：
argv[0]存放命令字，也就是可执行的文件名（包括文件的完整路径）
argv[1]存放命令行中第一个参数
argv[2]存放命令行中第二个参数
······
在上面的例子中，“./usbreset” 就存放在argv[0]， “/dev/bus/usb/002/003 ”就存放在argv[1]，而argc就等于2
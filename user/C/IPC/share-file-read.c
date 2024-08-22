/*
 * @*************************************:
 * @FilePath     : /user/C/IPC/share-memory-read.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-15 10:02:33
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-15 17:39:14
 * @Descripttion :  读进程  文件型共享内存
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "shm.test"

int main()
{
    /* 创建共享对象,可以查看/dev/shm目录 */
    int fd = shm_open(FILENAME, O_RDONLY, 0);
    if (fd == -1)
    {
        perror("open failed:");
        exit(1);
    }

    /* 获取属性 */
    struct stat buf;
    if (fstat(fd, &buf) == -1)
    {
        perror("fstat failed:");
        exit(1);
    }
    printf("the shm object size is %ld\n", buf.st_size);

    /* 建立映射关系 */
    char *ptr = (char *)mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap failed:");
        exit(1);
    }
    printf("mmap %s success\n", FILENAME);
    close(fd); /* 关闭套接字 */

    printf("the read msg is:%s\n", ptr);

    sleep(30);

    return 0;
}
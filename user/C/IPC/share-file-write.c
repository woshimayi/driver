/*
 * @*************************************:
 * @FilePath     : /user/C/IPC/share-memory-write.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-15 10:01:12
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-15 17:39:05
 * @Descripttion :  写进程   文件型共享内存
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

#define MAXSIZE 1024 * 4 /*共享内存的大小，建议设置成内存页的整数倍*/
#define FILENAME "shm.test"

int main()
{
    /* 创建共享对象,可以查看/dev/shm目录 */
    int fd = shm_open(FILENAME, O_CREAT | O_TRUNC | O_RDWR, 0777);
    if (fd == -1)
    {
        perror("open failed:");
        exit(1);
    }

    /* 调整大小 */
    if (ftruncate(fd, MAXSIZE) == -1)
    {
        perror("ftruncate failed:");
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
    char *ptr = (char *)mmap(NULL, MAXSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap failed:");
        exit(1);
    }
    printf("mmap %s success\n", FILENAME);
    close(fd); /* 关闭套接字 */

    /* 写入数据 */
    char *content = "hello world";
    strncpy(ptr, content, strlen(content));

    sleep(30);

    return 0;
}

/*
 * @*************************************:
 * @FilePath     : /user/C/IPC/share-memory_create.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-15 09:57:59
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-15 09:59:05
 * @Descripttion :  创建修改输出删除共享内存区
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

#define MAXSIZE 1024 * 1024 * 16 /*共享内存的大小，建议设置成内存页的整数倍*/
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

    sleep(30);

    /* 如果引用计数为0，系统释放内存对象 */
    if (shm_unlink(FILENAME) == -1)
    {
        perror("shm_unlink failed:");
        exit(1);
    }
    printf("shm_unlink %s success\n", FILENAME);

    return 0;
}

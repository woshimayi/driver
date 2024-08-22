/*
 * @*************************************:
 * @FilePath     : /user/C/ipc_mmap/mmap_server.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-15 17:46:47
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-15 18:21:18
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct
{
    char name[4];
    int age;
} people;

int main(int argc, char **argv) // map a normal file as shared mem:
{
    int fd, i;
    people *p_map;
    fd = open(argv[1], O_CREAT | O_RDWR, 00777);

    while (1)
    {
        p_map = (people *)mmap(NULL, sizeof(people) * 10, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        printf("p_map = %s\n", p_map);
        sleep(1);
    }

    // for (i = 0; i < 10; i++)
    // {
    //     printf("name: %s age %d;\n", (*(p_map + i)).name, (*(p_map + i)).age);
    // }
    munmap(p_map, sizeof(people) * 10);

    return 0;
}
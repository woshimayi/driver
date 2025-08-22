/*
 * @*************************************: 
 * @FilePath     : /user/C/list_test1.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-08-23 17:08:36
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-12 16:21:14
 * @Descripttion : 链表
 * @compile      :  
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/epoll.h>

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

struct link
{
    struct link *next;
};

void link_init(struct link *link)
{
    link->next = NULL;
}

/**
 * @brief
 *
 * @param link
 * @param it
 */
void link_add(struct link *link, struct link *it)
{
    it->next = link->next;
    link->next = it;
}

struct link *link_del(struct link *link)
{
    if (!link->next)
        return NULL;
    struct link *it = link->next;
    link->next = it->next;
    return it;
}


struct link * showall(struct link *it)
{
    while (it && it->next)
    {
        PP("");
    }
};


int main(int argc, char const *argv[])
{
    /* code */
    struct link *it = (struct link *)malloc(sizeof(struct link));
    link_init(it);

    struct link *it1, it2_tmp;
    it1 = &it2_tmp;
    link_add(it1);

    // #define TW_BITS (8)
    // #define TW_SIZE (1 << TW_BITS) // 单级时间轮大小（槽位数量）
    // #define TW_MASK (TW_SIZE - 1)
    // #define TW_LEVEL (3) // 时间轮层级数


    // #define IDX0(data) data & TW_MASK;
    // #define IDX1(data) (data >> TW_BITS) & TW_MASK;
    // #define IDX2(data) (data >> (2 * TW_BITS)) & TW_MASK;

    // int i = 0;
    // while (1)
    // {
    //     i++;
    //     uint8_t idx0 = IDX0(i);
    //     uint8_t idx1 = IDX1(i);
    //     uint8_t idx2 = IDX2(i);

    //     PP("idx0 = %d idx1 = %d idx2 = %d",  idx0, idx1, idx2);
    //     sleep(1);
    // }

    return 0;
}

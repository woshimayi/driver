/*
 * @*************************************:
 * @FilePath     : /user/C/list/main.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-12-04 19:21:41
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-14 14:11:11
 * @Descripttion :  kernel 链表风格 测试代码
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dof_os_list.h"

#define PW(fmt, args...) printf("\033[0;33;32m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)
#define PE(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)
#define PP(fmt, args...) printf("\033[0;32;33m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

typedef struct
{
    dof_list_head st_list;
    uword32 ui_status;
} dof_regstatus_node_s;

static uword32 g_ui_current_regstatus = 0;

/**
 * @brief 添加node 在第一个
 *
 * @param p_list 链表头指针
 * @param ui_status 状态值
 */
static void __regstatus_node_add_head(dof_list_head *p_list, uword32 ui_status)
{
    dof_regstatus_node_s *p_node = NULL;
    p_node = (dof_regstatus_node_s *)malloc(sizeof(dof_regstatus_node_s));
    if (NULL == p_node)
    {
        return;
    }
    p_node->ui_status = ui_status;
    PP("regstate [%u]\r\n", ui_status);
    g_ui_current_regstatus = ui_status;
    dof_list_add(&p_node->st_list, p_list);
    return;
}

/**
 * @brief 添加node 在前面
 *
 * @param p_list 链表头指针
 * @param ui_status 状态值
 */
static void __regstatus_node_add(dof_list_head *p_list, uword32 ui_status)
{
    dof_regstatus_node_s *p_node = NULL;
    p_node = (dof_regstatus_node_s *)malloc(sizeof(dof_regstatus_node_s));
    if (NULL == p_node)
    {
        return;
    }
    p_node->ui_status = ui_status;
    PP("regstate [%u]\r\n", ui_status);
    g_ui_current_regstatus = ui_status;
    dof_list_add_tail(&p_node->st_list, p_list);
    return;
}

/**
 * @brief  插入node 到循环处后面
 *
 * @param p_node 目标节点
 * @param ui_status 状态值
 */
static void __regstatus_node_insert_head(dof_regstatus_node_s *p_node, uword32 ui_status)
{
    dof_regstatus_node_s *newp_node = NULL;
    newp_node = (dof_regstatus_node_s *)malloc(sizeof(dof_regstatus_node_s));
    if (NULL == newp_node)
    {
        return;
    }
    newp_node->ui_status = ui_status;
    dof_list_add(&newp_node->st_list, &p_node->st_list);
}

/**
 * @brief 插入node后面
 *
 * @param p_node 目标节点
 * @param ui_status 状态值
 */
static void __regstatus_node_insert(dof_regstatus_node_s *p_node, uword32 ui_status)
{
    dof_regstatus_node_s *newp_node = NULL;
    newp_node = (dof_regstatus_node_s *)malloc(sizeof(dof_regstatus_node_s));
    if (NULL == newp_node)
    {
        return;
    }
    newp_node->ui_status = ui_status;
    dof_list_add_tail(&newp_node->st_list, &p_node->st_list);
}

/**
 * @brief  删除此处的node
 *
 * @param p_node 要删除的节点
 */
static void __regstatus_node_del(dof_regstatus_node_s *p_node)
{
    if (NULL == p_node)
    {
        return;
    }
    dof_list_del(&p_node->st_list);
    free(p_node);
    p_node = NULL;
}

/**
 * @brief 获取node 处的值
 *
 * @param p_list 链表头指针
 * @param pui_status 输出状态值
 * @return word32
 */
static word32 __regstatus_node_get(dof_list_head *p_list, uword32 *pui_status)
{
    dof_regstatus_node_s *p_node = NULL;

    dof_list_for_each_entry(p_node, p_list, st_list)
    {
        *pui_status = p_node->ui_status;
        return 0;
    }
    return -1;
}

/**
 * @brief 删除第一个
 *
 * @param p_list 链表头指针
 */
static void __regstatus_node_delFisrst(dof_list_head *p_list)
{
    uword32 ui_index = 0;
    dof_regstatus_node_s *p_node = NULL, *p_tmp_node = NULL;

    dof_list_for_each_entry_safe(p_node, p_tmp_node, p_list, st_list)
    {
        ui_index++;
    }

    if (ui_index > 1)
    {
        dof_list_for_each_entry_safe(p_node, p_tmp_node, p_list, st_list)
        {
            dof_list_del(&p_node->st_list);
            free(p_node);
            p_node = NULL;
            return;
        }
    }
    return;
}

/**
 * @brief 删除最后一个node
 *
 * @param p_list 链表头指针
 */
static void __regstatus_node_delLast(dof_list_head *p_list)
{
    dof_regstatus_node_s *p_node = NULL, *p_tmp_node = NULL;
    dof_regstatus_node_s *last_node = NULL;

    dof_list_for_each_entry_safe(p_node, p_tmp_node, p_list, st_list)
    {
        last_node = p_node;
    }

    if (last_node != NULL)
    {
        dof_list_del(&last_node->st_list);
        free(last_node);
        last_node = NULL;
    }
    return;
}

/**
 * @brief 删除全部
 *
 * @param p_list 链表头指针
 */
static void __regstatus_node_clear(dof_list_head *p_list)
{
    dof_regstatus_node_s *p_node = NULL, *p_tmp_node = NULL;

    dof_list_for_each_entry_safe(p_node, p_tmp_node, p_list, st_list)
    {
        dof_list_del(&p_node->st_list);
        free(p_node);
        p_node = NULL;
    }
    g_ui_current_regstatus = 0;
    return;
}

/**
 * @brief 打印所有的值
 *
 * @param p_list 链表头指针
 */
static void __regstatus_node_dump(dof_list_head *p_list)
{
    dof_regstatus_node_s *p_node = NULL, *p_tmp_node = NULL;

    dof_list_for_each_entry_safe(p_node, p_tmp_node, p_list, st_list)
    {
        PP("%d", p_node->ui_status);
    }
    return;
}

/**
 * @brief 查找指定状态的节点
 *
 * @param p_list 链表头指针
 * @param status 要查找的状态值
 * @return dof_regstatus_node_s* 找到的节点指针，未找到返回NULL
 */
dof_regstatus_node_s *__regstatus_node_find_status(dof_list_head *p_list, int status)
{
    dof_regstatus_node_s *p_node = NULL, *p_tmp_node = NULL;
    dof_regstatus_node_s *found_node = NULL;

    dof_list_for_each_entry_safe(p_node, p_tmp_node, p_list, st_list)
    {
        if (status == p_node->ui_status)
        {
            found_node = p_node;
            break; // 找到后立即退出
        }
    }

    return found_node;
}

int main(int argc, char const *argv[])
{
    PW("sss");

    // 将全局变量改为局部变量
    dof_list_head regstatus_tbl;
    dof_list_init_head(&regstatus_tbl);

    for (int i = 0; i < 10; i++)
    {
        PP("%d", i);
        __regstatus_node_add(&regstatus_tbl, i);
    }
    __regstatus_node_add_head(&regstatus_tbl, 152);
    __regstatus_node_add(&regstatus_tbl, 156);

    dof_regstatus_node_s *p_node = NULL, *n = NULL;
    dof_list_for_each_entry_safe(p_node, n, &regstatus_tbl, st_list)
    {
        PP("%p | %p | %p | %p", p_node, n, p_node->st_list, n->st_list);
        if (p_node && 6 == p_node->ui_status)
        {
            __regstatus_node_insert(p_node, 56);
            __regstatus_node_insert_head(p_node, 65);
            // __regstatus_node_del(p_node);
            p_node->ui_status = 555;
        }
    }
    __regstatus_node_dump(&regstatus_tbl);
    __regstatus_node_delFisrst(&regstatus_tbl);
    // __regstatus_node_clear(&regstatus_tbl);
    __regstatus_node_delLast(&regstatus_tbl);

    PP("\n");

    __regstatus_node_dump(&regstatus_tbl);

    return 0;
}



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dof_os_list.h"

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

typedef struct
{
    dof_list_head st_list;
    uword32 ui_status;
} dof_regstatus_node_s;

static dof_list_head g_regstatus_tbl;
static uword32 g_ui_current_regstatus = 0;

static void __regstatus_node_add_head(uword32 ui_status)
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
    dof_list_add(&p_node->st_list, &g_regstatus_tbl);
    return;
}

static void __regstatus_node_add(uword32 ui_status)
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
    dof_list_add_tail(&p_node->st_list, &g_regstatus_tbl);
    return;
}

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

static word32 __regstatus_node_get(uword32 *pui_status)
{
    dof_regstatus_node_s *p_node = NULL;

    dof_list_for_each_entry(p_node, &g_regstatus_tbl, st_list)
    {
        *pui_status = p_node->ui_status;
        return 0;
    }
    return -1;
}
static void __regstatus_node_alldel()
{
    uword32 ui_index = 0;
    dof_regstatus_node_s *p_node = NULL, *p_tmp_node = NULL;

    dof_list_for_each_entry_safe(p_node, p_tmp_node, &g_regstatus_tbl, st_list)
    {
        ui_index++;
    }

    if (ui_index > 1)
    {
        dof_list_for_each_entry_safe(p_node, p_tmp_node, &g_regstatus_tbl, st_list)
        {
            dof_list_del(&p_node->st_list);
            free(p_node);
            p_node = NULL;
            return;
        }
    }
    return;
}

static void __regstatus_node_clear()
{
    dof_regstatus_node_s *p_node = NULL, *p_tmp_node = NULL;

    dof_list_for_each_entry_safe(p_node, p_tmp_node, &g_regstatus_tbl, st_list)
    {
        dof_list_del(&p_node->st_list);
        free(p_node);
        p_node = NULL;
    }
    g_ui_current_regstatus = 0;
    return;
}

int main(int argc, char const *argv[])
{
    dof_list_init_head(&g_regstatus_tbl);
    for (int i = 0; i < 10; i++)
    {
        PP("%d", i);
        __regstatus_node_add(i);
    }
    __regstatus_node_add_head(152);
    __regstatus_node_add(156);

    dof_regstatus_node_s *p_node = NULL, *n = NULL;
    dof_list_for_each_entry_safe(p_node, n, &g_regstatus_tbl, st_list)
    {
        PP("%p | %p", p_node, n);
        if (p_node && 6 == p_node->ui_status)
        {
            __regstatus_node_insert(p_node, 56);
            __regstatus_node_insert_head(p_node, 56);
            __regstatus_node_del(p_node);
        }
    }

    PP("\n");

    p_node = NULL, n = NULL;
    dof_list_for_each_entry_safe(p_node, n, &g_regstatus_tbl, st_list)
    {
        PP("%d", p_node->ui_status);
    }

    return 0;
}

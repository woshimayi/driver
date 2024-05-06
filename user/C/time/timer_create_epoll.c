/*
 * @*************************************: 
 * @FilePath: /user/C/time/timer_create_epoll.c
 * @version: 
 * @Author: dof
 * @Date: 2024-04-09 10:43:35
 * @LastEditors: dof
 * @LastEditTime: 2024-04-09 10:45:02
 * @Descripttion: 
 * @**************************************: 
 */



#include <glib.h>

void timer_cb(gpointer data) {
    // 定时器处理函数
}

int main() {
    // 创建定时器
    guint timer_id = g_timeout_add(1000, timer_cb, NULL);

    // 等待超时
    while (1) {
        // ...
    }

    return 0;
}

/*
 * @*************************************:
 * @FilePath     : /user/C/tiem_epoll.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-21 11:17:19
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-21 11:22:36
 * @Descripttion :  epoll 实现定时器
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#define MAX_EVENTS 10

/**
 * @brief Get the time object
 * 
 * @param buffer 
 */
void get_time(char *buffer)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    strftime(buffer, 40, "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    sprintf(buffer + strlen(buffer), ".%03ld", tv.tv_usec / 1000);
}

/**
 * @brief 回调函数输出
 * 
 * @param timerfd 
 */
void timer_handler(int timerfd)
{
    uint64_t num_exp;
    read(timerfd, &num_exp, sizeof(num_exp));
    char buf[40] = {0};
    get_time(buf);
    printf("%s Timer expired!\n", buf);
    // 执行定时任务
}

int main()
{
    int epfd, timerfd;
    struct epoll_event ev, events[MAX_EVENTS];

    // 创建 epoll 实例
    epfd = epoll_create1(0);
    if (epfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // 创建定时器文件描述符
    timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd == -1)
    {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }

    // 将定时器文件描述符添加到 epoll 中
    ev.events = EPOLLIN;
    ev.data.fd = timerfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, timerfd, &ev) == -1)
    {
        perror("epoll_ctl: add");
        exit(EXIT_FAILURE);
    }

    // 设置定时器
    struct itimerspec its;
    its.it_value.tv_sec = 5;
    its.it_interval.tv_sec = 2;
    if (timerfd_settime(timerfd, 0, &its, NULL) == -1)
    {
        perror("timerfd_settime");
        exit(EXIT_FAILURE);
    }

    // 等待事件
    while (1)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == timerfd)
            {
                timer_handler(timerfd);
            }
        }
    }

    // 关闭文件描述符
    close(timerfd);
    close(epfd);

    return 0;
}
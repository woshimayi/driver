/*
 * @*************************************:
 * @FilePath     : /user/C/epoll_timer.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-27 17:16:24
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-27 19:44:24
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

#define MAX_EVENTS 10

int main()
{
    int timerfd, epollfd;
    struct itimerspec its;
    struct epoll_event ev, events[MAX_EVENTS];

    // 创建 timerfd
    timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd == -1)
    {
        perror("timerfd_create");
        return 1;
    }

    // 设置定时器属性
    its.it_interval.tv_sec = 1; // 每秒触发一次
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    if (timerfd_settime(timerfd, 0, &its, NULL) == -1)
    {
        perror("timerfd_settime");
        return 1;
    }

    // 创建 epoll 实例
    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        return 1;
    }

    // 将 timerfd 添加到 epoll 实例中
    ev.events = EPOLLIN;
    ev.data.fd = timerfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &ev) == -1)
    {
        perror("epoll_ctl: timerfd");
        return 1;
    }

    // 等待事件
    while (1)
    {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        PP("nfds = %d", nfds);
        if (nfds == -1)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == timerfd)
            {
                // 处理定时器事件
                PP("timer expired");
                uint64_t exp;
                read(timerfd, &exp, sizeof(exp));
                PP("timerfd = %d\n", timerfd);
            }
        }
        
        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, timerfd, &ev) == -1)
        {
            perror("epoll_ctl: timerfd");
            return 1;
        }

        ev.events = EPOLLIN;
        ev.data.fd = timerfd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &ev) == -1)
        {
            perror("epoll_ctl: timerfd");
            return 1;
        }
    }
    PP();

    close(timerfd);
    close(epollfd);
    return 0;
}
/*
 * @*************************************:
 * @FilePath: /user/C/timer_while_task.c
 * @version:
 * @Author: dof
 * @Date: 2024-08-07 14:02:56
 * @LastEditors: dof
 * @LastEditTime: 2024-08-08 09:38:02
 * @Descripttion: 时间轮 轮训 多个定时任务
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

#define PP_endl(fmt, args...)                                                               \
    do                                                                                      \
    {                                                                                       \
        printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m", __func__, __LINE__, ##args); \
        fflush(stdout);                                                                     \
    } while (0)

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)
#define PP_G(fmt, args...) printf("\033[0;32;32m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

#define TW_BITS (8)
#define TW_SIZE (1 << TW_BITS) // 单级时间轮大小（槽位数量）
#define TW_MASK (TW_SIZE - 1)
#define TW_LEVEL (3) // 时间轮层级数

#define IDX0(data) data & TW_MASK;
#define IDX1(data) (data >> TW_BITS) & TW_MASK;
#define IDX2(data) (data >> (2 * TW_BITS)) & TW_MASK;

#define ONESHOT_FLAG 0x1 // 一次性任务
#define PERIOD_FLAG 0x2  // 周期性任务

#define TICK_MS (1000) // 单次延时时间，单位毫秒
#define MS_TO_TICKS(ms, tick_ms) (ms / tick_ms)

#define TW_DRIVER_TYPE (1) // 时间驱动器类型
#define SLEEP_DRIVER (0)   // sleep时间驱动器
#define EPOLL_DRIVER (1)   // epoll时间驱动器

#define MAX_EVENTS (10)

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

typedef void (*func)(void *arg);
struct tw_node
{
    struct link link;     // 链表
    int32_t expire_tick;  // 到期时间
    int32_t period_ticks; // 周期时间
    int flags;
    func cb;
    void *arg;
    bool need_free;
};

/**
 * @brief 这个是总时间轮链表汇总
 *
 */
struct tw
{
    uint32_t tick_ms;                     // 循环定时器时长
    uint32_t cur_tick;                    // 当前时间
    struct link slots[TW_LEVEL][TW_SIZE]; // 3层 8级 时间轮槽数
    pthread_spinlock_t lock;
};

/**
 * @brief
 *
 * @param tw
 * @param expire_ms
 * @param period_ms
 * @param flags
 * @param cb
 * @param arg
 * @param need_free
 * @return struct tw_node*
 */
struct tw_node *tw_node_new(struct tw *tw, int expire_ms, int period_ms, int flags, func cb, void *arg, bool need_free)
{
    if ((expire_ms < TICK_MS) || (period_ms < TICK_MS))
    {
        return NULL;
    }

    struct tw_node *node = (struct tw_node *)malloc(sizeof(struct tw_node));
    if (!node)
        return NULL;
    memset(node, 0, sizeof(struct tw_node));
    node->expire_tick = MS_TO_TICKS(expire_ms, tw->tick_ms);
    node->period_ticks = MS_TO_TICKS(period_ms, tw->tick_ms);
    PP_G("tw node new expire tick:%u,%u, peroid ticks:%u,%u\n", expire_ms, node->expire_tick, period_ms, node->period_ticks);
    node->flags = flags;
    node->cb = cb;
    node->arg = arg;
    node->need_free = need_free;
    return node;
}

/**
 * @brief
 *
 * @param node
 */
void tw_node_free(struct tw_node *node)
{
    if (node->arg && node->need_free)
    {
        uint32_t *task_id = (uint32_t *)node->arg;
        PP("free task id:%u node\n", *task_id);
        free(node->arg);
    }
    free(node);
}

/**
 * @brief
 *
 * @param tw
 * @param tick_ms
 */
void tw_init(struct tw *tw, uint32_t tick_ms)
{
    memset(tw, 0, sizeof(struct tw));
    tw->tick_ms = tick_ms;
    pthread_spin_init(&tw->lock, PTHREAD_PROCESS_PRIVATE);
}

/**
 * @brief
 *
 * @param tw
 */
void tw_free(struct tw *tw)
{

    pthread_spin_lock(&tw->lock);
    for (uint32_t i = 0; i < TW_LEVEL; i++)
    {
        for (uint32_t j = 0; j < TW_SIZE; j++)
        {
            struct link *link = &tw->slots[i][j];
            struct link *it;
            while (it = link_del(link))
            {
                PP("free i:%u, j:%u\n", i, j);
                struct tw_node *node = (struct tw_node *)it;
                tw_node_free(node);
            }
        }
    }

    pthread_spin_unlock(&tw->lock);
    pthread_spin_destroy(&tw->lock);
}

/**
 * @brief
 *
 * @param tw
 * @param node
 * @param nolock
 */
void tw_add(struct tw *tw, struct tw_node *node, bool nolock)
{

    if (!nolock)
        pthread_spin_lock(&tw->lock);

    uint32_t expire_tick = node->expire_tick;
    node->expire_tick += tw->cur_tick;

#if 0
    PP("tw add cur tick:%u, period ticks:%u, old expire tick:%u, node expire tick:%u\n",
            tw->cur_tick,
            node->period_ticks,
            expire_tick,
            node->expire_tick);
#endif

    uint8_t idx0 = IDX0(tw->cur_tick);
    uint8_t idx1 = IDX1(tw->cur_tick);
    uint8_t idx2 = IDX2(tw->cur_tick);

    uint8_t e0 = IDX0(node->expire_tick);
    uint8_t e1 = IDX1(node->expire_tick);
    uint8_t e2 = IDX2(node->expire_tick);

    PP_G("tw add idx0,idx1,idx2:%u,%u,%u\n", idx0, idx1, idx2);
    PP_G("tw add e0,  e1,  e2:  %u,%u,%u\n", e0, e1, e2);

    struct link *it = &node->link;
    if (e2 != idx2)
    {
        struct link *link = &tw->slots[2][e2];
        PP_G("tw link add 2,e2:%u\n", e2);
        link_add(link, it);
    }
    else if (e1 != idx1)
    {
        struct link *link = &tw->slots[1][e1];
        PP_G("tw link add 1,e1:%u\n", e1);
        link_add(link, it);
    }
    else if (e0 != idx0)
    {
        struct link *link = &tw->slots[0][e0];
        PP_G("tw link add 0,e0:%u\n", e0);
        link_add(link, it);
    }

    if (!nolock)
        pthread_spin_unlock(&tw->lock);
}

/**
 * @brief
 *
 * @param tw
 * @return int
 */
int tw_update(struct tw *tw)
{
    pthread_spin_lock(&tw->lock);
    tw->cur_tick++;

    uint8_t idx0 = IDX0(tw->cur_tick);
    uint8_t idx1 = IDX1(tw->cur_tick);
    uint8_t idx2 = IDX2(tw->cur_tick);
    PP("tw add idx0,idx1,idx2:%u,%u,%u\n", idx0, idx1, idx2);

    struct link active = {0};
    if ((idx0 == 0) && (idx1 == 0) && (idx2 > 0))
    {
        struct link *link = &tw->slots[2][idx2];
        struct link *it;
        while (it = link_del(link))
        {
            PP("tw update cur tick:%u, idx0:%u,idx1:%u,idx2:%u\n", tw->cur_tick, idx0, idx1, idx2);
            struct tw_node *node = (struct tw_node *)it;
            uint8_t e0 = IDX0(node->expire_tick);
            uint8_t e1 = IDX1(node->expire_tick);
            if ((e0 == 0) && (e1 == 0))
            {
                link_add(&active, it);
            }
            else if (e1 > 0)
            {
                struct link *l = &tw->slots[1][e1];
                link_add(l, it);
            }
            else
            {
                struct link *l = &tw->slots[0][e0];
                link_add(l, it);
            }
        }
    }
    else if ((idx0 == 0) && (idx1 > 0))
    {
        struct link *link = &tw->slots[1][idx1];
        struct link *it;
        while (it = link_del(link))
        {
            PP("tw update cur tick:%u, idx0:%u,idx1:%u,idx2:%u\n", tw->cur_tick, idx0, idx1, idx2);
            struct tw_node *node = (struct tw_node *)it;
            uint8_t e0 = IDX0(node->expire_tick);
            if (e0 == 0)
            {
                link_add(&active, it);
            }
            else
            {
                struct link *l = &tw->slots[0][e0];
                link_add(l, it);
            }
        }
    }
    else if (idx0 > 0)
    {
        struct link *link = &tw->slots[0][idx0];
        struct link *it;
        while (it = link_del(link))
        {
            PP("tw update cur tick:%u, idx0:%u,idx1:%u,idx2:%u\n", tw->cur_tick, idx0, idx1, idx2);
            link_add(&active, it);
        }
    }

    struct link *it;
    while (it = link_del(&active))
    {
        struct tw_node *node = (struct tw_node *)it;
        PP("tw update callback cur tick:%u@@\n", tw->cur_tick);
        node->cb(node->arg);
        if (node->flags & PERIOD_FLAG)
        {
            node->expire_tick = node->period_ticks;
            tw_add(tw, node, true);
        }
        else
        {
            tw_node_free(node);
        }
    }
    pthread_spin_unlock(&tw->lock);

    return 0;
}

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
 * @brief 定时器驱动模型
 *
 * @param arg
 * @return void*
 */
void *tw_driver_thread(void *arg)
{
    struct tw *tw = (struct tw *)arg;
    char i = 0;

#if (TW_DRIVER_TYPE == EPOLL_DRIVER)
    int efd = epoll_create(1024);
    if (efd == -1)
    {
        perror("epoll create error");
        return NULL;
    }
    struct epoll_event ev, events[MAX_EVENTS];
#endif

    while (1)
    {
#if (TW_DRIVER_TYPE == SLEEP_DRIVER)
        usleep(TICK_MS * 1000);
        tw_update(tw);
        PP("sleep driver\n");
#else
        int nfds = epoll_wait(efd, events, MAX_EVENTS, TICK_MS);
        PP_endl("epoll driver nfds:%d|%d\r", nfds, i++);
        if (nfds == -1)
        {
            perror("epoll wait error");
            continue;
        }

        tw_update(tw);
#endif
    }
}

struct skill
{
    char skill_name[20]; // 技能名称
    char ch;             // 快捷键
    int cool_time;       //
    int duration_time;
    bool ready;
};

struct hero
{
    char hero_name[20];
    struct skill Q;
    struct skill W;
    struct skill E;
    struct skill R;
};

/**
 * @brief
 *
 * @param arg
 */
void skill_task(void *arg)
{
    struct skill *sk = (struct skill *)arg;
    sk->ready = true;
    char buf[40] = {0};
    get_time(buf);
    PP_G("%s %s 完成冷却!\n", buf, sk->skill_name);
}

/**
 * @brief 技能表
 *
 * @param arg
 * @return void*
 */
void *task_scheduler_thread(void *arg)
{
    struct tw *tw = (struct tw *)arg;
    struct hero yi = (struct hero){
        .hero_name = "易大师",
        .Q = {.skill_name = "Q技能", .ch = 'q', .cool_time = 14, .duration_time = 1, .ready = true},
        .W = {.skill_name = "W技能", .ch = 'w', .cool_time = 35, .duration_time = 5, .ready = true},
        .E = {.skill_name = "E技能", .ch = 'e', .cool_time = 14, .duration_time = 5, .ready = true},
        .R = {.skill_name = "R技能", .ch = 'r', .cool_time = 75, .duration_time = 10, .ready = true},
    };

    while (1)
    {
        int ch = getchar();
        if (ch == '\n')
            continue;

        PP_G();
        switch (ch)
        {
        case 'q':
            if (yi.Q.ready == false)
            {
                PP_G("%s %s 冷却中......\n", yi.hero_name, yi.Q.skill_name);
            }
            else
            {
                char buf[40] = {0};
                get_time(buf);
                PP_G("%s %s 施放 %s QQQQQQQQQQQQQQQ\n", buf, yi.hero_name, yi.Q.skill_name);
                yi.Q.ready = false;
                struct tw_node *node = tw_node_new(tw, yi.Q.cool_time * 1000, 1000, ONESHOT_FLAG, skill_task, (void *)&yi.Q, false);
                if (!node)
                {
                    PP_G("tw node new error\n");
                    break;
                }
                tw_add(tw, node, false);
            }
            break;
        case 'w':
            if (yi.W.ready == false)
            {
                PP_G("%s %s 冷却中......\n", yi.hero_name, yi.W.skill_name);
            }
            else
            {
                char buf[40] = {0};
                get_time(buf);
                PP_G("%s %s 施放 %s WWWWWWWWWWWWWW\n", buf, yi.hero_name, yi.W.skill_name);
                yi.W.ready = false;
                struct tw_node *node = tw_node_new(tw, yi.W.cool_time * 1000, 1000, ONESHOT_FLAG, skill_task, (void *)&yi.W, false);
                if (!node)
                {
                    PP_G("tw node new error\n");
                    break;
                }
                tw_add(tw, node, false);
            }
            break;
        case 'e':
            if (yi.E.ready == false)
            {
                PP_G("%s %s 冷却中......\n", yi.hero_name, yi.E.skill_name);
            }
            else
            {
                char buf[40] = {0};
                get_time(buf);
                yi.E.ready = false;
                PP_G("%s %s 施放 %s EEEEEEEEEEEEEE\n", buf, yi.hero_name, yi.E.skill_name);
                struct tw_node *node = tw_node_new(tw, yi.E.cool_time * 1000, 1000, ONESHOT_FLAG, skill_task, (void *)&yi.E, false);
                if (!node)
                {
                    PP_G("tw node new error\n");
                    break;
                }
                tw_add(tw, node, false);
            }
            break;
        case 'r':
            if (yi.R.ready == false)
            {
                PP_G("%s %s 冷却中......\n", yi.hero_name, yi.R.skill_name);
            }
            else
            {
                char buf[40] = {0};
                get_time(buf);
                PP_G("%s %s 施放 %s RRRRRRRRRRRRRRR\n", buf, yi.hero_name, yi.R.skill_name);
                yi.R.ready = false;
                struct tw_node *node = tw_node_new(tw, yi.R.cool_time * 1000, 1000, ONESHOT_FLAG, skill_task, (void *)&yi.R, false);
                if (!node)
                {
                    PP_G("tw node new error\n");
                    break;
                }
                tw_add(tw, node, false);
            }
            break;
        default:
            PP_G("xxx无效技能:%c\n", ch);
            break;
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{

    struct tw tw;
    PP("start timewheel\n");
    tw_init(&tw, TICK_MS);

    PP("loop timewheel stask start\n");

    pthread_t th1;
    pthread_create(&th1, NULL, task_scheduler_thread, (void *)&tw);

    pthread_t th2;
    pthread_create(&th2, NULL, tw_driver_thread, (void *)&tw);

    PP("loop timewheel\n");
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    PP("free timewheel\n");

    tw_free(&tw);

    return 0;
}
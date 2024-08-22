/*
 * @*************************************:
 * @FilePath     : /user/C/nolock_queue.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-07 17:54:12
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-21 10:51:59
 * @Descripttion : 无锁队列
 * @compile      :
 * @**************************************:
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#ifndef PP
#define PP(fmt, args...) printf("\033[0;33;31m[zzzzz :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)
#endif

#define PROD_THREAD_NUM (1)         // 生产者线程数量
#define CONS_THREAD_NUM (1)         // 消费者线程数量
#define TOTAL_PACKET_NUM (20000000) // 测试入队数据包总数量

#define LOCK_TYPE (1)
#define FREE_LOCK (0)
#define MUTEX_LOCK (1)

struct mini_ring;
struct mini_ring *g_ring;
_Atomic(uint32_t) g_seq;   // 全局数据包序列号，每产生一个新的数据包加1
_Atomic(uint32_t) g_count; // 全局出队数据包序列号，每出队一个数据包加1

int g_done;

pthread_mutex_t g_mutex;
pthread_cond_t g_cond;

struct mini_packet
{
    uint32_t seq; // 数据包序列号
};

struct mini_headtail
{
    _Atomic(uint32_t) head;
    _Atomic(uint32_t) tail;
};

struct mini_ring
{
    struct mini_headtail prod;
    struct mini_headtail cons;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t mask;
};

/**
 * @brief
 *
 * @param ring
 * @param obj_table
 * @param n
 * @return int
 */
int mini_ring_enqueue(struct mini_ring *ring, void *obj_table, uint32_t n)
{
#if (LOCK_TYPE == FREE_LOCK)
    uint32_t cur_head;
    uint32_t next_head;

    int ret = mini_ring_move_prod_head(ring, n, &cur_head, &next_head);
    if (ret)
    {
        // printf("mini ring move prod head error\n");
        return -1;
    }
    mini_ring_enqueue_elems(ring, &cur_head, obj_table, n);
    mini_ring_update_prod_tail(ring, &cur_head, &next_head);
#elif (LOCK_TYPE == MUTEX_LOCK)
    uint32_t cur_head;
    uint32_t next_head;
    uint32_t free_entries;

    pthread_mutex_lock(&g_mutex);
    free_entries = ring->size - (ring->head - ring->tail);
    if (n > free_entries)
    {
        pthread_cond_signal(&g_cond);
        pthread_mutex_unlock(&g_mutex);
        return -1;
    }

    cur_head = ring->head;
    next_head = ring->head + n;
    mini_ring_enqueue_elems(ring, &cur_head, obj_table, n);
    ring->head = next_head;
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
#else

#endif
    return 0;
}

/**
 * @brief
 *
 * @param ring
 * @param obj_table
 * @param n
 * @return int
 */
int mini_ring_dequeue(struct mini_ring *ring, void *obj_table, uint32_t n)
{
#if (LOCK_TYPE == FREE_LOCK)
    uint32_t cur_head;
    uint32_t next_head;

    int ret = mini_ring_move_cons_head(ring, n, &cur_head, &next_head);
    if (ret)
    {
        // printf("mini ring move cons head ret:%d error\n", ret);
        return -1;
    }

    mini_ring_dequeue_elems(ring, &cur_head, obj_table, n);
    mini_ring_update_cons_tail(ring, &cur_head, &next_head);
#elif (LOCK_TYPE == MUTEX_LOCK)
    uint32_t cur_tail;
    uint32_t next_tail;
    uint32_t entries;
    pthread_mutex_lock(&g_mutex);
    entries = ring->head - ring->tail;
#if 0
    if (n > entries) {
        pthread_mutex_unlock(&g_mutex);
        return -1;
    }
#else
    while ((n > entries) && (g_done == 0))
    {
        pthread_cond_wait(&g_cond, &g_mutex);
        entries = ring->head - ring->tail;
    }

    if (g_done != 0)
    {
        pthread_mutex_unlock(&g_mutex);
        return -1;
    }
#endif

    cur_tail = ring->tail;
    next_tail = ring->tail + n;
    mini_ring_dequeue_elems(ring, &cur_tail, obj_table, n);
    ring->tail = next_tail;
    pthread_mutex_unlock(&g_mutex);
#else
#endif
    return 0;
}

/**
 * @brief
 *
 * @param arg
 * @return void*
 */
void *prod_thread(void *arg)
{
    int thread_num = (int)arg;
    uint64_t **obj_table = NULL;
    while (1)
    {
        // for (int i = 0; i < 10000; i++) ;
        struct mini_packet *pkt = mini_packet_alloc();
        if (!pkt)
        {
            printf("入队:%d个数据包，生产者线程:%d 退出!\n", TOTAL_PACKET_NUM, thread_num);
            break;
        }
        // printf("prod thread num:%d, new pkt seq:%u\n", thread_num, pkt->seq);
        obj_table = (uint64_t **)&pkt;
    try_again:
        int ret = mini_ring_enqueue(g_ring, (void *)obj_table, 1);
        if (ret)
        {
            // printf("mini ring enqueue ret:%d error\n", ret);
            goto try_again;
        }
    }

    return NULL;
}

void *cons_thread(void *arg)
{
    int thread_num = (int)arg;
    int ret = 0;
    uint64_t **obj_table = NULL;
    struct mini_packet *pkt = NULL;
    int failed_times = 0;
    uint32_t count = 0;
    while (1)
    {
        count = atomic_load(&g_count);
        if (count > TOTAL_PACKET_NUM)
        {
            printf("出队:%d个数据包，消费者线程:%d 退出!\n", count - 1, thread_num);
            break;
        }
        obj_table = (uint64_t **)&pkt;
        ret = mini_ring_dequeue(g_ring, (void *)obj_table, 1);
        if (ret < 0)
        {
            if (failed_times < 1000)
            {
                failed_times++;
            }
            else
            {
                // printf("dequeue failed times:%d\n", failed_times);
                usleep(10);
                failed_times = 0;
            }
            continue;
        }

#if 0
        if ((pkt->seq % 1000) == 0){
            printf("cons thread num:%d, dequeue pkt seq:%u\n", thread_num, pkt->seq);
        }
#endif
        mini_packet_free(pkt);
        atomic_fetch_add(&g_count, 1);
    }

    g_done++;

    return NULL;
}

int main(int argc, char *argv[])
{
    atomic_init(&g_seq, 0);
    atomic_init(&g_count, 0);
    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);
    g_done = 0;

    int size = 4096;
    g_ring = mini_ring_create(size);

    pthread_t prod_th[PROD_THREAD_NUM];
    pthread_t cons_th[CONS_THREAD_NUM];
    for (int i = 0; i < CONS_THREAD_NUM; i++)
    {
        pthread_create(&cons_th[i], NULL, cons_thread, (void *)i);
    }
    for (int i = 0; i < PROD_THREAD_NUM; i++)
    {
        pthread_create(&prod_th[i], NULL, prod_thread, (void *)i);
    }

    for (int i = 0; i < PROD_THREAD_NUM; i++)
    {
        pthread_join(prod_th[i], NULL);
    }

    while (g_done < CONS_THREAD_NUM)
    {
        pthread_cond_signal(&g_cond);
        usleep(10);
        // printf("g_done:%d\n", g_done);
    }

    // printf("--g_done:%d\n", g_done);
    for (int i = 0; i < CONS_THREAD_NUM; i++)
    {
        pthread_join(cons_th[i], NULL);
    }

    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_cond);

    return 0;
}
/*
 * @*************************************:
 * @FilePath     : /user/C/string/shm_pubsub.c
 * @version      :
 * @Author       : dof
 * @Date         : 2026-01-13 11:29:38
 * @LastEditors  : dof
 * @LastEditTime : 2026-01-13 11:29:44
 * @Descripttion : 共享内存实现发布订阅
 * @compile      :
 * @**************************************:
 */

// shm_pubsub.c - 共享内存实现发布订阅
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define MAX_MESSAGES 10
#define MAX_TOPIC_LEN 32
#define MAX_CONTENT_LEN 256

typedef struct
{
    char topic[MAX_TOPIC_LEN];
    char content[MAX_CONTENT_LEN];
    int valid; // 0=无效, 1=有效
} Message;

typedef struct
{
    Message messages[MAX_MESSAGES];
    int write_index;
    int read_index;
    int message_count;
} SharedMemory;

// 信号量操作
void sem_wait(int semid, int semnum)
{
    struct sembuf op = {semnum, -1, 0};
    semop(semid, &op, 1);
}

void sem_signal(int semid, int semnum)
{
    struct sembuf op = {semnum, 1, 0};
    semop(semid, &op, 1);
}

void publisher()
{
    // 连接到共享内存
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);

    // 连接到信号量
    int semid = semget(SEM_KEY, 2, 0666);

    printf("发布者[%d]启动\n", getpid());

    for (int i = 0; i < 5; i++)
    {
        sem_wait(semid, 0); // 等待空位

        // 准备消息
        Message msg;
        snprintf(msg.topic, MAX_TOPIC_LEN, "sensor%d", i % 3);
        snprintf(msg.content, MAX_CONTENT_LEN, "value=%d", i * 10);
        msg.valid = 1;

        // 写入共享内存
        int index = shm->write_index;
        shm->messages[index] = msg;
        shm->write_index = (index + 1) % MAX_MESSAGES;
        shm->message_count++;

        printf("发布者[%d]: 发布消息 topic=%s, content=%s\n",
               getpid(), msg.topic, msg.content);

        sem_signal(semid, 1); // 通知有新消息
        sleep(1);
    }

    shmdt(shm);
}

void subscriber(int subscriber_id)
{
    // 连接到共享内存
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);

    // 连接到信号量
    int semid = semget(SEM_KEY, 2, 0666);

    printf("订阅者%d[%d]启动\n", subscriber_id, getpid());

    int last_index = 0;

    while (1)
    {
        sem_wait(semid, 1); // 等待新消息

        if (shm->message_count > 0)
        {
            int index = last_index;

            if (shm->messages[index].valid)
            {
                Message msg = shm->messages[index];
                printf("订阅者%d[%d]: 收到消息 topic=%s, content=%s\n",
                       subscriber_id, getpid(), msg.topic, msg.content);

                // 标记为已读（简单实现）
                shm->messages[index].valid = 0;
                shm->message_count--;

                last_index = (index + 1) % MAX_MESSAGES;
            }
        }

        sem_signal(semid, 0); // 释放空位
    }

    shmdt(shm);
}

int main()
{
    // 创建共享内存
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);

    // 初始化共享内存
    memset(shm, 0, sizeof(SharedMemory));
    shm->write_index = 0;
    shm->read_index = 0;
    shm->message_count = 0;

    // 创建信号量
    int semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, MAX_MESSAGES); // 空位信号量
    semctl(semid, 1, SETVAL, 0);            // 消息信号量

    // 创建多个进程
    pid_t pid;

    // 创建发布者
    pid = fork();
    if (pid == 0)
    {
        publisher();
        exit(0);
    }

    // 创建多个订阅者
    for (int i = 0; i < 3; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            subscriber(i);
            exit(0);
        }
    }

    // 等待一段时间
    sleep(10);

    // 清理
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}
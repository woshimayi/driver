/*
 * @*************************************: 
 * @FilePath     : /user/C/ipc_msg.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-09-18 16:47:53
 * @LastEditors  : dof
 * @LastEditTime : 2024-09-18 17:00:15
 * @Descripttion :  
 * @compile      :  
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY 1234

struct msgbuf {
    long mtype;
    char mtext[100];
};


int main() {
    int msqid;
    struct msgbuf buf;

    // 创建或获取消息队列
    if ((msqid = msgget(KEY, IPC_CREAT | 0666)) < 0) {
        perror("msgget");
        exit(1);
    }
    printf("msqid = %d\n", msqid);
    system("ipcs -a");

    // 发送消息
    buf.mtype = 1;
    strcpy(buf.mtext, "Hello, world!");
    msgsnd(msqid, &buf, sizeof(buf.mtext), 0);

    // 接收消息
    if (0 > msgrcv(msqid, &buf, sizeof(buf.mtext), 1, 0))
    {
        perror("msgrcv");
        exit(1);
    }
    printf("Received: %s\n", buf.mtext);

    // 删除消息队列
    msgctl(msqid, IPC_RMID, NULL);
    system("ipcs -a");

    return 0;
}
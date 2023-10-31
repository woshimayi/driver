/*
 * @*************************************: 
 * @FilePath: /user/C/string/fork_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-10-20 11:46:41
 * @LastEditors: dof
 * @LastEditTime: 2023-10-20 11:51:00
 * @Descripttion: 进程可以自我守护，父进程监控，子进程运行程序
 * @**************************************: 
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	printf("父进程 %d \n", getpid());
    while (1) {
        pid_t pid = fork();
        
        if (pid == -1) {
            // fork失败
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // 子进程
            printf("子进程ID: %d\n", getpid());
            sleep(4); // 子进程睡眠2秒，模拟某种处理
            printf("子进程 %d 退出\n", getpid());
            exit(EXIT_SUCCESS); // 子进程退出
        } else {
            // 父进程
            int status;
			printf("父进程 %d \n", getpid());
            waitpid(pid, &status, 0); // 等待子进程结束
        }
    }

    return 0;
}
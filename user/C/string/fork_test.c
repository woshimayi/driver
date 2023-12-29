/*
 * @*************************************: 
 * @FilePath: /user/C/string/fork_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-10-20 11:46:41
 * @LastEditors: dof
 * @LastEditTime: 2023-12-21 17:57:17
 * @Descripttion: 进程可以自我守护，父进程监控，子进程运行程序
 * @**************************************: 
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>
#include <signal.h>

void __main()
{
    printf("zzzzzzzzzzzzz \n");
    system("echo > 123.tmp");
    while (1)
    {}
}

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
            __main();
        } else {
            // 父进程
            int status;
			printf("父进程 %d %d\n", getpid(), pid);
            while (1)
            {
                sleep(2);
                if (0 > access("123.tmp", F_OK))
                {
                    printf("子进程 11111 %d 退出\n", pid);
                    kill(pid, SIGINT);
                    break;
                }
                
            }

            waitpid(pid, &status, 0); // 等待子进程结束
        }
    }

    return 0;
}
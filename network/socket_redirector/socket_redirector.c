//
// Created by hg-work on 25-6-12.
//
#include <stdio.h>      // For printf, perror
#include <stdlib.h>     // For exit, EXIT_FAILURE
#include <string.h>     // For memset
#include <unistd.h>     // For fork, execve, dup2, close, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO
#include <sys/socket.h> // For socket, bind, listen, accept
#include <netinet/in.h> // For sockaddr_in, INADDR_ANY, htons
#include <arpa/inet.h>  // For inet_ntop
#include <sys/wait.h>   // For waitpid
#include <errno.h>      // For errno

#define MAX_LISTEN_FDS 2 // 最大监听套接字数量
#define BUFFER_SIZE 256  // 缓冲区大小 (主要用于 my_echo_server 示例)

// 结构体：将端口号映射到要启动的程序路径
typedef struct
{
    int port;
    const char* program_path;
} ProgramMapping;

// 预定义程序映射，方便演示
ProgramMapping programs[] = {
    {12345, "./my_echo_server"}, // 端口 12345 对应回显服务器
    {12346, "./my_date_server"} // 端口 12346 对应日期服务器
};
const int num_programs = sizeof(programs) / sizeof(programs[0]); // 映射的数量

int main()
{
    int listen_fds[MAX_LISTEN_FDS]; // 存储所有监听套接字的文件描述符
    struct sockaddr_in server_addr[MAX_LISTEN_FDS]; // 存储服务器地址信息
    int i;

    // --- 1. 创建并绑定所有监听套接字 ---
    for (i = 0; i < num_programs; i++)
    {
        // 创建套接字：IPv4, 面向连接的流套接字 (TCP)
        listen_fds[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fds[i] < 0)
        {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        // 设置套接字选项：允许端口立即重用 (防止 TIME_WAIT 状态导致绑定失败)
        int opt = 1;
        if (setsockopt(listen_fds[i], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            perror("setsockopt SO_REUSEADDR failed");
            close(listen_fds[i]);
            exit(EXIT_FAILURE);
        }

        // 初始化服务器地址结构
        memset(&server_addr[i], 0, sizeof(server_addr[i]));
        server_addr[i].sin_family = AF_INET; // IPv4
        server_addr[i].sin_addr.s_addr = INADDR_ANY; // 监听所有可用的网络接口
        server_addr[i].sin_port = htons(programs[i].port); // 转换为网络字节序的端口号

        // 绑定套接字到指定地址和端口
        if (bind(listen_fds[i], (struct sockaddr*)&server_addr[i], sizeof(server_addr[i])) < 0)
        {
            perror("bind failed");
            close(listen_fds[i]);
            exit(EXIT_FAILURE);
        }

        // 使套接字进入监听状态，设置连接队列的最大长度为 5
        if (listen(listen_fds[i], 5) < 0)
        {
            perror("listen failed");
            close(listen_fds[i]);
            exit(EXIT_FAILURE);
        }
        printf("Listening on port %d for program %s\n", programs[i].port, programs[i].program_path);
    }

    // --- 2. 主循环：使用 select() 监听多个套接字并接受连接 ---
    fd_set master_fds; // 文件描述符集合

    while (1)
    {
        // 每次循环开始时，都需要重新设置 fd_set
        FD_ZERO(&master_fds); // 清空集合
        int max_fd = 0; // 记录集合中最大的文件描述符值

        // 将所有监听套接字添加到集合中
        for (i = 0; i < num_programs; i++)
        {
            FD_SET(listen_fds[i], &master_fds);
            if (listen_fds[i] > max_fd)
            {
                max_fd = listen_fds[i];
            }
        }

        printf("Waiting for connections...\n");
        // select()：等待文件描述符变得就绪
        // 第一个参数: 最大的文件描述符值加 1
        // 第二个参数: 读集合（检查是否有数据可读或新连接）
        // 第三、四个参数: 写集合、异常集合 (这里不关心)
        // 第五个参数: 超时时间 (NULL 表示无限等待)
        if (select(max_fd + 1, &master_fds, NULL, NULL, NULL) == -1)
        {
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        // 遍历所有监听套接字，检查哪个套接字有新连接
        for (i = 0; i < num_programs; i++)
        {
            if (FD_ISSET(listen_fds[i], &master_fds))
            {
                // 如果此监听套接字有活动
                struct sockaddr_in client_addr; // 存储客户端地址信息
                socklen_t client_len = sizeof(client_addr);

                // 接受新的客户端连接
                int client_sock_fd = accept(listen_fds[i], (struct sockaddr*)&client_addr, &client_len);
                if (client_sock_fd < 0)
                {
                    perror("accept failed");
                    continue; // 接受失败，继续监听其他连接
                }

                char client_ip[INET_ADDRSTRLEN]; // 存储客户端 IP 地址的字符串形式
                // 将客户端 IP 地址从网络字节序转换为可读的字符串形式
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                printf("Accepted connection from %s:%d on listening port %d\n",
                       client_ip, ntohs(client_addr.sin_port), programs[i].port);

                // --- 3. fork() 子进程来处理客户端连接 ---
                pid_t pid = fork();
                if (pid < 0)
                {
                    perror("fork failed");
                    close(client_sock_fd); // 如果 fork 失败，父进程关闭客户端套接字
                    continue; // 继续监听其他连接
                }
                else if (pid == 0)
                {
                    // 子进程
                    // 子进程不需要监听套接字，将其关闭
                    for (int j = 0; j < num_programs; j++)
                    {
                        close(listen_fds[j]);
                    }

                    // --- 4. dup2() 重定向标准输入/输出/错误到客户端套接字 ---
                    // 将客户端套接字复制到标准输入文件描述符 (0)
                    if (dup2(client_sock_fd, STDIN_FILENO) == -1)
                    {
                        perror("dup2 STDIN_FILENO failed");
                        exit(EXIT_FAILURE);
                    }
                    // 将客户端套接字复制到标准输出文件描述符 (1)
                    if (dup2(client_sock_fd, STDOUT_FILENO) == -1)
                    {
                        perror("dup2 STDOUT_FILENO failed");
                        exit(EXIT_FAILURE);
                    }
                    // 将客户端套接字复制到标准错误文件描述符 (2)
                    if (dup2(client_sock_fd, STDERR_FILENO) == -1)
                    {
                        perror("dup2 STDERR_FILENO failed");
                        exit(EXIT_FAILURE);
                    }

                    // 关闭原始的客户端套接字文件描述符。
                    // 因为 STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO 现在已经指向它了，
                    // 保留原始的 client_sock_fd 会浪费一个文件描述符。
                    close(client_sock_fd);

                    // --- 5. execve() 启动相应的程序 ---
                    // 准备传递给新程序的命令行参数 (argv)
                    // 第一个参数是程序路径，第二个是 NULL 终止符
                    char* argv[] = {(char*)programs[i].program_path, NULL};
                    // 准备传递给新程序的环境变量 (envp)
                    char* envp[] = {NULL}; // 这里不设置特定环境变量

                    printf("Child: Executing %s...\r\n", programs[i].program_path); // 此条信息现在会发给客户端！
                    execve(programs[i].program_path, argv, envp);

                    // 如果 execve 返回，说明执行失败（例如程序不存在或权限问题）
                    perror("execve failed");
                    exit(EXIT_FAILURE); // 子进程在 execve 失败时退出
                }
                else
                {
                    // 父进程
                    // 父进程不需要客户端套接字，将其关闭
                    close(client_sock_fd);
                    printf("Parent: Forked child %d for program %s\r\n", pid, programs[i].program_path);
                }
            }
        }

        // --- 6. 清理僵尸进程 ---
        // 非阻塞地检查是否有已终止的子进程需要清理
        // WNOHANG 意味着如果没有子进程终止，waitpid 会立即返回 0
        while (waitpid(-1, NULL, WNOHANG) > 0)
        {
            printf("Parent: Cleaned up a child process.\n");
        }
    }

    // 这部分代码在当前无限循环中是不可达的，但在实际生产服务中会进行资源释放
    for (i = 0; i < num_programs; i++)
    {
        close(listen_fds[i]);
    }
    return 0;
}

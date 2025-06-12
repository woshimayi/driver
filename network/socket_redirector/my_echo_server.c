//
// Created by hg-work on 25-6-12.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For STDIN_FILENO, STDOUT_FILENO

#if 0
int main()
{
    char buffer[256];
    ssize_t bytes_read;
    int i = 0;

    // 内部日志，发送到 stderr，方便调试，不影响客户端通信
    fprintf(stderr, "Echo Server: Started. Reading from stdin, writing to stdout.\n");

    // 从标准输入读取数据，直到 EOF 或错误
    // while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0)
    // while (1)
    // {
    //     fprintf(stderr, "Echo Server: Started. Reading from stdin, writing to stdout.\n");
    //     // buffer[bytes_read] = '\0'; // 确保 null 终止，以防后续打印或处理
    //     // fprintf(stderr, "Echo Server: Received %zd bytes: '%s'\n", bytes_read, buffer);
    //     //
    //     // // 向标准输出写入数据（这些数据会被重定向到客户端套接字）
    //     // write(STDOUT_FILENO, "Echo: ", 6); // 添加一个前缀
    //     // write(STDOUT_FILENO, buffer, bytes_read);
    //     // if (buffer[bytes_read - 1] != '\n')
    //     // {
    //     //     // 如果最后不是换行符，添加一个
    //     //     write(STDOUT_FILENO, "\n", 1);
    //     // }
    //     sleep(1);
    //
    //     if (i > 10);
    //     {
    //         break;
    //     }
    //     printf("xxxxxx\n");
    //     fflush(stdout);
    //     i++;
    // }

     while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // buffer 中包含了读取到的数据，包括换行符（如果存在）。
        // fprintf(stderr, "Echo Server: Received '%s'\n", buffer); // 调试信息

        // 向标准输出写入数据。
        // printf 会将数据写入 stdout。
        // 如果 stdout 被 dup2 重定向到套接字，printf 就会将数据发送给客户端。
        printf("Echo: %s", buffer); // 直接打印接收到的字符串，包括其可能带的换行符

        // 刷新标准输出缓冲区，确保数据立即发送给客户端。
        // 对于网络通信，通常需要显式刷新，否则数据可能留在缓冲区中不发送。
        fflush(stdout);
    }

    fprintf(stderr, "Echo Server: Exiting.\n");
    return 0;
}
#else
int main() {
    char buffer[256]; // 用于存储读取数据的缓冲区

    // --- 新增：打开终端设备文件用于内部日志输出 ---
    // /dev/tty 是当前控制终端的通用名称。
    // 这允许我们将日志输出到运行 socket_redirector 的终端上，
    // 而不影响重定向到客户端套接字的 stdout 和 stderr。
    FILE *terminal_log_file = fopen("/dev/tty", "w");
    if (terminal_log_file == NULL) {
        // 如果无法打开 /dev/tty，则退回到 stderr（这仍可能被重定向到客户端）
        fprintf(stderr, "Echo Server: Warning: Could not open /dev/tty for logging. Logs might go to client or nowhere.\n");
        terminal_log_file = stderr; // 备份到 stderr
    }

    // 内部日志，现在发送到终端，而不是默认的 stderr。
    // 如果 terminal_log_file 成功打开，这条日志将出现在运行 socket_redirector 的终端上。
    // 如果未能打开 /dev/tty，它将回退到 stderr。
    fprintf(terminal_log_file, "Echo Server: Started. Reading from stdin, writing to stdout. (Logs to terminal via /dev/tty)\n");
    fflush(terminal_log_file); // 立即刷新，确保日志输出

    // 从标准输入读取数据，直到 EOF 或错误。
    // fgets 会从 stdin 读取一行，并将其存储到 buffer 中。
    // 因为 stdin 被 dup2 重定向到套接字，fgets 会从套接字读取。
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // buffer 中包含了读取到的数据，包括换行符（如果存在）。
        // 这些日志仍然会发送到客户端，因为它们使用了 stderr。
        // 如果想让这些日志也只去终端，需要将它们也改为 fprintf(terminal_log_file, ...)
        fprintf(terminal_log_file, "Echo Server: Received from client: %s", buffer); // 调试信息
        fflush(terminal_log_file);

        // 向标准输出写入数据。
        // printf 会将数据写入 stdout。
        // 因为 stdout 被 dup2 重定向到套接字，printf 就会将数据发送给客户端
        printf("Echo: %s", buffer); // 直接打印接收到的字符串，包括其可能带的换行符
        buffer[strlen(buffer)-1] = '\0';
        fprintf(terminal_log_file, "xxxxx|%s|\r\n", buffer); // 调试信息
        if (!strcmp(buffer, "quit"))
        {
            break;
        }

        // 刷新标准输出缓冲区，确保数据立即发送给客户端。
        fflush(stdout);
        memset(buffer, 0, sizeof(buffer));
    }

    fprintf(terminal_log_file, "Echo Server: Exiting.\r\n"); // 退出日志
    fflush(terminal_log_file); // 确保最后一条日志也输出

    // 关闭终端日志文件，如果它被打开了的话
    if (terminal_log_file != NULL && terminal_log_file != stderr) {
        fclose(terminal_log_file);
    }

    return 0; // 正常退出
}
#endif
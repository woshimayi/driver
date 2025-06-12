//
// Created by hg-work on 25-6-12.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h> // For STDOUT_FILENO

int main() {
    time_t rawtime;
    struct tm *info;
    char buffer[80];

    fprintf(stderr, "Date Server: Started. Sending date to stdout.\r\n");

    // 获取当前时间
    time(&rawtime);
    info = localtime(&rawtime);

    // 格式化时间字符串
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S\r\n", info);

    // 将格式化后的时间写入标准输出（这些数据会被重定向到客户端套接字）
    write(STDOUT_FILENO, "Current Date/Time: ", 19);
    write(STDOUT_FILENO, buffer, strlen(buffer));

    fprintf(stderr, "Date Server: Exiting.\n");
    return 0;
}
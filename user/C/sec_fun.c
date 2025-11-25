 /*
 * @*************************************: 
 * @FilePath     : /user/unpv13e/test/tsnprintf.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-07-19 14:54:52
 * @LastEditors  : dof
 * @LastEditTime : 2025-09-08 14:47:19
 * @Descripttion :  深入探究 snprintf 的使用
 * @compile      :  
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int main(int argc, char **argv)
{
    // int		n;
    // char	buf[1024];

    // n = snprintf(buf, 4, "%d", 9999);
    // if (n > 3)
    //     printf("error: snprintf overflowed buffer, n = %d\n", n);
    // exit(0);


    char user_input[784] = "This is a sample user input string that might be very long and needs to be safely copied into a fixed-size buffer to prevent overflow.";
    char buffer[64];

    // 方法：使用 %.*s 格式
    // (sizeof(buffer) - 1) 是缓冲区最大可用长度（保留1字节给空字符）
    // -XX 是减去格式字符串中其他固定字符的长度（这里是 "Input: " 的7字节）
    // int max_copy_len = sizeof(buffer) - 1 - 7; // 128 - 1 - 7 = 120

    // snprintf(buffer, sizeof(buffer), "Input: %.*s", 20, user_input);
    // // snprintf(buffer, sizeof(buffer), "Input: %sa", user_input);
    // strlcpy(buffer, user_input, sizeof(buffer)); // 保证 dest 以 \0 终止
    // printf("%s\n", buffer);




    // char user_input[256] = "../../driver/";
    // char resolved_path[PATH_MAX]; // PATH_MAX 在 <limits.h> 中定义

    // 假设从用户那里获取了一个路径
    // scanf("%255s", user_input);

    // 解析并标准化路径
    // if (realpath(user_input, resolved_path) == NULL) {
    //     // 路径无效或无法访问
    //     perror("realpath");
    //     exit(EXIT_FAILURE);
    // }
    // printf("Resolved path: %s\n", resolved_path);

    // 检查解析后的路径是否在我们允许的目录下（例如 /safe/dir/）
    // if (strncmp(resolved_path, "/safe/dir/", strlen("/safe/dir/")) != 0) {
    //     fprintf(stderr, "Access denied: Path is outside the allowed directory.\n");
    //     exit(EXIT_FAILURE);
    // }

    // 现在可以安全地使用 resolved_path 了
    // FILE *fp = fopen(resolved_path, "r");


}

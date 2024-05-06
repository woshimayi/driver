/*
 * @*************************************: 
 * @FilePath: /user/C/fileIO/traceroute_test1.c
 * @version: 
 * @Author: dof
 * @Date: 2024-04-10 13:20:25
 * @LastEditors: dof
 * @LastEditTime: 2024-04-10 13:24:27
 * @Descripttion: 
 * @**************************************: 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char buf[1024];
    int hop;
    char hostname[1024];
    double rtt;
    FILE *pf_file = NULL;

	if (NULL == (pf_file = fopen("bucpe_traceroute", "r")))
	{
		perror("xxxxx ");
		return -1;
	}

    // 读取 traceroute 输出
    while (fgets(buf, sizeof(buf), pf_file)) {
        // 解析 hop
        sscanf(buf, "%d ", &hop);

        // 解析 hostname
        if (sscanf(buf, "%*s %[^ ]", hostname) != 1) {
            fprintf(stderr, "Error parsing hostname\n");
            return 1;
        }

        // 解析 RTT
        if (sscanf(buf, "%*s %*s %*s %lf %lf %lf", &rtt) != 1) {
            fprintf(stderr, "Error parsing RTT\n");
            return 1;
        }

        // 打印结果
        printf("Hop %d: %s (%f ms)\n", hop, hostname, rtt);
    }

    return 0;
}

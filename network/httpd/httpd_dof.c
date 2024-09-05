/*
 * @*************************************:
 * @FilePath     : /network/httpd/httpd_dof.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-26 09:38:15
 * @LastEditors  : dof
 * @LastEditTime : 2024-09-03 11:41:04
 * @Descripttion : 单线程tcp server 模拟
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

int local_socket()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    char buf[128] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    // address.sin_addr.s_addr = inet_addr(buf);        // ip address
    address.sin_port = htons(9090); // port

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int local_accept(int server_fd)
{
    int new_socket;
    ssize_t len;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[2048] = {0};
    int isGisLock = 0;

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        PP("while --------------- new_socket = %d", new_socket);

        memset(buffer, '\0', sizeof(buffer));

        while (0)
        {
            len = read(new_socket, buffer, sizeof(buffer));
            if (len <= 0)
            {
                break;
            }
            buffer[len] = '\0';

            if (strlen(buffer))
            {
                send(new_socket, buffer, strlen(buffer), 0);
            }
        }

        PP("while ---------------end");
        close(new_socket);
    }

    close(server_fd);

    return 0;
}

int main(int argc, char const *argv[])
{
    int server_fd = local_socket();

    local_accept(server_fd);
    return 0;
}

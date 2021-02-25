/*
 * @FilePath: /network/udp/udp-server-client/udp_client.c
 * @version: 
 * @Author: dof
 * @Date: 2020-10-10 09:55:53
 * @LastEditors: dof
 * @LastEditTime: 2021-02-25 17:04:12
 * @Descripttion: 
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 8888
#define BUFF_LEN 512
#define SERVER_IP "127.0.0.1"


void udp_msg_sender(int fd, struct sockaddr* dst)
{
    socklen_t len;
    struct sockaddr_in src;
    while(1)
    {
        char buf[BUFF_LEN] = "TEST UDP MSG!\n";
        len = sizeof(*dst);
        printf("client:%s\n",buf);  //��ӡ�Լ����͵���Ϣ
        sendto(fd, buf, BUFF_LEN, 0, dst, len);
        memset(buf, 0, BUFF_LEN);
        recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&src, &len);  //��������server����Ϣ
        printf("server:%s\n",buf);
        sleep(1);  //һ�뷢��һ����Ϣ
    }
}

/*
    client:
            socket-->sendto-->revcfrom-->close
*/

int main(int argc, char* argv[])
{
    int client_fd;
    struct sockaddr_in ser_addr;

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_fd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    //ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //ע��������ת��
    ser_addr.sin_port = htons(SERVER_PORT);  //ע��������ת��

    udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);

    close(client_fd);

    return 0;
}

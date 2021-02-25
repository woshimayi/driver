/*
 * @FilePath: /network/udp/udp-timeserver/udp_client.c
 * @version: 
 * @Author: dof
 * @Date: 2020-10-10 10:09:58
 * @LastEditors: dof
 * @LastEditTime: 2021-02-25 17:04:23
 * @Descripttion: 
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 123
#define BUFF_LEN 512
#define SERVER_IP "209.249.181.53"

void udp_handler(int s, struct sockaddr* to)
{
    char buf[1024] = "TEST UDP !";
    int n = 0;
    connect(s, to, sizeof(*to));
 	printf("1 %s\n", buf);
    n = write(s, buf, 1024);
    memset(buf, 0, sizeof(buf));
    if (read(s, buf, n))
    {
    	printf("2 %s\n", buf);
	}
}

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
	
	udp_handler(client_fd, (struct sockaddr*)&ser_addr);
    udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);

    close(client_fd);

    return 0;
}

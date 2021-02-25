/*
 * @FilePath: /network/udp/udp-server-client/udp_server.c
 * @version: 
 * @Author: dof
 * @Date: 2020-10-10 09:57:00
 * @LastEditors: dof
 * @LastEditTime: 2021-02-25 17:04:14
 * @Descripttion: 
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 8888
#define BUFF_LEN 1024

void handle_udp_msg(int fd)
{
    char buf[BUFF_LEN];  //���ջ�������1024�ֽ�
    socklen_t len;
    int count;
    struct sockaddr_in clent_addr;  //clent_addr���ڼ�¼���ͷ��ĵ�ַ��Ϣ
    while(1)
    {
        memset(buf, 0, BUFF_LEN);
        len = sizeof(clent_addr);
        count = recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clent_addr, &len);  //recvfrom��ӵ��������û�����ݾ�һֱӵ��
        if(count == -1)
        {
            printf("recieve data fail!\n");
            return;
        }
        printf("client:%s\n",buf);  //��ӡclient����������Ϣ
        memset(buf, 0, BUFF_LEN);
        sprintf(buf, "I have recieved %d bytes data!\n", count);  //�ظ�client
        printf("server:%s\n",buf);  //��ӡ�Լ����͵���Ϣ��
        sendto(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clent_addr, len);  //������Ϣ��client��ע��ʹ����clent_addr�ṹ��ָ��

    }
}


/*
    server:
            socket-->bind-->recvfrom-->sendto-->close
*/

int main(int argc, char* argv[])
{
    int server_fd, ret;
    struct sockaddr_in ser_addr;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0); //AF_INET:IPV4;SOCK_DGRAM:UDP
    if(server_fd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP��ַ����Ҫ����������ת����INADDR_ANY�����ص�ַ
    ser_addr.sin_port = htons(SERVER_PORT);  //�˿ںţ���Ҫ������ת��

    ret = bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
    if(ret < 0)
    {
        printf("socket bind fail!\n");
        return -1;
    }

    handle_udp_msg(server_fd);   //�������յ�������

    close(server_fd);
    return 0;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

#define MYPORT  9806
#define BUFFER_SIZE 1024

char *SERVER_IP = "182.125.102.45";
//char* SERVER_IP = "111.198.172.102";

int main()
{
    ///����sockfd
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);
    int sock_cli_1 = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *hptr;

    ///����sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT);  ///�������˿�
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);  ///������ip

    printf("����%s:%d %d %d\n", SERVER_IP, MYPORT, sock_cli, sock_cli_1);
    ///���ӷ��������ɹ�����0�����󷵻�-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("conecting fail \n");
        perror("connect");
        exit(1);
    }
    printf("���������ӳɹ�\n");

    char sendbuf[BUFFER_SIZE] = {0};
    char recvbuf[BUFFER_SIZE] = {0};

    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        printf("��������������ݣ�%s\n", sendbuf);
        send(sock_cli, sendbuf, strlen(sendbuf), 0); ///����
        if (strcmp(sendbuf, "exit\n") == 0)
            break;
        recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///����
        printf("�ӷ������������ݣ�%s\n", recvbuf);

        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    close(sock_cli);
    return 0;
}


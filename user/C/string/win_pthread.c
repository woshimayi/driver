#include <stdio.h>

#include <stdlib.h>

#include <process.h>

#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAXCLIENTS 10           //�궨�壬���3���ͻ�������

DWORD WINAPI ProcessClientRequests(LPVOID lpParam)

{

    SOCKET *clientsocket = (SOCKET *)lpParam; //������Ҫǿ��ת����ע�⣺ָ�����͵�

    char *msg = "Hello, my client.\r\n";

    send(*clientsocket, msg, strlen(msg) + sizeof(char), NULL);

    printf("***SYS***    HELLO.\n");

    while (TRUE)

    {

        char buffer[MAXBYTE] = {0};

        recv(*clientsocket, buffer, MAXBYTE, NULL);

        if (strcmp(buffer, "exit") == 0)

        {

            char *msg_bye = "Bye.\r\n";

            send(*clientsocket, msg_bye, strlen(msg_bye) + sizeof(char), NULL);

            break;

        }

        printf("***Client***    %s\n", buffer);

    }

    closesocket(*clientsocket);

    return 0;

}

int main()

{

    WSADATA wsaData;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    HANDLE threads[10];

    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sockaddr;

    sockaddr.sin_family = PF_INET;

    sockaddr.sin_addr.S_un.S_addr = inet_addr("192.168.1.102");

    sockaddr.sin_port = htons(5000);

    bind(s, (SOCKADDR *)&sockaddr, sizeof(SOCKADDR));

    listen(s, 20);

    //  printf("listening on port [%d].\n", 5000);

    int existingClientCount = 0;

    while (TRUE)

    {

        printf("�ȴ�����...\n");

        SOCKADDR clientAddr;

        int size = sizeof(SOCKADDR);

        SOCKET clientsocket;

        clientsocket = accept(s, &clientAddr, &size);

        //     printf("***SYS***    New client touched.\n");

        if (existingClientCount < 10)    //�ж��Ƿ��Ѿ����������������

        {

            threads[existingClientCount++] = CreateThread(NULL, 0, ProcessClientRequests, &clientsocket, 0,
                                             NULL); //�������̣߳����ҽ�socket����

        }

        else

        {

            char *msg = "Exceeded Max incoming requests, will refused this connect!\r\n";

            send(clientsocket, msg, strlen(msg) + sizeof(char), NULL);     //���;ܾ�������Ϣ���ͻ���

            printf("***SYS***    REFUSED.\n");

            closesocket(clientsocket);                                     //�ͷ���Դ

            break;

        }

    }

    printf("Maximize clients occurred for d%.\r\n", 10);

    WaitForMultipleObjects(10, threads, TRUE, INFINITE);           //�ȴ��������̣߳�ֱ�����Ϊֹ

    closesocket(s);

    for (int i = 0; i < 10; i++)

    {

        CloseHandle(threads[i]);                                           //�����߳���Դ

    }

    WSACleanup();

    printf("Cleared all.\r\n");

    getchar();

    return 0;

}

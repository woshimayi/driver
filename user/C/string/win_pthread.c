#include <stdio.h>

#include <stdlib.h>

#include <process.h>

#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAXCLIENTS 10           //宏定义，最多3个客户端连接

DWORD WINAPI ProcessClientRequests(LPVOID lpParam)

{

    SOCKET *clientsocket = (SOCKET *)lpParam; //这里需要强制转换，注意：指针类型的

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

        printf("等待连接...\n");

        SOCKADDR clientAddr;

        int size = sizeof(SOCKADDR);

        SOCKET clientsocket;

        clientsocket = accept(s, &clientAddr, &size);

        //     printf("***SYS***    New client touched.\n");

        if (existingClientCount < 10)    //判断是否已经超出最大连接数了

        {

            threads[existingClientCount++] = CreateThread(NULL, 0, ProcessClientRequests, &clientsocket, 0,
                                             NULL); //启动新线程，并且将socket传入

        }

        else

        {

            char *msg = "Exceeded Max incoming requests, will refused this connect!\r\n";

            send(clientsocket, msg, strlen(msg) + sizeof(char), NULL);     //发送拒绝连接消息给客户端

            printf("***SYS***    REFUSED.\n");

            closesocket(clientsocket);                                     //释放资源

            break;

        }

    }

    printf("Maximize clients occurred for d%.\r\n", 10);

    WaitForMultipleObjects(10, threads, TRUE, INFINITE);           //等待所有子线程，直到完成为止

    closesocket(s);

    for (int i = 0; i < 10; i++)

    {

        CloseHandle(threads[i]);                                           //清理线程资源

    }

    WSACleanup();

    printf("Cleared all.\r\n");

    getchar();

    return 0;

}

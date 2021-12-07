#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int lookup(char *host, int *d)
{
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    int ret;
    struct sockaddr_in *addr;
    char ipbuf[16];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(host, NULL, &hints, &res);

    if (ret == -1)
    {
        perror("getaddrinfo");
        exit(1);
    }

    for (cur = res; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in *)cur->ai_addr;
        printf("%s\n", inet_ntop(AF_INET,
                                 &addr->sin_addr, ipbuf, 16));
    }
    freeaddrinfo(res);
    *d = inet_addr(ipbuf);
    exit(0);
}


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usag...\n");
        exit(1);
    }
    int d = 0;
    printf("dns = %s\n", argv[1]);
    system("date");
    lookup(argv[1], &d);
    system("date");
    printf("%d\n", d);
}

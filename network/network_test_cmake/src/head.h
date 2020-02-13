#ifndef HEAD_H
#define HEAD_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>

#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


int www_ParseUrl(const char *url, char *proto, char *host, int *port, char *uri);

int ifname_test(char *hostname, char *portnr, char *ifname);

int lookup(char *host, char *portnr, struct addrinfo **res);

int fromIfnameGetIp(char **gwip, char **ifname);

#endif
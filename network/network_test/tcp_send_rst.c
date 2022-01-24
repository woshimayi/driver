/*
 * @*************************************: 
 * @FilePath: /network/network_test/tcp_send_rst.c
 * @version: 
 * @Author: dof
 * @Date: 2022-01-24 14:32:06
 * @LastEditors: dof
 * @LastEditTime: 2022-01-24 14:32:08
 * @Descripttion: tcp send RST
 * @**************************************: 
 */

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#define PEER_ADDR		(0x7F000001)
#define PEER_PORT		(34567)

int main(void)
{
	struct sockaddr_in local, peer;
	struct linger linger;
	int ret;
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	memset(&peer, 0, sizeof(peer));

	peer.sin_family = AF_INET;
	peer.sin_port = htons(PEER_PORT);
	peer.sin_addr.s_addr = htonl(PEER_ADDR);
	local = peer;

	int flag = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	if (ret == -1) {
		printf("Fail to setsocket SO_REUSEADDR: %s\n", strerror(errno));
		exit(1);
	}

	ret = bind(sock, (const struct sockaddr *)&local, sizeof(local));
	if (ret) {
		printf("Fail to bind: %s\n", strerror(errno));
		exit(1);
	}
	ret = connect(sock, (const struct sockaddr *)&peer, sizeof(peer));

	if (ret) {
		printf("Fail to connect myself: %s\n", strerror(errno));
		exit(1);
	}
	printf("Connect successfully\n");

	memset(&linger, 0, sizeof(linger));
	linger.l_onoff = 1;
	linger.l_linger = 0;

	ret = setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
	if (ret) {
		printf("Fail to set linger\n");
		exit(1);
	}

	close(sock);

	printf("Done\n");
	return 0;
}

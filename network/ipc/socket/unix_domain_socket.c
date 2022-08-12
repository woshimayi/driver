/*
 * @*************************************:
 * @FilePath: /network/ipc/socket/unix_domain_socket.c
 * @version:
 * @Author: dof
 * @Date: 2022-07-06 14:55:13
 * @LastEditors: dof
 * @LastEditTime: 2022-07-06 15:22:33
 * @Descripttion: unix domain socket IPC
 * 				会生成 foo.socket s 文件，删除文件后即可重新绑定
 * UNIX Domain Socket与网络socket编程最明显的不同在于地址格式不同，
 * 用结构体sockaddr_un表示，网络编程的socket地址是IP地址加端口号，
 * 而UNIX Domain Socket的地址是一个socket类型的文件在文件系统中的路径，
 * 这个socket文件由bind()调用创建，如果调用bind()时该文件已存在，则bind()错误返回。
 * @**************************************:
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(void)
{
	int fd, size;

	struct sockaddr_un un;

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, "foo.socket");      // 绑定的地址
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(1);
	}

	size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (bind(fd, (struct sockaddr *)&un, size) < 0)
	{
		perror("bind error");
		exit(1);
	}

	printf("UNIX domain socket bound\n");
	exit(0);
}

/*
 * @*************************************: 
 * @FilePath: /network/ipc/socket/unxi_domain_socket.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-06 15:24:52
 * @LastEditors: dof
 * @LastEditTime: 2022-07-06 15:49:41
 * @Descripttion: 
 * @**************************************: 
 */


#include "unxi_domain_socket.h"


#define QLEN 10


int serv_listen(const char * name)
{
	int fd, len, err, rval;
	struct sockaddr_un un;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}

	unlink(name);

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strncpy(un.sun_path, name, sizeof(un.sun_path));

	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

	if (bind(fd, (struct socketaddr_un*)&un, len) < 0)
	{
		rval = -2;
		goto errout;
	}

	if (listen(fd, QLEN) < 0)
	{
		// tell kernel we'er server
		rval = -3;
		goto errout;
	}

	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}


int serv_accept(int listenfd, uid_t *uidptr)
{
	int clifd, len, err, rval;
	time_t staletimer;
	struct sockaddr_un un;
	struct stat statbuf;

	len = sizeof(un);
	if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0)
	{
		return -1;
	}

	len -= offsetof(struct sockaddr_un, sun_path);

	un.sun_path[len] = 0;

	if (stat(un.sun_path, &statbuf) < 0)
	{
		rval = -2;
		goto errout;
	}

	if (S_ISSOCK(statbuf.st_mode) == 0)
	{
		rval = -3;
		goto errout;
	}

	if (uidptr != NULL)
	{
		*uidptr = statbuf.st_uid;
	}
	unlink(un.sun_path);
	return (clifd);


errout:
	err = errno;
	close(clifd);
	errno = err;
	return (rval);
}


#define CLI_PATH "/var/tmp/"



int cli_conn(const char * name)
{
	int fd, len, err, rval;

	struct sockaddr_un un;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	snprintf(un.sun_path, "%s%05d", CLI_PATH, getpid());
	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

	unlink(un.sun_path);

	if (bind(fd, (struct sockaddr *)&un, len) < 0)
	{
		rval = -2;
		goto errout;
	}

	memsete(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);

	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

	if (connect(fd, (struct sockaddr *)&un, len) < 0)
	{
		rval = -4;
		goto errout;
	}
	return (fd);

errout:
	err = errno;
	close(fd);
	errno = err;
	return (rval);
}

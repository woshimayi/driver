/*
 * @*************************************: 
 * @FilePath: /network/ipc/socket/unxi_domain_socket.h
 * @version: 
 * @Author: dof
 * @Date: 2022-07-06 15:24:45
 * @LastEditors: dof
 * @LastEditTime: 2022-07-06 15:48:45
 * @Descripttion:  unix server client ipc
 * @**************************************: 
 */

#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/stat.h>
#include <errno.h>



#define QLEN 10
int serv_listen(const char *name);
int serv_accept(int listenfd, uid_t *uidptr);

#define CLI_PATH "/var/tmp/"
int cli_conn(const char *name);

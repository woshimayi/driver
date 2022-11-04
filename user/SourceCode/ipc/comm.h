/*
 * @*************************************: 
 * @FilePath: /user/SourceCode/ipc/comm.h
 * @version: 
 * @Author: dof
 * @Date: 2022-09-29 13:21:22
 * @LastEditors: dof
 * @LastEditTime: 2022-09-29 13:28:41
 * @Descripttion: 
 * @**************************************: 
 */


#ifndef _COMM_H__
#define _COMM_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define PATHNAME "."
#define PROJ_ID 0x6666

int CreateShm(int size);
int DestoryShm(int shmid);
int GetShm(int size);

#endif
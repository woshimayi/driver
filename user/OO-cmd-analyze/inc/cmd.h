/******************************************************************************
* 文件名称： cmd.h
* 摘 要：    命令解析器头文件
* 当前版本： 1.0
* 作 者：    邵国际
* 完成日期： 2017年11月18日
*
* 取代版本： 
* 原作者 ： 
* 完成日期： 
******************************************************************************/
#ifndef __CMD_H_
#define __CMD_H_

#include "STC12C5A60S2.h"

#define		MAX_CMD_NAME_LENGTH		20	  // 最大命令名长度，过大 51 内存会炸
#define		MAX_CMDS_COUNT			10	  // 最大命令数，过大 51 内存会炸

typedef void (*handler)(void);  	  // 命令操作函数指针类型

/* 命令结构体类型 */
typedef struct cmd 
{
 	char cmd_name[MAX_CMD_NAME_LENGTH + 1];   // 命令名  
	handler cmd_operate;			  	      // 命令操作函数
} CMD;

/* 命令列表结构体类型 */
typedef struct cmds
{
 	CMD cmds[MAX_CMDS_COUNT];                 // 列表内容
	int num;	                              // 列表长度
} CMDS;

/* 外部函数声明 */
void register_cmds(CMD reg_cmds[], int num);
void match_cmd(char *str);

#endif /* #ifndef __CMD_H_ */

/******************************************************************************
* 文件名称： cmd.c
* 摘 要：    命令解析器定义文件
* 当前版本： 1.0
* 作 者：    邵国际
* 完成日期： 2017年11月18日
*
* 取代版本： 
* 原作者 ： 
* 完成日期： 
******************************************************************************/
#include <string.h>

#include "cmd.h"
#include "uart.h"

static xdata CMDS commands = {NULL, 0};  // 全局命令列表，保存已注册命令集合

/******************************************************************************
* 函数介绍： 命令注册函数
* 输入参数： reg_cmds 待注册命令数组
*            num      待注册命令数组长度
* 输出参数： 无
* 返回值 ：  无
* 备    注： num 不得超过 MAX_CMDS_COUNT 
******************************************************************************/
void register_cmds(CMD reg_cmds[], int length)
{
 	int i;

	if (length > MAX_CMDS_COUNT)
	{
	 	return;
	}

	for (i = 0; i < length; i++)
	{
		if (commands.num < MAX_CMDS_COUNT)  // 命令列表未满
		{
			strcpy(commands.cmds[commands.num].cmd_name, reg_cmds[i].cmd_name);
			commands.cmds[commands.num].cmd_operate = reg_cmds[i].cmd_operate;
			commands.num++;	
		} 	
	}	
}

/******************************************************************************
* 函数介绍： 命令匹配执行函数
* 输入参数： str 待匹配命令字符串
* 输出参数： 无
* 返回值 ：  无
* 备    注： str 长度不得超过 MAX_CMD_NAME_LENGTH
******************************************************************************/
void match_cmd(char *str)
{
	int i;

	if (strlen(str) > MAX_CMD_NAME_LENGTH)
	{
	 	return;
	}

	for (i = 0; i < commands.num; i++)	// 遍历命令列表
	{
	 	if (strcmp(commands.cmds[i].cmd_name, str) == 0)
		{
		 	commands.cmds[i].cmd_operate();
		}
	}
}

/********************************END OF FILE**********************************/

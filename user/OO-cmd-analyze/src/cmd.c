/******************************************************************************
* �ļ����ƣ� cmd.c
* ժ Ҫ��    ��������������ļ�
* ��ǰ�汾�� 1.0
* �� �ߣ�    �۹���
* ������ڣ� 2017��11��18��
*
* ȡ���汾�� 
* ԭ���� �� 
* ������ڣ� 
******************************************************************************/
#include <string.h>

#include "cmd.h"
#include "uart.h"

static xdata CMDS commands = {NULL, 0};  // ȫ�������б�������ע�������

/******************************************************************************
* �������ܣ� ����ע�ắ��
* ��������� reg_cmds ��ע����������
*            num      ��ע���������鳤��
* ��������� ��
* ����ֵ ��  ��
* ��    ע�� num ���ó��� MAX_CMDS_COUNT 
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
		if (commands.num < MAX_CMDS_COUNT)  // �����б�δ��
		{
			strcpy(commands.cmds[commands.num].cmd_name, reg_cmds[i].cmd_name);
			commands.cmds[commands.num].cmd_operate = reg_cmds[i].cmd_operate;
			commands.num++;	
		} 	
	}	
}

/******************************************************************************
* �������ܣ� ����ƥ��ִ�к���
* ��������� str ��ƥ�������ַ���
* ��������� ��
* ����ֵ ��  ��
* ��    ע�� str ���Ȳ��ó��� MAX_CMD_NAME_LENGTH
******************************************************************************/
void match_cmd(char *str)
{
	int i;

	if (strlen(str) > MAX_CMD_NAME_LENGTH)
	{
	 	return;
	}

	for (i = 0; i < commands.num; i++)	// ���������б�
	{
	 	if (strcmp(commands.cmds[i].cmd_name, str) == 0)
		{
		 	commands.cmds[i].cmd_operate();
		}
	}
}

/********************************END OF FILE**********************************/

/******************************************************************************
* �ļ����ƣ� cmd.h
* ժ Ҫ��    ���������ͷ�ļ�
* ��ǰ�汾�� 1.0
* �� �ߣ�    �۹���
* ������ڣ� 2017��11��18��
*
* ȡ���汾�� 
* ԭ���� �� 
* ������ڣ� 
******************************************************************************/
#ifndef __CMD_H_
#define __CMD_H_

#include "STC12C5A60S2.h"

#define		MAX_CMD_NAME_LENGTH		20	  // ������������ȣ����� 51 �ڴ��ը
#define		MAX_CMDS_COUNT			10	  // ��������������� 51 �ڴ��ը

typedef void (*handler)(void);  	  // �����������ָ������

/* ����ṹ������ */
typedef struct cmd 
{
 	char cmd_name[MAX_CMD_NAME_LENGTH + 1];   // ������  
	handler cmd_operate;			  	      // �����������
} CMD;

/* �����б�ṹ������ */
typedef struct cmds
{
 	CMD cmds[MAX_CMDS_COUNT];                 // �б�����
	int num;	                              // �б���
} CMDS;

/* �ⲿ�������� */
void register_cmds(CMD reg_cmds[], int num);
void match_cmd(char *str);

#endif /* #ifndef __CMD_H_ */

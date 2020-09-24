/******************************************************************************
* �ļ����ƣ� main.c
* ժ Ҫ��    OO ���������������
*
* ��ǰ�汾�� 1.0
* �� �ߣ�    �۹���
* ������ڣ� 2017��11��18��
*
* ȡ���汾�� 
* ԭ���� �� 
* ������ڣ� 
******************************************************************************/
#include <string.h>

#include "STC12C5A60S2.h"
#include "uart.h"
#include "led.h"
#include "beep.h"
#include "cmd.h"

void main()
{
	unsigned char str[20];

	uart_init();
	led_init();
	beep_init();

	while (1)
	{	
		/* ��ȡ���������ַ��� */
		uart_get_string(str);

		/* ƥ�����ִ�� */
		match_cmd(str);

		/* ������� */




		uart_send_string(str);
		uart_send_byte('\n');				 	
	}
}

/********************************END OF FILE**********************************/

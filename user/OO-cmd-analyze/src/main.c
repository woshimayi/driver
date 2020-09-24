/******************************************************************************
* 文件名称： main.c
* 摘 要：    OO 串口命令解析程序
*
* 当前版本： 1.0
* 作 者：    邵国际
* 完成日期： 2017年11月18日
*
* 取代版本： 
* 原作者 ： 
* 完成日期： 
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
		/* 获取串口命令字符串 */
		uart_get_string(str);

		/* 匹配命令并执行 */
		match_cmd(str);

		/* 命令回显 */




		uart_send_string(str);
		uart_send_byte('\n');				 	
	}
}

/********************************END OF FILE**********************************/

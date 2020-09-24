/******************************************************************************
* 文件名称： uart.h
* 摘 要：    串口相关操作头文件
*
* 硬件连接： RXD --> P3.0
*			 TXD --> P3.1
*            
* 当前版本： 1.0
* 作 者：    邵国际
* 完成日期： 2017年11月18日
*
* 取代版本： 
* 原作者 ： 
* 完成日期： 
******************************************************************************/
#ifndef __UART_H_
#define __UART_H_

#include "STC12C5A60S2.h"

/* 外部函数声明 */
void uart_init(void);
void uart_send_byte(unsigned char byte);
unsigned char uart_get_byte(void);
void uart_send_string(unsigned char *str);
void uart_get_string(unsigned char *str);

#endif /* #ifndef __UART_H_ */

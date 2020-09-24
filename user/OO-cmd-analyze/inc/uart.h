/******************************************************************************
* �ļ����ƣ� uart.h
* ժ Ҫ��    ������ز���ͷ�ļ�
*
* Ӳ�����ӣ� RXD --> P3.0
*			 TXD --> P3.1
*            
* ��ǰ�汾�� 1.0
* �� �ߣ�    �۹���
* ������ڣ� 2017��11��18��
*
* ȡ���汾�� 
* ԭ���� �� 
* ������ڣ� 
******************************************************************************/
#ifndef __UART_H_
#define __UART_H_

#include "STC12C5A60S2.h"

/* �ⲿ�������� */
void uart_init(void);
void uart_send_byte(unsigned char byte);
unsigned char uart_get_byte(void);
void uart_send_string(unsigned char *str);
void uart_get_string(unsigned char *str);

#endif /* #ifndef __UART_H_ */

#ifndef _BSP_INT_H
#define _BSP_INT_H
#include "imx6ul.h"
/***************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 bsp_int.c
作者	   : 左忠凯
版本	   : V1.0
描述	   : 中断驱动头文件。
其他	   : 无
论坛 	   : www.openedv.com
日志	   : 初版V1.0 2019/1/4 左忠凯创建
***************************************************************/

/* 中断服务函数形式 */
typedef void (*system_irq_handler_t)(unsigned int giccIar, void *param);


/* 中断服务函数结构体*/
typedef struct _sys_irq_handle
{
    system_irq_handler_t irqHandler; /* 中断服务函数 */
    void *userParam;                 /* 中断服务函数参数 */
} sys_irq_handle_t;


/* 函数声明 */
/**
 * [int_init description]
 */
void int_init(void);
/**
 * [system_irqtable_init 初始化中断]
 */
void system_irqtable_init(void);
/**
 * [system_register_irqhandler 注册中断函数]
 * @param irq       [description]
 * @param handler   [description]
 * @param userParam [description]
 */
void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userParam);
/**
 * [system_irqhandler C 语言中断函数 irq 汇编会调用子函数，
 * 此函数通过在中断服务列表中查找指定指定中断号锁对应的中断处理函数进行执行]
 * @param giccIar [description]
 */
void system_irqhandler(unsigned int giccIar);
/**
 * [default_irqhandler description]
 * @param giccIar   [description]
 * @param userParam [description]
 */
void default_irqhandler(unsigned int giccIar, void *userParam);



#endif

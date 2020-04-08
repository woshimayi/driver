#ifndef __MAIN_H
#define __MAIN_H
/*************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 main.h
作者	   : 左忠凯
版本	   : V1.0
描述	   : 时钟GPIO1_IO03相关寄存器地址定义。
其他	   : 无
日志	   : 初版V1.0 2019/1/3 左忠凯创建
*************************************/

/*
 * CCM相关寄存器地址  使能全部时钟寄存器
 */
#define CCM_CCGR0 			*((volatile unsigned int *)0X020C4068)
#define CCM_CCGR1 			*((volatile unsigned int *)0X020C406C)

#define CCM_CCGR2 			*((volatile unsigned int *)0X020C4070)
#define CCM_CCGR3 			*((volatile unsigned int *)0X020C4074)
#define CCM_CCGR4 			*((volatile unsigned int *)0X020C4078)
#define CCM_CCGR5 			*((volatile unsigned int *)0X020C407C)
#define CCM_CCGR6 			*((volatile unsigned int *)0X020C4080)

/*
 * IOMUX相关寄存器地址   GPIO1_3 起始地址
 */
#define SW_MUX_GPIO1_IO00 	*((volatile unsigned int *)0x020E005C)      /* gpio 引脚复用功能 */
#define SW_PAD_GPIO1_IO00 	*((volatile unsigned int *)0x020E02E8)      /* 设置属性 速度 上下拉 输出模式 */

/*
 * GPIO1相关寄存器地址   GPIO1 io口控制寄存器
 * 搜索 GPIO Memory Map/Register Definition
 */
#define GPIO1_DR 			*((volatile unsigned int *)0X0209C000)      //GPIO data register (GPIOx_DR)
#define GPIO1_GDIR 			*((volatile unsigned int *)0X0209C004)      //GPIO direction register (GPIOx_GDIR)
#define GPIO1_PSR 			*((volatile unsigned int *)0X0209C008)      //GPIO pad status register
#define GPIO1_ICR1 			*((volatile unsigned int *)0X0209C00C)      //GPIO interrupt configuration register1 (GPIOx_ICR1)
#define GPIO1_ICR2 			*((volatile unsigned int *)0X0209C010)      //GPIO interrupt configuration register2 (GPIOx_ICR2)
#define GPIO1_IMR 			*((volatile unsigned int *)0X0209C014)      //GPIO interrupt mask register (GPIOx_IMR)
#define GPIO1_ISR 			*((volatile unsigned int *)0X0209C018)      //GPIO interrupt status register (GPIOx_ISR)
#define GPIO1_EDGE_SEL 		*((volatile unsigned int *)0X0209C01C)      //GPIO edge select register (GPIOx_EDGE_SEL)

#endif

/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/call_struct.c
 * @version:
 * @Author: dof
 * @Date: 2023-06-21 13:33:57
 * @LastEditors: dof
 * @LastEditTime: 2023-06-21 13:36:39
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned char uint8_t;

typedef struct
{
	uint8_t duty_cycle;							// 占空比
	void (*set_duty_cycle)(uint8_t duty_cycle); // 设置占空比的函数指针
	void (*start)(void);						// 启动PWM输出的函数指针
	void (*stop)(void);							// 停止PWM输出的函数指针
} pwm_control_t;

// 设置占空比
void set_duty_cycle(uint8_t duty_cycle)
{
	// 设置占空比的代码
	printf("set %d\n", duty_cycle);
}

// 启动PWM输出
void start_pwm(void)
{
	// 启动PWM输出的代码
	printf("start\n");
}

// 停止PWM输出
void stop_pwm(void)
{
	// 停止PWM输出的代码
	printf("stop\n");
}

int main(void)
{
	pwm_control_t pwm;

	pwm.duty_cycle = 50; // 设置占空比为50%
	pwm.set_duty_cycle = set_duty_cycle;
	pwm.start = start_pwm;
	pwm.stop = stop_pwm;

	pwm.set_duty_cycle(pwm.duty_cycle); // 设置占空比
	pwm.start();						// 启动PWM输出

	int i = 0;
	while (1)
	{
		// 循环执行其他任务
		pwm.set_duty_cycle(i++);
	}
}
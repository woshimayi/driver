/*
 * @*************************************:
 * @FilePath: /user/C/string/io_operate.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-04 13:12:20
 * @LastEditors: dof
 * @LastEditTime: 2022-03-04 13:12:24
 * @Descripttion: IO控制小程序
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include "hstGpioAL.h"

int PrintfInputTips(char *ps8Name)
{
	printf("=========== error!!! ========\n\n");
	printf("usage Write: %s GPIO bit value \n", ps8Name);
	printf("usage Read : %s GPIO bit \n", ps8Name);
	printf("eg Write 1 to GPIO1_bit02  :     %s 1 2 1\n", ps8Name);
	printf("eg Read  GPIO1_bit02 Value :     %s 1 2 \n\n", ps8Name);

	printf("=============BT20==================\n")
	printf("USB HUB    GPIO_0_2  1_UP; 0_Down \n");
	printf("RESET_HD   GPIO_13_0 0_EN; 1_disEN\n");
	printf("Power_HD   GPIO_13_3 1_UP; 0_Down \n");
	return 0;
}

int main(int argc, char **argv)
{
	if ((3 != argc) && (4 != argc))
	{
		PrintfInputTips(argv[0]);
		return -1;
	}

	unsigned char l_u8GPIONum = 0;
	unsigned char l_u8GPIOBit = 0;
	unsigned char l_u8SetValue = 0;

	GPIO_GROUP_E l_eGpioGroup;
	GPIO_BIT_E l_eBit;
	GPIO_DATA_E l_eData;

	l_u8GPIONum = atoi(argv[1]);
	l_u8GPIOBit = atoi(argv[2]);

	if (l_u8GPIONum < 14)
	{
		l_eGpioGroup = (GPIO_GROUP_E)l_u8GPIONum;
	}
	else
	{
		printf("l_u8GPIONum error l_u8GPIONum = %d\n", l_u8GPIONum);
		return -1;
	};

	if (l_u8GPIOBit < 8)
	{
		l_eBit = (GPIO_BIT_E)l_u8GPIOBit;
	}
	else
	{
		printf("l_u8GPIOBit error l_u8GPIOBit = %d\n", l_u8GPIOBit);
		return -1;
	}

	if (NULL != argv[3])
	{
		l_u8SetValue = atoi(argv[3]);
		if (0 == l_u8SetValue)
		{
			l_eData = (GPIO_DATA_E)l_u8SetValue;
		}
		else if (1 == l_u8SetValue)
		{
			l_eData = (GPIO_DATA_E)l_u8SetValue;
		}
		else
		{
			printf("l_u8SetValue error l_u8SetValue = %d\n", l_u8SetValue);
		}
	}

	if (3 == argc)
	{
		/**read**/
		printf("read GPIO%d Bit%d \n", l_u8GPIONum, l_u8GPIOBit);
		/**set input**/
		HstGpio_Set_Direction(l_eGpioGroup, l_eBit, GPIO_INPUT);

		/**read **/
		char l_s8bit_val = 0;
		HstGpio_Get_Value(l_eGpioGroup, l_eBit, &l_s8bit_val);

		printf("read Data = %d \n", l_s8bit_val);
	}
	else if (4 == argc)
	{
		/**write**/
		printf("Write GPIO %d; Bit %d; Value %d\n", l_u8GPIONum, l_u8GPIOBit, l_u8SetValue);

		/***set IO output*/
		HstGpio_Set_Direction(l_eGpioGroup, l_eBit, GPIO_OUPUT);

		/**Write To IO**/
		HstGpio_Set_Value(l_eGpioGroup, l_eBit, l_eData);
	}
	else
	{
	}

	return 0;
}
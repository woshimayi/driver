/*
 * @*************************************: 
 * @FilePath: /user/C/string/shift_test_webday.c
 * @version: 
 * @Author: dof
 * @Date: 2022-12-06 14:55:53
 * @LastEditors: dof
 * @LastEditTime: 2022-12-26 13:19:39
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <time.h>


int week_calc(unsigned int weekday, char *retStr, unsigned int len)
{
	unsigned short Mon = 0, Tue = 0, Wed = 0, Thu = 0, Fri = 0, Sat = 0, Sun = 0;
	char cmd[128] = {0};
	char str[128] = {0};
	int j = 0;

	if (weekday == 0x7f)
	{
		return 0;
	}

	for (int i = 0; i < 8; i++)
	{
		switch (i)
		{
		case 0:
			Sun = (weekday >> i & 0x1)?7:0;
			break;
		case 1:
			Mon = (weekday >> i & 0x1)?1:0;
			break;
		case 2:
			Tue = (weekday >> i & 0x1)?2:0;
			break;
		case 3:
			Wed = (weekday >> i & 0x1)?3:0;
			break;
		case 4:
			Thu = (weekday >> i & 0x1)?4:0;
			break;
		case 5:
			Fri = (weekday >> i & 0x1)?5:0;
			break;
		case 6:
			Sat = (weekday >> i & 0x1)?6:0;
			break;
		default:
		break;
		}
	}
	snprintf(cmd, sizeof(cmd), "%d,%d,%d,%d,%d,%d,%d", Mon, Tue, Wed, Thu, Fri, Sat, Sun);
	printf("cmd = %s\n", cmd);

	for (int i = 0; i < strlen(cmd); i++)
	{
		if (cmd[i] == '0')
		{
			i++;
			continue;
		}
		str[j++] = cmd[i];
	}
	printf("cmd = %s, str = %s\n", cmd, str);
	strncpy(retStr, str, len);

	return 0;
}


int week_calc_1(unsigned int weekday, char *retStr, unsigned int len)
{
	unsigned short Mon = 0, Tue = 0, Wed = 0, Thu = 0, Fri = 0, Sat = 0, Sun = 0;
	char cmd[128] = {0};
	char str[128] = {0};
	int j = 0;
	int s = 0;

	if (weekday == 0x7f)
	{
		return 0;
	}

	for (int i = 0; i < 8; i++)
	{
		s = (weekday >> i & 0x1);
		if (!s)
		{
			continue;
		}

		strcat(cmd, (i&&s)?",":"");
		switch (i)
		{
		case 0:
			strcat(cmd, "Sun");
			break;
		case 1:
			strcat(cmd, "Mon");
			break;
		case 2:
			strcat(cmd, "Tue");
			break;
		case 3:
			strcat(cmd, "Wed");
			break;
		case 4:
			strcat(cmd, "Thu");
			break;
		case 5:
			strcat(cmd, "Fri");
			break;
		case 6:
			strcat(cmd, "Sat");
			break;
		default:
		break;
		}
	}
	printf("cmd = %s\n", cmd);
	strncpy(retStr, cmd, len);

	return 0;
}


int main(int argc, char const *argv[])
{
	unsigned int weekday = 0b1111011;
	char cmd[128] = {0};

	week_calc_1(weekday, cmd, sizeof(cmd));
	printf("cmd = %s\n", cmd);

	unsigned int sec = 3800;
	printf("%d:%d:%d\n", sec / 3600, (sec % 3600) / 60, sec%60);


	printf("%s line: %d\n", __FILE__, __LINE__);    
	printf("%s line: %d\n", __FILE__, __LINE__);

	return 0;
}


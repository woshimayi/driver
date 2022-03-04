/*
 * @*************************************:
 * @FilePath: /user/C/string/string_alarm_De-duplication.c
 * @version:
 * @Author: dof
 * @Date: 2022-01-06 17:11:07
 * @LastEditors: dof
 * @LastEditTime: 2022-01-06 17:21:12
 * @Descripttion: De-duplication   字符串 数字去重
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void cmc_getDiffAlarmNumber(const char *newAlarmNumber, const char *oldAlarmNumber, char *alarmNumber, int len)
{
	char newAlarmNumberTmp[128] = {0};
	char oldAlarmNumberTmp[128] = {0};
	char *token = NULL;

	if ((NULL == newAlarmNumber && NULL == oldAlarmNumber) || NULL == alarmNumber)
	{
		printf("Paraments is illegal: <%s|%s|%s>\n", newAlarmNumber, oldAlarmNumber, alarmNumber);
		return;
	}

	strncpy(newAlarmNumberTmp, newAlarmNumber, sizeof(newAlarmNumberTmp));
	strncpy(oldAlarmNumberTmp, oldAlarmNumber, sizeof(oldAlarmNumberTmp));

	token = strtok(newAlarmNumberTmp, ",");
	while (NULL != token)
	{
		if (!strstr(oldAlarmNumberTmp, token))
		{
			strncpy(alarmNumber, token, len);
			return;
		}

		token = strtok(NULL, ",");
	}

	token = NULL;
	strncpy(newAlarmNumberTmp, newAlarmNumber, sizeof(newAlarmNumberTmp));
	strncpy(oldAlarmNumberTmp, oldAlarmNumber, sizeof(oldAlarmNumberTmp));
	token = strtok(oldAlarmNumberTmp, ",");
	while (NULL != token)
	{
		if (!strstr(newAlarmNumberTmp, token))
		{
			strncpy(alarmNumber, token, len);
			return;
		}

		token = strtok(NULL, ",");
	}

	memset(alarmNumber, 0, len);
}



int main(int argc, char const *argv[])
{
	char alarmNumber[8] = {0};
	char *newalarm = "104030,104032,104050,104051,104052,104053,104054,104057,104058";
	char *oldalarm = "104030,104032,104051,104052,104054,104057,104058";

	cmc_getDiffAlarmNumber(newalarm, oldalarm, alarmNumber, sizeof(alarmNumber));


	printf("zzzzzz %s\n", alarmNumber);

	return 0;
}

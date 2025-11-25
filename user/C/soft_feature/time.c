/*
 * @*************************************: 
 * @FilePath     : /user/C/soft_feature/time.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-07-19 14:54:51
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-28 16:17:56
 * @Descripttion : time format transform
 * @compile      :  
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void getTheCurrentTime(char *time_now)
{
	time_t now;
	struct tm *timenow;

	time(&now);
	timenow = localtime(&now);
	snprintf(time_now, 32, "%d%02d%02d%02d%02d%02d", timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday,
			 timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
}

int convert_cst_to_iso(const char *input, char *output, size_t output_size)
{
	if (input == NULL || output == NULL || output_size < 20)
	{
		return -1; // 参数检查
	}

	struct tm tm = {0};
	const char *format = "%b %d %H:%M:%S CST %Y";

	// 解析输入字符串
	if (strptime(input, format, &tm) == NULL)
	{
		// 尝试其他可能的格式变体
		if (strptime(input, "%b %d %H:%M:%S UTC %Y", &tm) == NULL)
		{
			return -1; // 解析失败
		}
	}

	// 格式化为目标字符串
	if (strftime(output, output_size, "%Y-%m-%d %H:%M.%S", &tm) == 0)
	{
		return -1; // 格式化失败
	}

	return 0; // 成功
}

char *convert_cst_to_iso_alloc(const char *input)
{
	if (input == NULL)
	{
		return NULL;
	}

	// 分配足够的内存（YYYY-MM-DD HH:MM.SS + null终止符 = 20字节）
	char *result = (char *)malloc(20 * sizeof(char));
	if (result == NULL)
	{
		return NULL; // 内存分配失败
	}

	if (convert_cst_to_iso(input, result, 20) != 0)
	{
		free(result);
		return NULL; // 转换失败
	}

	return result;
}

int main()
{
	// char *str = NULL;
	// getTheCurrentTime(str);
	//
	// time_t t;
	// char buf[1024];
	// time(&t);
	// printf("%d\n", t.tm_sec);

	// ctime_r(&t, buf);
	// printf("%s\n", buf);

	const char *input = "Aug 28 11:46:45 CST 2025";
	char output[20] = {0}; // 存储结果：YYYY-MM-DD HH:MM.SS
	convert_cst_to_iso(input, output, sizeof(output));

	printf("输入: %s\n", input);
	printf("输出: %s\n", output);

	// printf("%s\n", str);
	return 0;
}
/*
 * @*************************************:
 * @FilePath: /user/C/string/two_ptr.c
 * @version:
 * @Author: dof
 * @Date: 2022-07-05 19:28:04
 * @LastEditors: dof
 * @LastEditTime: 2022-07-05 19:35:07
 * @Descripttion:  二级指针
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const *msg[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Webnesday",
	"Thursday",
	"Friday",
	"Saturday"};

void get_a_day(const char **pp)
{
	static int i = 0;
	printf("%d\n", i);
	*pp = msg[i % 7];
	i++;
}

int main(int argc, char const *argv[])
{
	const char *firstday = NULL;
	const char *secondday = NULL;

	get_a_day(&firstday);
	get_a_day(&secondday);

	printf("%s\v%s\n", firstday, secondday);
	return 0;
}

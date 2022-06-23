/*
 * @*************************************:
 * @FilePath: /user/C/string/curl_popen_sata.c
 * @version:
 * @Author: dof
 * @Date: 2022-06-18 15:43:37
 * @LastEditors: dof
 * @LastEditTime: 2022-06-18 15:51:44
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	char *cmd = "curl -O http://down.360safe.com/setup.exe";

	FILE *fp;
	char buffer[80];
	fp = popen(cmd, "r");
	int ret;
	char buf[128] = {0};

	while ((fgets(buf, sizeof(buf), fp)) > 0) //从out1中读取256个字节数据，存放在buf
	{
		// fwrite(buf, 1, ret, fp);                        //将buf的数据写到int2(此时gerp命令一直在获取int2，直到进行退出)
		printf("%s", buf);
	}

	// fgets(buffer, sizeof(buffer), fp);
	// printf("%s", buffer);
	pclose(fp);

	return 0;
}

/*
 * @*************************************:
 * @FilePath: /user/C/string/strtok_ip.c
 * @version:
 * @Author: dof
 * @Date: 2023-06-05 11:04:59
 * @LastEditors: dof
 * @LastEditTime: 2023-06-05 11:46:47
 * @Descripttion: 分割字符串
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>

int getFileText_for_addr_and_time(const char *path, char array[][64],
								  const char *tm_path, char tm_array[][64])
{
	FILE *fp = NULL;
	char *p = NULL;
	int i = 0;

	FILE *fpt = NULL;
	char *pt = NULL;
	int k = 0;
	char tmp_tm_array[30][64] = {0};

	fp = fopen(path, "r+");
	char fBuf[512] = {0};

	fpt = fopen(tm_path, "r+");
	char tBuf[512] = {0};

	int found = 0;
	int loop = 0;

	if (!fp || !fpt)
	{
		printf("fail to open %s\n", path);
	}
	else
	{
		fgets(tBuf, sizeof(tBuf), fpt);
		fclose(fpt);
		fpt = NULL;
		
		pt = strtok(tBuf, "|");
		while (pt)
		{
			strcpy(tmp_tm_array[k++], pt);
			pt = strtok(NULL, "|");
		}

		pt = NULL;
		k = 0;

		fgets(fBuf, sizeof(fBuf), fp);
		printf("=%s:fBuf:%s,\n", path, fBuf);
		fclose(fp);
		fp = NULL;
		p = strtok(fBuf, "|");
		while (p)
		{
			// for (loop = 0; loop < i; loop++)
			// {
			// 	if ((strcmp(array[loop], p) == 0)
			// 		// && strcmp(array[loop], " ") != 0)
			// 		&& (strcmp(p, " ") != 0))
			// 	{
			// 		found = 1;
			// 		break;
			// 	}
			// }

			if (!found)
			{
				strcpy(tm_array[i], tmp_tm_array[k++]);
				strcpy(array[i++], p);
			}
			else
				k++;

			p = strtok(NULL, "|");
			found = 0;
		}
		//        unlink(path);
		//        unlink(tm_path);
	}
	return i;
}

int main(void)
{
#if 0
	int j, in = 0;
	char buffer[] = "172.16.1.1|172.16.1.1|114.93.248.1|61.152.1.85|61.152.24.78|202.97.29.110|202.97.29.110|202.97.29.110|202.97.29.110|202.97.29.110|222.186.184.150|";
	char *p[20];
	char *buf = buffer;
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;
	while ((p[in] = strtok_r(buf, "|", &outer_ptr)) != NULL)
	{
		buf = p[in];
		while ((p[in] = strtok_r(buf, " ", &inner_ptr)) != NULL)
		{
			in++;
			buf = NULL;
		}
		buf = NULL;
	}
	printf("Here we have %d strings\n", in);
	for (j = 0; j < in; j++)
	{
		printf(">%s<\n", p[j]);
	}
#else
	char mAdress[30][64] = {"0"};
	char mTime[30][64] = {"0"};
	int i = 0;
	i = getFileText_for_addr_and_time("/home/zs/Documents/driver/user/C/string/log_traceroute_pAddr.txt", mAdress,
									  "/home/zs/Documents/driver/user/C/string/log_traceroute_P_Time.txt", mTime);

	printf("i = %d\n", i);

	for (int j = 0; j < i; j++)
	{
		printf("addr = %s, time = %s\n", mAdress[j], mTime[j]);
	}

#endif
	return 0;
}

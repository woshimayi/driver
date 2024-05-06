/*
 * @*************************************:
 * @FilePath: /user/C/fileIO/traceroute_getinfo.cpp
 * @version:
 * @Author: dof
 * @Date: 2024-04-09 16:31:29
 * @LastEditors: dof
 * @LastEditTime: 2024-04-10 13:58:39
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

using namespace std;
// using namespace neb;

int __tracert_resptime(char *pc_instr, int *pui_resp)
{
	int ui_i = 0, ui_count = 0, ui_total = 0;
	char c_err = 0;
	char *pc_pos = pc_instr;
	char *pc_ts = NULL;
	char *pc_te = NULL;
	int em_error = 0;

	while ((pc_te = strstr(pc_pos, "*")) != NULL)
	{
		ui_total++;
		pc_pos = pc_te + strlen("*");
	}

	pc_pos = pc_instr;
	pc_te = NULL;
	for (ui_i = 0; ui_i < 4; ui_i++)
	{
		pc_te = strstr(pc_pos, " ms");
		if (NULL != pc_te)
		{
			pc_pos = pc_te + strlen(" ms");
			*pc_te = '\0';
			pc_ts = pc_te - 1;
			while ((*pc_ts >= '0' && *pc_ts <= '9') || *pc_ts == '.')
			{
				pc_ts--;
			}
			pc_ts++;
			if (*pc_pos == ' ' && *(++pc_pos) == '!')
			{
				switch (*(pc_pos + 1))
				{
				case 'N':
					em_error = -1;
					break;
				case 'H':
					em_error = -1;
					break;
				case 'P':
					em_error = -1;
					break;
				case 'F':
					em_error = -1;
					break;
				case 'S':
					em_error = -1;
					break;
				case ' ':
					em_error = -1;
				case '\0':
					em_error = -1;
					break;
				default:
					break;
				}
			}
			if (em_error != 0)
			{
				printf("em_error=0x%x pc_instr=%s ui_i=%u\n", em_error, pc_instr, ui_i);
				pui_resp[ui_i] = 0xffff;
				ui_count++;
			}
			else
			{
				pui_resp[ui_i] = (int)(atof(pc_ts) + 0.5);
				if (pui_resp[ui_i] == 0)
				{
					pui_resp[ui_i] = 1;
				}
				ui_count++;
			}
		}
	}

	if (ui_count + ui_total > 4)
	{
		return -1;
	}
	for (ui_i = ui_count; ui_i < ui_count + ui_total; ui_i++)
	{
		pui_resp[ui_i] = 0xffff;
	}

	return 0;
}

int  getinfo()
{
	FILE *pf_file = NULL;
	int i_cnt = 0;
	int i_ttl = 0;
	int i_ret = 0;
	char ac_sbuf[256] = {0};
	char ac_target[48] = {0};
	char ac_str1[48] = {0};
	char ac_str2[48] = {0};
	char ac_shoptime[3][64] = {0};
    char ac_resp_host[256] = {0};
    char ac_resp_addr[40] = {0};
	char *pt_s = NULL, *pt_e = NULL;
	int ui_hops_cnt = 0;


	char buf[1024];
    regex_t regex;
    regmatch_t matches[10];
    int i;

	snprintf(ac_sbuf, sizeof(ac_sbuf), "bucpe_traceroute");
	if (NULL == (pf_file = fopen(ac_sbuf, "r")))
	{
		perror("");
		return -1;
	}

	ac_sbuf[0] = 0;
	if (NULL != fgets(ac_sbuf, 256, pf_file))
	{
		if (strstr(ac_sbuf, "bad address"))
		{
			fclose(pf_file);
			return 0;
		}
		if (strstr(ac_sbuf, "traceroute"))
		{
			/* ���� */
			sscanf(ac_sbuf, "%*[^(](%[^)])", ac_target, sizeof(ac_target));
		}
	}

	while (NULL != fgets(ac_sbuf, 256, pf_file))
	{
		memset(ac_str1, 0, sizeof(ac_str1));
		memset(ac_str2, 0, sizeof(ac_str2));
		memset(ac_shoptime, 0, sizeof(ac_shoptime));
		// pt_s = strstr(ac_sbuf, "(");
		// if (pt_s != NULL)
		// {
		// 	// pt_e = strstr(pt_s, ")");
		// 	i_ret = sscanf(ac_sbuf, "%d %[^ ] (%[^)]) %[^\n]", &i_ttl, ac_str1, ac_str2, ac_shoptime);
		// }
		// else
		{
			// i_ret = sscanf(ac_sbuf, "%d %[^\n]", &i_ttl, ac_shoptime, sizeof(ac_shoptime));
			// if (4 == sscanf(ac_sbuf, "%d $s $s %[^\n]", &i_ttl, ac_str1, ac_shoptime[0], ac_shoptime[1], ac_shoptime[2]))
			// {
			// 	printf("3\n");
			// }
			// else if (5 == sscanf(ac_sbuf, "%d $s $s $s %[^\n]", &i_ttl, ac_str1, ac_shoptime[0], ac_shoptime[1], ac_shoptime[2]))
			// {
			// 	printf("4\n");
			// }
		}

		{
			// 编译正则表达式
			regcomp(&regex, "\\s+", 0);

			// 执行正则表达式匹配
			int n = regexec(&regex, buf, 10, matches, 0);

			// 遍历匹配结果
			for (i = 0; i < n; i++) {
				// 打印匹配的子字符串
				printf("%.*s\n", matches[i].rm_eo - matches[i].rm_so, buf + matches[i].rm_so);
			}

			// 释放正则表达式
			regfree(&regex);

		}

		// printf("dRet=%d sBuf=%s \vTTL=%d host=%s addr=%s time=%s\n", i_ret, ac_sbuf, i_ttl, ac_str1, ac_str2, ac_shoptime);

		printf("TTL=%d \thost=%s \ttime=%s\n",i_ttl, ac_str1, ac_shoptime[0]);

		// if (pt_s != NULL && pt_e != NULL)
		// {
		// 	*pt_e = '\0';
		// 	pt_s++;
		// 	strncpy(ac_str2, pt_s, 48);
		// }
		// int aui_resp_time[4] = {0};
		// i_ret = __tracert_resptime(ac_shoptime, aui_resp_time);
		// if (i_ret < 0)
		// {
		// 	continue;
		// }

		strncpy(ac_resp_host, ac_str1, 256);
		strncpy(ac_resp_addr, ac_str2, sizeof(ac_resp_addr) - 1);

		// printf("ac_resp_host = %s ac_resp_addr = %s\n", ac_resp_host, ac_resp_addr);

		// pst_hop++;
		// i_cnt++;
		// if (0 == strcmp(ac_str2, ac_target))
		// {
		// 	printf("Reached TTL=%d dCnt=%d Target=%s\n", i_ttl, i_cnt, ac_str2);
		// 	break;
		// }
	}
	ui_hops_cnt = i_cnt;
	fclose(pf_file);

	return 0;
}

int main(int argc, char const *argv[])
{
	getinfo();
	return 0;
}

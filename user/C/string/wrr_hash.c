/*
 * @*************************************:
 * @FilePath: /user/C/string/wrr_hash.c
 * @version:
 * @Author: dof
 * @Date: 2023-06-19 16:25:49
 * @LastEditors: dof
 * @LastEditTime: 2023-06-26 14:46:47
 * @Descripttion: wrr 根据hash 表索引, 调整全部队列的权重, 保证所有队列数据全部可以流通
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
	int queue_nbr;
	int weight[8];
} qos_sched_cfg;

int main(int argc, char const *argv[])
{
	int weight[8] = {0, 0, 0, 0, 0, 0, 0, 100};
	int idx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	int maxWeight = 0;
	int maxIndex = 0;
	int total_zero = 0;
	qos_sched_cfg sched_cfg;
	memset(&sched_cfg, 1, sizeof(sched_cfg));

	for (int k = 0; k < 8; k++)
	{
		sched_cfg.weight[k] = weight[k];
		if (!weight[k])
		{
			total_zero++;
			sched_cfg.weight[k] = 1;
		}
	}

	int wei = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8 - 1 - i; j++)
		{
			if (weight[j] < weight[j + 1])
			{
				wei = weight[j];
				weight[j] = weight[j + 1];
				weight[j + 1] = wei;

				wei = idx[j];
				idx[j] = idx[j + 1];
				idx[j + 1] = wei;
			}
		}
	}

	for (int i = 0; i < 8; i++)
	{
		printf("%4d", weight[i]);
		printf("%4d\n", idx[i]);
	}

	for (int n = 0; n < total_zero; n++)
	{
		sched_cfg.weight[idx[n%(8-total_zero)]] -= 1;
		printf("%4d %4d %4d %4d %4d\n", n, weight[n], sched_cfg.weight[idx[n]], idx[n%(8-total_zero)], n%(8-total_zero));
	}

	int sum = 0;
	printf("\n");
	for (int j = 0; j < 8; j++)
	{
		printf("zzzzz %d|%d\n", j, sched_cfg.weight[j]);
		sum += sched_cfg.weight[j];
	}
	printf("zzzzz sum = %d\n", sum);

	return 0;
}

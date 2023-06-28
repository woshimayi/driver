/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/twosum.c
 * @version:
 * @Author: dof
 * @Date: 2023-06-21 17:20:33
 * @LastEditors: dof
 * @LastEditTime: 2023-06-21 18:09:04
 * @Descripttion:  two sum
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int *twosum(int *nums, int numsize, int target, int *retureSize)
{
	int *result = (int *)malloc(sizeof(int) * 2);
	result[0] = 0;
	result[1] = 0;

	for (int i = 0; i < numsize - 1; i++)
	{
		for (int j = i + 1; j < numsize; j++)
		{
			if (nums[i] + nums[j] == target)
			{
				printf("%d|%d %4d\n", i, j, nums[i] + nums[j]);
				result[0] = i;
				result[1] = j;
				printf("%4d%4d\n", result[0], result[1]);
				return result;
			}
		}
	}
}

int *twosum_1(int *nums, int numsize, int target, int *ret)
{
	// int *result = (int *)malloc(sizeof(int) * 2);
	// result[0] = 0;
	// result[1] = 0;

	for (int i = 0; i < numsize - 1; i++)
	{
		for (int j = i + 1; j < numsize; j++)
		{
			if (nums[i] + nums[j] == target)
			{
				printf("%d|%d %4d\n", i, j, nums[i] + nums[j]);
				ret[0] = i;
				ret[1] = j;
				printf("%4d%4d\n", ret[0], ret[1]);
				return 0;
			}
		}
	}
}

int main(int argc, char const *argv[])
{
	int nums[] = {1, 7, 11, 15, 6};
	int target = 17;
	int idx = 0;
	int count = sizeof(nums) / sizeof(int);
	int res[2] = {0};
	int *result = &res[0];

	// result = twosum_1(nums, count, target, res);
	twosum_1(nums, count, target, result);
	// for (int i = 0; i < sizeof(res) / sizeof(int); i++)
	{
		printf("%4d%4d%4d%4d\n", res[0], res[1], result[0], result[1]);
		printf("%p %p %p %p\n", &res[0], &res[1], result, result + 1);
	}
	return 0;
}

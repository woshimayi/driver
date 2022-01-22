/*
 * @*************************************:
 * @FilePath: /user/C/string/sort-1.c
 * @version:
 * @Author: dof
 * @Date: 2021-12-30 16:12:21
 * @LastEditors: dof
 * @LastEditTime: 2021-12-30 16:23:27
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

//交换数值
void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

//打印数组
void printArray(char msg[], int arr[], int len)
{
	printf("%s:", msg);
	for (int i = 0; i < len; i++)
	{
		printf("%4d ", arr[i]);
	}
	printf("\n");
}

//二分法插入排序
void binary_insert(int a[], int len)
{
	for (int i = 1; i <= len - 1; i++)
	{
		int left = 0;
		int right = i - 1;
		int temp = a[i];
		//如果a[i]比最大值还大或者相等，则不交换位置,跳过二分查找
		if (temp >= a[right])
		{
			continue;
		}

		// left>right时循环终止
		while (left <= right)
		{
			//折半
			int mid = (left + right) / 2;
			if (temp < a[mid])
			{
				right = mid - 1;
			}
			else
			{
				left = mid + 1;
			}
		}

		printf("要插入的数据：a[%d]=%d\n", i, temp);
		printArray("插入数据前", a, len);
		//大于等于left的位置后移一位
		for (int j = i - 1; j >= left; j--)
		{
			a[j + 1] = a[j];
		}
		a[left] = temp;
		printArray("插入数据后", a, len);
		printf("\n");
	}
}
int main()
{
	int len = 10;
	int arr[len];
	srand((unsigned)time(NULL));
	//随机生成长度为"len"的数组
	for (int i = 0; i < len; i++)
	{
		arr[i] = rand() % 200;
	}
	printArray("未排序前", arr, len);
	binary_insert(arr, len);
	printArray("二分法插入排序", arr, len);
	//防止windows下控制台窗口闪退
	int s;
	scanf("%d", &s);
	return 0;
}
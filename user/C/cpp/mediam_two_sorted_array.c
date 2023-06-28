/*
 * @*************************************:
 * @FilePath: /user/C/cpp/mediam_two_sorted_array.c
 * @version:
 * @Author: dof
 * @Date: 2023-06-21 18:11:20
 * @LastEditors: dof
 * @LastEditTime: 2023-06-21 19:34:14
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

double findMedianSortedArrays(int *nums1, int n, int *nums2, int m)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int max = m + n;
	int *merged_array;
	merged_array = malloc(sizeof(int) * max);
	int median_pos = max / 2;
	while (i < n || j < m && k <= median_pos)
	{
		if (i == n)
		{
			merged_array[k++] = nums2[j++];
		}
		else if (j == m)
		{
			merged_array[k++] = nums1[i++];
		}
		else if (nums1[i] < nums2[j])
		{
			merged_array[k++] = nums1[i++];
		}
		else
		{
			merged_array[k++] = nums2[j++];
		}
	}
	double median;
	if (max % 2)
	{
		median = (double)merged_array[median_pos];
	}
	else
	{
		median = (double)(merged_array[median_pos - 1] + merged_array[median_pos]) / 2;
	}
	free(merged_array);
	return median;
}

int main(int argc, char const *argv[])
{
	int nums[] = {3, 6, 9};
	int nums2[] = {4, 7, 10};
	double i = 0;

	i = findMedianSortedArrays(nums, 2, nums2, 2);

	return 0;
}

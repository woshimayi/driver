#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int *twoSum(int *nums, int numsSize, int target, int *returnSize)
{
	int i = 0;
	int j = 0;
	for (i = 0; i < numsSize; i++)
	{
		for (j = i + 1; j < numsSize; j++)
		{
			printf("i = %d j = %d sum = %d\n", nums[i], nums[j], nums[i] + nums[j]);
			if (nums[i] + nums[j] == target)
			{
				int *ret = malloc(sizeof(int) * 2);
				ret[0] = i;
				ret[1] = j;
				*returnSize = 2;
				printf("return i = %d j = %d sum = %d\n", nums[i], nums[j], nums[i] + nums[j]);
				return ret;
			}
		}
	}
	*returnSize = 0;
	return NULL;
}

#if 0
#include <uthash.h>

struct hashTable
{
	int key;
	int val;
	UT_hash_handle hh;
};

struct hashTable *hashtable;

struct hashTable *find(int ikey)
{
	struct hashTable *tmp;
	HASH_FIND_INT(hashtable, &ikey, tmp);
	return tmp;
}

void insert(int ikey, int ival)
{
	struct hashTable *it = find(ikey);
	if (it == NULL)
	{
		struct hashTable *tmp = malloc(sizeof(struct hashTable));
		tmp->key = ikey, tmp->val = ival;
		HASH_ADD_INT(hashtable, key, tmp);
	}
	else
	{
		it->val = ival;
	}
}

int *twoSum(int *nums, int numsSize, int target, int *returnSize)
{
	hashtable = NULL;
	for (int i = 0; i < numsSize; i++)
	{
		struct hashTable *it = find(target - nums[i]);
		if (it != NULL)
		{
			int *ret = malloc(sizeof(int) * 2);
			ret[0] = it->val, ret[1] = i;
			*returnSize = 2;
			return ret;
		}
		insert(nums[i], i);
	}
	*returnSize = 0;
	return NULL;
}
#endif

int main(int argc, char const *argv[])
{
	int nums[] = {2, 3, 5, 7, 9, 13};
	int target = 10;
	int returnSize = 0;
	twoSum(nums, sizeof(nums) / sizeof(int), target, &returnSize);
	printf("%d\n", target);
	return 0;
}


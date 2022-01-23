/*
 * @*************************************:
 * @FilePath: /user/C/time/hash_list.c
 * @version:
 * @Author: dof
 * @Date: 2022-01-22 17:32:40
 * @LastEditors: dof
 * @LastEditTime: 2022-01-22 17:45:46
 * @Descripttion:  散列 实例
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define HASHSIZE 12 //定义散列表长为数组的长度
#define NULLKEY -32768

typedef struct HashTable
{
	int *elem; //数据元素存储基址，动态分配数组
	int count; //当前数据元素个数
} HashTable;

int numHash = 0; //散列表长，全局变量

/* 初始化散列表 */
void InitHashTable(HashTable *H)
{
	int i;
	numHash = HASHSIZE;
	H->count = numHash;
	H->elem = (int *)malloc(numHash * sizeof(int));
	for (i = 0; i < numHash; i++)
		H->elem[i] = NULLKEY;
}

/* 用到的散列函数 */
int Hash(int key)
{
	return key % numHash; //除留余数法
}

/* 插入关键字进散列表 */
void InsertHash(HashTable *H, int key)
{
	int addr = Hash(key);			 //求散列地址
	while (H->elem[addr] != NULLKEY) //如果不是默认值，说明有数值存在
		addr = (addr + 1) % numHash; //开放定址法的线性探测
	H->elem[addr] = key;			 //找到位置后插入
}

/* 查找关键字 */
bool SearchKeyInHash(HashTable H, int key, int *addr)
{
	*addr = Hash(key); //先求出散列地址
	while (H.elem[*addr] != key)
	{														//不是关键字，说明被其他key占用了
		*addr = (*addr + 1) % numHash;						//一个一个找
		if (H.elem[*addr] == NULLKEY || *addr == Hash(key)) //如果回到原点，或者找到了默认值，说明表里没有这个key
			return false;
	}
	return true;
}

/* 打印哈希表 */
void printHashTable(HashTable H)
{
	int i;
	for (i = 0; i < numHash; i++)
		printf("%d ", H.elem[i]);
	printf("\n");
}

int main()
{
	int i;
	HashTable *H = (HashTable *)malloc(sizeof(HashTable));
	int a[64] = {1, 13, 2, 3, 4, 5, 6, 15, 20, 40};
	InitHashTable(H);
	printHashTable(*H);
	for (i = 0; i < 10; i++)
		InsertHash(H, a[i]);
	printf("hash list: ");
	printHashTable(*H);
	//查找
	if (SearchKeyInHash(*H, 12, &i))
		printf("找到了！\n");
	else
		printf("关键字不存在！\n");

	free(H);
	return 0;
}

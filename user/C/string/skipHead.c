/*
 * @*************************************:
 * @FilePath: /uboot-2016_03/home/zs/Documents/driver/user/C/string/skipHead.c
 * @version:
 * @Author: dof
 * @Date: 2023-06-16 19:38:10
 * @LastEditors: dof
 * @LastEditTime: 2023-06-16 19:39:27
 * @Descripttion: skip tables  list + index
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int MaxLevel = 8;  // 最大层数
int currLevel = 0; // 当前层数

// 数据节点
typedef struct
{
	int data;
	struct Node *next;
	struct Node *last;
	struct Node *up;
	struct Node *down;
} Node;

// 记录索引寻址过程
typedef struct
{
	int level;
	struct Node *node;
} skipStep;

// 判断是否需要新增索引, 抛硬币
bool randNum()
{
	if (0 == (rand() % 2))
		return true;

	return false;
}

// 新增节点
bool add(Node *SL[], int data)
{

	printf("新增节点: %d\n", data);
	int level = currLevel;

	Node *Head = NULL;
	Node *tmp = NULL;
	Node *last = NULL;

	// 初始化索引 数据为 Head 地址
	skipStep steps[MaxLevel];
	int i;
	for (i = 0; i < MaxLevel; i++)
	{
		steps[i].level = 0;
		steps[i].node = SL[i];
		Node *ss = steps[i].node;
	}

	// 赛选无效层
	Head = SL[level];
	tmp = Head->next;

	while ((level > 0) && (data < tmp->data))
	{
		level--;
		Head = SL[level];
		tmp = Head->next;
	}

	// 根据索引寻找Head0数据节点
	while ((level > 0))
	{

		while (tmp != NULL)
		{
			if (data < tmp->data)
			{
				steps[level].level = level;
				if (NULL != last)
					steps[level].node = last;
				tmp = last->down;
				level--;
				break;
			}

			last = tmp;
			tmp = tmp->next;
		}
		if (NULL == tmp)
		{
			steps[level].level = level;
			if (NULL != last)
				steps[level].node = last;
			tmp = last->down;
			level--;
		}
	}

	// Head0 数据合适的节点
	while (tmp != NULL)
	{
		if (data < tmp->data)
		{
			break;
		}
		last = tmp;
		tmp = tmp->next;
	}

	// 新增节点
	Node *newData = (Node *)malloc(sizeof(Node));
	newData->data = data;
	newData->up = NULL;
	newData->down = NULL;
	newData->last = NULL;
	newData->next = NULL;

	int k = 0;

	// Head0 插入原始数据
	if (NULL == last)
	{
		// 头结点

		Head = SL[0];
		Node *headNext = Head->next;
		if (NULL != headNext)
		{
			newData->next = headNext;
			headNext->last = newData;

			newData->last = Head;
		}

		Head->next = newData;
		newData->last = Head;
	}
	else if (NULL == tmp)
	{
		// 尾节点
		last->next = newData;
		newData->last = last;
	}
	else
	{
		// 中间节点
		newData->next = tmp;
		tmp->last = newData;

		newData->last = last;
		last->next = newData;
	}

	// 构建索引
	while (randNum())
	{
		k++;
		if (k >= MaxLevel)
			break;

		// 新增索引数据
		Node *newIndex = (Node *)malloc(sizeof(Node));
		newIndex->data = data;
		newIndex->up = NULL;
		newIndex->down = NULL;
		newIndex->next = NULL;
		newIndex->last = NULL;

		// 建立上下级关系
		newIndex->down = newData;
		newData->up = newIndex;

		Node *node = steps[k].node;

		// node->next
		Node *nextIndex = node->next;

		node->next = newIndex;
		newIndex->last = node;

		newIndex->next = nextIndex;
		if (NULL != nextIndex)
			nextIndex->last = newIndex;

		newData = newIndex;

		// 判断是否需要新增索引层数
		if (k > currLevel)
			currLevel = k;
	}
}

// 初始化头结点
Node *initSkipList(Node *skipList[])
{
	int i;
	for (i = 0; i < MaxLevel; i++)
	{
		Node *newHead = (Node *)malloc(sizeof(Node));
		if (NULL == newHead)
		{
			printf("%d 层 头结点申请失败\n");
			return NULL;
		}
		newHead->data = -1 - i;
		newHead->down = NULL;
		newHead->up = NULL;
		newHead->next = NULL;
		newHead->last = NULL;

		skipList[i] = newHead;
	}
	return skipList;
}

// 打印跳表数据
void PrintSkipList(Node *SL[])
{
	if (NULL == SL)
	{
		return;
	};

	int level = currLevel;
	// int level = MaxLevel;

	int i;
	for (i = level; i >= 0; i--)
	{
		Node *Head = SL[i];

		Node *tmp = Head->next;
		printf("第%d层\t\t", i);
		while (NULL != tmp)
		{
			printf(" %d\t", tmp->data);

			tmp = tmp->next;
		}
		printf("\n");
	}
}

// 查询数据
Node *query(Node *SL[], int data)
{
	printf("查询数据: %d\n", data);

	int level = currLevel;

	Node *Head = NULL;
	Node *tmp = NULL;
	Node *last = NULL;

	Head = SL[level];
	tmp = Head->next;
	int endQuery = -1;

	// 筛除无效层
	while ((level > 0) && (data < tmp->data))
	{
		level--;
		endQuery = tmp->data;
		Head = SL[level];
		tmp = Head->next;
	}

	// 根据索引定位到Head0层
	while ((level > 0))
	{

		while (tmp != NULL)
		{
			if (data < (tmp->data))
			{
				level--;
				endQuery = tmp->data;
				tmp = last->down;
				break;
			}

			last = tmp;
			tmp = tmp->next;
		}
		if (NULL == tmp)
		{
			tmp = last->down;
			endQuery = -1;
			level--;
		}
	}

	// 查询实际数据
	while (NULL != tmp)
	{
		if (endQuery != -1)
			if (tmp->data > endQuery)
			{
				tmp = NULL;
				break;
			}
		if (tmp->data == data)
		{
			break;
		}
		tmp = tmp->next;
	}
	// 返回查询的数据节点，若没有查询到，应当返回NULL ,否则返回实际的地址
	return tmp;
}

// 删除数据
bool del(Node *SL[], int data)
{
	printf("删除数据: %d\n", data);

	// 找到节点地址
	Node *tmp = query(SL, data);

	if (NULL == tmp)
	{
		printf("未找到节点,删除失败\n");
		return false;
	}
	int level = 0;
	Node *t_last = NULL;
	Node *t_next = NULL;

	// 找到该数据最高索引
	while (NULL != tmp->up)
	{
		level++;
		tmp = tmp->up;
	}

	// 由上至下删除索引/数据
	while (tmp != NULL)
	{
		t_last = tmp->last;
		t_next = tmp->next;

		Node *t_down = tmp->down;

		if (t_last == NULL)
		{
			printf("上一个节点不可能为空，删除失败,层数: %d\n", level);
			return false;
		}

		t_last->next = t_next;

		if (NULL != t_next)
			t_next->last = t_last;
		else
			t_last->next = NULL;

		if ((t_last == SL[level]) && (NULL == t_next))
		{
			currLevel--;
		}
		free(tmp);

		tmp = t_down;
		level--;
	}

	return true;
}

int main()
{

	Node *SL[MaxLevel];

	Node *skipList = initSkipList(SL);
	if (NULL == SL)
	{
		printf("skipList 申请失败\n");
		return -1;
	}

	// 测试新增
	int num[] = {1, 3, 2, 10, 8, 9, 22, 30, 29, 120, 99, 78, 55, 76, 21};
	int i;
	for (i = 0; i < sizeof(num) / sizeof(int); i++)
	{
		add(skipList, num[i]);
	}
	PrintSkipList(SL);

	// 测试删除
	int delNum[] = {99, 9, 78, 55, 3, 1, 28, 78};
	for (i = 0; i < sizeof(delNum) / sizeof(int); i++)
	{
		del(skipList, delNum[i]);
	}
	PrintSkipList(SL);

	printf("\n");
	return 0;
}
/*
 * @*************************************:
 * @FilePath: /user/C/string/hasg_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-09-15 13:27:39
 * @LastEditors: dof
 * @LastEditTime: 2023-09-15 13:27:40
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>

#define N 13		// 余数：也是指针数组的元素个数：也是链表的个数
#define ADDR_SIZE 8 // 这个定义的是 指针数组的 每个指针的大小，(64位系统)8个字节

// hash表 链表的节点
typedef struct node
{
	int data;		   // 存数据
	struct node *next; // 存指针
} HASH;				   // 类型重命名-->HASH

// 创建hash表(创建了十三个链表头节点)
HASH **create_hash()
{
	// 申请创建一个指针数组，存13个头节点
	// 先创建一个指针数组，指针数组可以在栈区申请（int *h[]），但是当前函数结束会被释放
	// 所以在堆区申请空间，指针数组的返回值 是二级指针 所以用HASH **h来接收
	// 这块申请了一个 有13个位置的指针数组
	HASH **h = (HASH **)malloc(N * ADDR_SIZE); //(返回值类型)malloc(申请的空间的大小)
	int i = 0;
	// 这块要填充这个指针数组
	// 创建链表的头结点--先malloc申请出来一个头节点，分别把他们放到指针数组h[0]--h[12] 里
	for (i = 0; i < N; i++)
	{
		h[i] = (struct node *)malloc(sizeof(struct node)); // 创建头节点
		h[i]->next = NULL;								   // 初始化头节点 指针域
	}													   // 注意：再此申请的节点 都被保存到 （指针数组）h 里了
	return h;
}

// 插入数据
int insert_hash_table(HASH **h, int data) // 参数：指针数组，需要插入的数据
{

	// 然后将'需要插入的数据'对'质数13取余'--确定好数据对应的 指针数组下标
	// 找到指定的链表
	int key = data % N;

	// 根据指针数组的下标，确定对应的链表的头节点，
	// 定义了一个结构体指针变量p 指向 指针数组的第[key]位对应的 链表
	struct node *p = h[key]; // 也可以直接操作h[key],定义一个指针好理解点

	//--头插法--插入数据
	struct node *temp;								   // 定义了一个 结构体指针变量
	temp = (struct node *)malloc(sizeof(struct node)); // malloc申请空间
	temp->data = data;								   // 初始化一下

	// 头插法 插入
	temp->next = p->next; // 新定义节点的指针域 指向 头节点的下一个节点
	p->next = temp;		  // 头节点的指针域，指向新定义的节点

	return 0;
}

// 打印排好的hash表--遍历
int show_hash_table(struct node *head) // 参数：对应链表的头节点--main函数多次调用
{
	// 如果链表后面没有数据，则用---0---表示链表存在但是没有数据
	if (head->next == NULL)
	{
		puts("---0---");
		return -1;
	}

	// 如果链表后面有数据，遍历链表，打印数据
	while (head->next != NULL)
	{
		// 由于头节点没有数据，所以，先移动指针，然后输出数据
		head = head->next;
		printf("%d  ", head->data);
	}
	putchar(10); // 输出个换行符
	return 0;
}
// 释放链表节点
int free_hash_table(struct node *head)
{
	// 如果链表后面没有数据，则无需释放
	if (head->next == NULL)
	{
		return 0;
	}

	// 遍历这个链表-头删法释放
	while (head->next != NULL)
	{
		// 定义一个结构体指针变量 来指向这个即将被删除的结构体 以便释放
		struct node *temp = head->next;
		head->next = head->next->next; // 改变头结点指针域指向，删除节点
		printf("--%d--将被释放\n", temp->data);
		free(temp);	 // 释放
		temp = NULL; // 置空（防止被别的函数修改）
	}
	return 0;
}
// 查找数据
int search_hash_table(HASH **h, int data) // 参数：指针数组，需要查找的数据
{
	int key = data % N;		 // 先把要查找的数据对 质数 取余，得到对应的下标
	struct node *p = h[key]; // 根据下标找到对应链表，定义了一个结构体指针变量p，指向该链表

	//--循环遍历--对比--
	// 循环遍历的结束条件是，p->next 域 为空(NULL)
	while (p->next != NULL)
	{
		if (p->next->data == data)
		{
			return 1; // 找到返回1
		}
		p = p->next; // 移动指针
	}

	// 没有找到返回0
	return 0;
}

// 程序的入口：
// 假定数组有11个元素--> 11/0.75 ==> 14.67 ==> 最大质数 为 13
int main(int argc, const char *argv[])
{
	int a[11] = {100, 34, 14, 45, 46, 98, 68, 69, 7, 31, 26};
	// 直接初始化了11个数值的数组

	// 创建hash表
	HASH **h = create_hash(); // 为啥用二级指针：指针数组的返回值是二级指针

	// 将数据按照格式插入到链表中
	int i = 0;
	int num = 0;

	// 链表增加--多次调用-插入数组a的每个元素
	for (i = 0; i < 11; i++) // 给 a[i] 使的
	{
		insert_hash_table(h, a[i]); // 链表的插入
	}

	printf("-------这是hash--------------------\n");
	// 打印hash表--打印每个指针数组元素所存储的链表
	for (i = 0; i < N; i++)
	{
		show_hash_table(h[i]); // 链表的遍历
	}

	printf("--------hash表结束--------------------\n");
	printf("数组数据如下-->用于测试，无实质意义,遍历HASH表也是<---\n");
	for (i = 0; i < 11; i++)
	{
		printf("%d  ", a[i]);
	}
	putchar(10);

	//  while(1)
	//  {
	// 查找
	printf("please input need 查找 de number >>");
	scanf("%d", &num);
	// 由于输入字母，会造成死循环，所以也可以用char类型定义，或者加个判断（ASCII码）
	// 指定数据判断是否存在-----查找
	if (search_hash_table(h, num) == 1)
	{
		printf("---data %d is exists---\n", num);
	}
	else
	{
		printf("---data %d is not exists---\n", num);
	}
	//  }

	// 链表的释放
	for (i = 0; i < 11; i++)
	{
		free_hash_table(h[i]);
	}
	printf("---链表释放完成---\n");
	free(h);
	printf("---指针数组释放---\n");

	return 0;
}
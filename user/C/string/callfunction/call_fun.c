/*
 * @*************************************:
 * @FilePath: /user/C/string/call_fun.c
 * @version:
 * @Author: dof
 * @Date: 2022-07-06 09:44:55
 * @LastEditors: dof
 * @LastEditTime: 2022-07-06 14:29:10
 * @Descripttion: 回调函数
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0

/**
 * @brief 回调函数定义类型
 * 
 */
typedef void (*callback_t)(void *);

/**
 * @brief 
 * 
 */
// void repeat_three_times(callback_t, void *);

/**
 * @brief 
 * 
 * @param f 回调函数类型
 * @param para 回调函数 传入参数
 */
void repeat_three_times(callback_t f, void *para)
{
	f(para);
	f(para);
	f(para);
}

/**
 * @brief 
 * 
 * @param str 
 */
void say_hello(void *str)
{
	printf("Hello %s\n", (const char *)str);
}

/**
 * @brief 
 * 
 * @param num 
 */
void count_numbers(void *num)
{
	int i = 0;
	for (i = 0; i < (int)num; i++)
	{
		printf("%4d ", i);
	}
	putchar('\n');
}

int main(int argc, char const *argv[])
{
	repeat_three_times(say_hello, "sss");
	repeat_three_times(count_numbers, (void *)4);
	return 0;
}
#elif 0

// 同步回调

typedef struct 
{
	const char *name;
	int score;
} student_t;

typedef int (*cmp_t)(void *, void *);


/**
 * @brief 回调函数执行机构
 * 
 * @param data 数据
 * @param num  数据大小
 * @param cmp  操作函数
 * @return void* 
 */
void *max(void *data[], int num, cmp_t cmp)
{
	int i = 0;

	void *temp = data[0];
	for (i = 1; i < num; i++)
	{
		if (cmp(temp, data[i]) < 0)
		{
			temp = data[i];
			printf("temp = %d\n", ((student_t *)temp)->score);
		}
	}

	return temp;
}

/**
 * @brief 具体类型数据比较
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int cmp_student(void *a, void *b)
{
	if ( ((student_t *)a)->score >  ((student_t *)b)->score )
	{
		return 1;
	}
	else if (((student_t *)a)->score ==  ((student_t *)b)->score)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int main(int argc, char const *argv[])
{
	student_t list[4] = {
		{"Tom", 60},
		{"Jerray", 72},
		{"Moby", 60},
		{"Kitay", 89}
	};

	student_t *plist[4] = {&list[0], &list[1], &list[2], &list[3]};
	student_t *pmax = max((void **)&plist, 4, cmp_student);

	printf("%s gets the highest score %d\n", pmax->name, pmax->score);

	return 0;
}

#elif 1
//  异步回调 加入线程

#pragma comment(lib, "pthread")

#include <pthread.h>
#include <unistd.h>

typedef void (*pcb)(int a);

typedef struct paraneter
{
	int a;
	pcb callbck;
} parameter;


/**
 * @brief 主线程函数 分支线程
 * 
 * @param p1 
 * @return void* 
 */
void *callback_thread(void * p1)
{
	parameter *p = (parameter *)p1;

	while (1)
	{
		printf("GetCallback print\n\n");
		sleep(1);
		
		p->callbck(p->a);
	}
}

/**
 * @brief Set the Call Back Fun object 主线程处理函数
 * 
 * @param a 
 * @param callback 
 */
void setCallBackFun(int a, pcb callback)
{

	printf("setcallbackfun print\n");
	parameter *p = (parameter *)malloc(sizeof(parameter));
	p->a = 10;
	p->callbck = callback;

	pthread_t thing1;
	pthread_create(&thing1, NULL, callback_thread, (void *)p);
	pthread_join(thing1, NULL);
}


/**
 * @brief 主线程 次线程 回调函数
 * 
 * @param a 
 */
void fCallBack(int a)
{
	printf("a = %d\n", a);
	printf("fCallBack print\n");
}


/**
 * @brief 主函数
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
	setCallBackFun(4, fCallBack);

	return 0;
}


#else


#endif
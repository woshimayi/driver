/*
 * @*************************************: 
 * @FilePath     : /user/C/constructor_test.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-11-22 11:36:46
 * @LastEditors  : dof
 * @LastEditTime : 2024-11-22 11:38:19
 * @Descripttion :  __attribute__可用于为函数或者数据声明赋属性值。给函数分配属性的主要目的是为了让编译程序可以优化处理。分配给函数的属性位于函数原型的声明中
 *                  __attribute__((constructor)) 先于main()函数调用
 *                  __attribute__((destructor))在main()函数后调用
 * @compile      :  
 * @**************************************: 
 */


#if 0
#include <stdio.h>
#include <stdlib.h>

static void before(void) __attribute__((constructor));
static void after(void) __attribute__((destructor));

static void before()
{
    printf("before main\n");
}

static void after(void)
{
    printf("after main\n");
}

int main()
{

    printf("main\n");
    return 0;
}

#else

// 通过参数设置优先级关系 
#include <stdio.h>
#include <stdlib.h>
 
static void before(void) __attribute__((constructor));
 
static void before3(void) __attribute__((constructor(103)));
static void before2(void) __attribute__((constructor(102)));
static void before1(void) __attribute__((constructor(101)));
 
static void before2()
{
	printf("before  102\n");
}
 
static void before1()
{
	printf("before  101\n");
}
 
static void before3()
{
	printf("before  103\n");
}
 
static void before()
{
	printf("before main\n");
}
 
 
int main()
{	
	printf("main\n");
	return 0;
}

#endif
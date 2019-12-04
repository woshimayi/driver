#include<stdio.h>
//方法指针的格式为：int (*ptr)(char *p) 即：返回值(指针名)(参数列表)
typedef int (*CallBackFun)(char *p);    //为回调函数命名，类型命名为 CallBackFun，参数为char *p

//方法 Afun，格式符合 CallBackFun 的格式，因此可以看作是一个 CallBackFun
int Afun(char *p)
{
    printf("Afun 回调打印出字符%s!\n", p);
    return 0;
}

// 方法 Cfun，格式符合 CallBackFun 的格式，因此可以看作是一个 CallBackFun
int Cfun(char *p)
{
    printf("Cfun 回调打印:%s, Nice to meet you!\n", p);
    return 0;
}

// 执行回调函数，方式一：通过命名方式，pCallBack可以看做是CallBackFun的别名
int call(CallBackFun pCallBack, char *p)
{
    printf("call 直接打印出字符%s!\n", p);
    pCallBack(p);
    return 0;
}

// 执行回调函数，方式二：直接通过方法指针
int call2(char *p, int (*ptr)())  //或者是int call2(char *p, int (*ptr)(char *)) 同时ptr可以任意取名
{
    printf("==============\n", p);
    (*ptr)(p);
}

int main()
{
    char *p = "hello";
    call(Afun, p);
    call(Cfun, p);
    call2(p, Afun);
    call2(p, Cfun);
    return 0;
}
//再看一个回调函数的例子：

#include <stdio.h>
typedef void (*callback)(char *);
void repeat(callback function, char *para)
{
    function(para);
    function(para);
}

void hello(char* a)
{
     printf("Hello %s\n",(const char *)a);
}

void count(char *num)
{
     int i;
     for(i=1;i<(int)num;i++)
          printf("%d",i);
     putchar('\n');
}

int main(void)
{
     repeat(hello,"Huangyi");
     printf("sdfsd");
     repeat(count, (char *)4);
}

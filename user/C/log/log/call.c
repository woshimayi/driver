#include<stdio.h>
//����ָ��ĸ�ʽΪ��int (*ptr)(char *p) ��������ֵ(ָ����)(�����б�)
typedef int (*CallBackFun)(char *p);    //Ϊ�ص�������������������Ϊ CallBackFun������Ϊchar *p

//���� Afun����ʽ���� CallBackFun �ĸ�ʽ����˿��Կ�����һ�� CallBackFun
int Afun(char *p)
{
	printf("Afun �ص���ӡ���ַ�%s!\n", p);
	return 0;
}

// ���� Cfun����ʽ���� CallBackFun �ĸ�ʽ����˿��Կ�����һ�� CallBackFun
int Cfun(char *p)
{
	printf("Cfun �ص���ӡ:%s, Nice to meet you!\n", p);
	return 0;
}

// ִ�лص���������ʽһ��ͨ��������ʽ��pCallBack���Կ�����CallBackFun�ı���
int call(CallBackFun pCallBack, char *p)
{
	printf("call ֱ�Ӵ�ӡ���ַ�%s!\n", p);
	pCallBack(p);
	return 0;
}

// ִ�лص���������ʽ����ֱ��ͨ������ָ��
int call2(char *p, int (*ptr)())  //������int call2(char *p, int (*ptr)(char *)) ͬʱptr��������ȡ��
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
//�ٿ�һ���ص����������ӣ�

#include <stdio.h>
typedef void (*callback)(char *);
void repeat(callback function, char *para)
{
	function(para);
	function(para);
}

void hello(char *a)
{
	printf("Hello %s\n", (const char *)a);
}

void count(char *num)
{
	int i;
	for (i = 1; i < (int)num; i++)
		printf("%d", i);
	putchar('\n');
}

int main(void)
{
	repeat(hello, "Huangyi");
	printf("sdfsd");
	repeat(count, (char *)4);
}

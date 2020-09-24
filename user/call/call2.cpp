#include <stdio.h>
 
typedef void (*callback)(char *);
 
void repeat(callback function, char *para)
{
	function(para);
}
 
void hello(char* a)
{
	printf("Hello %s\n",(const char *)a);
}
 
void count(char *num)
{	
	int i;
	printf("%d",(int*)num);
	putchar('\n');
}
 
int main(void)
{
	repeat(hello,"xiaoqiang");
	repeat(count, (char *)4);
}


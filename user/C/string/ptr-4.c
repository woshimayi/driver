#include <stdio.h>
#include <string.h>

int b[3] = {1,2,3};

void fcn(int* *a)
{
    *a = b;
}


void call((*fun)(int), int)
{
	fun()
}


int main()
{
    unsigned int i = 0;
    int* a;

//    for(i=0;i<3;i++)
//     {
//         printf(">> %d \n",a[i]);
//     }
	
	printf("a addr %p %p %p\n", a, &a, b);
     fcn(&a);
     printf("a addr %p %p\n", a, &a);

     for(i=0;i<3;i++)
     {
         printf(">> %d \n",a[i]);
     }
}

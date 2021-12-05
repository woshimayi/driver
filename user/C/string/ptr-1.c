#include <stdio.h>
#include <string.h>

void test1(int a, int b)
{
   printf(">> formal addr a: %d, formal addr b: %d\n",&a,&b); //打印形参地址
   printf(">> formal value a: %d, formal value b: %d\n",a,b); //打印形参值
}

int main()
{
    int a = 1, b = 2;

    printf(">> actual addr a: %d, actual addr b: %d\n", &a,&b);
    printf(">> actual value a: %d, actual value b: %d\n",a,b);
    test1(a,b);

    return 0;
}

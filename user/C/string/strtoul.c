#include<stdio.h>
#include<stdlib.h>
int main()
{
    int a;
    char pNum[] = "0x77";
    a = strtoul(pNum, 0, 0); //����0����ʾ�Զ�ʶ��pNum�Ǽ�����
    printf("%u\n", a);
    return 0;
}

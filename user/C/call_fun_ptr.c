/*
 * @*************************************: 
 * @FilePath: /user/C/call_fun_ptr.c
 * @version: 
 * @Author: dof
 * @Date: 2021-09-20 16:16:22
 * @LastEditors: dof
 * @LastEditTime: 2021-09-29 22:51:11
 * @Descripttion:  函数指针调用
 * @**************************************: 
 */

#include <stdio.h>

void call(void (*pf)(int, int), int op1, int op2);
void add(int a, int b);
void sub(int a, int b);
void mult(int a, int b);

int main(int argc, char *argv[])
{
    int sel, x1, x2;
    printf("select the operator:");
    scanf("%d", &sel);
    printf("Input two number:");
    scanf("%d%d", &x1, &x2);

    switch (sel)
    {
    case 1:
        call(&add, x1, x2);
        break;
    case 2:
        call(&sub, x1, x2);
        break;
    case 3:
        call(&mult, x1, x2);
        break;

    default:
        printf("default\n");
        break;
    }
    return 0;
}

void call(void (*pf)(int, int), int op1, int op2)
{
    pf(op1, op2);
}
void add(int a, int b)
{
    printf("%d + %d = %d\n", a, b, a + b);
}
void sub(int a, int b)
{
    printf("%d - %d = %d\n", a, b, a - b);
}
void mult(int a, int b)
{
    printf("%d * %d = %d\n", a, b, a * b);
}

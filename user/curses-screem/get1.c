#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//分配内存的大小
#define     SIZE    10
//定义按键们的宏
#define     ESC     "\033"
#define     UP      "\033[A"
#define     DOWN    "\033[B"
#define     LEFT    "\033[D"
#define     RIGHT   "\033[C"

int main()
{
    char *get = (char*)malloc(SIZE);

    for ( ; ; ) 
    {   
        fgets(get, SIZE, stdin);
        /*    用fgets()函数从stdin中读取字符串时，会自动在字符串末尾追加"\n"，这里将末尾字符改为"\0"    */
        get[strlen(get) - 1] = '\0';

        if (!strcmp(get, ESC))
            printf("This is \"ESC\" button!\n");
        if (!strcmp(get, UP))
            printf("This is \"UP\" button!\n");
        if (!strcmp(get, DOWN))
            printf("This is \"DOWN\" button!\n");
        if (!strcmp(get, LEFT))
            printf("This is \"LEFT\" button!\n");
        if (!strcmp(get, RIGHT))
            printf("This is \"RIGHT\" button!\n");
    }   

    return 0;
}
#include <stdio.h>
#include <string.h>

int main()
{
    const char *str = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.Enable";
    const char ch = '.';
    char *ptr;
    char tmp[1024] = {0};

    ptr = strrchr(str, ch);

    if (ptr != NULL)
    {
        printf("字符 '%c 出现的位置为 %ld. \n",ch,  ptr - str + 1);
        printf("|%c| 之后的字符串是 - |%s|\n", ch, ptr);
        strncpy(tmp, str, ptr-str+1);
        printf(" tmp = %s \n", tmp);
    }
    else
    {
        printf("没有找到字符 'd' 。\n");
    }
    return (0);
}

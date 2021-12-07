#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


int func(int a, int b);

int fun_1(int a, int b, char *function);

#define fun_1 fun

int getSysRunTime(void)
{
    struct sysinfo sysRunInfo;

    /*success*/
    if (sysinfo(&sysRunInfo) == 0)
    {
        return sysRunInfo.uptime;
    }
    else
    {
        return 0;
    }
}



int main()
{
    func(2, 3);
    return 0;
}


int func_2(int a, int b)
{
    printf("%d+%d=%d\n", a, b, a + b);
    return 0;
}

int fun_1(int a, int b, char *function)
{
    printf("%s\n", function);
    func(a, b);
    return 0;
}

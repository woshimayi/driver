#include <stdio.h>
#include <string.h>
#include <math.h>

void for_loop()
{
    for (int i = 0; i < 1000; i++)
    {
        for (int j = 0; j < 10000; j++)
        {
            int x = sin(i) + cos(j);
        }
    }
}

void loop_small()
{
    for (int i = 0; i < 10; i++)
    {
        for_loop();
    }
}

void loop_big()
{
    for (int i = 0; i < 100; i++)
    {
        for_loop();
    }
}

int main()
{
    loop_big();
    loop_small();

    return 0;
}
#include <stdio.h>

int main()
{
    int c = 1;

    void *label_array[] = {&&op_hello,  &&op_world, &&op_end};

    scanf("%d", &c);


    goto *label_array[c];

op_hello:
    printf("hello\n");
    goto op_end;
op_world:
    printf("world\n");
    goto op_end;
op_end:
    printf("111\n");
    return 0;
}

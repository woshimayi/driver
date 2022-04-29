#include <stdio.h>
#include <stdlib.h>

int compar_int(const void *p1, const void *p2)
{
    return (*((int *)p1) - *((int *)p2));
}

void test_qsort(void)
{
    int arr[5] = {8, 5, 10, 1, 100};

    printf("排序前：");
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", arr[i]);
    }

    qsort((int *)arr, 5, 4, compar_int);

    printf("\n排序后：");
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", arr[i]);
    }
}

int main(void)
{
    test_qsort();
    return 0;
}
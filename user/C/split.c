/*
 * ����: split()
 * ����: ָ���ָ����ָ��ַ���
 * ����ֵ: һ��ָ�����char*ָ��������ָ�룬��char **pt
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char **split(const char *source, char flag);

int main()
{
    char str1[] = " abs   mk  oi pp";
    char str2[] = "*hello, world*";
    char **p1, **p2;
    p1 = split(str1, ' ');
    int i = 0;

    for (i = 0; p1[i] != NULL; i++)
        printf("p1[%d] = %s\n", i, p1[i]);
    putchar('\n');

    p2 = split(str2, '*');
    for (i = 0; p2[i] != NULL; i++)
        printf("p2[%d] = %s\n", i, p2[i]);
    // �ͷ��ڴ�
    free(p2);
    free(p1);
    return 0;
}


char **split(const char *source, char flag)
{
    char **pt;
    int j, n = 0;
    int count = 1;
    int len = strlen(source);
    char tmp[len + 1];
    tmp[0] = '\0';

    int i = 0;
    for (i = 0; i < len; ++i)
    {
        if (source[i] == flag && source[i + 1] == '\0')
            continue;
        else if (source[i] == flag && source[i + 1] != flag)
            count++;
    }
    // �����һ��char*����Ϊ�����ý�����־
    pt = (char **)malloc((count + 1) * sizeof(char *));

    count = 0;
    for (i = 0; i < len; ++i)
    {
        if (i == len - 1 && source[i] != flag)
        {
            tmp[n++] = source[i];
            tmp[n] = '\0';  // �ַ���ĩβ��ӿ��ַ�
            j = strlen(tmp) + 1;
            pt[count] = (char *)malloc(j * sizeof(char));
            strcpy(pt[count++], tmp);
        }
        else if (source[i] == flag)
        {
            j = strlen(tmp);
            if (j != 0)
            {
                tmp[n] = '\0';  // �ַ���ĩβ��ӿ��ַ�
                pt[count] = (char *)malloc((j + 1) * sizeof(char));
                strcpy(pt[count++], tmp);
                // ����tmp
                n = 0;
                tmp[0] = '\0';
            }
        }
        else
            tmp[n++] = source[i];
    }
    // ���ý�����־
    pt[count] = NULL;

    return pt;
}

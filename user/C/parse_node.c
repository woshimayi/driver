/*
 * @*************************************:
 * @FilePath     : /user/C/parse_node.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-09-24 11:47:09
 * @LastEditors  : dof
 * @LastEditTime : 2024-09-24 13:42:27
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int extract_number(const char *str)
{
    int len = strlen(str);
    int last_dot = -1, second_last_dot = -1;

    // 从后往前查找两个点的位置
    for (int i = len - 1; i >= 0; i--)
    {
        if (str[i] == '.')
        {
            if (second_last_dot == -1)
            {
                second_last_dot = i;
            }
            else
            {
                last_dot = i;
                break;
            }
        }
    }

    // 如果没有找到两个点，返回错误
    if (last_dot == -1 || second_last_dot == -1)
    {
        return -1;
    }

    // 提取子字符串并转换为数字
    char *sub_str = strndup(str + last_dot + 1, second_last_dot - last_dot - 1);
    int num = atoi(sub_str);
    free(sub_str);

    return num;
}

int main()
{
    char *str = "InternetGatewayDevice.AhsapiQos.VlanPriority.44.SourcePort";
    int result = extract_number(str);

    if (result == -1)
    {
        printf("Invalid string format\n");
    }
    else
    {
        printf("The extracted number is: %d\n", result);
    }

    return 0;
}
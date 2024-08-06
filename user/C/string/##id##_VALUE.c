/*
 * @*************************************:
 * @FilePath: /user/C/string/##id##_VALUE.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2024-08-06 13:10:55
 * @Descripttion: ## 连接符
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SF_GetFeature(id)  _##id##_


// #define STR(x) #x
// #define STR1(x, y) #x#y
// #define STR2(x, y) x ## y
// #define CONCAT(x, y) x##y

// #define CONCAT1(x, y) STR((x##y))

int main()
{
    // int var1 = 10;
    // int var2 = 20;

    // // 字符串化
    // printf("var1 的值是: %s\n", STR(var1)); // 输出：var1 的值是: var1
    // printf("var1 的值是: %s\n", STR1(var1, var1)); // 输出：var1 的值是: var1
    // // char *str = STR2(var2, var2);
    // // printf("var1 的值是: %s\n", str); // 输出：var1 的值是: var1

    // // 标记粘贴
    // int CONCAT(var, 3) = 30;
    // printf("var3 的值是: %d\n", var3); // 输出：var3 的值是: 30

    // // 结合使用
    // char filename[20];
    // sprintf(filename, "data_%s.txt", STR(var1));
    // printf("文件名: %s\n", filename); // 输出：文件名: data_var1.txt

// #define CREATE_STRING(prefix, var) #prefix ## var
// #define PREFIX "file_"

//     int file_num = 5;
//     char *filename = CREATE_STRING(PREFIX, file_num); // filename 的值为 "file_5"

// #define GENERATE_FILENAME(base, ext) base ## "." ## ext
// char *filename = GENERATE_FILENAME("data", "txt"); // filename 的值为 "data.txt"

#define STR(x) #x
#define A 2
#define B wan

#define _INT(a,b) a##.##b
#define FLOT(a,b) _INT(a,b)

// #define _CONS(a,b) STR(a) ## . ## STR(b)
// #define CAT(a,b) _CONS(a,b)



#define CAT(cmd, a, b) snprintf(cmd, sizeof(cmd), "%s.%s", a, b)

    printf("%f\n", FLOT(A, A));
    printf("%s\n", CAT(B, A));


	printf("SF_GetFeature(3) %s\n", SF_GetFeature(4));

	return 0;
}



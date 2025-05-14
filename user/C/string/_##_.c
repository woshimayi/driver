/*
 * @*************************************: 
 * @FilePath: /user/C/string/_##_.c
 * @version: 
 * @Author: dof
 * @Date: 2022-06-16 14:05:29
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:04:44
 * @Descripttion:  ## 连接字符
 * @**************************************: 
 */
#include <stdio.h>

#define A 12
#define B 13

#define _LINE(AA, BB) AA##BB
#define LINE(AA, BB) _LINE(AA, BB)

// C11 标准引入了 _Generic 关键字，可以根据表达式的类型选择不同的结果。这可以用来创建一个更通用的宏
// #define PRINT(x) _Generic((x), \
//     int: printf(#x " = %d\n", (x)), \
//     float: printf(#x " = %f\n", (x)), \
//     double: printf(#x " = %lf\n", (x)), \
//     char: printf(#x " = %c\n", (x)), \
//     default: printf(#x " = (unknown type) %s\n", (x)) \
// )


typedef struct {
	int    i;
	float  f;
	double d;
	char   c;
	char *str;
} _dof;

// int main()
// {
// 	// int n = LINE(A, B);
// 	// printf("n = %d\n", n);

// 	_dof dof = {1, 2.3, 2.3333, 'S', "ddddddddd"};

// 	PRINT(dof.i);
// 	PRINT(dof.f);
// 	PRINT(dof.d);
// 	PRINT(dof.c);
// 	PRINT(dof.str); // 会匹配 default
// 	PRINT(1 + 2.0); // 会匹配 double

// 	return 0;
// }



#include <stdio.h>
#include <stdarg.h>

#define PRINT(type, value) _PRINT(type, #value, value)

void _PRINT(const char *type, const char *name, ...) {
    va_list args;
    va_start(args, name);

    printf("%s = ", name);

    if (strcmp(type, "int") == 0) {
        printf("%d\n", va_arg(args, int));
    } else if (strcmp(type, "float") == 0) {
        printf("%f\n", (double)va_arg(args, double)); // float 会提升为 double
    } else if (strcmp(type, "double") == 0) {
        printf("%lf\n", va_arg(args, double));
    } else if (strcmp(type, "char") == 0) {
        printf("%c\n", va_arg(args, int)); // char 会提升为 int
    } else if (strcmp(type, "string") == 0) {
        printf("%s\n", va_arg(args, char*));
    } else {
        printf("(unknown type)\n");
    }

    va_end(args);
}

int main() {
    int i = 10;
    float f = 3.14;
    double d = 2.71828;
    char c = 'A';
    const char* str = "hello";

    PRINT("int", i);
    PRINT("float", f);
    PRINT("double", d);
    PRINT("char", c);
    PRINT("string", str);

    return 0;
}
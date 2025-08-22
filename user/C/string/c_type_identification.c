/*
 * @*************************************:
 * @FilePath     : /user/C/string/c_type_identification.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-08-12 14:38:27
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-12 16:13:12
 * @Descripttion : 识别变量的类型
 * @compile      :
 * @**************************************:
 */

#if 1
#include <stdio.h>

// 定义一个宏，根据传入参数的类型打印不同的信息
#define print_type(x) _Generic((x),           \
    int: printf("这是一个 int 类型\n"),       \
    float: printf("这是一个 float 类型\n"),   \
    double: printf("这是一个 double 类型\n"), \
    char *: printf("这是一个 char* 类型\n"),  \
    default: printf("这是其他类型\n"))

typedef enum
{
    TYPE_NONE,
    TYPE_INT    = 1,
    TYPE_FLOAT  = 2,
    TYPE_DOUBLE = 3,
    TYPE_CHAR   = 4,
    TYPE_MAX
} _type_T;

#define Get_type(x) _Generic((x), \
    int: TYPE_INT,         \
    float:  TYPE_FLOAT,     \
    double: TYPE_DOUBLE,   \
    char *: TYPE_CHAR,     \
    default: TYPE_MAX)

int main()
{
    int a = 10;
    float b = 3.14f;
    double c = 2.718;
    char *s = "Hello";
    long d = 100L; // 默认类型

    print_type(a);
    print_type(b);
    print_type(c);
    print_type(s);
    print_type(d); // 将匹配 default

    printf("type = %d\n", Get_type(b));

    return 0;
}
#else

#include <stdio.h>

// 定义一个泛型交换宏，可以交换任何相同类型的变量
#define SWAP(a, b)                                           \
    do                                                       \
    {                                                        \
        typeof(a) __temp = (a); /* __temp 的类型与 a 相同 */ \
        (a) = (b);                                           \
        (b) = __temp;                                        \
    } while (0)

int main()
{
    int x = 5, y = 10;
    SWAP(x, y);
    printf("After SWAP(int): x = %d, y = %d\n", x, y); // 输出: x = 10, y = 5

    double f1 = 3.14, f2 = 6.28;
    SWAP(f1, f2);
    printf("After SWAP(double): f1 = %f, f2 = %f\n", f1, f2); // 输出: f1 = 6.280000, f2 = 3.140000

    char c1 = 'A', c2 = 'B';
    SWAP(c1, c2);
    printf("After SWAP(char): c1 = %c, c2 = %c\n", c1, c2); // 输出: c1 = B, c2 = A

    return 0;
}
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mcheck.h>

void infinite_loop()
{
    infinite_loop(); // 无限递归导致栈空间耗尽
}


typedef enum
{
    JSON_NULL,
    JSON_OBJ,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INT,
    JSON_DOUBLE,
    JSON_BOOL,
    JSON_MAX
} __json_type_enum;

typedef struct
{
    __json_type_enum type;
    char name[64];
} __json_type;

__json_type g_jsontype[] = {
    [JSON_NULL] = {JSON_NULL, "JSON_NULL"},
    [JSON_OBJ] = {JSON_OBJ, "JSON_OBJ"},
    [JSON_ARRAY] = {JSON_ARRAY, "JSON_ARRAY"},
    [JSON_STRING] = {JSON_STRING, "JSON_STRING"},
    [JSON_INT] = {JSON_INT, "JSON_INT"},
    [JSON_DOUBLE] = {JSON_DOUBLE, "JSON_DOUBLE"},
    [JSON_BOOL] = {JSON_BOOL, "JSON_BOOL"},
    [JSON_MAX] = {JSON_MAX, "JSON_MAX"},
};



int main(int argc, char const *argv[])
{

    // char cmd[128] = {0};
    // char str[128] = {0};
    // snprintf(cmd, sizeof(cmd), "asdas \"%s\"", str);
    // printf("cmd = %s\n", cmd);
    // mtrace();    //开启内存分配跟踪
    // char *p = (char *)malloc(32);

    // int a = 1, b = 2, c = 3;
     // 注释语句 ??\
    a = b+c;
    // printf("ssss %d\n", a);
    // printf("Eh???/n");
    // printf("Delete file (are you really sure??):\n");

    // muntrace();   // 结束跟踪，并生成日志信息

    // 10种初学者最常见的c语言段错误实例及原因分析
    // 1
    // int *p = NULL;
    // printf("%p\n", p); // 解引用空指针
    // printf("%d\n", *p); // 解引用空指针

    // 2
    // char *str = "hello";  // 字符串常量存储在只读区
    // str[0] ='H';        // 尝试修改常量区数据

    // 3
    // infinite_loop();

    // 4
    // int *p;  // 未初始化指针
    // *p = 42; // 野指针指向无效地址

    // 5
    // int *p = malloc(sizeof(int));
    // free(p);
    // p = NULL;
    // *p = 10; // 内存释放后继续使用

    // 6
    // char buffer[5];
    // strcpy(buffer, "HelloWorld"); // 超出 buffer 容量

    // 7
    // int *p = malloc(sizeof(int));
    // free(p);
    // free(p); // 重复释放同一块内存

    // 8
    // int num = 42;
    // char *p = (char*)&num;  // 将整数值强制转换为地址
    // *p = 'A';              // 访问非法地址

    // 9
    // char str[5] = {'H','e','l','l','o'}; // 缺少 '\0'
    // printf("%s\n", str);

    // 10
    // char dest[5];
    // strcpy(dest, "Hello, World!"); // 目标缓冲区太小
    // printf("%s\n", dest);

    printf("%s\n", g_jsontype[3].name);

    int j = 0, sumNum = 1;
    if (0 == j && sumNum)
    {
        printf("ssss\n");
    }

    return 0;
}

/*
 * @*************************************:
 * @FilePath     : /user/C/struct_for_parem.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-16 15:48:07
 * @LastEditors  : dof
 * @LastEditTime : 2024-09-14 13:08:56
 * @Descripttion :   打印结构体所有元素
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stddef.h>
#include <stdio.h>

// 获得结构体(TYPE)的变量成员(MEMBER)在此结构体中的偏移量。
// #define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

struct student
{
    char gender;
    int id;
    int age;
    char name[20];
};

// #include <stddef.h>

struct MyStruct
{
    const char *name;
    int offset;
    int value;
};

static const struct MyStruct members[] = {
    {"a", offsetof(struct MyStruct, name), 12},
    {"b", offsetof(struct MyStruct, value), 23},
};

// void main()
// {
//     int gender_offset, id_offset, age_offset, name_offset;

// gender_offset = offsetof(struct student, gender);
// id_offset = offsetof(struct student, id);
// age_offset = offsetof(struct student, age);
// name_offset = offsetof(struct student, name);

// printf("gender_offset = %d\n", gender_offset);
// printf("id_offset = %d\n", id_offset);

// printf("age_offset = %d\n", age_offset);
// printf("name_offset = %d\n", name_offset);

// }

struct Person
{
    int age;
    char name[20];
};

int main()
{
    struct Person *person = (struct Person *)malloc(sizeof(struct Person));

    // 计算name成员的偏移量
    size_t offset = offsetof(struct Person, name);

    // 将name成员赋值为"Alice"
    char *name_ptr = (char *)person + offset;
    strcpy(name_ptr, "Alice");

    printf("Person's name: %s\n", person->name);

    free(person);
    return 0;
}
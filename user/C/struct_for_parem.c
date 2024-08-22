/*
 * @*************************************:
 * @FilePath     : /user/C/struct_for_parem.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-16 15:48:07
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-19 10:28:02
 * @Descripttion :   打印结构体所有元素
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stddef.h>
#include <stdio.h>

// 获得结构体(TYPE)的变量成员(MEMBER)在此结构体中的偏移量。
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

struct student
{
    char gender;
    int id;
    int age;
    char name[20];
};




void main()
{
    // int gender_offset, id_offset, age_offset, name_offset;

    // gender_offset = offsetof(struct student, gender);
    // id_offset = offsetof(struct student, id);
    // age_offset = offsetof(struct student, age);
    // name_offset = offsetof(struct student, name);

    // printf("gender_offset = %d\n", gender_offset);
    // printf("id_offset = %d\n", id_offset);

    // printf("age_offset = %d\n", age_offset);
    // printf("name_offset = %d\n", name_offset);
    char str[32] = {0};
    int cpu = 0;
    while (1)
    {
        printf("input cpu:");
        scanf("%s", str);
        cpu = atoi(str);
        printf("%d\n", (cpu%30)?(cpu%30):1);
    }
}
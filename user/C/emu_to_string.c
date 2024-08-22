/*
 * @*************************************:
 * @FilePath     : /user/C/emu_to_string.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-21 10:31:35
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-21 10:40:43
 * @Descripttion :  根据枚举id 打印 id 变量
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

enum Weekday
{
    Sunday,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
};

#define WEEKDAY_STRINGS(x)            \
    [Sunday] = TOSTRING(Sunday), \
    [Monday] = TOSTRING(Monday), \
    [Tuesday] = TOSTRING(Tuesday), \
    [Wednesday] = TOSTRING(Wednesday), \
    [Thursday] = TOSTRING(Thursday), \
    [Friday] = TOSTRING(Friday), \
    [Saturday] = TOSTRING(Saturday),

const char *weekdayStrings[] = {
    WEEKDAY_STRINGS(Weekday)};

int main(int argc, char const *argv[])
{
    printf("%s\n", weekdayStrings[3]);
    return 0;
}

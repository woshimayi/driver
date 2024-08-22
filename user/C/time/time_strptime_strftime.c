/*
 * @*************************************:
 * @FilePath     : /user/C/time/time_strptime_strftime.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-20 10:41:55
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-20 10:42:57
 * @Descripttion : linux 时间格式化函数strftime和strptime使用
 *                  这两个函数都是时间日期的格式控制函数，在功能上看起来正好相反,
 *                  strftime将一个struct tm结构格式化为一个字符串，
 *                  strptime则是将一个字符串格式化为一个struct tm 结构
 *                  https://blog.csdn.net/hittata/article/details/8090228
 * @compile      :
 * @**************************************:
 */

#include <time.h>
#include <stdio.h>
#include <string.h>

int main()
{
    struct tm tm;
    char buf[255];

    strptime("24/Aug/2011:09:42:35", "%d/%b/%Y:%H:%M:%S", &tm);
    printf("asctime:%s\n", asctime(&tm));

    memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
    puts(buf);
    return 0;
}
/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/10.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-02 22:16:36
 * @LastEditors: dof
 * @LastEditTime: 2021-10-02 22:48:29
 * @Descripttion:  
 * @**************************************: 
 */

// setiosflags(ios::fixed) 固定的浮点显示
// setiosflags(ios::scientific) 指数表示
// setiosflags(ios::left) 左对齐
// setiosflags(ios::right) 右对齐
// setiosflags(ios::skipws 忽略前导空白
// setiosflags(ios::uppercase) 16进制数大写输出
// setiosflags(ios::lowercase) 16进制小写输出
// setiosflags(ios::showpoint) 强制显示小数点
// setiosflags(ios::showpos) 强制显示符号
// setiosflags(ios::fixed) 固定的浮点显示
// setiosflags(ios::scientific) 指数表示
// setiosflags(ios::left) 左对齐
// setiosflags(ios::right) 右对齐
// setiosflags(ios::skipws 忽略前导空白
// setiosflags(ios::uppercase) 16进制数大写输出
// setiosflags(ios::lowercase) 16进制小写输出
// setiosflags(ios::showpoint) 强制显示小数点
// setiosflags(ios::showpos) 强制显示符号

// boolalpha	可以使用单词”true”和”false”进行输入/输出的布尔值.
// oct	用八进制格式显示数值.
// dec	用十进制格式显示数值.
// hex	用十六进制格式显示数值.
// left	输出调整为左对齐.
// right	输出调整为右对齐.
// scientific	用科学记数法显示浮点数.
// fixed	用正常的记数方法显示浮点数(与科学计数法相对应).
// showbase	输出时显示所有数值的基数.
// showpoint	显示小数点和额外的零，即使不需要.
// showpos	在非负数值前面显示”＋（正号）”.
// skipws	当从一个流进行读取时，跳过空白字符(spaces, tabs, newlines).
// unitbuf	在每次插入以后，清空缓冲区.
// internal	将填充字符回到符号和数值之间.
// uppercase	以大写的形式显示科学记数法中的”e”和十六进制格式的”x”.

// boolalpha	启用boolalpha标志	√	√
// dec	启用dec标志	√	√
// endl	输出换行标示，并清空缓冲区		√
// ends	输出空字符		√
// fixed	启用fixed标志		√
// flush	清空流		√
// hex	启用 hex 标志	√	√
// internal	启用 internal 标志		√
// left	启用 left 标志		√
// noboolalpha	关闭boolalpha 标志	√	√
// noshowbase	关闭showbase 标志		√
// noshowpoint	关闭showpoint 标志		√
// noshowpos	关闭showpos 标志		√
// noskipws	关闭skipws 标志	√
// nounitbuf	关闭unitbuf 标志		√
// nouppercase	关闭uppercase 标志		√
// oct	启用 oct 标志	√	√
// right	启用 right 标志		√
// scientific	启用 scientific 标志		√
// showbase	启用 showbase 标志		√
// showpoint	启用 showpoint 标志		√
// showpos	启用 showpos 标志		√
// skipws	启用 skipws 标志	√
// unitbuf	启用 unitbuf 标志		√
// uppercase	启用 uppercase 标志		√
// ws	跳过所有前导空白字符	√

// resetiosflags(long f)	关闭被指定为f的标志	√	√
// setbase(int base)	设置数值的基本数为base		√
// setfill(int ch)	设置填充字符为ch		√
// setiosflags(long f)	启用指定为f的标志	√	√
// setprecision(int p)	设置数值的精度(四舍五入)		√
// setw(int w)	设置域宽度为w

#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

int main(int argc, char const *argv[])
{
    // cout << setiosflags(ios::right | ios::showpoint);
    // cout << setiosflags(ios::right);
    // cout.precision(10);
    // cout << 123.34567 << endl;
    // cout.width(10);
    // cout.fill('*');

    // cout << resetiosflags(ios::left);
    // cout << setiosflags(ios::left | ios::fixed);
    // cout << 999.234 << endl;
    // cout << resetiosflags(ios::left | ios::fixed);
    // cout << setiosflags(ios::left | ios::scientific);
    // cout.precision(3);
    // cout << 123.456789 << endl;

    cout << setiosflags(ios::left | ios::showpoint); // 设左对齐，以一般实数方式显示
    cout.precision(5);                               // 设置除小数点外有五位有效数字
    cout << 123.456789 << endl;
    cout.width(10);                   // 设置显示域宽10
    cout.fill('*');                   // 在显示区域空白处用*填充
    cout << resetiosflags(ios::left); // 清除状态左对齐
    cout << setiosflags(ios::right);  // 设置右对齐
    cout << 123.456789 << endl;
    cout << setiosflags(ios::left | ios::fixed); // 设左对齐，以固定小数位显示
    cout.precision(3);                           // 设置实数显示三位小数
    cout << 999.123456 << endl;
    cout << resetiosflags(ios::left | ios::fixed);    //清除状态左对齐和定点格式
    cout << setiosflags(ios::left | ios::scientific); //设置左对齐，以科学技术法显示
    cout.precision(3);                                //设置保留三位小数
    cout << 123.45678 << endl;
    return 0;

    return 0;
}

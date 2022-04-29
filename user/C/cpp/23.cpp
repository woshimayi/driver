/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/23.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 02:43:17
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 02:45:40
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>

using namespace std;

inline int Max(int x, int y)
{
    return (x < y) ? y : x;
}

int main(int argc, const char **argv)
{
    cout << "Max (20 ,10): " << Max(20, 10) << endl;
    cout << "Max (10, 20):" << Max(10, 20) << endl;
    return 0;
}
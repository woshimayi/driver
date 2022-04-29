/*
 * @*************************************: 
 * @FilePath: /user/.vscode/5.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-01 13:35:53
 * @LastEditors: dof
 * @LastEditTime: 2021-10-01 14:20:52
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char const *argv[])
{
    int i;
    double d;

    int &r = i;
    double &s = d;

    printf("%p  %p  %p  %p\n", &i, &d, &r, &s);

    i = 5;
    cout << "Vlaue of i : " << i << endl;
    cout << "Vlaue of i reference:" << r << endl;

    d = 213.555;
    cout << "Vlaue of d : " << d << endl;
    cout << "Vlaue of r reference:" << s << endl;
    printf("%p  %p  %p  %p\n", &i, &d, &r, &s);
    return 0;
}

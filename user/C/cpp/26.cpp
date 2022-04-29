/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/26.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 11:21:41
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 11:23:27
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>

using namespace std;

class printData
{
public:
    void print(int i)
    {
        cout << "int :" << i << endl;
    }
    void print(double f)
    {
        cout << "double :" << f << endl;
    }
    void print(char c[])
    {
        cout << "char :" << c << endl;
    }
};

int main(int argc, const char **argv)
{
    printData pd;

    pd.print(34);
    pd.print(23.5);
    pd.print("ssss");
    return 0;
}
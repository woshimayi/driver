/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/7.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-01 14:43:45
 * @LastEditors: dof
 * @LastEditTime: 2021-10-02 21:01:27
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>
#include <string>

using namespace std;

double vals[] = {10.1, 12.4, 33.3, 24.9, 39.8};

double &setVlaues(int i)
{
    double &ref = vals[i];

    return ref;
}

int main(int argc, char const *argv[])
{
    cout << "pre change value" << endl;

    for (int i = 0; i < 5; i++)
    {
        cout << vals[i] << "\t";
    }

    setVlaues(1) = 23.5;
    setVlaues(3) = 43.5;

    cout << "change value " << endl;

    for (int i = 0; i < 5; i++)
    {
        cout << vals[i] << "\t";
    }

    return 0;
}

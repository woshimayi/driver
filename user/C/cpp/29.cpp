/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/29.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 22:18:12
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 22:19:44
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>

using namespace std;

int main(int argc, const char **argv)
{
    double *pvalue = NULL;
    pvalue = new double;

    *pvalue = 234.345;
    cout << "value of vlaue" << *pvalue << endl;

    delete pvalue;
    return 0;
}
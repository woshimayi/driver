/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/22.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 02:31:12
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 02:36:44
 * @Descripttion:  友元函数
 * @**************************************: 
 */

#include <iostream>

using namespace std;

class Box
{
    double width;

public:
    friend void printWidth(Box box);
    void setWidth(double wid);
};

void Box::setWidth(double wid)
{
    width = wid;
}

void printWidth(Box box)
{
    cout << "Width of box : " << box.width << endl;
}

int main(int argc, const char **argv)
{
    Box box;
    box.setWidth(23.5);
    printWidth(box);
    return 0;
}
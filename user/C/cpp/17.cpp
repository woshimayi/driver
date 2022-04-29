/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/17.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 01:47:21
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 01:53:29
 * @Descripttion: 构造函数
 * @**************************************: 
 */

#include <iostream>

using namespace std;

class Line
{
private:
    double length;

public:
    void setLength(double len);
    double getLength(void); // 构造函数
    Line();
};

Line::Line(void)
{
    cout << "Object is being created" << endl;
}

double Line::getLength(void)
{
    return length;
}

void Line::setLength(double len)
{
    length = len;
}

int main(int argc, const char **argv)
{
    Line line;

    line.setLength(2.6);
    cout << "length is : " << line.getLength() << endl;

    return 0;
}
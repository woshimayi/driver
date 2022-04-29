/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/18.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 01:53:38
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 02:01:54
 * @Descripttion: 
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
    double getLength(void);
    Line(double len);
};

// **********************************************************************************
// Line::Line(double len)
// {
//     cout << "object is being created length: " << len << endl;
//     length = len;
// }
// 注释以上部分的等同于一下部分

Line::Line(double len) : length(len)
{
    cout << "object is being created length: " << len << endl;
}
// **********************************************************************************

void Line::setLength(double len)
{
    length = len;
}

double Line::getLength(void)
{
    return length;
}

int main(int argc, const char **argv)
{
    Line line(1.5);

    cout << "Length of line: " << line.getLength() << endl;

    line.setLength(3.6);
    cout << "length of line :" << line.getLength() << endl;
    return 0;
}
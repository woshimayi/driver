/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/13.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 00:58:28
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 01:09:30
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>

using namespace std;

class Box
{
protected:
    double width;
};

class SmallBox : Box // 继承Box 类
{
public:
    void setSmallWidth(double wid);
    double getSmallwidth(void);
};

double SmallBox::getSmallwidth(void)
{
    return width;
}

void SmallBox::setSmallWidth(double wid)
{
    width = wid;
}

int main(int argc, char const *argv[])
{
    SmallBox box;

    box.setSmallWidth(10.5);
    cout << "width of box: " << box.getSmallwidth() << endl;
    return 0;
}

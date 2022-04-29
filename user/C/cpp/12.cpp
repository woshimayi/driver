/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/12.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 00:29:34
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 00:55:47
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>

using namespace std;

class Box
{

public:
    double length;
    void setWidth(double wid);
    double getWidth(void);

private:
    double width; // 类内 的函数可以访问， 不能被直接访问
};

void Box::setWidth(double wid)
{
    width = wid;
}

double Box::getWidth(void)
{
    return width;
}

int main(int argc, char const *argv[])
{
    Box box;

    box.length = 10.0;
    cout << "len of box: " << box.length << endl;

    box.setWidth(12.0);
    cout << "wid if box: " << box.getWidth() << endl;

    return 0;
}

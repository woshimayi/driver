/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/27.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 11:31:13
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 11:39:13
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>

using namespace std;

class Box
{
private:
    double length;
    double breadth;
    double height;

public:
    double getVloume(void)
    {
        return length * breadth * height;
    }

    void setLength(double len)
    {
        length = len;
    }
    void setHeight(double hei)
    {
        height = hei;
    }

    void setBreadth(double bre)
    {
        breadth = bre;
    }

    Box operator+(const Box &b)
    {
        Box box;
        box.length = this->length + b.length;
        box.breadth = this->breadth + b.breadth;
        box.height = this->height + b.height;
        return box;
    }
};

int main(int argc, const char **argv)
{
    Box box1;
    Box box2;
    Box box3;
    double volume = 0.0;

    box1.setLength(3.3);
    box1.setBreadth(4.4);
    box1.setHeight(5.5);

    box2.setLength(3.3);
    box2.setBreadth(4.4);
    box2.setHeight(5.5);

    volume = box1.getVloume();
    volume = box2.getVloume();

    box3 = box1 + box2;

    volume = box3.getVloume();

    cout << "vlome of box3: \t" << volume << endl;

    return 0;
}
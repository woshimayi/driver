/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/24.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 02:49:34
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 02:57:01
 * @Descripttion: this 指针
 * @**************************************: 
 */

#include <iostream>

using namespace std;

class Box
{
private:
    double length;
    double breadth;
    double heigth;

public:
    Box(double l, double b, double h);
    double Vloume()
    {
        return length * breadth * heigth;
    }

    int compare(Box box)
    {
        return this->Vloume() > box.Vloume();
    }
    ~Box();
};

Box::Box(double l, double b, double h)
{
    cout << "Constructor called : " << endl;
    length = l;
    breadth = b;
    heigth = h;
}

Box::~Box()
{
    cout << "delete obj" << endl;
}

int main(int argc, const char **argv)
{

    Box box1(2.2, 3.3, 4.4);
    Box box2(1.1, 2.2, 3.3);

    if (box1.compare(box2))
    {
        cout << "Box2 is sample than Box1" << endl;
    }
    else
    {
        cout << "Box2 is sssssssssss sample than Box1" << endl;
    }
    return 0;
}

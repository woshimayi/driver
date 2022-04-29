/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/25.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 10:54:04
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 11:18:55
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
    double heigth;

public:
    static int objectCount;
    Box(double l = 2.0, double b = 2.0, double h = 2.0)
    {
        cout << "Construct called. " << endl;
        length = l;
        breadth = b;
        heigth = h;
        objectCount++;
    }
    double Vloume()
    {
        return length * breadth * heigth;
    }
    static double getobj() // 必须是静态的
    {
        return objectCount;
    }
    ~Box();
};

Box::~Box()
{
    cout << "delete Box" << endl;
}

int Box::objectCount = 0;

int main(int argc, const char **argv)
{
    Box box1(3.3, 1.2, 1.5);
    Box box2(8.5, 6.0, 2.0);
    Box box3(8.5, 6.0, 2.0);

    cout << "Total object: " << Box::objectCount << endl;
    cout << "Total object: " << Box::getobj() << endl;

    return 0;
}
/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/11.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-02 22:56:40
 * @LastEditors: dof
 * @LastEditTime: 2021-10-02 23:18:48
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>
#include <string>

using namespace std;

class Box
{
    // private:
    //     /* data */
public:
    double len;
    double breadth;
    double heigth;

    double get(void);
    void set(double len, double bre, double hei);
};

double Box::get(void)
{
    return len * breadth * heigth;
}

void Box::set(double len1, double bre, double hei) // 类中的变量不能同名，负责使用局部变量
{
    len = len1;
    breadth = bre;
    heigth = hei;
}

int main(int argc, char const *argv[])
{
    Box box1;
    Box box2;
    Box box3;

    double volume = 0.0;

    box1.heigth = 5.0;
    box1.len = 6.0;
    box1.breadth = 7.0;

    volume = box1.heigth * box1.len * box1.breadth;
    cout << "box1 volume: " << volume << endl;

    volume = box2.heigth * box2.len * box2.breadth;
    cout << "box2 volume: " << volume << endl;

    box3.set(16.0, 8.0, 12.0);
    volume = box3.get();
    cout << "box3 vlume: " << volume << endl;

    return 0;
}

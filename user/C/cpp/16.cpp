/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/16.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 01:44:31
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 01:44:41
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>
#include <assert.h>

using namespace std;

class A
{

public:
    int a;
    A()
    {
        a1 = 1;
        a2 = 2;
        a3 = 3;
        a = 4;
    }
    void fun()
    {
        cout << a << endl;
        cout << a1 << endl;
        cout << a2 << endl;
        cout << a3 << endl;
    }

public:
    int a1;

protected:
    int a2;

private:
    int a3;
};

class B : private A
{
public:
    int a;
    B(int i)
    {
        A();
        a = i;
    }
    void fun()
    {
        cout << a << endl;
        cout << a1 << endl;
        cout << a2 << endl; // 派生类中 基于 protectd 的变量可以访问
        cout << a3 << endl; //  派生类 中 基于 private 的变量和成员不能访问
    }
};

int main(int argc, const char **argv)
{
    B b(10);
    cout << b.a << endl;
    cout << b.a1 << endl;
    cout << b.a2 << endl; // 派生类中 基于 protectd 的变量可以访问
    cout << b.a3 << endl; //派生类 中 基于 private 的变量和成员不能访问
    system("pause");
    return 0;
}
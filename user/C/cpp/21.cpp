/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/21.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 02:27:04
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 02:31:00
 * @Descripttion:  拷贝函数
 * @**************************************: 
 */

#include <iostream>
using namespace std;

class Line
{
private:
    int *ptr;

public:
    int getLength(void);
    Line(int len);
    Line(const Line &obj);
    ~Line();
};

Line::Line(int len)
{
    cout << "调用构造函数" << endl;
    ptr = new int;
    *ptr = len;
}

Line::Line(const Line &obj)
{
    cout << "调用拷贝构造函数并未指针ptr分配指针" << endl;
    ptr = new int;
    *ptr = *obj.ptr;
}

Line::~Line(void)
{
    cout << "释放内存" << endl;
    delete ptr;
}

int Line::getLength(void)
{
    return *ptr;
}

void display(Line obj)
{
    cout << "Line max ? : " << obj.getLength() << endl;
}

int main(int argc, const char **argv)
{
    Line line(10);
    Line line2 = line;

    display(line);
    display(line2);
    return 0;
}

#include <iostream>
using namespace std;

class Distance
{
private:
    int m_feet;   // 0 到无穷
    int m_inches; // 0 到 12
    double m_age; // 0 到 12
public:
    // 所需的构造函数
    Distance()
    {
        m_feet = 0;
        m_inches = 0;
    }
    Distance(int f, int i)
    {
        m_feet = f;
        m_inches = i;
    }
    void operator=(const Distance &D)
    {
        m_feet = D.m_feet;
        m_inches = D.m_inches;
    }
    void operator=(const int input)
    {
        m_feet = input;
    }
    void operator=(const double age)
    {
        m_age = age;
    }
    void operator[](const double age)
    {
        m_age = age;
    }
    // 显示距离的方法
    void displayDistance()
    {
        cout << "F: " << m_feet << " I:" << m_inches << " age: " << m_age << endl;
    }
};
int main()
{
    Distance D1(11, 10), D2(5, 11);

    cout << "First Distance : ";
    D1.displayDistance();
    cout << "Second Distance :";
    D2.displayDistance();

    // 使用赋值运算符
    D1 = D2;
    cout << "First Distance :";
    D1.displayDistance();

    D1 = 23;
    D1 = 22.45;
    D1.displayDistance();

    return 0;
}
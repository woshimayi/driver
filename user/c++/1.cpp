/*
 * @*************************************: 
 * @FilePath: /user/.vscode/1.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-09-30 21:07:45
 * @LastEditors: dof
 * @LastEditTime: 2021-09-30 21:09:53
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>

using namespace std;


class Ctest
{
private:
    /* data */
    int a;
    int b;

public:
    Ctest();
    Ctest(int x, int y)
    :a(x),
    b(y+x)
    {
        printf("%d %d\n", a, b);
    }

    ~Ctest();
};

Ctest::Ctest()
{
}

Ctest::~Ctest()
{
}


int main(int argc, char const *argv[])
{
    cout << "Hello world" << endl;
    cout << "Hello world"
         << "\n";

    string str = "8021";
    if (str.c_str() == "8021")
    {
        printf("ssss\n");
    }

    Ctest test(2,5);

    return 0;
}

/*
 * @*************************************: 
 * @FilePath     : /user/C/cpp/31_string_to_int_change.cpp
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-02-20 14:45:31
 * @LastEditors  : dof
 * @LastEditTime : 2025-02-20 14:51:45
 * @Descripttion :  
 * @compile      :  
 * @**************************************: 
 */



#include <iostream>
#include <list>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
    int i = 12;
    string str = itoa(i);
    cout << to_string(i) << str << endl;
    cout << typeid(i).name() << endl;
    return 0;
}


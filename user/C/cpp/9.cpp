/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/9.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-02 22:04:10
 * @LastEditors: dof
 * @LastEditTime: 2021-10-02 22:12:40
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char const *argv[])
{
    char name[50];
    // cout << "pls input: ";
    // cin >> name;
    // cout << "you name is : " << name << endl;

#if 0
    char age[20];
    cout << "pls input name age:";
    cin >> name >> age;
    cout << "name: " << name << "\tage: " << age << endl;
#endif

    char err[30] = "asdfghjkl;";

    cerr << "Errror message : " << err << endl;

    return 0;
}

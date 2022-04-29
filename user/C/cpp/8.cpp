/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/8.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-02 21:48:22
 * @LastEditors: dof
 * @LastEditTime: 2021-10-02 21:50:31
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>
#include <ctime>

using namespace std;

int main(int argc, char const *argv[])
{
    time_t now = time(0);

    char *dt = ctime(&now);

    cout << "local time" << dt << endl;

    tm *gmtm = gmtime(&now);

    dt = asctime(gmtm);

    cout << "UTC time date" << dt << endl;
    return 0;
}

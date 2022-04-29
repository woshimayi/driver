/*
 * @*************************************: 
 * @FilePath: /user/C/cpp/28.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-03 21:46:54
 * @LastEditors: dof
 * @LastEditTime: 2021-10-03 21:52:48
 * @Descripttion: 
 * @**************************************: 
 */

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, const char **argv)
{
    char data[100];

    ofstream outfile;
    outfile.open("file.dat");

    cout << "write ti the file " << endl;
    cout << "Enter your name : " << endl;

    cin.getline(data, 100);
    outfile << data << endl;

    cout << "Enter your age : ";
    cin >> data;
    cin.ignore();

    outfile << data << endl;

    outfile.close();

    ifstream infile;

    infile.open("file.dat");

    cout << "Read form the file " << endl;

    infile >> data;

    cout << data << endl;

    infile.close();
    return 0;
}
/*
 * @*************************************: 
 * @FilePath     : /user/C/cpp/substr_test_crash.cpp
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-08-27 10:37:26
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-27 11:23:24
 * @Descripttion :  
 * @compile      :  
 * @**************************************: 
 */


#include <iostream>
#include <string>

int main () {
   std::string str="Tutorialspoit is a one the best site in the world, hope so it will move same .";

   std::string str2 = str.substr (3,5);

   std::size_t pos = str.find("live");

   std::string str3 = str.substr(pos);

   std::cout << str2 << ' ' <<str3<< '\n';

   return 0;
}
 
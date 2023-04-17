/*
 * @*************************************:
 * @FilePath: /user/C/cpp/30.cpp
 * @version:
 * @Author: dof
 * @Date: 2023-03-17 18:08:42
 * @LastEditors: dof
 * @LastEditTime: 2023-03-19 13:59:24
 * @Descripttion: cpp list 列表
 * @**************************************:
 */

#include <iostream>
#include <list>
#include <string>
using namespace std;
int main()
{
#if 0
    //创建空的 list 容器
    std::list<double> values;
    //向容器中添加元素
    values.push_back(3.1);
    values.push_back(2.2);
    values.push_back(2.9);
    cout << "values size：" << values.size() << endl;
    //对容器中的元素进行排序
    values.sort();
    //使用迭代器输出list容器中的元素
    for (std::list<double>::iterator it = values.begin(); it != values.end(); ++it) {
		std::cout << *it << endl;
	}
#else
	// 创建空的 list 容器
	std::list<string> values;
	// 向容器中添加元素
	// values.push_back("aaa");
	// values.push_back("ccc");
	// values.push_back("bbb");
	values << "aaaa"
		   << "ssss"
		   << "dddd";
	cout << "values size：" << values.size() << endl;
	// 对容器中的元素进行排序
	values.sort();
	// 使用迭代器输出list容器中的元素
	for (std::list<string>::iterator it = values.begin(); it != values.end(); ++it)
	{
		std::cout << *it << endl;
	}




#endif
	return 0;
}
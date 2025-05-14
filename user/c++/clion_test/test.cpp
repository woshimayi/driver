//
// Created by root on 25-5-13.
//

#include "test.h"
#include <iostream>

using namespace std;


test::test(/* args */)
{
}

test::~test()
{
}


void test::dump()
{
	cout << __func__ << " " << __LINE__ << " Hello world" << endl;
}

/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/design_pattern/status_oop.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-20 14:41:11
 * @LastEditors: dof
 * @LastEditTime: 2023-01-20 14:45:05
 * @Descripttion: c 语言实现继承
 * @**************************************:
 */

#include <stdio.h>

struct Animal
{
	int eyeColor;
	void (*ShowEyeColor)(struct Animal *ths);
	int callNum;
	void (*Call)(struct Animal *ths);
};

void ShowEyeColor(struct Animal *ths)
{
	if (ths->eyeColor == 1)
	{
		printf("眼睛是绿色\n");
	}
	else
	{
		printf("眼睛是蓝色\n");
	}
	return;
}

void Call(struct Animal *ths)
{
	printf("叫%d声\n", ths->callNum);
	return;
}

// struct Animal 的构造函数
void Animal(struct Animal *ths, int eyeColor, int callNum)
{
	ths->eyeColor = eyeColor;
	ths->ShowEyeColor = ShowEyeColor;
	ths->callNum = callNum;
	ths->Call = Call;
	return;
}

struct Dog
{
	struct Animal animal;
};

// struct Dog 的构造函数
void Dog(struct Dog *ths, int eyeColor, int callNum)
{
	Animal(ths, eyeColor, callNum);
	// 狗类的其它属性，略
	return;
}

// struct Dog 的析构函数
void _Dog(struct Dog *ths)
{
}

struct Cat
{
	struct Animal animal;
	// 猫类的其它属性，略
};

// struct Cat 的构造函数
void Cat(struct Cat *ths, int eyeColor, int callNum)
{
	Animal(ths, eyeColor, callNum);
	return;
}

// struct Cat 的析构函数
void _Cat(struct Cat *ths)
{
}

main()
{
	struct Dog myDog;
	Dog(&myDog, 1, 3);
	myDog.animal.ShowEyeColor(&myDog);
	myDog.animal.Call(&myDog);
	_Dog(&myDog);

	struct Cat myCat;
	Cat(&myCat, 2, 5);
	myCat.animal.ShowEyeColor(&myCat);
	myCat.animal.Call(&myCat);
	_Cat(&myCat);

	return;
}
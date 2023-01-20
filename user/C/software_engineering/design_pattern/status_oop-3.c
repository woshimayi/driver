/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/design_pattern/status_oop-3.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-20 15:35:26
 * @LastEditors: dof
 * @LastEditTime: 2023-01-20 15:40:30
 * @Descripttion: c语言实现多态  相同的属性，不通的特性，如叫声, 添加私有属性
 * @**************************************:
 */

#include <stdio.h>

struct Animal
{
	int eyeColor;
	void (*ShowEyeColor)(struct Animal *this);
	int callNum;
	void (*Call)(struct Animal *this);
};

void ShowEyeColor(struct Animal *this)
{
	if (this->eyeColor == 1)
	{
		printf("眼睛是绿色\n");
	}
	else
	{
		printf("眼睛是蓝色\n");
	}
	return;
}

void Call(struct Animal *this)
{
	printf("叫%d声\n", this->callNum);
	return;
}

/* struct Animal 的构造函数 */
void Animal(struct Animal *this, int eyeColor, int callNum)
{
	this->eyeColor = eyeColor;
	this->ShowEyeColor = ShowEyeColor;
	this->callNum = callNum;
	this->Call = Call;
	return;
}

struct Dog
{
	struct Animal animal;
	void (*watch)(void);
};

void watch(void)
{
	printf("watch \n");
}

void Bark(struct Dog *this)
{
	int i;
	for (i = 0; i < this->animal.callNum; i++)
	{
		printf("汪 ");
	}
	printf("\n");
	return;
}

/* struct Dog 的构造函数 */
void Dog(struct Dog *this, int eyeColor, int callNum)
{
	Animal(this, eyeColor, callNum);
	this->animal.Call = Bark;
	this->watch = watch;
	return;
}

// struct Dog 的析构函数
void _Dog(struct Dog *this)
{
}

struct Cat
{
	struct Animal animal;
	void (*catchMouse)(void);
};

void catchMouse(void)
{
	printf("catch mouse \n");
}

void Meow(struct Cat *this)
{
	int i;
	for (i = 0; i < this->animal.callNum; i++)
	{
		printf("喵 ");
	}
	printf("\n");
	return;
}

/* struct Cat 的构造函数 */
void Cat(struct Cat *this, int eyeColor, int callNum)
{
	Animal(this, eyeColor, callNum);
	this->animal.Call = Meow;
	this->catchMouse = catchMouse;
	return;
}

// struct Cat 的析构函数
void _Cat(struct Cat *this)
{
}

int main()
{
	struct Dog myDog;
	Dog(&myDog, 1, 3);

	struct Cat myCat;
	Cat(&myCat, 2, 5);

	// dog attribute
	struct Animal *animal = &myDog;
	animal->Call(&myDog);
	myDog.watch();

	// cat attribute
	animal = &myCat;
	animal->Call(&myCat);
	myCat.catchMouse();

	_Dog(&myDog);
	_Cat(&myCat);

	return 0;
}
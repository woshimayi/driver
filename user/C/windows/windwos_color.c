#include <windows.h> 
#include <stdio.h>

void print_black()		//黑色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,0);
} 

void print_blue()		//蓝色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,1);
}

void print_green()		//绿色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,2);
}

void print_reseda()		//浅绿色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,3);
}

void print_red()		//红色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,4);
}

void print_purple()		//紫色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,5);
}

void print_yellow()		//黄色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,6);
}

void print_white()		//白色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,7);
}

void print_gray()		//灰色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,8);
}

void print_bluish()		//淡蓝色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,9);
}

void print_ondine()		//淡绿色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,10);
}

void print_light_ondine()	//淡浅绿色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,11);
}

void print_reddish()		//淡红色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,12);
}

void print_lavender()		//淡紫色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,13);
}

void print_faint_yellow()	//淡黄色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,14);
}

void print_gloss_while()	//亮白色
{
	HANDLE hOut;		//  获取输出流的句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,15);
}
  
int main() 
{ 
     print_blue();			//蓝色
	 printf("\t蓝色\n");
	 print_gloss_while();
	 print_green();			//绿色
	 printf("\t绿色\n");
	 print_gloss_while();
	 print_reseda();		//浅绿色
	 printf("\t浅绿色\n");
	 print_gloss_while();
	 print_red();			//红色
	 printf("\t红色\n");
	 print_gloss_while();
	 print_purple();		//紫色
	 printf("\t紫色\n");
	 print_gloss_while();
	 print_yellow();		//黄色
	 printf("\t黄色\n");
	 print_gloss_while();
	 print_white();			//白色
	 printf("\t白色\n");
	 print_gloss_while();
	 print_gray();			//灰色
	 printf("\t灰色\n");
	 print_gloss_while();
	 print_bluish();		//淡蓝色
	 printf("\t淡蓝色\n");
	 print_gloss_while();
	 print_ondine();		//淡绿色
	 printf("\t淡绿色\n");
	 print_gloss_while();
	 print_light_ondine();	//淡浅绿色
	 printf("\t淡浅绿色\n");
	 print_gloss_while();
	 print_reddish();		//淡红色
	 printf("\t淡红色\n");
	 print_gloss_while();
	 print_lavender();		//淡紫色
	 printf("\t淡紫色\n");
	 print_gloss_while();
	 print_faint_yellow();	//淡黄色
	 printf("\t淡黄色\n");
	 print_gloss_while();	//亮白色
	 printf("\t颜色\n");

	 system("pause");
     return 0; 
}

#include <windows.h> 
#include <stdio.h>

void print_black()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,0);
} 

void print_blue()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,1);
}

void print_green()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,2);
}

void print_reseda()		//ǳ��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,3);
}

void print_red()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,4);
}

void print_purple()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,5);
}

void print_yellow()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,6);
}

void print_white()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,7);
}

void print_gray()		//��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,8);
}

void print_bluish()		//����ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,9);
}

void print_ondine()		//����ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,10);
}

void print_light_ondine()	//��ǳ��ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,11);
}

void print_reddish()		//����ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,12);
}

void print_lavender()		//����ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,13);
}

void print_faint_yellow()	//����ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,14);
}

void print_gloss_while()	//����ɫ
{
	HANDLE hOut;		//  ��ȡ������ľ��
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);   
	SetConsoleTextAttribute(hOut,15);
}
  
int main() 
{ 
     print_blue();			//��ɫ
	 printf("\t��ɫ\n");
	 print_gloss_while();
	 print_green();			//��ɫ
	 printf("\t��ɫ\n");
	 print_gloss_while();
	 print_reseda();		//ǳ��ɫ
	 printf("\tǳ��ɫ\n");
	 print_gloss_while();
	 print_red();			//��ɫ
	 printf("\t��ɫ\n");
	 print_gloss_while();
	 print_purple();		//��ɫ
	 printf("\t��ɫ\n");
	 print_gloss_while();
	 print_yellow();		//��ɫ
	 printf("\t��ɫ\n");
	 print_gloss_while();
	 print_white();			//��ɫ
	 printf("\t��ɫ\n");
	 print_gloss_while();
	 print_gray();			//��ɫ
	 printf("\t��ɫ\n");
	 print_gloss_while();
	 print_bluish();		//����ɫ
	 printf("\t����ɫ\n");
	 print_gloss_while();
	 print_ondine();		//����ɫ
	 printf("\t����ɫ\n");
	 print_gloss_while();
	 print_light_ondine();	//��ǳ��ɫ
	 printf("\t��ǳ��ɫ\n");
	 print_gloss_while();
	 print_reddish();		//����ɫ
	 printf("\t����ɫ\n");
	 print_gloss_while();
	 print_lavender();		//����ɫ
	 printf("\t����ɫ\n");
	 print_gloss_while();
	 print_faint_yellow();	//����ɫ
	 printf("\t����ɫ\n");
	 print_gloss_while();	//����ɫ
	 printf("\t��ɫ\n");

	 system("pause");
     return 0; 
}

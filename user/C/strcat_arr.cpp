#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
	char str[1024];
	int i = 0;
	char * num = "asdfgh";
	char * num1 = "zxcvbn";
	char * num2 = "qwerty";
	strncat(&str[i], num, 32);
	printf("str = %s\n", &str[i]);
	i+=32;
	strncat(&str[i], num1, 32);
	printf("str = %s\n", &str[i]);
	i+=32;
	strncat(&str[i], num2, 32);
	printf("str = %s\n", &str[i]);
	printf("%s %d\n", str, 8*sizeof(num));
	return 0;
}


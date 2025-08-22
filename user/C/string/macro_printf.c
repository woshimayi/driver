#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define FIRWARE "/tmp/firware"


#define PP(fmt, ...) printf("\033[0;32;31m[zzzzz :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##__VA_ARGS__)

#define PRINT_INT(var) printf("\033[0;32;31m[zzzzz :%s(%d)] %s = %d \033[1;37m\n", __func__, __LINE__, #var, (int)(var))

// --- Macro for Unsigned Integers ---
#define PRINT_UINT(var) printf("\033[0;32;31m[zzzzz :%s(%d)] %s = %u \033[1;37m\n", __func__, __LINE__, #var, (unsigned int)(var))

// --- Macro for Long Long Integers ---
#define PRINT_LL(var) printf("\033[0;32;31m[zzzzz :%s(%d)] %s = %lld \033[1;37m\n", __func__, __LINE__, #var, (long long)(var))

// --- Macro for Floating-Point Numbers (double, float) ---
#define PRINT_DOUBLE(var) printf("\033[0;32;31m[zzzzz :%s(%d)] %s = %lf \033[1;37m\n", __func__, __LINE__, #var, (double)(var))

// --- Macro for Characters ---
#define PRINT_CHAR(var) printf("\033[0;32;31m[zzzzz :%s(%d)] %s = '%c' \033[1;37m\n", __func__, __LINE__, #var, (char)(var))

// --- Macro for String Pointers (char*) ---
#define PRINT_STRING(var) printf("\033[0;32;31m[zzzzz :%s(%d)] %s = \"%s\" \033[1;37m\n", __func__, __LINE__, #var, (char*)(var))

// --- Macro for Generic Pointers (void*) ---
#define PRINT_PTR(var) printf("\033[0;32;31m[zzzzz :%s(%d)] %s = %p \033[1;37m\n", __func__, __LINE__, #var, (void*)(var))


int main(int argc, const char *argv[])
{
	//	printf("##FIRWARE %s", FIRWARE);
	char *str = NULL;
	char buf[64] = {0};
	char pNum[] = "19";
	int a;

	printf("ss %d\n", atoi(pNum));
	printf("ss %d\n", buf);
	//	sscanf(str, "%s", buf);
	//	printf("buf = %s\n", buf);

	a = strtoul(pNum, NULL, 10); //最后的0，表示自动识别pNum是几进制
	printf("%d\n", a);
	PRINT_INT(a);

	return 0;
}


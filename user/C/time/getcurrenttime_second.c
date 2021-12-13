#include <stdio.h>
#include <time.h>

int main(void)
{
	time_t result;

	result = time(NULL);
	printf("%ld \n", result); // 获取从1970到现在的秒数
	printf("%s \n", asctime(localtime(&result))); // 转化为当地时间 ascii
	return (0);
}

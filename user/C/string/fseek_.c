#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void main(void)
{
	FILE *stream;
	long l;
	float fp;
	char s[81];
	char c;
	stream = fopen("firewall", "r");
	if (stream == NULL) // 打开文件失败
	{
		printf("the file is opeaned error!\n");
	}
	else  // 输出信息
	{
		//        fprintf(stream,"%s %ld %f %c","a_string",6500,3.1415,'x');  // 格式化
		fseek(stream, 100, 0); // 文件定位
		fread(s, sizeof(char), sizeof(s), stream);
		printf("s = %s\n", s);
		fseek(stream, 20, 1); // 文件定位
		//        fscanf(stream,"%s",s);
		//        fscanf(stream,"%ld",&l);
		//        fscanf(stream,"%f",&fp);
		//        fscanf(stream," %c",&c);
		//        printf("%s\n",s);
		//        printf("%ld\n",l);
		//        printf("%f\n",fp);
		//        printf("%c\n",c);
		fread(s, sizeof(char), sizeof(s), stream);
		printf("s = %s\n", s);
		fclose(stream);  // 关闭
	}
}

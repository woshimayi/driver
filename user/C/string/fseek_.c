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
	if (stream == NULL) // ���ļ�ʧ��
	{
		printf("the file is opeaned error!\n");
	}
	else  // �����Ϣ
	{
		//        fprintf(stream,"%s %ld %f %c","a_string",6500,3.1415,'x');  // ��ʽ��
		fseek(stream, 100, 0); // �ļ���λ
		fread(s, sizeof(char), sizeof(s), stream);
		printf("s = %s\n", s);
		fseek(stream, 20, 1); // �ļ���λ
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
		fclose(stream);  // �ر�
	}
}

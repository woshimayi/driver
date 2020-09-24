#include <stdio.h>
#include <stdlib.h>
#include <string.h>



char * getFileName(const char * file)
{
	strncpy(string, argv[1], sizeof(string));
	tok = strtok(string, ".");
	printf("tok = %s\n", tok);
	while(tok = strtok(NULL, "."))
	{
		printf("tok = %s %ld\n", tok, strlen(tok));
	}
	printf("filename = %s\n", );
	
}


int main(int argc, const char * argv[])
{
	char string[128] =  "asdasda.bin";
	char cmd[128] = {0};
	char * tok;
	
	
	return 0;
}



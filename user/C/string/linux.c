#include <stdio.h>
#include <unistd.h>

int main()
{
#ifdef linux
	printf("linux");
#endif

#ifdef  __WIN32
	printf("windows");
#endif


	char buffer[100];
	getcwd(buffer, sizeof(buffer));
	printf("The current directory is: %s\n", buffer);
	//printf("prog name : %s \n" , argv[0]);
	return 0;

}

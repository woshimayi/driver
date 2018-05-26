#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, const char *argv[])
{
	int  fd;
	char buf[1024];

	if(argc  <  2)
	{
	
		printf("./a.out filename  = %s\n",argv[0]);
		exit(EXIT_FAILURE);
	
	}
	fd  = open(argv[1],O_RDWR);
	if(fd  <  0)
	{
	
		perror("open file fail");
		exit(EXIT_FAILURE);
	
	}
	read(fd,buf,sizeof(buf));
	write(fd,buf,sizeof(buf));


	printf("open file  success  \n");
	return 0;
}

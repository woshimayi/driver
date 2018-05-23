#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "head.h"
int main(int argc, char *argv[])
{
	int ret;
	int fd;
	char buf[125];
	char wbuf[125] = "i am from test";

	fd = open("/dev/char_dev",O_RDWR);
	if (fd < 0)
	{
		printf(" open ./char_dev fail!!!\n");
		return -1;
	}
	else
	{
		printf(" open success,fd = %d\n",fd);
	}

	printf("wbuf = %s\n", wbuf);
	ret = write(fd,wbuf,strlen(wbuf)+1);
	if (ret >= 0)
	{
		printf("success write\n");
//		ioctl(fd,HELLO_ONE);
//		ioctl(fd,HELLO_TWO,99);
	}


	memset(buf,'\0',sizeof(buf));
	ret = read(fd,buf,sizeof(buf));
	if (ret <= 0)
	{
		printf(" read fail!!!\n");
	}
	printf("buf = %s,ret = %d\n",buf,ret);

	close(fd);
	return 0;
}

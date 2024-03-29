/*****************************************************
** Name         : crc32.c
** Author       : gzshun
** Version      : 1.0
** Date         : 2011-12
** Description  : CRC32 Checking
******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFSIZE     1024*4

static unsigned int crc_table[256];
const static char *program_name = "crc32";

static void usage(void);
static void init_crc_table(void);
static unsigned int crc32(unsigned int crc, unsigned char *buffer, unsigned int size);
static int calc_img_crc(const char *in_file, unsigned int *img_crc);

static void usage(void)
{
	fprintf(stderr, "Usage: %s input_file\n", program_name);
}

/*
**初始化crc表,生成32位大小的crc表
**也可以直接定义出crc表,直接查表,
**但总共有256个,看着眼花,用生成的比较方便.
*/
static void init_crc_table(void)
{
	unsigned int c;
	unsigned int i, j;

	for (i = 0; i < 256; i++)
	{
		c = (unsigned int)i;
		for (j = 0; j < 8; j++)
		{
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[i] = c;
	}
}

/*计算buffer的crc校验码*/
static unsigned int crc32(unsigned int crc, unsigned char *buffer, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++)
	{
		crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
	return crc ;
}

/*
**计算大文件的CRC校验码:crc32函数,是对一个buffer进行处理,
**但如果一个文件相对较大,显然不能直接读取到内存当中
**所以只能将文件分段读取出来进行crc校验,
**然后循环将上一次的crc校验码再传递给新的buffer校验函数,
**到最后，生成的crc校验码就是该文件的crc校验码.(经过测试)
*/
static int calc_img_crc(const char *in_file, unsigned int *img_crc)
{
	int fd;
	int nread;
	int ret;
	unsigned char buf[BUFSIZE];
	/*第一次传入的值需要固定,如果发送端使用该值计算crc校验码,
	**那么接收端也同样需要使用该值进行计算*/
	unsigned int crc = 0xffffffff;

	fd = open(in_file, O_RDONLY);
	if (fd < 0)
	{
		printf("%d:open %s.\n", __LINE__, strerror(errno));
		return -1;
	}

	while ((nread = read(fd, buf, BUFSIZE)) > 0)
	{
		crc = crc32(crc, buf, nread);
	}
	*img_crc = crc;

	close(fd);

	if (nread < 0)
	{
		printf("%d:read %s.\n", __LINE__, strerror(errno));
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret;
	unsigned int img_crc;
	const char *in_file = argv[1];

	if (argc < 2)
	{
		usage();
		exit(1);
	}

	init_crc_table();

	ret = calc_img_crc(in_file, &img_crc);
	if (ret < 0)
	{
		exit(1);
	}

	printf("The crc of %s is:%x\n", in_file, img_crc);

	return 0;
}

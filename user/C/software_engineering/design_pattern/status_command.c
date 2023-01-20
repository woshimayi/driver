/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/design_pattern/status_command.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-20 14:24:28
 * @LastEditors: dof
 * @LastEditTime: 2023-01-20 14:29:46
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>

// 当心字节对齐的问题
typedef struct
{
	unsigned char head;
	unsigned char cmd;
	unsigned short int  length;
	unsigned char data[1];
} package_t;

static void parse_temperature(unsigned char *buffer)
{
	int value = *buffer;
	printf("temperature = %d\n", value);
}
static void parse_humidity(unsigned char *buffer)
{
	int value = *buffer;
	printf("humidity = %d\n", value);
}
static void parse_illumination(unsigned char *buffer)
{
	int value = *buffer;
	printf("illumination = %d\n", value);
}

typedef struct
{
	unsigned char cmd;
	void (*handle)(unsigned char *buffer);
} parse_handler;

static const parse_handler handlers[] = {
	{0x01, parse_temperature},
	{0x02, parse_humidity},
	{0x03, parse_illumination},
	{0xFF, NULL},
};

static unsigned char parse(unsigned char *buffer, unsigned short int  length)
{
	package_t *frame = (package_t *)buffer;
	unsigned char tail = buffer[length - 1];
	const parse_handler *entry;

	if (frame->head != 0xFF)
	{
		return 0;
	}

	for (entry = handlers; entry->handle != NULL; ++entry)
	{
		if (frame->cmd == entry->cmd)
		{
			entry->handle(frame->data);
			break;
		}
	}

	return 1;
}

int main(void)
{
	unsigned char buffer[] = {0xFF, 0x01, 0x01, 0x02, 0x03};
	parse(buffer, 5);
	unsigned char buffer_2[] = {0xFF, 0x02, 0x02, 0x03, 0x04};
	parse(buffer_2, 5);
	unsigned char buffer_3[] = {0xFF, 0x03, 0x03, 0x04, 0x05};
	parse(buffer_3, 5);
}
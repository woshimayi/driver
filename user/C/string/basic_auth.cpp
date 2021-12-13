/*
 * @*************************************:
 * @FilePath: /user/C/string/basic_auth.cpp
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:09:06
 * @Descripttion: bash64 加密
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char table64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t b64_encode(const char *inp, size_t insize, char **outptr)
{
	unsigned char ibuf[3];
	unsigned char obuf[4];
	int i;
	int inputparts;
	char *output;
	char *base64data;

	char *indata = (char *)inp;

	*outptr = NULL; /* set to NULL in case of failure before we reach the end */

	if (0 == insize)
		insize = strlen(indata);

	base64data = output = (char *)malloc(insize * 4 / 3 + 4);
	if (NULL == output)
		return 0;

	while (insize > 0)
	{
		for (i = inputparts = 0; i < 3; i++)
		{
			if (insize > 0)
			{
				inputparts++;
				ibuf[i] = *indata;
				indata++;
				insize--;
			}
			else
				ibuf[i] = 0;
		}

		obuf [0] = (ibuf [0] & 0xFC) >> 2;
		obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
		obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
		obuf [3] = ibuf [2] & 0x3F;

		switch (inputparts)
		{
			case 1: /* only one byte read */
				snprintf(output, 5, "%c%c==",
				         table64[obuf[0]],
				         table64[obuf[1]]);
				break;
			case 2: /* two bytes read */
				snprintf(output, 5, "%c%c%c=",
				         table64[obuf[0]],
				         table64[obuf[1]],
				         table64[obuf[2]]);
				break;
			default:
				snprintf(output, 5, "%c%c%c%c",
				         table64[obuf[0]],
				         table64[obuf[1]],
				         table64[obuf[2]],
				         table64[obuf[3]]);
				break;
		}
		output += 4;
	}
	*output = 0;
	*outptr = base64data; /* make it return the actual data memory */

	return strlen(base64data); /* return the length of the new data */
}

static void generateBasicAuth(char *user, char *pwd)
{
	char    raw[256];
	size_t  dataLen;
	char    *b64Buf;
	size_t  b64Len;

	strncpy(raw, user, sizeof(raw));
	strncat(raw, ":", sizeof(raw));
	strncat(raw, pwd, sizeof(raw));
	dataLen = strlen(raw);
	b64Len = b64_encode(raw, dataLen, &b64Buf);
	printf("%s\n", b64Buf);
}




int main()
{
	char usr[] = "";
	char pwd[] = "";
	generateBasicAuth(usr, pwd);
	return 0;
}



/*
 * @*************************************:
 * @FilePath: /user/C/printf_memory_data.c
 * @version:
 * @Author: dof
 * @Date: 2024-06-13 16:23:12
 * @LastEditors: dof
 * @LastEditTime: 2024-06-13 17:34:40
 * @Descripttion: 打印内存数据
 * @**************************************:
 */

void mutils_dump_hex_more_data(const unsigned char *head, int len)
{
#define LINE_CHAR_NUM 16
#define ADDR_LEN 12
    int i;
    int n;
    const unsigned char *curPtr = head;
    char lineBuf[160];
    char tmpBuf[40] = {0};
    char *p;
    char *pText;

    if (!head)
        return;

    printf("\n");
    for (i = 0; i < len;)
    {
        p = lineBuf;
        p += sprintf(p, "%xh:  ", (unsigned long)(head + i));
        pText = tmpBuf;
        for (n = 0; (i < len) && (n < LINE_CHAR_NUM); n++, i++, curPtr++)
        {
            p += sprintf(p, "%02X ", *curPtr);
            if (n == 7)
            {
                p += sprintf(p, " ");
            }
            pText += sprintf(pText, "%c", ((*curPtr >= 0x20) && (*curPtr <= 0x7e) ? *curPtr : '.'));
        }

        if (n < 7)
        {
            p += sprintf(p, " ");
        }

        while (n < LINE_CHAR_NUM)
        {
            p += sprintf(p, "   ");
            n++;
        }
        p += sprintf(p, "%s\n", tmpBuf);

        printf("%s", lineBuf);
    }
}

char *str = "ssssssss";

int main(int argc, char const *argv[])
{

    mutils_dump_hex_more_data(str, 1024);
    char p = str + 20;
    p = "dddddddddd";
    mutils_dump_hex_more_data(str, 1024);


    return 0;
}

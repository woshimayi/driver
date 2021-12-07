/*
 * @*************************************: 
 * @FilePath: /user/C/string/7525_mac_telnet_passwd.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:04:18
 * @Descripttion: 根据mac 计算crc 登录密码 7525 mac telnet passwd
 * @**************************************: 
 */

# include <stdio.h>
# include <string.h>
// char*,unsigned char*记录的就是二进制转换到的十六进制的字节码[整数原码补码，浮点数IEEE74原码，字符编码原码]。
// char,unsigned char会转换到字符类型; 但是char,unsigned char整数运算还是十六进制的字节码。
// 所以可以提供一个对于十六进制的字节码直接进行CRC计算。
typedef unsigned int UINT32 ;
static UINT32 POLYNOMIAL = 0xEDB99320 ;
UINT32 g_CRCTable[256] = {0};

void init_table()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < 256; i++)
        for (j = 0, g_CRCTable[i] = i ; j < 8 ; j++)
            g_CRCTable[i] = (g_CRCTable[i] >> 1) ^ ((g_CRCTable[i] & 1) ? POLYNOMIAL : 0);
}

UINT32 transition_mac_crc(UINT32 crc, char *buff, int len)
{
    int i = 0;
    crc = ~crc;
    for (i = 0; i < len; i++)
    {
        int data = buff[i];
        crc = (crc >> 8) ^ g_CRCTable[(crc ^ buff[i]) & 0xff];
    }
    return ~crc;
}


int main()
{
    char passwdCrc[128] = {0};
    char szMAC[128] = "78d99f694d92";
    while (1)
    {
        printf("input mac(eg:78d99f694d92):\n");
        scanf("%s", &szMAC);
        if (strlen(szMAC) != 12)
        {
            printf("Input error, pls check it!\n");
            return -1;
        }
        // 初始化表
        init_table();
        snprintf(passwdCrc, sizeof(passwdCrc), "%08X", transition_mac_crc(1314520, szMAC, strlen(szMAC)));
        printf("password: %s\n", passwdCrc);
    }
    return 0 ;
}


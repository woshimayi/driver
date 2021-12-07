/*
 * @*************************************: 
 * @FilePath: /user/C/string/[]_{}_NULL.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 13:55:22
 * @Descripttion: 结构体赋值
 * @**************************************: 
 */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define PP printf("[%s:%d]\n", __FUNCTION__, __LINE__);

typedef char UINT8;

typedef struct
{
    UINT8 SsidNo;
    UINT8 SsidAdmin;
    UINT8 SsidBroadCastAdmin;
    UINT8 SsidName[32];
    UINT8 EncryptMode;
    UINT8 EncryptKey[64];
} _OamSvaSsidConfig;

int edc(_OamSvaSsidConfig *oamSvaSsidConfig)
{

    printf("struct addr is %p\n", oamSvaSsidConfig);
    printf("oamSvaSsidConfig->SsidNo = %d\n", oamSvaSsidConfig->SsidNo);
    printf("oamSvaSsidConfig->SsidAdmin = %d\n", oamSvaSsidConfig->SsidAdmin);
    printf("oamSvaSsidConfig->SsidBroadCastAdmin = %s\n", oamSvaSsidConfig->SsidBroadCastAdmin);
    printf("oamSvaSsidConfig->SsidName = %s\n", oamSvaSsidConfig->SsidName);
    printf("oamSvaSsidConfig->EncryptMode = %d\n", oamSvaSsidConfig->EncryptMode);
    printf("oamSvaSsidConfig->EncryptKey = %s\n", oamSvaSsidConfig->EncryptKey);
    char num[7] = "";
    strncpy(num, "wl0", sizeof("wl0"));
    printf("%s\n", num);
    return 0;
}

int abc()
{
    _OamSvaSsidConfig oamSvaSsidConfig = {1, 1, 0, "JQM-www3w", 2, "werrtyyui"};
    printf("struct addr is %p\n", &oamSvaSsidConfig);
    edc(&oamSvaSsidConfig);
    return 0;
}

int main()
{
    abc();
    return 0;
}


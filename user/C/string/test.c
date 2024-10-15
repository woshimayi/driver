/*
 * @*************************************: 
 * @FilePath: /user/C/string/test.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-06-01 13:55:16
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

int main()
{
    //	char str[1024] = "Some JSON:
    //	\{
    //	    \"name\": "Jack (\"Bee\") Nimble\",
    //	    \"format\": {
    //	        \"type\":       \"rect\",
    //	        \"width\":      1920,
    //	        \"height\":     1080,
    //	        \"interlace\":  false,
    //	        \"frame rate\": 24
    //	    \}
    //	\}";

    char pv[128]  = "10.10.10.10/24";
    char tmp[128] = {0};

    struct in_addr v4_addr;
    strncpy(tmp, pv, sizeof(pv));
    if (inet_pton(AF_INET, strtok(tmp, "/"), &v4_addr.s_addr))
    {
        printf("0 %s\n", pv);
    }
    else
    {
        printf("1 %s\n", pv);
    }

    if (-1)
    {
        printf("if\n");
    }
    else
    {
        printf("else\n");
    }
        // printf("pv = %s\n", &pv[15]);
        // printf("pv = %s\n", &pv[15]);

        //	pv = &pv[15];
#define CWMP_WRITE     0x01
#define CWMP_READ      0x02
#define CWMP_LNKLIST   0x04
#define CWMP_DENY_ACT  0x08
#define CWMP_FORCE_ACT 0x10
#define CWMP_ISPASSWORD    0x20
#define CWMP_ISVOIP    0x40
#define CWMP_ISWLAN    0x80
#define CWMP_HIDDEN    0x100

    int flag = CWMP_WRITE|CWMP_READ|CWMP_HIDDEN;
    printf("flag = 0x%x\n", flag);
    flag  &= ~(CWMP_HIDDEN|CWMP_READ);
    printf("flag = 0x%x\n", flag);

    return 0;
}



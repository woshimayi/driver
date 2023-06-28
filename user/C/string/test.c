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
        // printf("pv = %s\n", &pv[15]);
        // printf("pv = %s\n", &pv[15]);

        //	pv = &pv[15];

    return 0;
}



/*
 * @*************************************: 
 * @FilePath: /user/C/string/password_check.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-08 10:33:35
 * @LastEditors: dof
 * @LastEditTime: 2022-07-08 10:35:04
 * @Descripttion: password check
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief 密码复杂性检查
 * 
 * @param strPwd 
 * @return int 
 */
int util_password_verification(char *strPwd)
{
#define X_CMCC_TeleCom_Password_Min_Len 8
#define X_CMCC_TeleCom_Password_Max_Len 64

    char *ch = NULL;
    unsigned char flags = 0x00;
    
    if (NULL == strPwd) {
        cmsLog_error("The strPwd is NULL.");
        return -1;
    }
    
    size_t len = strlen(strPwd);
    if ((len < X_CMCC_TeleCom_Password_Min_Len) || (len > X_CMCC_TeleCom_Password_Max_Len)) {
        cmsLog_error("The length of password is wrong.");
        return -1;
    }

    ch = strPwd;
    while (*ch != '\0') {
        if (isdigit(*ch))
        {
            flags |= 0x01;
        }
        else if (isalpha(*ch))
        {
            flags |= 0x02;
        }
        else if (isInextCH(*ch))
        {
            flags |= 0x04;
        }
        else
        {
            return -1;
        }

        ch++;
    }

    if ((flags & 0x07) == 0x07 )
    {
        return 0;
    }
    else
    {
        return -1;
    }    
}

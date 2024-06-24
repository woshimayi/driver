/*
 * @*************************************:
 * @FilePath: /network/ifname_status/getifname_maxSpeed.c
 * @version:
 * @Author: dof
 * @Date: 2024-06-06 13:59:55
 * @LastEditors: dof
 * @LastEditTime: 2024-06-06 14:03:46
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cmsUtl_devGetLanAbility(char *LanAbility, int len, int count)
{
    char line[256] = {0};
    char line1[256] = {0};
    int maxSpeed = 0;
    char result[512] = {0};
    char tmp_cmd[512] = {0};
    int num = 0;

    if (NULL == LanAbility || 0 == len)
    {
        return -1;
    }

    FILE *fp1 = popen("ifconfig | grep enp0s3 | awk -F ': ' '{print $1}'", "r");
    while (fgets(line1, sizeof(line1), fp1))
    {
        if (strchr(line1, '.'))
        {
            continue;
        }
        line1[strlen(line1) - 1] = 0;
        snprintf(tmp_cmd, sizeof(tmp_cmd), "ethtool %s | grep Full", line1);
        FILE *fp = popen(tmp_cmd, "r");
        memset(line, 0, sizeof(line));
        while (fgets(line, sizeof(line), fp))
        {
            if (strstr(line, "Advertised link modes"))
            {
                break;
            }
            sscanf(line, "%dbaseT/Full", &maxSpeed);
        }
        switch (maxSpeed)
        {
        case 1000:
            strcat(result, "GE");
            break;
        case 2500:
            strcat(result, "2.5GE");
            break;
        case 5000:
            strcat(result, "5GE");
            break;
        case 10000:
            strcat(result, "10GE");
            break;
        default:
            strcat(result, "FE");
            break;
        }
        strcat(result, ",");

        pclose(fp);
        if (++num == count)
            break;
    }

    result[strlen(result) - 1] = '\0';
    strncpy(LanAbility, result, len);
    pclose(fp1);

    return num;
}

int main(int argc, char const *argv[])
{
    int getCount = 0;
    char tmp_LanAbility[128] = {0};
    getCount = cmsUtl_devGetLanAbility(tmp_LanAbility, sizeof(tmp_LanAbility), 4);
    printf("getCount = %d tmp_LanAbility = %s\n", getCount, tmp_LanAbility);
    return 0;
}

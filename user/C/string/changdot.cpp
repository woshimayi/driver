#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char pp[1024] = "InternetGatewayDevice.Device.DeviceInfo.TemperatureStatus.";
    char *temp = pp;
    char *pos = strstr(temp, ".");

    while (pos)
    {
        temp = pos;
        pos = strstr(temp + 1, ".");
    }

    if (temp && (NULL == pos))
    {
        *temp = ' ';
    }
    printf("pp = %s", pp);

    return 0;
}


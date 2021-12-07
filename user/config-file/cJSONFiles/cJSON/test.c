#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    char pv[128]  = "InternetGatewayDevice.DeviceInfo.ProvisioningCode";

    //	pv = &pv[15];
    printf("pv = %s\n", &pv[15]);


    return 0;
}



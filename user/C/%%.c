#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int s = 100;
    int k = 0;
    //	char str[1024] = "{"RPCMethod": "Result", "ID": 101, "ObjectPath": "BUCPE.SiteSpeed", "TaskStatus": 0, "Params": { "BUCPE": { "SiteSpeedResult": { "DestUrl": "http://down.360safe.com/setup.exe", "DestIP": "180.153.197.44", "Threads": "1", "SpeedTable": [ 4622175, 6309792, 6265840, 6294784, 6303360, 6298000, 6312252, 6313008, 6609952, 4058890 ], "Error": null } } } }";
    while (1)
    {
        s++;
        k = (s - 101) % 8;
        printf("k = %d\n", k);
        if (s > 500)
        {
            break;
        }
    }
    return 0;
}



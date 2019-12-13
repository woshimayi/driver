#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{

    char buf[128] = {0};
    char *str = NULL;
    FILE *fd = NULL;

    fd = fopen("./default.cfg", "r");
    if (fd == NULL)
    {
        printf("fail open default.cfg");
    }

    while (NULL != fgets(buf, sizeof(buf), fd))
    {
        if (strstr(buf, "COM_IPProtocolVersion"))
        {
            fgets(buf, sizeof(buf), fd);
            if (buf != NULL)
            {
                if (strstr(buf,  "<Mode>3</Mode>"))
                {
                    printf("%s\n", buf);
                }
                break;
            }
        }
    }

    return 0;
}



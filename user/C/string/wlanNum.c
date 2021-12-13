#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int OamCtcGetWlanEnableNum()
{
	FILE *fd = NULL;
	char  tmp[100] = {0};
	int num = 0;
	fd = fopen("ifconfig.log", "r");
	if (fd != NULL)
	{
		while (NULL != fgets(tmp, sizeof(tmp), fd))
		{
			if (NULL != strstr(tmp, "wl0"))
			{
				num++;
			}
		}
	}
	fclose(fd);
	if (num >= 2)
	{
		num = 4;
	}

	printf("num = %d\n", num);

	return num;
}




int main()
{
	//	char  tmp[100] = {0};
	//	FILE * fd = NULL;
	int num = 0;
	//	fd = fopen("ifconfig.log", "r");
	//	if(fd != NULL)
	//	{
	//			while(NULL != fgets(tmp, sizeof(tmp), fd))
	//			{
	//				if(NULL != strstr(tmp, "wl0"))
	//				{
	//					printf("tmp = %s\n", tmp);
	//					num++;
	//				}
	//			}
	//	}

	//    char *p=NULL;
	//    FILE *fd=NULL;
	//    char line[128] = {0};
	//    int num = 0;
	//
	//    fd = fopen("ifconfig.log", "r");
	//    if(NULL == fd)
	//    {
	// 		printf("sdfsdfs\n");
	//    }
	//    else
	//    {
	//        while(fgets(line, sizeof(line), fd) != NULL)
	//        {
	//            if((p = strstr(line, "wl0")))
	//            {
	//                num = num + 1;
	//            }
	//        }
	//    }
	//    fclose(fd);


	num = OamCtcGetWlanEnableNum();

	printf("num = %d", num);
	return 0;
}



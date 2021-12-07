#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void token_informUrl(char *informURL, char url1[], char url2[])
{
    printf("url1 = %s\n\n", informURL);
    char *token = NULL;
    strncpy(url1, informURL, sizeof(informURL));
    token = strtok(url1, ";");
    token =  strtok(NULL, ";");
    strncpy(url2, token, sizeof(url2));
    printf("url1 = %s  url2 = %s\n\n", url1, url2);
}


int main()
{
    char sg_postURL[128]  = "http://www.bbums.cn/reg/;http://nmg-ct.cnspeedtest.com:9806/nsupload/uploadfile";
    char sg_postURL1[128]  = "http://nmg-ct.cnspeedtest.com:9806/nsupload/uploadfile";

    char *url1 = {0};
    char *url2 = {0};

    //	printf("url1 = %s  url2 = %s\n\n", url1, url2);
    //	token_informUrl(sg_postURL, url1, url2);
    //	printf("url1 = %s url2 = %s", url1, url2);



    int len = 0;
    char url[][128] = {0};
    int i = 0;
    char location[][128] = {0};
    char *token = NULL;
    char *token1 = NULL;
    char tmp[][128] = {0};

    strncpy(tmp[0], sg_postURL, sizeof(tmp[0]));
    token = strtok(tmp[0], ";");
    token =  strtok(NULL, ";");
    strncpy(tmp[1], token, sizeof(tmp[1]));

    printf("%s\n\n%s\n\n\n%s\n\n\n", tmp[0], tmp[1], sg_postURL);

    //	token = strtok(tmp[0], "/");
    //    token = strtok(NULL, "/");
    //    strncpy(url[0], token, sizeof(url[0]));
    //	printf("url[0] = %s\n", url[0]);
    //
    //	token = strtok(NULL, "/");
    //	strncpy(location[0], token, sizeof(location[0]));
    //	printf("location = %s\n", location[0]);
    //=============================================================
    token1 = strtok(tmp[0], "/");
    token1 = strtok(NULL, "/");

    printf("url[1] = %s\n", token1);
    token1 = strtok(token1, ":");
    printf("url[1] = %s\n", token1);
    token1 = strtok(NULL, ":");
    if (NULL != token1)
    {
        printf("url[1] = %d\n", atoi(token1));
    }
    //=============================================================


    //	token1 = strtok(tmp[0], "/\n");
    //	printf("url1 = %s\n", token1);
    //
    //    token1 = strtok(NULL, "/\n");
    //    printf("url2 = %s\n", token1);
    //
    //	token1 = strtok(NULL, ":\n");
    //    printf("url3 = %s\n", token1);


    return 0;
}


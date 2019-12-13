#include <stdio.h>
#include <stdlib.h>
#include <string.h>


URL_SERVER *create_informUrl(const char *informURL)
{
    char *token = NULL;
    char   url1[1024] = {0};

    strncpy(url1, informURL, sizeof(url1));
    token = strtok(url1, ";");

    while (token != NULL)
    {
        //		printf("token = %s\n", token);
        strncpy(newNode->url1, token, sizeof(newNode->url1));
        token = strtok(NULL, ";");
    }

    return ;
}


int main()
{
    char informurl3[1024] =
        "http://www.bbums.cn/reg/;http://nmg-ct.cnspeedtest.com:9806/nsupload/uploadfile;http://www.bbums.org.cn/reg123/;http://nmg-ct.cnspeedtest.com:9806/nsupload/upload";
    char informurl4[1024] =
        "http://www.bbums.cn1/reg/;http://nmg-ct.cnspeedtest.com:98061/nsupload/uploadfile;http://www.bbums.org.cn/reg123/;http://nmg-ct.cnspeedtest.com:9806/nsupload/upload";



    return 0;
}



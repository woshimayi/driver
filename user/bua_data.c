#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct URL_SERVER 
{
	char url1[128];
	int  sk1;
	char url2[128];
	int  sk2;
	struct URL_SERVER *next;
} URL_SERVER;


URL_SERVER* create_informUrl(const char *informURL, URL_SERVER *urlNode)
{
	char * token = NULL;
	char   url1[1024] = {0};
	
	if (NULL == urlNode)
	{
		printf("fail malloc\n");
		exit(-1);
	}
	URL_SERVER *tmpNode = (URL_SERVER *)malloc(sizeof(URL_SERVER));
	tmpNode->next = NULL;
	
	urlNode = tmpNode;
	
	strncpy(url1, informURL, sizeof(url1));
	token = strtok(url1, ";");
	
	while(token != NULL)
	{
//		printf("token = %s\n", token);
		URL_SERVER *newNode = (URL_SERVER *)malloc(sizeof(URL_SERVER));
		if (NULL == newNode)
		{
			printf("fail malloc\n");
			exit(-1);
		}
		strncpy(newNode->url1, token, sizeof(newNode->url1));
		
		newNode->next = NULL;
		tmpNode->next = newNode;
		tmpNode = tmpNode->next;

		token = strtok(NULL, ";");
	}
	
	return urlNode;
}


URL_SERVER* amend_informUrl(const char *informURL, URL_SERVER *urlNode)
{
	char * token = NULL;
	char   url1[1024] = {0};
	
	if (NULL == urlNode)
	{
		printf("fail malloc\n");
		exit(-1);
	}
	URL_SERVER *tmpNode = urlNode;
	tmpNode = tmpNode->next;

	strncpy(url1, informURL, sizeof(url1));
	token = strtok(url1, ";");
	
	while(token != NULL)
	{
//		printf("token = %s\n", token);
		strncpy(tmpNode->url2, token, sizeof(tmpNode->url2));
//		printf("newNode->url2 = %s\n", tmpNode->url2);
		tmpNode = tmpNode->next;
		token = strtok(NULL, ";");
	}
	
	return urlNode;
}


void list_informUrl(URL_SERVER *urlNode)
{
	while (NULL != urlNode->next)
	{
		urlNode = urlNode->next;
		if (NULL != urlNode)
		{
			printf("url = %s\n", urlNode->url1);
			printf("ur2 = %s\n\n", urlNode->url2);
		}
	}
}


int main()
{
	char informurl1[1024] = "http://www.bbums.cn/reg/;http://nmg-ct.cnspeedtest.com:9806/nsupload/uploadfile";
	char informurl2[1024] = "http://www.bbums.org.cn/reg/;http://nmg-ct.cnspeedtest.com:9806/nsupload/uploadfile";

	char informurl3[1024] = "http://www.bbums.cn/reg/;http://nmg-ct.cnspeedtest.com:9806/nsupload/uploadfile;http://www.bbums.org.cn/reg123/;http://nmg-ct.cnspeedtest.com:9806/nsupload/upload";
	char informurl4[1024] = "http://www.bbums.cn1/reg/;http://nmg-ct.cnspeedtest.com:98061/nsupload/uploadfile;http://www.bbums.org.cn/reg123/;http://nmg-ct.cnspeedtest.com:9806/nsupload/upload";	
	URL_SERVER *urlNode = (URL_SERVER *)malloc(sizeof(URL_SERVER));
	
	
	urlNode = create_informUrl(informurl1, urlNode);
	urlNode = amend_informUrl(informurl2, urlNode);
	
	list_informUrl(urlNode);
	
	return 0;
}



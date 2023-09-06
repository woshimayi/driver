/*
 * @*************************************:
 * @FilePath: /t5/src/main.c
 * @version:
 * @Author: dof
 * @Date: 2023-09-06 15:09:19
 * @LastEditors: dof
 * @LastEditTime: 2023-09-06 15:31:00
 * @Descripttion:
 * @**************************************:
 */
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
FILE *fp;

int write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, (FILE *)fp);
	return written;
}

int main()
{
	const char *path = "/tmp/curl-test";
	const char *mode = "w";
	
	fp = fopen(path, mode);
	curl_global_init(CURL_GLOBAL_ALL);
	CURLcode res;
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "http://www.baidu.com");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return 0;
}
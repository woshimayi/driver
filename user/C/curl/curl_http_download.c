/*
 * @*************************************:
 * @FilePath: /user/C/curl/curl_http_download.c
 * @version:
 * @Author: dof
 * @Date: 2023-08-10 15:48:22
 * @LastEditors: dof
 * @LastEditTime: 2024-04-08 19:59:15
 * @Descripttion: ota 升级
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

long content_length = 0;

// 回调函数，处理接收到的包头数据
size_t header_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t total_size = size * nmemb;
	printf("收到包头数据：%.*s", (int)total_size, (char *)ptr);

	return total_size;
}

// 回调函数，将下载的数据写入文件
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	printf("%s\nsize = %d\n", ptr, nmemb);
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

unsigned int http_download(const char *url, char **str)
{
	CURL *curl;
	FILE *fp;
	CURLcode res;
	long response_code = -1;

	if (NULL == url)
	{
		return -1;
	}

	// 打开文件，准备写入下载的数据
	fp = fopen("config_tmp.ini", "wb");
	if (fp == NULL)
	{
		printf("无法打开文件\n");
		return 1;
	}

	// 初始化 curl
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl)
	{
		// 设置 URL
		curl_easy_setopt(curl, CURLOPT_URL, url);

		// 设置回调函数，处理包头数据
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, NULL);

		// 设置回调函数，将数据写入文件
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

		// 设置超时时间为10秒
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		// 执行请求
		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			printf("下载失败: %s\n", curl_easy_strerror(res));
		}
		else
		{
			double content_length;
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
			printf("Content-Length: %.0f\n", content_length);

			char *content;
			curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content);
			printf("Content-Length: %s\n", content);
			strcpy(str, content);

			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			printf("status code: %ld\n", response_code);
		}

		// 清理 curl
		curl_easy_cleanup(curl);
	}

	// 关闭文件
	fclose(fp);

	// 清理 curl 的全局状态
	curl_global_cleanup();

	return response_code;
}

#define PP(fmt, args...) printf("[%s:%d]zzzzz "fmt"\n", __FUNCTION__, __LINE__, ##args );

int main()
{
	char str[128] = {0};

	PP();

	while (1)
	{
		system("rm -rf config_tmp.ini");
		int code = http_download("http://172.16.28.157:8080/config.ini", &str);
		switch (code)
		{
		case 200:
		{
			printf("str = %s\n", str);

			FILE *fp;
			fp = fopen("config_tmp.ini", "r");
			if (fp == NULL)
			{
				printf("无法打开文件\n");
				return 1;
			}
			char line[128] = {0};
			char key[128] = {0};
			char value[128] = {0};
			char upgradefile[128] = {0};
			char cmd[128] = {0};
			while (fgets(line, sizeof(line), fp))
			{
				printf("line = %s\n", line);
				sscanf(line, "%[^=] = %[^;#]", key, value); // get key,value
				// sscanf(line, "[%[^]]", key, value); // get section
				printf("key = %s\nvalue = %s\n", key, value);
				if (strcmp(key, "upgradefile") == 0)
				{
					strcpy(upgradefile, value);
					char cmd[128] = {0};
					snprintf(cmd, sizeof(cmd), "wget http://172.16.28.157:8080/%s -O ./upgrade.bin\n", value);
					system(cmd);
				}
				else if (strcmp(key, "cmd") == 0)
				{
                    strcpy(cmd, value);
					char cmd[128] = {0};
					snprintf(cmd, sizeof(cmd), "%s\n", value);
					system(cmd);
                }
			}
			fclose(fp);
		}
		break;
		case 404:
			printf("Not found\n");
			break;
		default:
			break;
		}
	}

	return 0;
}
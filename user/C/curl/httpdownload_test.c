/*
 * @*************************************:
 * @FilePath: /user/C/curl/httpdownload_test.c
 * @version:
 * @Author: dof
 * @Date: 2022-11-04 13:43:08
 * @LastEditors: dof
 * @LastEditTime: 2023-08-10 15:51:26
 * @Descripttion: curl 下载测试程序
 * @build:  gcc httpdownload_test.c -lcurl
 * @**************************************:
 */

#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

char DOWNLOAD_PATH[300];

int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	CURL *easy_handle = static_cast<CURL *>(clientp);
	char timeFormat[9] = "Unknow";

	// Defaults to bytes/second
	double speed;
	string unit = "B";

	curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed); // curl_get_info必须在curl_easy_perform之后调用

	if (speed != 0)
	{
		// Time remaining
		double leftTime = (downloadFileLength - dlnow - resumeByte) / speed;
		int hours = leftTime / 3600;
		int minutes = (leftTime - hours * 3600) / 60;
		int seconds = leftTime - hours * 3600 - minutes * 60;

#ifdef _WIN32
		sprintf_s(timeFormat, 9, "%02d:%02d:%02d", hours, minutes, seconds);
#else
		sprintf(timeFormat, "%02d:%02d:%02d", hours, minutes, seconds);
#endif
	}

	if (speed > 1024 * 1024 * 1024)
	{
		unit = "G";
		speed /= 1024 * 1024 * 1024;
	}
	else if (speed > 1024 * 1024)
	{
		unit = "M";
		speed /= 1024 * 1024;
	}
	else if (speed > 1024)
	{
		unit = "kB";
		speed /= 1024;
	}

	printf("speed:%.2f%s/s", speed, unit.c_str());

	if (dltotal != 0)
	{
		double progress = (dlnow + resumeByte) / downloadFileLength * 100;
		printf("\t%.2f%%\tRemaing time:%s\n", progress, timeFormat);
	}

	return 0;
}

int download_function(char *url, char *down_path)
{
	if ((NULL == url) || (NULL == down_path))
	{
		printf("param_error");
		return -1;
	}
	CURL *curl = curl_easy_init(); // 创建一个简单的句柄,操作完必须调用curl_easy_cleanup();函数
	if (NULL == curl)
	{
		printf("curl_easy_init error");
		return -1;
	}

	FILE *file = fopen(down_path, "wb");
	int errnum = 0;
	if (NULL == file)
	{
		errnum = errno;
		printf("open fail errno=%d,reason=%s", errnum,
			   strerror(errnum));
		return -1;
	}

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);	// 下载路径
	curl_easy_setopt(curl, CURLOPT_URL, url);			// 下载地址，要访问的网址链接
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);		// 使用毫秒超时 需要关闭这个signal功能,curl_setopt($ch, CURLOPT_NOSIGNAL, true);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // 连接超时10s
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);		// 下载响应超时30s，30s没下载完退出
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);		// 调试信息打开
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);

	CURLcode res = curl_easy_perform(curl); // 以阻塞方式执行整个请求
	if (CURLE_OK != res)
	{
		fclose(file);
		curl_easy_cleanup(curl); // 结束一个curl_easy_init()创建的简易句柄
		printf("curl_easy_perform failed:%s\n",
			   curl_easy_strerror(res)); // 解析curl请求发生错误时返回的错误编码的含义
		return -1;
	}

	fclose(file);
	curl_easy_cleanup(curl); // 结束一个curl_easy_init()创建的简易句柄
	return 0;
}

int main()
{
	int ret;
	char url[300];

	ret = download_function("http://192.168.1.100:8080/", "456.w");
	printf("ret = %d\n", ret);
	return 0;
}
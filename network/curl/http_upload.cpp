/*
 * @*************************************:
 * @FilePath: /network/curl/http_upload.cpp
 * @version:
 * @Author: dof
 * @Date: 2023-09-11 14:06:39
 * @LastEditors: dof
 * @LastEditTime: 2023-09-11 14:19:00
 * @Descripttion: curl 上传文件
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

typedef struct
{
	bool send_state;  // 发送的状态
	char * data; // 接收到的数据
} CURL_CMD;

// curl 调用的回调函数
size_t response_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if (!stream)
		return (size * nmemb);
	CURL_CMD *Curl_Ptr = (CURL_CMD *)stream;
	Curl_Ptr->data.assign((char *)ptr, nmemb * size);
	Curl_Ptr->send_state = true;

	return (size * nmemb);
}

int UploadFile(char *  url, char *  file_path, void *response)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);
	struct curl_slist *list = NULL;
	///* get a curl handle */
	curl = curl_easy_init();
	if (curl)
	{
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		/* Now specify the POST data */
		struct curl_httppost *post = NULL;
		struct curl_httppost *last = NULL;

		int pos = file_path.find_last_of("/");
		char *  file_name = file_path.substr(pos + 1, file_path.length());

		// form-data key(file) "./test.jpg"为文件路径  "hello.jpg" 为文件上传时文件名
		curl_formadd(&post, &last, CURLFORM_PTRNAME, "file", CURLFORM_FILE, file_path.c_str(), CURLFORM_FILENAME, file_name.c_str(), CURLFORM_END);
		// 构造post参数
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s cmd=%s\n",
					curl_easy_strerror(res), (char*)response);
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
		curl_slist_free_all(list); /* free the list again */
	}

	curl_global_cleanup();

	return res;
}

int main()
{
	CURL_CMD cmd;
	cmd.send_state = false;
	UploadFile("http://172.16.28.157:8080/u/", "http2-download.c", &cmd);
	printf("send state:%d\n", cmd.send_state);
	printf("recv:%s\n", cmd.data.c_str());

	return 0;
}
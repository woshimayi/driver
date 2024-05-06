#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>


using namespace std;
using namespace neb;



string bucpe_sendInform()
{
    // 定义 URL
    const char *url = "http://172.16.28.157:8056/api/u";

    // 定义 JSON 数据
    const char *json_data = "{ \"name\": \"John Doe\", \"age\": 30 }";

    // 初始化 CURL
    CURL *curl = curl_easy_init();

    // 设置 CURLOPT_URL 选项
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // 设置 CURLOPT_POST 选项
    curl_easy_setopt(curl, CURLOPT_POST, 1);

    // 设置 CURLOPT_POSTFIELDS 选项
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

    // 设置 CURLOPT_POSTFIELDSIZE 选项
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(json_data));


    //   设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_data_writer);

    //设置写数据
    string resStr;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resStr);

    // 设置 CURLOPT_HTTPHEADER 选项
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // 执行请求
    CURLcode res = curl_easy_perform(curl);
    // 检查返回值
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return NULL;
    }

    // 获取响应数据
    char *response = NULL;
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    fprintf(stderr, "HTTP error: %ld\n", response_code);
    if (response_code == 200)
    {
        printf("HTTP error: %ld\n", response_code);
    }
    else
    {
        printf("HTTP error: %ld\n", response_code);
    }

    // 释放资源
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(response);
    //  printf("resStr = %s\n", resStr.c_str());

    return resStr;
}


int main()
{

}

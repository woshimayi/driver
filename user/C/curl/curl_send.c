/*
 * @*************************************:
 * @FilePath: /user/C/curl/curl_send.c
 * @version:
 * @Author: dof
 * @Date: 2024-04-08 19:28:37
 * @LastEditors: dof
 * @LastEditTime: 2024-04-10 18:06:08
 * @Descripttion:
 * @**************************************:
 */

#include <curl/curl.h>

#include <curl/curl.h>

int main() {
    // 初始化 curl
    CURL *curl = curl_easy_init();

    // 设置 URL
    curl_easy_setopt(curl, CURLOPT_URL, "http://172.16.28.157:8080/u");

    // 设置 HTTP 方法
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

    // 设置请求头
    struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 设置 JSON 数据
    char *json_data = "{\"name\":\"John Doe\",\"email\":\"johndoe@example.com\"}";
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    // 检查结果
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return 1;
    }

    // 清理 curl
    curl_easy_cleanup(curl);

    // 释放请求头
    curl_slist_free_all(headers);

    return 0;
}


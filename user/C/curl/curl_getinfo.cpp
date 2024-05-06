/*
 * @*************************************:
 * @FilePath: /user/C/curl/curl_getinfo.cpp
 * @version:
 * @Author: dof
 * @Date: 2024-04-08 20:02:41
 * @LastEditors: dof
 * @LastEditTime: 2024-04-08 20:20:02
 * @Descripttion:
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <iconv.h>
#include <iostream>
#include <sstream>

#include "cJSON/cJSON.h"

using namespace std;

size_t push_string(void *buffer, size_t size, size_t nmemb, void *stream)
{
    string data((const char *)buffer, (size_t)size * nmemb);
    *((stringstream *)stream) << data << endl;
    return size * nmemb;
}

char *send_post(char *url, char *param)
{
    std::stringstream res_str;

    CURL *curl_handle = NULL;
    CURLcode curl_res;
    curl_res = curl_global_init(CURL_GLOBAL_ALL);

    //  printf("param is: %s\n", param);
    if (curl_res == CURLE_OK)
    {
        curl_handle = curl_easy_init();
        if (curl_handle != NULL)
        {
            curl_easy_setopt(curl_handle, CURLOPT_URL, url);
            curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(param));
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, param);
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30);
            curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10L);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, push_string);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &res_str);
            curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);

            struct curl_slist *pList = NULL;
            pList = curl_slist_append(pList, "Content-Type: application/json;charset=utf-8");

            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, pList);
            curl_res = curl_easy_perform(curl_handle);
            if (curl_res != CURLE_OK)
            {
                printf("curl_easy_perform error, err_msg:[%ld]\n", curl_res);
            }
            curl_easy_cleanup(curl_handle);
        }
    }
    else
    {
        printf("CURL ERROR : %s", curl_easy_strerror(curl_res));
    }

    std::string str_json = res_str.str();
    char *str;
    str = (char *)malloc(200);
    strcpy(str, str_json.c_str());
    return str;
}

// 解析json格式的返回值
void get_ret_info(char *res_str, char *flag, char *password, char *msg)
{
    // Json::Reader json_reader;
    // Json::Value json_value;
    // if(json_reader.parse(res_str, json_value))
    // {
    // 	std::string flag1 = json_value["flag"].asString();
    // 	std::string password1 = json_value["password"].asString();
    // 	std::string msg1 = json_value["msg"].asString();

    // 	strcpy(flag, flag1.c_str());
    // 	strcpy(password, password1.c_str());
    // 	strcpy(msg, msg1.c_str());
    // }
    printf("res_str = %s", res_str);
}

int main(int argc, char *argv[])
{
    char url[100] = "http://www.baidu.com";
    char param[2048] = {};
    char *res_str;

    // 构造json格式的参数
    Json::Value item;
    item["businessName"] = Json::Value("YeWuFuWuName");
    item["businessIp"] = Json::Value("192.168.212.20");
    item["dbType"] = Json::Value(2);
    item["dbIp"] = Json::Value("192.168.212.20");
    item["accountName"] = Json::Value("imp_admin");
    item["protocolPort"] = Json::Value("13507");
    item["dbName"] = Json::Value("imp_commondb");
    item["sign"] = Json::Value("323d31db5ccf4a1aa9ec312fcbd2ae78");

    std::string str = item.toStyledString();
    strcpy(param, str.c_str());

    res_str = send_post(url, param);
    printf("return string is: %s", res_str);

    char flag[10] = {0};
    char password[30] = {0};
    char msg[200] = {0};
    get_ret_info(res_str, flag, password, msg);
}
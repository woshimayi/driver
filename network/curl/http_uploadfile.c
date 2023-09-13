/*
 * @*************************************: 
 * @FilePath: /network/curl/http_uploadfile.c
 * @version: 
 * @Author: dof
 * @Date: 2023-09-11 14:16:06
 * @LastEditors: dof
 * @LastEditTime: 2023-09-11 15:01:50
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <curl/curl.h>

int main(void) {
  CURL *curl;
  CURLcode res;

  // 初始化libcurl
  curl_global_init(CURL_GLOBAL_ALL);

  // 创建CURL对象
  curl = curl_easy_init();
  if(curl) {
    // 设置上传的URL
    curl_easy_setopt(curl, CURLOPT_URL, "http://172.16.28.157:8080/u");
    
    // 设置要上传的文件
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "curl_ftp_download.cpp");

    // 执行请求
    res = curl_easy_perform(curl);

    // 检查请求结果
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    // 清理CURL对象
    curl_easy_cleanup(curl);
  }

  // 释放libcurl全局资源
  curl_global_cleanup();

  return 0;
}

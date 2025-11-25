
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// ----------------------------------------------------
// 请替换为您要测试的重定向 URL
// ----------------------------------------------------
#define TARGET_URL "http://google.com"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

char *get_final_redirect_url(const char *start_url)
{
    CURL *curl_handle = NULL;
    CURLcode res;
    char *effective_url_ptr = NULL;
    char *result_url = NULL; // 用于存储最终返回的堆分配字符串

    // 1. 初始化 libcurl 库环境 (如果之前没有初始化过，但为了健壮性，这里再次调用)
    // 注意：在多线程环境中，curl_global_init/cleanup 需要谨慎处理。
    curl_global_init(CURL_GLOBAL_NOTHING);

    // 2. 获取一个 easy handle 句柄
    curl_handle = curl_easy_init();
    if (!curl_handle)
    {
        fprintf(stderr, "libcurl API: curl_easy_init() failed.\n");
        // 即使失败，也需要清理全局资源
        curl_global_cleanup();
        return NULL;
    }

    // --- 设置 curl 选项 ---

    // 3. 设置要请求的 URL
    curl_easy_setopt(curl_handle, CURLOPT_URL, start_url);

    // 4. *** 关键选项 A: 开启跟随重定向 ***
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // 5. 设置静默模式：不将页面内容输出到 stdout
    curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1L);

    // 6. 禁用进度条
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    // 7. 设置超时 (可选，提高健壮性)
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);

    // 8. 执行请求
    res = curl_easy_perform(curl_handle);

    // 9. 检查请求是否成功
    if (res != CURLE_OK)
    {
        fprintf(stderr, "libcurl API: curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        // 清理并返回 NULL
        goto cleanup;
    }

    // 10. *** 关键选项 B: 获取最终有效 URL ***
    res = curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &effective_url_ptr);

    if (res == CURLE_OK && effective_url_ptr)
    {
        // 11. 成功获取 URL，分配内存并复制字符串
        size_t len = strlen(effective_url_ptr);
        result_url = (char *)malloc(len + 1);

        if (result_url)
        {
            // 复制字符串内容
            strcpy(result_url, effective_url_ptr);
        }
        else
        {
            fprintf(stderr, "libcurl API: Memory allocation failed.\n");
            // 内存分配失败，保持 result_url 为 NULL
        }
    }
    // else: 如果获取失败，result_url 保持为 NULL

// 使用 goto 确保资源被清理
cleanup:
    // 12. 清理 easy handle 句柄
    if (curl_handle)
    {
        curl_easy_cleanup(curl_handle);
    }

    // 13. 清理 libcurl 库环境
    curl_global_cleanup();

    return result_url;
}

int main()
{
    // 定义一个会发生重定向的 URL
    const char *initial_url = "http://172.16.27.192:5000/302-test";

    // 调用 API
    char *final_url = get_final_redirect_url(initial_url);

    printf("原始 URL: %s\n", initial_url);

    if (final_url)
    {
        printf("最终重定向 URL: %s\n", final_url);

        // !!! 必须释放 get_final_redirect_url 返回的内存 !!!
        free(final_url);
        printf("成功释放内存。\n");
    }
    else
    {
        printf("无法获取重定向 URL。\n");
    }

    // 另一个测试：一个不会重定向的 URL
    const char *no_redirect_url = "https://www.baidu.com/";
    final_url = get_final_redirect_url(no_redirect_url);

    printf("\n原始 URL: %s\n", no_redirect_url);
    if (final_url)
    {
        printf("最终重定向 URL: %s\n", final_url);
        free(final_url);
    }
    else
    {
        printf("无法获取重定向 URL。\n");
    }

    return 0;
}

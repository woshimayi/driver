/*
 * @*************************************: 
 * @FilePath: /user/C/software_engineering/design_pattern/status_watch.c
 * @version: 
 * @Author: dof
 * @Date: 2023-01-18 18:08:21
 * @LastEditors: dof
 * @LastEditTime: 2023-01-18 18:08:21
 * @Descripttion: disgen patter : watch mode
 * @**************************************: 
 */


//微信公众号：嵌入式系统
#include <string.h>
#include <stdio.h>

#define PAL_GNSS_SUBSCRIPTIONS_MAX 5

typedef unsigned char uint8_t;
typedef void (*gnss_info_callback)(void* data);

typedef struct
{
    gnss_info_callback m_cb;
} pal_gnss_subscription_info;

//订阅池
static pal_gnss_subscription_info g_gnss_subscription_pool[PAL_GNSS_SUBSCRIPTIONS_MAX] = {0};

//订阅
uint8_t pal_gnss_subscribe(gnss_info_callback callback)
{
    uint8_t i;
    uint8_t ret=0;

    if(callback != NULL)
    {
        for(i = 0; i < PAL_GNSS_SUBSCRIPTIONS_MAX; i++)
        {
            if(g_gnss_subscription_pool[i].m_cb == NULL)
            {
                //RTOS注意竞争
                g_gnss_subscription_pool[i].m_cb = callback;
                ret = 1;
                break;
            }
        }
    }
    else
    {
        ret = 0;
    }
    return ret;
}

//取消订阅
void pal_gnss_unsubscribe(gnss_info_callback callback)
{
    uint8_t i;

    for(i = 0; i < PAL_GNSS_SUBSCRIPTIONS_MAX; i++)
    {
        if(g_gnss_subscription_pool[i].m_cb == callback)
        {
            //RTOS注意竞争
            g_gnss_subscription_pool[i].m_cb = NULL;
            break;
        }
    }
}

//广播给观察者，执行回调
void pal_gnss_info_update(void)
{
    uint8_t i;
    uint8_t data=1;//test

    for(i = 0; i < PAL_GNSS_SUBSCRIPTIONS_MAX; i++)
    {
        if(g_gnss_subscription_pool[i].m_cb != NULL)
        {
            //RTOS中使用消息队列更好，这里只是演示效果
            g_gnss_subscription_pool[i].m_cb((void*)&data);
        }
    }
}

//微信公众号：嵌入式系统
int main(void)
{
    printf("embedded-system\r\n");
    return 0;
}
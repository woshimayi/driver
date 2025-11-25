/*
 * @*************************************:
 * @FilePath     : /user/C/PID/rgb-ysv.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-10-21 18:01:42
 * @LastEditors  : dof
 * @LastEditTime : 2025-10-21 19:19:27
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <string.h>


typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// HSV到RGB转换 (H: 0-360, S: 0-100, V: 0-100)
/**
 * @brief 
 * 
 * @param h  
 * @param s 
 * @param v 
 * @param r 
 * @param g 
 * @param b 
 */
void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t region, remainder, p, q, t;

    if (s == 0)
    {
        *r = *g = *b = v;
        return;
    }

    region = h / 60;
    remainder = (h - (region * 60)) * 256 / 60;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        *r = v;
        *g = t;
        *b = p;
        break;
    case 1:
        *r = q;
        *g = v;
        *b = p;
        break;
    case 2:
        *r = p;
        *g = v;
        *b = t;
        break;
    case 3:
        *r = p;
        *g = q;
        *b = v;
        break;
    case 4:
        *r = t;
        *g = p;
        *b = v;
        break;
    default:
        *r = v;
        *g = p;
        *b = q;
        break;
    }
}

void SetRGB(uint8_t r, uint8_t g, uint8_t b)
{
    // 这里添加设置RGB灯颜色的代码
    // 例如，通过PWM信号控制LED亮度
    printf("Set RGB to 0x%02x%02x%02x\n", r, g, b);
}

// 使用HSV创建彩虹循环
void rainbow_loop(void)
{
    static uint16_t hue = 0;
    uint8_t r, g, b;

    hsv_to_rgb(hue, 100, 100, &r, &g, &b);
    SetRGB(r, g, b);

    hue = (hue + 1) % 360; // 在色相环上移动
}

int main(int argc, char const *argv[])
{
    /* code */
    while (1)
    {
        rainbow_loop();
        usleep(10000); // 10ms延时
    }
    return 0;
}

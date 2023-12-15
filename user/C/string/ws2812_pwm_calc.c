/*
 * @*************************************:
 * @FilePath: /user/C/string/ws2812_pwm_calc.c
 * @version:
 * @Author: dof
 * @Date: 2023-12-14 11:51:45
 * @LastEditors: dof
 * @LastEditTime: 2023-12-15 18:03:46
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <math.h>

// int main()
// {
//     float arr, psc, tclk, fpwm, n;
//     int choose;
//     printf("Enter 1 or 2 to choose mode0 or mode1  : ");
//     scanf("%d", &choose);
//     if (choose == 1)
//     {
//         printf("Enter Fpwm (hz): ");
//         scanf("%f", &fpwm);
//         printf("Enter tclk (mhz): ");
//         scanf("%f", &tclk);
//         printf("Enter psc : ");
//         scanf("%f", &psc);
//         arr = (tclk * pow(10, 6) / (fpwm * (psc + 1))) - 1;
//         printf("%f", arr);
//     }
//     else
//     {
//         printf("Enter Fpwm (hz): ");
//         scanf("%f", &fpwm);
//         printf("Enter tclk (mhz): ");
//         scanf("%f", &tclk);
//         for (psc = 0; psc <= 65535; psc++)
//         {
//             for (arr = 0; arr <= 65535; arr++)
//             {
//                 n = (tclk * pow(10, 6) / ((psc + 1) * (arr + 1)));
//                 if (n == fpwm)
//                 {
//                     printf("psc=%.0f,arr=%.0f\n", psc, arr);
//                     break;
//                 }
//             }
//         }
//     }
// }

#define CODE_1 (38) // 1码定时器计数次数
#define CODE_0 (19) // 0码定时器计数次数

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long int	uint64_t;

/*建立一个定义单个LED三原色值大小的结构体*/
typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} RGB_Color_TypeDef;

#define Pixel_NUM 24 // LED数量宏定义，这里我使用一个LED，（单词pixel为像素的意思）

void RGB_SetColor(uint8_t LedId, RGB_Color_TypeDef Color); // 给一个LED装载24个颜色数据码（0码和1码）
void Reset_Load(void);                                     // 该函数用于将数组最后24个数据变为0，代表RESET_code
void RGB_SendArray(void);                                  // 发送最终数组
void RGB_RED(uint16_t Pixel_Len);                          // 显示红灯
void RGB_GREEN(uint16_t Pixel_Len);                        // 显示绿灯
void RGB_BLUE(uint16_t Pixel_Len);                         // 显示蓝灯
void RGB_WHITE(uint16_t Pixel_Len);                        // 显示白灯

/*Some Static Colors------------------------------*/
const RGB_Color_TypeDef RED = {255, 0, 0}; // 显示红色RGB数据
const RGB_Color_TypeDef ORANGE = {127, 106, 0};
const RGB_Color_TypeDef YELLOW = {127, 216, 0};
const RGB_Color_TypeDef GREEN = {0, 255, 0};
const RGB_Color_TypeDef CYAN = {0, 255, 255};
const RGB_Color_TypeDef BLUE = {0, 0, 255};
const RGB_Color_TypeDef PURPLE = {238, 130, 238};
const RGB_Color_TypeDef BLACK = {0, 0, 0};
const RGB_Color_TypeDef WHITE = {255, 255, 255};
const RGB_Color_TypeDef MAGENTA = {255, 0, 220};

/*二维数组存放最终PWM输出数组，每一行24个
数据代表一个LED，最后一行24个0代表RESET码*/
uint32_t Pixel_Buf[Pixel_NUM + 1][24];

/*
功能：设定单个RGB LED的颜色，把结构体中RGB的24BIT转换为0码和1码
参数：LedId为LED序号，Color：定义的颜色结构体
*/
void RGB_SetColor(uint8_t LedId, RGB_Color_TypeDef Color)
{
    uint8_t i;
    if (LedId > Pixel_NUM)
        return; // avoid overflow 防止写入ID大于LED总数

    for (i = 0; i < 8; i++)
        Pixel_Buf[LedId][i] = ((Color.G & (1 << (7 - i))) ? (CODE_1) : CODE_0); // 数组某一行0~7转化存放G
    for (i = 8; i < 16; i++)
        Pixel_Buf[LedId][i] = ((Color.R & (1 << (15 - i))) ? (CODE_1) : CODE_0); // 数组某一行8~15转化存放R
    for (i = 16; i < 24; i++)
        Pixel_Buf[LedId][i] = ((Color.B & (1 << (23 - i))) ? (CODE_1) : CODE_0); // 数组某一行16~23转化存放B
}

/*
功能：最后一行装在24个0，输出24个周期占空比为0的PWM波，作为最后reset延时，这里总时长为24*1.2=30us > 24us(要求大于24us)
*/
void Reset_Load(void)
{
    uint8_t i;
    for (i = 0; i < 24; i++)
    {
        Pixel_Buf[Pixel_NUM][i] = 0;
    }
}

/*
功能：发送数组
参数：(&htim1)定时器1，(TIM_CHANNEL_2)通道2，((uint32_t *)Pixel_Buf)待发送数组，
            (Pixel_NUM+1)*24)发送个数，数组行列相乘
*/
void RGB_SendArray(void)
{
    // HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t *)Pixel_Buf, (Pixel_NUM + 1) * 24);
}

/*
功能：显示红色
参数：Pixel_Len为显示LED个数
*/
void RGB_RED(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入红色
    {
        RGB_SetColor(i, RED);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示绿色
参数：Pixel_Len为显示LED个数
*/
void RGB_GREEN(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入绿色
    {
        RGB_SetColor(i, GREEN);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示蓝色
参数：Pixel_Len为显示LED个数
*/
void RGB_BLUE(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入蓝色
    {
        RGB_SetColor(i, BLUE);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示白色
参数：Pixel_Len为显示LED个数
*/
void RGB_WHITE(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入白色
    {
        RGB_SetColor(i, WHITE);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示黑色
参数：Pixel_Len为显示LED个数
*/
void RGB_BLACK(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入白色
    {
        RGB_SetColor(i, BLACK);
    }
    Reset_Load();
    RGB_SendArray();
}

// 也可以继续添加其他颜色，和颜色变化函数等

/*******************************************************************************/
/*									添加部分									   */

// 显示指定颜色
static void rgb_show(uint32_t Pixel_Len, RGB_Color_TypeDef rgb)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++)
    {
        RGB_SetColor(i, rgb);
    }
    Reset_Load();
    RGB_SendArray();
}

// 颜色循环转换
static RGB_Color_TypeDef Wheel(uint8_t WheelPos)
{
    RGB_Color_TypeDef rgb;
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        rgb.R = 255 - WheelPos * 3;
        rgb.G = 0;
        rgb.B = WheelPos * 3;
        return rgb;
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        rgb.R = 0;
        rgb.G = WheelPos * 3;
        rgb.B = 255 - WheelPos * 3;
        return rgb;
    }
    WheelPos -= 170;
    rgb.R = WheelPos * 3;
    rgb.G = 255 - WheelPos * 3;
    rgb.B = 0;
    return rgb;
}

// 彩虹呼吸灯
static void rainbow(uint8_t wait)
{
    uint32_t timestamp = 0;
    uint16_t i;
    static uint8_t j;
    static uint32_t next_time = 0;

    uint32_t flag = 0;
    if (next_time < wait)
    {
        if ((uint64_t)timestamp + wait - next_time > 0)
            flag = 1;
    }
    else if (timestamp > next_time)
    {
        flag = 1;
    }
    if (flag) // && (timestamp - next_time < wait*5))
    {
        j++;
        next_time = timestamp + wait;
        for (i = 0; i < Pixel_NUM; i++)
        {
            RGB_SetColor(i, Wheel((i + j) & 255));
        }
    }
    RGB_SendArray();
}

// 彩虹灯旋转
static void rainbowCycle(uint8_t wait)
{
    uint32_t timestamp = 0;
    uint16_t i;
    static uint8_t j;
    static uint32_t next_time = 0;

    static uint8_t loop = 0;
    if (loop == 0)
        next_time = timestamp;
    loop = 1; // 首次调用初始化

    if ((timestamp > next_time)) // && (timestamp - next_time < wait*5))
    {
        j++;
        next_time = timestamp + wait;
        for (i = 0; i < Pixel_NUM; i++)
        {
            RGB_SetColor(i, Wheel(((i * 256 / (Pixel_NUM)) + j) & 255));
        }
    }
    RGB_SendArray();
}


void HAL_Delay(int msec)
{
    return;
}

static uint8_t rainbow_change_flag = 0;
void led_loop(void)
{
    int i;
    rgb_show(8, BLACK);
    HAL_Delay(300);
    for (i = 1; i <= 8; i++)
    { // 红
        rgb_show(i, RED);
        HAL_Delay(50);
    }
    for (i = 1; i <= 8; i++)
    { // 橙
        rgb_show(i, ORANGE);
        HAL_Delay(50);
    }
    for (i = 1; i <= 8; i++)
    { // 黄
        rgb_show(i, YELLOW);
        HAL_Delay(50);
    }
    for (i = 1; i <= 8; i++)
    { // 绿
        rgb_show(i, GREEN);
        HAL_Delay(50);
    }
    for (i = 1; i <= 8; i++)
    { // 青
        rgb_show(i, CYAN);
        HAL_Delay(50);
    }
    for (i = 1; i <= 8; i++)
    { // 蓝
        rgb_show(i, BLUE);
        HAL_Delay(50);
    }
    for (i = 1; i <= 8; i++)
    { // 紫
        rgb_show(i, PURPLE);
        HAL_Delay(50);
    }

    // HAL_TIM_Base_Start_IT(&htim3); // 使能定时器中断->时间：1ms
    while (1)
    {
        if (!rainbow_change_flag)
            rainbow(5);
        else
            rainbowCycle(2);
    }
}

int main(int argc, char const *argv[])
{
    // RGB_RED(24);
    RGB_SetColor(1, RED);
    return 0;
}

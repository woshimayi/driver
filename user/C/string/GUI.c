/************************ (C) COPYRIGHT 2013 yang_yulei ************************
* File Name          : GUI.cpp
* Author             : yang_yulei
* Date First Issued  : 12/18/2013
* Description        :
*
********************************************************************************
*
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "head.h"

/* Variables -----------------------------------------------------------------*/
//预览区位置
RockLocation_t previewLocation = {GUI_WALL_SQUARE_WIDTH *GUI_xWALL_SQUARE_NUM + 70, 50} ;

extern RockType rockArray[] ;

/*******************************************************************************
* Function Name  : DrawRock
* Description    : 在游戏区画出编号为rockIndex的方块
* Be called      : PlayGame()
* Input          : rockIndex       :
                   currentLocatePtr: 此方块的位置
                   displayed       : 此方块是否显示
* Output         : None
* Return         : None
*******************************************************************************/
void DrawRock(int rockIndex, const struct LOCATE *currentLocatePtr, BOOL displayed)
{
    int i ;
    int mask  ;
    int rockX ;     //俄罗斯方块的4*4模型的左上角点x轴的坐标
    int rockY ;     //俄罗斯方块的4*4模型的左上角点y轴的坐标
    int spaceFlag ; //占位标记(用于g_gameBoard，1表示某处有方块 0表示此处无方块)
    int color ;     //画出的方块的颜色

    //若此方块是用于显示的，则设置其颜色为白色，其占位标记设为1
    //否则设置其颜色为黑色(背景色),占位标记设为0
    displayed ? (color = WHITE, spaceFlag = 1)
    : (color = BLACK, spaceFlag = 0) ;

    setcolor(color) ;                 //设置画笔颜色
    setlinestyle(PS_SOLID, NULL, 2) ; //设置线形为1像素的实线
    rockX = currentLocatePtr->left ;
    rockY = currentLocatePtr->top ;

    //逐位扫描由unsigned int的低2字节
    //16个位组成的俄罗斯方块形状点阵(其代表4*4的方块形状)
    mask = (unsigned int)1 << 15 ;
    for (i = 1; i <= 16; i++)
    {
        //与掩码相与为1的 即为方块上的点
        if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
        {
            //在屏幕上画出此方块
            rectangle(rockX + 2,
                      rockY + 2,
                      rockX + ROCK_SQUARE_WIDTH - 2,
                      rockY + ROCK_SQUARE_WIDTH - 2) ;
        }

        //每4次 换行 转到下一行继续画
        i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
        :  rockX += ROCK_SQUARE_WIDTH ;

        mask >>= 1 ;
    }
}

/*******************************************************************************
* Function Name  : DrawGameGUI
* Description    : 画出游戏界面
* Be called      : main()
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DrawGameGUI(void)
{
    int i = 0 ;
    int wallHigh = GUI_yWALL_SQUARE_NUM * GUI_WALL_SQUARE_WIDTH ;//围墙的高度(像素)

    setcolor(RED) ;                      //设置围墙的颜色
    setlinestyle(PS_SOLID, NULL, 0)    ; //设置围墙方格的线形(1像素的实线)

    //画出围墙(画矩形是 先确定左上顶点的坐标，再确定右下顶点坐标)
    //先画出上下墙
    for (i = GUI_WALL_SQUARE_WIDTH;
            i <= GUI_WALL_WIDTH_PIX;
            i += GUI_WALL_SQUARE_WIDTH)
    {
        rectangle(i - GUI_WALL_SQUARE_WIDTH,
                  0,
                  i,
                  GUI_WALL_SQUARE_WIDTH) ; //上墙

        rectangle(i - GUI_WALL_SQUARE_WIDTH,
                  wallHigh - GUI_WALL_SQUARE_WIDTH,
                  i,
                  wallHigh) ; //下墙
    }

    //再画出左右墙
    for (i = 2 * GUI_WALL_SQUARE_WIDTH;
            i <= wallHigh - GUI_WALL_SQUARE_WIDTH;
            i += GUI_WALL_SQUARE_WIDTH)
    {
        rectangle(0,
                  i - GUI_WALL_SQUARE_WIDTH,
                  GUI_WALL_SQUARE_WIDTH,
                  i) ; //左墙

        rectangle(GUI_WALL_WIDTH_PIX - GUI_WALL_SQUARE_WIDTH,
                  i - GUI_WALL_SQUARE_WIDTH,
                  GUI_WALL_WIDTH_PIX,
                  i) ; //右墙
    }

    //画分隔线
    setcolor(WHITE) ;                                              //设置画笔颜色
    setlinestyle(PS_DASH, NULL, 2) ;                               //设置线形为2像素的虚线
    line(GUI_WALL_WIDTH_PIX + 20, 0, GUI_WALL_WIDTH_PIX + 20, wallHigh) ; //在偏移右围墙的20处画线

    //画右边统计分数及版权信息栏
    //先设置字体
    LOGFONT   f ;                       //定义字体属性结构体
    getfont(&f) ;                       //获得当前字体
    f.lfHeight = 18 ;                   //设置字体高度为 38（包含行距）
    strcpy(f.lfFaceName, "黑体") ;      //设置字体为“黑体”
    f.lfQuality = ANTIALIASED_QUALITY ; //设置输出效果为抗锯齿
    setfont(&f) ;                       //设置字体样式

    //1,显示预览
    outtextxy(GUI_WALL_WIDTH_PIX + 80, 20, "预览") ;
    //2,显示等级栏
    outtextxy(GUI_WALL_WIDTH_PIX + 80, 140, "等级") ;
    //3,显示得分栏
    outtextxy(GUI_WALL_WIDTH_PIX + 80, 190, "得分") ;

    //4,显示操作说明
    outtextxy(GUI_WALL_WIDTH_PIX + 65, 255, "操作说明") ;
    getfont(&f) ;
    strcpy(f.lfFaceName, "宋体") ;
    f.lfHeight = 15 ;
    setfont(&f) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 45, 290, "w.a.s.d控制方向") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 45, 313, "回车键 暂停") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 45, 336, "空格键 快速下落") ;

    //5.版权信息
    line(GUI_WALL_WIDTH_PIX + 20, wallHigh - 65, WINDOW_WIDTH, wallHigh - 65) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 40, wallHigh - 50, "  杨溢之  作品") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 40, wallHigh - 30, "  QQ:702080167") ;

    //显示等级，得分信息
    setcolor(RED) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 163, "1") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 223, "0") ;
}

/*******************************************************************************
* Function Name  : UpdataScore
* Description    : 增加一次得分，并把游戏界面的得分区显示 更新
* Be called      : ProcessFullRow()
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UpdataScore(void)
{
    char scoreStr[5] ; //用字符串的形式存储得分
    extern int g_score ;
    extern int g_grade ;

    //分数的增长的单位是10
    g_score += 10 ;
    //得分是100的倍数，则等级加1 (等级在5级以上的就 保持不变)
    if (g_score == (g_score / 100) * 100 && g_grade < 5)
        UpdataGrade(++g_grade) ;

    //删除原先信息
    setfillstyle(BLACK) ;
    bar(GUI_WALL_WIDTH_PIX + 90, 220, GUI_WALL_WIDTH_PIX + 99, 229) ;

    //显示信息
    setcolor(RED) ;
    sprintf(scoreStr, "%d", g_score) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 223, scoreStr)  ;
}

/*******************************************************************************
* Function Name  : UpdataGrade
* Description    : 增加一次等级，并把游戏界面的等级区显示 更新
* Be called      :
* Input          : grade ：新的等级值
* Output         : None
* Return         : None
*******************************************************************************/
void UpdataGrade(int grade)
{
    char gradeStr[5] ;

    //删除原先信息
    setfillstyle(BLACK) ;
    bar(GUI_WALL_WIDTH_PIX + 90, 160, GUI_WALL_WIDTH_PIX + 99, 169) ;

    //显示信息
    setcolor(RED)    ;
    sprintf(gradeStr, "%d", grade) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 163, gradeStr) ;
}


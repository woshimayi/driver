/************************ (C) COPYRIGHT 2013 yang_yulei ************************
* File Name          : main.cpp
* Author             : yang_yulei
* Date First Issued  : 1/16/2012
* Description        : 开发环境 VC++ 6.0 含EasyX图形库(http://www.easyx.cn)
*                      俄罗斯方块
*                      
*                      
********************************************************************************
* History:
*  1/16/2012 : V0.1
* 12/18/2013 : V0.2
********************************************************************************
* 
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "head.h"
#include <windows.h>
#include <dos.h>
 
/* Typedef -------------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/
//全局变量-游戏板的状态描述(即表示当前界面哪些位置有方块)
//0表示没有，1表示有(多加了两行和两列，形成一个围墙，便于判断方块是否能够移动)
char g_gameBoard[Y_ROCK_SQUARE_NUM+2][X_ROCK_SQUARE_NUM+2] = {0} ;
//统计分数
int  g_score = 0 ; 
//等级
int  g_grade = 0 ;
 
int  g_rockTypeNum = 0 ; //共有多少种俄罗斯方块
RockType rockArray[50] = {(0,0)} ;
 
/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int
main(void)
{
    //画出游戏界面
    initgraph(WINDOW_WIDTH, WINDOW_HIGH) ; //初始化图形窗口
    cleardevice() ;
    DrawGameGUI() ; 
 
    //使用 API 函数修改窗口名称
    HWND hWnd = GetHWnd();
    SetWindowText(hWnd, "俄罗斯方块");
 
    //初始化参数
    InitProcParameters() ;
 
    //游戏过程
    PlayGame()    ;
    
    closegraph()    ;
    return 0 ;
}


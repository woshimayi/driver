/************************ (C) COPYRIGHT 2013 yang_yulei ************************
* File Name          : init.cpp
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
extern char g_gameBoard[][X_ROCK_SQUARE_NUM + 2] ;
extern int  g_rockTypeNum ;
extern RockType rockArray[] ;

/* Function prototypes -------------------------------------------------------*/
static int ReadRockShape(void) ;
static unsigned int ShapeStr2uInt(char *const);

/*******************************************************************************
* Function Name  : InitProcParameters
* Description    : 在正式开始运行游戏前，初始化一些参数：g_gameBoard
                   从配置文件中读取系统中俄罗斯方块的形状
* Be called      : main
* Input          : None
* Output         : g_gameBoard rockArray
* Return         : None
*******************************************************************************/
//初始化程序参数
int InitProcParameters(void)
{
    int  i ;

    //初始化游戏板(把这个二维数组的四周置1,当作围墙,用于判断边界)
    for (i = 0; i < X_ROCK_SQUARE_NUM + 2; i++)
    {
        g_gameBoard[0][i] = 1 ;
        g_gameBoard[Y_ROCK_SQUARE_NUM + 1][i] = 1 ;
    }
    for (i = 0; i < Y_ROCK_SQUARE_NUM + 2; i++)
    {
        g_gameBoard[i][0] = 1 ;
        g_gameBoard[i][X_ROCK_SQUARE_NUM + 1] = 1 ;
    }

    //从配置文件中读取游戏中所有方块的形状点阵
    ReadRockShape() ;

    return 0 ;
}

/*******************************************************************************
* Function Name  : ReadRockShape
* Description    : 从配置文件中读取系统中俄罗斯方块的形状 把它记录在rockArray中
* Be called      : InitProcParameters
* Input          : rockshape.ini
* Output         : rockArray
* Return         : 成功返回0 失败返回1
*******************************************************************************/
int ReadRockShape(void)
{
    FILE *fp ;
    int  i = 0 ;
    int  len = 0 ;
    int  rockArrayIdx = 0 ;
    int  shapeNumPerRock = 0 ; //一种方块的形态数目(用于计算方块的nextRockIndex)

    char rdBuf[128] ;
    char rockShapeBitsStr[128] = {0};

    unsigned int  shapeBits = 0 ;

    g_rockTypeNum  = 0 ;

    //打开配置文件 从中读取方块的形状
    fp = fopen(".\\rockshape.ini", "r") ;
    if (fp == NULL)
    {
        perror("open file error!\n") ;
        return 1 ;
    }

    while (fgets(rdBuf, 128, fp) != NULL)
    {
        len = strlen(rdBuf) ;
        rdBuf[len - 1] = '\0' ;

        switch (rdBuf[0])
        {
            case '@':
            case '#':
                strcat(rockShapeBitsStr, rdBuf) ;
                break ;

            case 0 : //一个方块读取结束
                shapeBits = ShapeStr2uInt(rockShapeBitsStr) ;
                rockShapeBitsStr[0] = 0 ;
                shapeNumPerRock++ ;
                rockArray[rockArrayIdx].rockShapeBits = shapeBits ;
                rockArray[rockArrayIdx].nextRockIndex = rockArrayIdx + 1 ;
                rockArrayIdx++ ;
                g_rockTypeNum++ ; //记录方块数量的全局变量+1
                break ;

            case '-'://一种方块读取结束(更新其nextRockIndex值)
                rockArray[rockArrayIdx - 1].nextRockIndex = rockArrayIdx - shapeNumPerRock ;
                shapeNumPerRock = 0 ;
                break ;

            default :
                break ;
        }
    }//while()

    return 0 ;
}

/*******************************************************************************
* Function Name  : ShapeStr2uInt
* Description    : 把配置文件中的描述方块形状的字符串 转化为 unsigned int型
* Be called      :
* Input          : shapeStr 描述方块形状的字符串(从文件中读取的)
* Output         : None
* Return         : unsigned int型的方块形状点阵(用其低16位表示)
*******************************************************************************/
unsigned int ShapeStr2uInt(char *const shapeStr)
{
    unsigned int  shapeBitsRet = 0 ;
    char *p = shapeStr ;

    for (p += 15; p >= shapeStr; p--)
    {
        if (*p == '@')
        {
            shapeBitsRet |= ((unsigned int)1 << (&shapeStr[15] - p)) ;
        }
    }

    return shapeBitsRet ;
}


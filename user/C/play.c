/************************ (C) COPYRIGHT 2013 yang_yulei ************************
* File Name          : play.cpp
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
extern char     g_gameBoard[][X_ROCK_SQUARE_NUM + 2] ;
extern int      g_rockTypeNum ;
extern RockType rockArray[] ;

/* Function prototypes -------------------------------------------------------*/
static BOOL MoveAble(int, const struct LOCATE *, int) ;
static void SetOccupyFlag(int, const struct LOCATE *) ;
static void ProcessFullRow(void) ;
static BOOL isGameOver() ;
static void ProccessUserHit(int, int *, struct LOCATE *) ;
static void FastFall(int, struct LOCATE *, struct LOCATE *) ;
static void DelFullRow(int f_row) ;

/*******************************************************************************
* Function Name  : PlayGame
* Description    : 此程序的主要设计逻辑
* Be called      : main
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PlayGame(void)
{
    int   userHitChar ;      //用户敲击键盘的字符
    int   currentRockIndex ; //当前方块在rockArray数组中下标
    int   nextRockIndex ;    //准备的下个方块的下标
    BOOL  moveAbled = FALSE ;//记录方块能否落下
    DWORD oldtime = 0;
    extern int g_grade ;

    //当前方块位置
    RockLocation_t currentRockLocation ;
    //初始方块位置(由当中开始下落)
    RockLocation_t initRockLocation = {(GUI_xWALL_SQUARE_NUM / 2 - 4) *GUI_WALL_SQUARE_WIDTH,
                                       GUI_WALL_SQUARE_WIDTH
                                      };
    //预览区位置
    extern RockLocation_t previewLocation ;

    //为第一次下落，初始化参数
    //随机选择当前的俄罗斯方块形状 和下一个俄罗斯方块形状
    srand(time(NULL)) ;
    currentRockIndex = rand() % g_rockTypeNum ;
    nextRockIndex = rand() % g_rockTypeNum ;
    currentRockLocation.left = initRockLocation.left ;
    currentRockLocation.top = initRockLocation.top ;

    while (1)
    {
        DrawRock(currentRockIndex, ¤tRockLocation, TRUE) ;
        FlushBatchDraw();    //用批绘图功能，可以消除闪烁

        //判断能否下落
        moveAbled = MoveAble(currentRockIndex, ¤tRockLocation, DIRECT_DOWN) ;

        //如果不能下落则生成新的方块
        if (!moveAbled)
        {
            //设置占位符(此时方块已落定)
            SetOccupyFlag(currentRockIndex, ¤tRockLocation) ;
            //擦除预览
            DrawRock(nextRockIndex, &previewLocation, FALSE) ;
            //生成新的方块
            currentRockIndex = nextRockIndex ;
            nextRockIndex = rand() % g_rockTypeNum ;
            currentRockLocation.left = initRockLocation.left ;
            currentRockLocation.top = initRockLocation.top ;
        }

        //显示预览
        DrawRock(nextRockIndex, &previewLocation, TRUE) ;

        //如果超时(且能下落)，自动下落一格
        //  这个超时时间400-80*g_grade 是本人根据实验自己得出的
        //  一个速度比较适中的一个公式(g_grade不会大于等于5)
        DWORD newtime = GetTickCount();
        if (newtime - oldtime >= (unsigned int)(400 - 80 * g_grade) && moveAbled == TRUE)
        {
            oldtime = newtime ;
            DrawRock(currentRockIndex, ¤tRockLocation, FALSE) ; //擦除原先位置
            currentRockLocation.top += ROCK_SQUARE_WIDTH ; //下落一格
        }

        //根据当前游戏板的状况判断是否满行，并进行满行处理
        ProcessFullRow() ;

        //判断是否游戏结束
        if (isGameOver())
        {
            MessageBox(NULL, "游戏结束", "GAME OVER", MB_OK) ;
            exit(0) ;
        }

        //测试键盘是否被敲击
        if (kbhit())
        {
            userHitChar = getch() ;
            ProccessUserHit(userHitChar, ¤tRockIndex, ¤tRockLocation) ;
        }

        Sleep(20) ; //降低CPU使用率
    }//结束外层while(1)

}

/*******************************************************************************
* Function Name  : ProccessUserHit
* Description    : 处理用户敲击键盘
* Be called      : PlayGame()
* Input          : userHitChar     用户敲击键盘的ASCII码
                   rockIndexPtr    当前俄罗斯方块在rockArray中的下标
                   rockLocationPtr 当前方块在游戏界面中的位置

* Output         : rockIndexPtr    响应用户敲击后 新方块的下标
                   rockLocationPtr 响应用户敲击后 新方块的位置
* Return         : None
*******************************************************************************/
void ProccessUserHit(int userHitChar, int *rockIndexPtr, struct LOCATE *rockLocationPtr)
{
    switch (userHitChar)
    {
        case 'w' :
        case 'W' :  //“上”键
            //检查是否能改变方块形状
            if (MoveAble(rockArray[*rockIndexPtr].nextRockIndex, rockLocationPtr, DIRECT_UP))
            {
                DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
                *rockIndexPtr = rockArray[*rockIndexPtr].nextRockIndex ;
            }
            break ;

        case 's' :
        case 'S' : //“下”键
            DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ; //擦除原先位置
            rockLocationPtr->top += ROCK_SQUARE_WIDTH    ;
            break ;

        case 'a' :
        case 'A' :  //“左”键
            if (MoveAble(*rockIndexPtr, rockLocationPtr, DIRECT_LEFT))
            {
                DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
                rockLocationPtr->left -= ROCK_SQUARE_WIDTH    ;
            }
            break ;

        case 'd' :
        case 'D' :  //“右”键
            if (MoveAble(*rockIndexPtr, rockLocationPtr, DIRECT_RIGHT))
            {
                DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
                rockLocationPtr->left += ROCK_SQUARE_WIDTH    ;
            }
            break ;

        case ' ' : //空格(快速下落)
            DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
            FastFall(*rockIndexPtr, rockLocationPtr, rockLocationPtr) ;
            break ;

        case 13 : //回车键(暂停)
            while (1)
            {
                userHitChar = getch() ;
                if (userHitChar == 13)
                    break ;
            }
            break ;

        default :
            break ;
    }

}

/*******************************************************************************
* Function Name  : MoveAble
* Description    : 判断编号为rockIndex 在位置currentLocatePtr的方块
                   能否向direction移动
* Be called      :
* Input          : None
* Output         : None
* Return         : TRUE  可以移动
                   FALSE 不可以移动
*******************************************************************************/
BOOL MoveAble(int rockIndex, const struct LOCATE *currentLocatePtr, int f_direction)
{
    int i ;
    int mask  ;
    int rockX ;
    int rockY ;

    rockX = currentLocatePtr->left ;
    rockY = currentLocatePtr->top ;

    mask = (unsigned int)1 << 15 ;
    for (i = 1; i <= 16; i++)
    {
        //与掩码相与为1的 即为方块上的点
        if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
        {
            //判断能否移动(即扫描即将移动的位置 是否与设置的围墙有重叠)
            //若是向上(即翻滚变形)
            if (f_direction == DIRECT_UP)
            {
                //因为此情况下传入的是下一个方块的形状，故我们直接判断此方块的位置是否已经被占
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] == 1)
                    return FALSE ;
            }
            //如果是向下方向移动
            else if (f_direction == DIRECT_DOWN)
            {
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 2]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] == 1)
                    return FALSE ;
            }
            else //如果是左右方向移动
            {
                //f_direction的DIRECT_LEFT为-1，DIRECT_RIGHT为1，故直接加f_direction即可判断。
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1 + f_direction] == 1)
                    return FALSE ;
            }
        }

        //每4次 换行 转到下一行继续
        i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
        :  rockX += ROCK_SQUARE_WIDTH ;

        mask >>= 1 ;
    }

    return TRUE ;
}

/*******************************************************************************
* Function Name  : SetOccupyFlag
* Description    : 更新游戏板状态(把一些位置设置为已占用)
* Be called      :
* Input          : rockIndex        方块的下标(定位了方块的形状)
                   currentLocatePtr 方块的位置(用来设定已占用标识)
* Output         : None
* Return         : None
*******************************************************************************/
void SetOccupyFlag(int rockIndex, const struct LOCATE *currentLocatePtr)
{
    int i ;
    int mask  ;
    int rockX ;
    int rockY ;

    rockX = currentLocatePtr->left ;
    rockY = currentLocatePtr->top  ;

    mask = (unsigned int)1 << 15 ;
    for (i = 1; i <= 16; i++)
    {
        //与掩码相与为1的 即为方块上的点
        if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
        {
            g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
            [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] = 1 ;
        }

        //每4次 换行 转到下一行继续画
        i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
        :  rockX += ROCK_SQUARE_WIDTH ;

        mask >>= 1 ;
    }
}

/*******************************************************************************
* Function Name  : ProcessFullRow
* Description    : 检查是否有满行，若有，则删除满行(并更新得分信息)
* Be called      :
* Input          : g_gameBoard
* Output         : None
* Return         : None
*******************************************************************************/
void ProcessFullRow(void)
{
    int  i = 1 ;
    int  cnt = 0 ;

    BOOL rowFulled = TRUE ;
    int  rowIdx = Y_ROCK_SQUARE_NUM ; //从最后一行开始往上检查

    while (cnt != X_ROCK_SQUARE_NUM) //直到遇到是空行的为止
    {
        rowFulled = TRUE ;
        cnt = 0 ;

        //判断是否有满行 并消除满行
        for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
        {
            if (g_gameBoard[rowIdx][i] == 0)
            {
                rowFulled = FALSE ;
                cnt++ ;
            }
        }
        if (rowFulled) //有满行 (并更新得分信息)
        {
            DelFullRow(rowIdx)    ;
            //更新得分信息
            UpdataScore() ;
            rowIdx++ ;
        }
        rowIdx--    ;
    }
}

/*******************************************************************************
* Function Name  : DelFullRow
* Description    : 删除游戏板的第rowIdx行
* Be called      :
* Input          : g_gameBoard
                   rowIdx 要删除的行 在g_gameBoard中的下标
* Output         : None
* Return         : None
*******************************************************************************/
void DelFullRow(int rowIdx)
{
    int cnt = 0 ;
    int i ;

    //把此行擦除
    setcolor(BLACK) ;
    for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
    {
        rectangle(GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - ROCK_SQUARE_WIDTH + 2,
                  GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - ROCK_SQUARE_WIDTH + 2,
                  GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - 2,
                  GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - 2) ;
    }

    //把此行之上的游戏板方块全向下移动一个单位
    while (cnt != X_ROCK_SQUARE_NUM) //直到遇到是空行的为止
    {
        cnt = 0 ;
        for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
        {
            g_gameBoard[rowIdx][i] = g_gameBoard[rowIdx - 1][i] ;

            //擦除上面的一行
            setcolor(BLACK) ;
            rectangle(GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - ROCK_SQUARE_WIDTH + 2,
                      GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * (rowIdx - 1) - ROCK_SQUARE_WIDTH + 2,
                      GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - 2,
                      GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * (rowIdx - 1) - 2) ;

            //显示下面的一行
            if (g_gameBoard[rowIdx][i] == 1)
            {
                setcolor(WHITE) ;
                rectangle(GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - ROCK_SQUARE_WIDTH + 2,
                          GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - ROCK_SQUARE_WIDTH + 2,
                          GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - 2,
                          GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - 2) ;
            }

            if (g_gameBoard[rowIdx][i] == 0)
                cnt++ ;            //统计一行是不是 都是空格
        }//for

        rowIdx-- ;
    }
}

/*******************************************************************************
* Function Name  : FastFall
* Description    : 让编号为rockIndex 且初始位置在currentLocatePtr的方块
                   快速下落到底部
* Be called      :
* Input          : rockIndex currentLocatePtr
* Output         : endLocatePtr  下落后方块的位置
* Return         : None
*******************************************************************************/
void
FastFall
(int rockIndex,
 struct LOCATE *currentLocatePtr,
 struct LOCATE *endLocatePtr)
{
    int i ;
    int mask ;  //掩码，用于判断方块的形状
    int rockX ; //方块的坐标(4*4方格的左上角点的x轴坐标)
    int rockY ; //方块的坐标(4*4方格的左上角点的y轴坐标)

    while (currentLocatePtr->top <= GUI_WALL_SQUARE_WIDTH + Y_ROCK_SQUARE_NUM * ROCK_SQUARE_WIDTH)
    {
        rockX = currentLocatePtr->left ;
        rockY = currentLocatePtr->top  ;

        mask = (unsigned int)1 << 15 ;
        for (i = 1; i <= 16; i++)
        {
            //与掩码相与为1的 即为方块上的点
            if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
            {
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] == 1) //遇到底部
                {
                    endLocatePtr->top = currentLocatePtr->top - ROCK_SQUARE_WIDTH    ;
                    return ;
                }
            }

            //每4次 换行 转到下一行继续画
            i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
            :  rockX += ROCK_SQUARE_WIDTH ;

            mask >>= 1 ;
        }

        currentLocatePtr->top += ROCK_SQUARE_WIDTH    ;
    }//while()
}

/*******************************************************************************
* Function Name  : isGameOver
* Description    : 判断是否游戏结束
* Be called      :
* Input          : None
* Output         : None
* Return         : TRUE  游戏结束
                   FALSE 游戏继续
*******************************************************************************/
BOOL isGameOver()
{
    int i ;
    BOOL topLineHaveRock = FALSE ;    //在界面的最高行有方块的标记
    BOOL bottomLineHaveRock = FALSE ; //在界面的最低行有方块的标记

    for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
    {
        if (g_gameBoard[1][i] == 1)
            topLineHaveRock = TRUE ;
        if (g_gameBoard[Y_ROCK_SQUARE_NUM][i] == 1)
            bottomLineHaveRock = TRUE ;
    }

    //若底层行和顶层行都有方块 则说明在所有行都有方块，游戏结束
    if (topLineHaveRock && bottomLineHaveRock)
        return TRUE ;
    else
        return FALSE ;
}



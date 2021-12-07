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
* Description    : �˳������Ҫ����߼�
* Be called      : main
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PlayGame(void)
{
    int   userHitChar ;      //�û��û����̵��ַ�
    int   currentRockIndex ; //��ǰ������rockArray�������±�
    int   nextRockIndex ;    //׼�����¸�������±�
    BOOL  moveAbled = FALSE ;//��¼�����ܷ�����
    DWORD oldtime = 0;
    extern int g_grade ;

    //��ǰ����λ��
    RockLocation_t currentRockLocation ;
    //��ʼ����λ��(�ɵ��п�ʼ����)
    RockLocation_t initRockLocation = {(GUI_xWALL_SQUARE_NUM / 2 - 4) *GUI_WALL_SQUARE_WIDTH,
                                       GUI_WALL_SQUARE_WIDTH
                                      };
    //Ԥ����λ��
    extern RockLocation_t previewLocation ;

    //Ϊ��һ�����䣬��ʼ������
    //���ѡ��ǰ�Ķ���˹������״ ����һ������˹������״
    srand(time(NULL)) ;
    currentRockIndex = rand() % g_rockTypeNum ;
    nextRockIndex = rand() % g_rockTypeNum ;
    currentRockLocation.left = initRockLocation.left ;
    currentRockLocation.top = initRockLocation.top ;

    while (1)
    {
        DrawRock(currentRockIndex, ��tRockLocation, TRUE) ;
        FlushBatchDraw();    //������ͼ���ܣ�����������˸

        //�ж��ܷ�����
        moveAbled = MoveAble(currentRockIndex, ��tRockLocation, DIRECT_DOWN) ;

        //������������������µķ���
        if (!moveAbled)
        {
            //����ռλ��(��ʱ�������䶨)
            SetOccupyFlag(currentRockIndex, ��tRockLocation) ;
            //����Ԥ��
            DrawRock(nextRockIndex, &previewLocation, FALSE) ;
            //�����µķ���
            currentRockIndex = nextRockIndex ;
            nextRockIndex = rand() % g_rockTypeNum ;
            currentRockLocation.left = initRockLocation.left ;
            currentRockLocation.top = initRockLocation.top ;
        }

        //��ʾԤ��
        DrawRock(nextRockIndex, &previewLocation, TRUE) ;

        //�����ʱ(��������)���Զ�����һ��
        //  �����ʱʱ��400-80*g_grade �Ǳ��˸���ʵ���Լ��ó���
        //  һ���ٶȱȽ����е�һ����ʽ(g_grade������ڵ���5)
        DWORD newtime = GetTickCount();
        if (newtime - oldtime >= (unsigned int)(400 - 80 * g_grade) && moveAbled == TRUE)
        {
            oldtime = newtime ;
            DrawRock(currentRockIndex, ��tRockLocation, FALSE) ; //����ԭ��λ��
            currentRockLocation.top += ROCK_SQUARE_WIDTH ; //����һ��
        }

        //���ݵ�ǰ��Ϸ���״���ж��Ƿ����У����������д���
        ProcessFullRow() ;

        //�ж��Ƿ���Ϸ����
        if (isGameOver())
        {
            MessageBox(NULL, "��Ϸ����", "GAME OVER", MB_OK) ;
            exit(0) ;
        }

        //���Լ����Ƿ��û�
        if (kbhit())
        {
            userHitChar = getch() ;
            ProccessUserHit(userHitChar, ��tRockIndex, ��tRockLocation) ;
        }

        Sleep(20) ; //����CPUʹ����
    }//�������while(1)

}

/*******************************************************************************
* Function Name  : ProccessUserHit
* Description    : �����û��û�����
* Be called      : PlayGame()
* Input          : userHitChar     �û��û����̵�ASCII��
                   rockIndexPtr    ��ǰ����˹������rockArray�е��±�
                   rockLocationPtr ��ǰ��������Ϸ�����е�λ��

* Output         : rockIndexPtr    ��Ӧ�û��û��� �·�����±�
                   rockLocationPtr ��Ӧ�û��û��� �·����λ��
* Return         : None
*******************************************************************************/
void ProccessUserHit(int userHitChar, int *rockIndexPtr, struct LOCATE *rockLocationPtr)
{
    switch (userHitChar)
    {
        case 'w' :
        case 'W' :  //���ϡ���
            //����Ƿ��ܸı䷽����״
            if (MoveAble(rockArray[*rockIndexPtr].nextRockIndex, rockLocationPtr, DIRECT_UP))
            {
                DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
                *rockIndexPtr = rockArray[*rockIndexPtr].nextRockIndex ;
            }
            break ;

        case 's' :
        case 'S' : //���¡���
            DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ; //����ԭ��λ��
            rockLocationPtr->top += ROCK_SQUARE_WIDTH    ;
            break ;

        case 'a' :
        case 'A' :  //���󡱼�
            if (MoveAble(*rockIndexPtr, rockLocationPtr, DIRECT_LEFT))
            {
                DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
                rockLocationPtr->left -= ROCK_SQUARE_WIDTH    ;
            }
            break ;

        case 'd' :
        case 'D' :  //���ҡ���
            if (MoveAble(*rockIndexPtr, rockLocationPtr, DIRECT_RIGHT))
            {
                DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
                rockLocationPtr->left += ROCK_SQUARE_WIDTH    ;
            }
            break ;

        case ' ' : //�ո�(��������)
            DrawRock(*rockIndexPtr, rockLocationPtr, FALSE) ;
            FastFall(*rockIndexPtr, rockLocationPtr, rockLocationPtr) ;
            break ;

        case 13 : //�س���(��ͣ)
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
* Description    : �жϱ��ΪrockIndex ��λ��currentLocatePtr�ķ���
                   �ܷ���direction�ƶ�
* Be called      :
* Input          : None
* Output         : None
* Return         : TRUE  �����ƶ�
                   FALSE �������ƶ�
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
        //����������Ϊ1�� ��Ϊ�����ϵĵ�
        if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
        {
            //�ж��ܷ��ƶ�(��ɨ�輴���ƶ���λ�� �Ƿ������õ�Χǽ���ص�)
            //��������(����������)
            if (f_direction == DIRECT_UP)
            {
                //��Ϊ������´��������һ���������״��������ֱ���жϴ˷����λ���Ƿ��Ѿ���ռ
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] == 1)
                    return FALSE ;
            }
            //��������·����ƶ�
            else if (f_direction == DIRECT_DOWN)
            {
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 2]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] == 1)
                    return FALSE ;
            }
            else //��������ҷ����ƶ�
            {
                //f_direction��DIRECT_LEFTΪ-1��DIRECT_RIGHTΪ1����ֱ�Ӽ�f_direction�����жϡ�
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1 + f_direction] == 1)
                    return FALSE ;
            }
        }

        //ÿ4�� ���� ת����һ�м���
        i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
        :  rockX += ROCK_SQUARE_WIDTH ;

        mask >>= 1 ;
    }

    return TRUE ;
}

/*******************************************************************************
* Function Name  : SetOccupyFlag
* Description    : ������Ϸ��״̬(��һЩλ������Ϊ��ռ��)
* Be called      :
* Input          : rockIndex        ������±�(��λ�˷������״)
                   currentLocatePtr �����λ��(�����趨��ռ�ñ�ʶ)
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
        //����������Ϊ1�� ��Ϊ�����ϵĵ�
        if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
        {
            g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
            [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] = 1 ;
        }

        //ÿ4�� ���� ת����һ�м�����
        i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
        :  rockX += ROCK_SQUARE_WIDTH ;

        mask >>= 1 ;
    }
}

/*******************************************************************************
* Function Name  : ProcessFullRow
* Description    : ����Ƿ������У����У���ɾ������(�����µ÷���Ϣ)
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
    int  rowIdx = Y_ROCK_SQUARE_NUM ; //�����һ�п�ʼ���ϼ��

    while (cnt != X_ROCK_SQUARE_NUM) //ֱ�������ǿ��е�Ϊֹ
    {
        rowFulled = TRUE ;
        cnt = 0 ;

        //�ж��Ƿ������� ����������
        for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
        {
            if (g_gameBoard[rowIdx][i] == 0)
            {
                rowFulled = FALSE ;
                cnt++ ;
            }
        }
        if (rowFulled) //������ (�����µ÷���Ϣ)
        {
            DelFullRow(rowIdx)    ;
            //���µ÷���Ϣ
            UpdataScore() ;
            rowIdx++ ;
        }
        rowIdx--    ;
    }
}

/*******************************************************************************
* Function Name  : DelFullRow
* Description    : ɾ����Ϸ��ĵ�rowIdx��
* Be called      :
* Input          : g_gameBoard
                   rowIdx Ҫɾ������ ��g_gameBoard�е��±�
* Output         : None
* Return         : None
*******************************************************************************/
void DelFullRow(int rowIdx)
{
    int cnt = 0 ;
    int i ;

    //�Ѵ��в���
    setcolor(BLACK) ;
    for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
    {
        rectangle(GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - ROCK_SQUARE_WIDTH + 2,
                  GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - ROCK_SQUARE_WIDTH + 2,
                  GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - 2,
                  GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - 2) ;
    }

    //�Ѵ���֮�ϵ���Ϸ�巽��ȫ�����ƶ�һ����λ
    while (cnt != X_ROCK_SQUARE_NUM) //ֱ�������ǿ��е�Ϊֹ
    {
        cnt = 0 ;
        for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
        {
            g_gameBoard[rowIdx][i] = g_gameBoard[rowIdx - 1][i] ;

            //���������һ��
            setcolor(BLACK) ;
            rectangle(GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - ROCK_SQUARE_WIDTH + 2,
                      GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * (rowIdx - 1) - ROCK_SQUARE_WIDTH + 2,
                      GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - 2,
                      GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * (rowIdx - 1) - 2) ;

            //��ʾ�����һ��
            if (g_gameBoard[rowIdx][i] == 1)
            {
                setcolor(WHITE) ;
                rectangle(GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - ROCK_SQUARE_WIDTH + 2,
                          GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - ROCK_SQUARE_WIDTH + 2,
                          GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * i - 2,
                          GUI_WALL_SQUARE_WIDTH + ROCK_SQUARE_WIDTH * rowIdx - 2) ;
            }

            if (g_gameBoard[rowIdx][i] == 0)
                cnt++ ;            //ͳ��һ���ǲ��� ���ǿո�
        }//for

        rowIdx-- ;
    }
}

/*******************************************************************************
* Function Name  : FastFall
* Description    : �ñ��ΪrockIndex �ҳ�ʼλ����currentLocatePtr�ķ���
                   �������䵽�ײ�
* Be called      :
* Input          : rockIndex currentLocatePtr
* Output         : endLocatePtr  ����󷽿��λ��
* Return         : None
*******************************************************************************/
void
FastFall
(int rockIndex,
 struct LOCATE *currentLocatePtr,
 struct LOCATE *endLocatePtr)
{
    int i ;
    int mask ;  //���룬�����жϷ������״
    int rockX ; //���������(4*4��������Ͻǵ��x������)
    int rockY ; //���������(4*4��������Ͻǵ��y������)

    while (currentLocatePtr->top <= GUI_WALL_SQUARE_WIDTH + Y_ROCK_SQUARE_NUM * ROCK_SQUARE_WIDTH)
    {
        rockX = currentLocatePtr->left ;
        rockY = currentLocatePtr->top  ;

        mask = (unsigned int)1 << 15 ;
        for (i = 1; i <= 16; i++)
        {
            //����������Ϊ1�� ��Ϊ�����ϵĵ�
            if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
            {
                if (g_gameBoard[(rockY - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1]
                        [(rockX - GUI_WALL_SQUARE_WIDTH) / ROCK_SQUARE_WIDTH + 1] == 1) //�����ײ�
                {
                    endLocatePtr->top = currentLocatePtr->top - ROCK_SQUARE_WIDTH    ;
                    return ;
                }
            }

            //ÿ4�� ���� ת����һ�м�����
            i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
            :  rockX += ROCK_SQUARE_WIDTH ;

            mask >>= 1 ;
        }

        currentLocatePtr->top += ROCK_SQUARE_WIDTH    ;
    }//while()
}

/*******************************************************************************
* Function Name  : isGameOver
* Description    : �ж��Ƿ���Ϸ����
* Be called      :
* Input          : None
* Output         : None
* Return         : TRUE  ��Ϸ����
                   FALSE ��Ϸ����
*******************************************************************************/
BOOL isGameOver()
{
    int i ;
    BOOL topLineHaveRock = FALSE ;    //�ڽ����������з���ı��
    BOOL bottomLineHaveRock = FALSE ; //�ڽ����������з���ı��

    for (i = 1; i <= X_ROCK_SQUARE_NUM; i++)
    {
        if (g_gameBoard[1][i] == 1)
            topLineHaveRock = TRUE ;
        if (g_gameBoard[Y_ROCK_SQUARE_NUM][i] == 1)
            bottomLineHaveRock = TRUE ;
    }

    //���ײ��кͶ����ж��з��� ��˵���������ж��з��飬��Ϸ����
    if (topLineHaveRock && bottomLineHaveRock)
        return TRUE ;
    else
        return FALSE ;
}



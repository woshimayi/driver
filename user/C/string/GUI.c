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
//Ԥ����λ��
RockLocation_t previewLocation = {GUI_WALL_SQUARE_WIDTH *GUI_xWALL_SQUARE_NUM + 70, 50} ;

extern RockType rockArray[] ;

/*******************************************************************************
* Function Name  : DrawRock
* Description    : ����Ϸ���������ΪrockIndex�ķ���
* Be called      : PlayGame()
* Input          : rockIndex       :
                   currentLocatePtr: �˷����λ��
                   displayed       : �˷����Ƿ���ʾ
* Output         : None
* Return         : None
*******************************************************************************/
void DrawRock(int rockIndex, const struct LOCATE *currentLocatePtr, BOOL displayed)
{
    int i ;
    int mask  ;
    int rockX ;     //����˹�����4*4ģ�͵����Ͻǵ�x�������
    int rockY ;     //����˹�����4*4ģ�͵����Ͻǵ�y�������
    int spaceFlag ; //ռλ���(����g_gameBoard��1��ʾĳ���з��� 0��ʾ�˴��޷���)
    int color ;     //�����ķ������ɫ

    //���˷�����������ʾ�ģ�����������ɫΪ��ɫ����ռλ�����Ϊ1
    //������������ɫΪ��ɫ(����ɫ),ռλ�����Ϊ0
    displayed ? (color = WHITE, spaceFlag = 1)
    : (color = BLACK, spaceFlag = 0) ;

    setcolor(color) ;                 //���û�����ɫ
    setlinestyle(PS_SOLID, NULL, 2) ; //��������Ϊ1���ص�ʵ��
    rockX = currentLocatePtr->left ;
    rockY = currentLocatePtr->top ;

    //��λɨ����unsigned int�ĵ�2�ֽ�
    //16��λ��ɵĶ���˹������״����(�����4*4�ķ�����״)
    mask = (unsigned int)1 << 15 ;
    for (i = 1; i <= 16; i++)
    {
        //����������Ϊ1�� ��Ϊ�����ϵĵ�
        if ((rockArray[rockIndex].rockShapeBits & mask) != 0)
        {
            //����Ļ�ϻ����˷���
            rectangle(rockX + 2,
                      rockY + 2,
                      rockX + ROCK_SQUARE_WIDTH - 2,
                      rockY + ROCK_SQUARE_WIDTH - 2) ;
        }

        //ÿ4�� ���� ת����һ�м�����
        i % 4 == 0 ? (rockY += ROCK_SQUARE_WIDTH, rockX = currentLocatePtr->left)
        :  rockX += ROCK_SQUARE_WIDTH ;

        mask >>= 1 ;
    }
}

/*******************************************************************************
* Function Name  : DrawGameGUI
* Description    : ������Ϸ����
* Be called      : main()
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DrawGameGUI(void)
{
    int i = 0 ;
    int wallHigh = GUI_yWALL_SQUARE_NUM * GUI_WALL_SQUARE_WIDTH ;//Χǽ�ĸ߶�(����)

    setcolor(RED) ;                      //����Χǽ����ɫ
    setlinestyle(PS_SOLID, NULL, 0)    ; //����Χǽ���������(1���ص�ʵ��)

    //����Χǽ(�������� ��ȷ�����϶�������꣬��ȷ�����¶�������)
    //�Ȼ�������ǽ
    for (i = GUI_WALL_SQUARE_WIDTH;
            i <= GUI_WALL_WIDTH_PIX;
            i += GUI_WALL_SQUARE_WIDTH)
    {
        rectangle(i - GUI_WALL_SQUARE_WIDTH,
                  0,
                  i,
                  GUI_WALL_SQUARE_WIDTH) ; //��ǽ

        rectangle(i - GUI_WALL_SQUARE_WIDTH,
                  wallHigh - GUI_WALL_SQUARE_WIDTH,
                  i,
                  wallHigh) ; //��ǽ
    }

    //�ٻ�������ǽ
    for (i = 2 * GUI_WALL_SQUARE_WIDTH;
            i <= wallHigh - GUI_WALL_SQUARE_WIDTH;
            i += GUI_WALL_SQUARE_WIDTH)
    {
        rectangle(0,
                  i - GUI_WALL_SQUARE_WIDTH,
                  GUI_WALL_SQUARE_WIDTH,
                  i) ; //��ǽ

        rectangle(GUI_WALL_WIDTH_PIX - GUI_WALL_SQUARE_WIDTH,
                  i - GUI_WALL_SQUARE_WIDTH,
                  GUI_WALL_WIDTH_PIX,
                  i) ; //��ǽ
    }

    //���ָ���
    setcolor(WHITE) ;                                              //���û�����ɫ
    setlinestyle(PS_DASH, NULL, 2) ;                               //��������Ϊ2���ص�����
    line(GUI_WALL_WIDTH_PIX + 20, 0, GUI_WALL_WIDTH_PIX + 20, wallHigh) ; //��ƫ����Χǽ��20������

    //���ұ�ͳ�Ʒ�������Ȩ��Ϣ��
    //����������
    LOGFONT   f ;                       //�����������Խṹ��
    getfont(&f) ;                       //��õ�ǰ����
    f.lfHeight = 18 ;                   //��������߶�Ϊ 38�������оࣩ
    strcpy(f.lfFaceName, "����") ;      //��������Ϊ�����塱
    f.lfQuality = ANTIALIASED_QUALITY ; //�������Ч��Ϊ�����
    setfont(&f) ;                       //����������ʽ

    //1,��ʾԤ��
    outtextxy(GUI_WALL_WIDTH_PIX + 80, 20, "Ԥ��") ;
    //2,��ʾ�ȼ���
    outtextxy(GUI_WALL_WIDTH_PIX + 80, 140, "�ȼ�") ;
    //3,��ʾ�÷���
    outtextxy(GUI_WALL_WIDTH_PIX + 80, 190, "�÷�") ;

    //4,��ʾ����˵��
    outtextxy(GUI_WALL_WIDTH_PIX + 65, 255, "����˵��") ;
    getfont(&f) ;
    strcpy(f.lfFaceName, "����") ;
    f.lfHeight = 15 ;
    setfont(&f) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 45, 290, "w.a.s.d���Ʒ���") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 45, 313, "�س��� ��ͣ") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 45, 336, "�ո�� ��������") ;

    //5.��Ȩ��Ϣ
    line(GUI_WALL_WIDTH_PIX + 20, wallHigh - 65, WINDOW_WIDTH, wallHigh - 65) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 40, wallHigh - 50, "  ����֮  ��Ʒ") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 40, wallHigh - 30, "  QQ:702080167") ;

    //��ʾ�ȼ����÷���Ϣ
    setcolor(RED) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 163, "1") ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 223, "0") ;
}

/*******************************************************************************
* Function Name  : UpdataScore
* Description    : ����һ�ε÷֣�������Ϸ����ĵ÷�����ʾ ����
* Be called      : ProcessFullRow()
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UpdataScore(void)
{
    char scoreStr[5] ; //���ַ�������ʽ�洢�÷�
    extern int g_score ;
    extern int g_grade ;

    //�����������ĵ�λ��10
    g_score += 10 ;
    //�÷���100�ı�������ȼ���1 (�ȼ���5�����ϵľ� ���ֲ���)
    if (g_score == (g_score / 100) * 100 && g_grade < 5)
        UpdataGrade(++g_grade) ;

    //ɾ��ԭ����Ϣ
    setfillstyle(BLACK) ;
    bar(GUI_WALL_WIDTH_PIX + 90, 220, GUI_WALL_WIDTH_PIX + 99, 229) ;

    //��ʾ��Ϣ
    setcolor(RED) ;
    sprintf(scoreStr, "%d", g_score) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 223, scoreStr)  ;
}

/*******************************************************************************
* Function Name  : UpdataGrade
* Description    : ����һ�εȼ���������Ϸ����ĵȼ�����ʾ ����
* Be called      :
* Input          : grade ���µĵȼ�ֵ
* Output         : None
* Return         : None
*******************************************************************************/
void UpdataGrade(int grade)
{
    char gradeStr[5] ;

    //ɾ��ԭ����Ϣ
    setfillstyle(BLACK) ;
    bar(GUI_WALL_WIDTH_PIX + 90, 160, GUI_WALL_WIDTH_PIX + 99, 169) ;

    //��ʾ��Ϣ
    setcolor(RED)    ;
    sprintf(gradeStr, "%d", grade) ;
    outtextxy(GUI_WALL_WIDTH_PIX + 90, 163, gradeStr) ;
}


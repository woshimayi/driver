/***************************************************************/
/*俄罗斯方块的实现
* 基于VC 6.0  编译链接即可运行
* 已实现的功能：
* 1、初步的规划及背景图案的显示
* 2、四种方块实现左右移动、下键加速、上键变形（两种变形）功能
* 3、下落方块碰壁及触碰到其它方块的检测
* 4、方块积满一行的消除并加分的功能
* 5、预测方块的功能
* 6、引入set_windows_pos函数解决闪屏问题
* 未解决的缺陷或者代码存在的问题
* 1、预测方块处莫名其妙的出现背景块
* 2、代码耦合性太高，接口封装还是不够好
* 2017.3.22
* 版权：通渭县西关小学四年级一班田刚
*/
/***************************************************************/
#include<stdio.h>
#include<windows.h>
#include<conio.h>
#include<string.h>

#define BACK				176//背景图案
#define BACK_INT            -80//背景的整数表示
#define FRAME               178//分割框图案
#define NODE				219//方块图案
#define NODE_INT            -37//方块的整数表示
#define ERROR               -1
#define OK                  0

int score = 0;//计分
static int time = 500;//时间，初始为500毫秒
char back[20][30] = {0};//竖直为x轴，横轴为y轴，原点在右上角
int block_type = 0;//方块类型

void backgroud_init(void);//画面背景的初始化
void set_windows_pos(int i, int j);//指定光标位置
void block_display(int block_type, int dir_type, int coor_x, int coor_y, int color_type);//显示或者消除方块
void time_add(void);//加速函数
void block_type_change(void);//方块类型变化
int  block_move_check_x(int block_type, int dir_type, int coor_x, int coor_y);//检测方块是否触到下面已经停止的方块
int  block_move_check_y(int block_type, int dir_type, int coor_x, int coor_y, int dir_block);//检测方块是否触碰到左右的方块
int new_back_y_check(int block_type, int dir_type, int coor_y);//检测方块是否触碰到墙壁
int block_clear_sort(void);//检测是否满一行，需要消除得分
void block_clear_x(int row);//消除一个满行（都是方块）

int main(void)
{
    int c = 0;
	int new_dir_type = 0;//新的方块方向
	int befor_dir_type = 0;//旧的方块方向
	int new_back_x = 0;//新的方块头坐标x
	int new_back_y = 0;//新的方块头坐标y
	int befor_back_x = 0;//旧的方块头坐标x
	int befor_back_y = -1;//旧的方块头坐标y
	int block_dir = -1;//方块的移动方向

	backgroud_init();//背景的显示

	while(1)
	{

		block_type_change();//方块类型变化
		new_back_y = 8;//每次方块从最顶端出现时，其y坐标都为8
        time = 500;

        //怎样在这里清除键盘输入，flush不起作用

		for(new_back_x = 0; new_back_x < 20; new_back_x++)
		{
		    befor_dir_type = new_dir_type;
			befor_back_x = new_back_x - 1;
			befor_back_y = new_back_y;
			block_dir = -1;//方块的方向变量初始为-1

			block_display(block_type, 0, 17, 26, NODE);//画上预测区域的方块
			if(kbhit())//检测有输入
			{
				c = getch();
				if(c > 0)
				{
					switch(c)
					{
						case 119://上
				    			if(0 == new_dir_type)
								{
									new_dir_type = 1;//竖向
								}
								else
								{
									new_dir_type = 0;//横向
								}
                    			break;
						case 97://左
				    			new_back_y--;
								block_dir = 0;
                    			break;
						case 100://右
				    			new_back_y++;
								block_dir = 1;
                    			break;
						case 115://下
								time_add();//加速
                   				break;
						default://ESC
								break;
					}
				}
			}

			new_back_y = new_back_y_check(block_type, new_dir_type, new_back_y);//检查是否触碰到左右壁

			block_display(block_type, befor_dir_type, befor_back_x, befor_back_y, BACK);//原来的方块位置清除掉

			if(-1 != block_dir)//左右移动了方块
			{
				if(ERROR == block_move_check_y(block_type, new_dir_type, new_back_x, new_back_y, block_dir))//检查移动的方块是触碰到左右的方块
				{
					new_back_y = befor_back_y;
				}
			}

			block_display(block_type, new_dir_type, new_back_x, new_back_y, NODE);//画下一个方块位置

			if(ERROR == block_move_check_x(block_type, new_dir_type, new_back_x, new_back_y))//检查移动的方块是否触发到下面的方块
			{
				break;
			}

			Sleep(time);
		}

		block_display(block_type, 0, 17, 26, BACK);//清楚预测区域的方块

		if(OK == block_clear_sort())//检查下面方块是否等够得分，能得分则消除得分
		{
			set_windows_pos(8, 22);//更新得分
			printf("%d", score);
		}
	}

	return 0;
}

/***************************************************************/
/***  				画面背景的初始化                         ***/
/***其中原点在右上角，竖轴为x轴，横轴为y轴。y(0 - 19)为方块区***/
/***域，20 - 30 为计分区域及方块预测提示区域                 ***/
/***************************************************************/
void backgroud_init(void)
{
	int x = 0, y = 0;

	for(x = 0; x < 20; x++)
	{
		for(y = 0; y < 20; y++)
		{
			back[x][y] = BACK;
		}
	}
	for(x = 0; x < 20; x++)
	{
		for(y = 20; y < 30; y++)
		{
            if((0 == x) || (4 == x) || (10 == x) || (19 == x))
			{
				back[x][y] = FRAME;
			}
			if((20 == y) || (29 == y))
			{
				back[x][y] = FRAME;
			}
		}
	}
	//背景图案的显示
	for(x = 0; x < 20; x++)
	{
		for(y = 0; y < 30; y++)
		{
			printf("%c", back[x][y]);
		}

		printf("\n");
	}
	//显示TETRIS
	set_windows_pos(2, 22);//移动windows的光标
	printf("TETRIS\n");
	//显示积分
	set_windows_pos(6, 22);//移动windows的光标
	printf("SORT\n");
    set_windows_pos(8, 22);
	printf("%d\n", score);
	//显示下一个要出现的方块
    set_windows_pos(12, 22);//移动windows的光标
	printf("EXPECT\n");
}
/***************************************************************/
/***  		设置windows光标，类似于TC中的gotoxy              ***/
/***i,j 为要传入的x,y坐标									 ***/
/***************************************************************/
void set_windows_pos(int i, int j)//设置windows光标，类似于TC中的gotoxy
{
    /*if((0 > i) || (0 > j))
	{
		return ERROR;
	}*/

    /*windows的横轴是x轴，而本程序设计的是竖轴是X轴
	 这里做个转换*/
	COORD pos={j,i};
	HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hOut,pos);
}

/***************************************************************/
/***********************消除/填上方块***************************/
/***输入:block_type 方块类型  1：长条  2：2型  3:7型  4：田型***/
/***输入:dir_type 方向类型  0：横向  1：竖向                 ***/
/***输入:coor_x coor_y   方块当前头的坐标                    ***/
/***输入:color_type    图标类型   BACK 背景色  NODE 方块色   ***/
/***初始coor_x,coor_y为一个方块的最右下的一个方块的坐标      ***/
/***************************************************************/
void block_display(int block_type, int dir_type, int coor_x, int coor_y, int color_type)
{
    int x = 0, y = 0;

	switch (block_type)
	{
		case 1://长条
			if(0 == dir_type)//横向
			{
				for(y = coor_y; y >= (coor_y - 3); y--)
				{
					back[coor_x][y] = color_type;//方块色
					set_windows_pos(coor_x, y);//移动windows的光标
					printf("%c", back[coor_x][y]);
				}
			}
			else if(1 == dir_type)//竖向
			{
				for(x = coor_x; (x >= (coor_x - 3)) && (x >= 0); x--)
				{
					back[x][coor_y] = color_type;//方块色
					set_windows_pos(x, coor_y);//移动windows的光标
					printf("%c", back[x][coor_y]);
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 2://2型
			if(0 == dir_type)//横向
			{
				for(y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//方块色
					set_windows_pos(coor_x, y);//移动windows的光标
					printf("%c", back[coor_x][y]);
				}

				coor_x--;
				coor_y--;

				if(coor_x < 0)
				{
					return;
				}

				for(y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//方块色
					set_windows_pos(coor_x, y);//移动windows的光标
					printf("%c", back[coor_x][y]);
				}
			}
			else if(1 == dir_type)//竖向
			{
				for(x = coor_x; (x >= coor_x - 1) && (x >= 0); x--)
				{
					back[x][coor_y] = color_type;//方块色
					set_windows_pos(x, coor_y);//移动windows的光标
					printf("%c", back[x][coor_y]);
				}

				coor_x--;
				coor_y--;

				if(coor_x < 0)
				{
					return;
				}

				for(x = coor_x; x >= coor_x - 1; x--)
				{
					back[x][coor_y] = color_type;//方块色
					set_windows_pos(x, coor_y);//移动windows的光标
					printf("%c", back[x][coor_y]);
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 3://7型
			if(0 == dir_type)//横向
			{
				for(y = coor_y; y >= coor_y - 2; y--)
				{
					back[coor_x][y] = color_type;//方块色
					set_windows_pos(coor_x, y);//移动windows的光标
					printf("%c", back[coor_x][y]);
				}

				coor_x--;
				coor_y = coor_y - 2;

				if(coor_x < 0)
				{
					return;
				}

				back[coor_x][coor_y] = color_type;//方块色
				set_windows_pos(coor_x, coor_y);//移动windows的光标
				printf("%c", back[coor_x][coor_y]);
			}
			else if(1 == dir_type)//竖向
			{
				for(x = coor_x; (x >= coor_x - 2) && (x >= 0); x--)
				{
					back[x][coor_y] = color_type;//方块色
					set_windows_pos(x, coor_y);//移动windows的光标
					printf("%c", back[x][coor_y]);
				}

				coor_x = coor_x - 2;
				coor_y--;

                if(coor_x < 0)
				{
					return;
				}
				back[coor_x][coor_y] = color_type;//方块色
				set_windows_pos(coor_x, coor_y);//移动windows的光标
				printf("%c", back[coor_x][coor_y]);

			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 4://田型
			if((0 == dir_type) || (1 == dir_type))//横向
			{
				for(y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//方块色
					set_windows_pos(coor_x, y);//移动windows的光标
					printf("%c", back[coor_x][y]);
				}

				coor_x--;

				if(coor_x < 0)
				{
					return;
				}

				for(y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//方块色
					set_windows_pos(coor_x, y);//移动windows的光标
					printf("%c", back[coor_x][y]);
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		default:
			printf("block_type is  error!\n");
			break;
	}
}

void time_add(void)//加速函数
{
	if(500 == time)
	{
		time = 100;//减少的100毫秒
	}
	else if(100 == time)
	{
		time = 500;
	}
	else
	{
		;//暂时留空
	}
}

void block_type_change(void)//方块类型变化
{
	block_type++;

	if(block_type > 4)
	{
		block_type = 1;
	}
}

/***************************************************************/
/*******************方块y坐标撞墙检测***************************/
/***输入:block_type 方块类型  1：长条  2：2型  3:7型  4：田型***/
/***输入:dir_type 方向类型  0：横向  1：竖向                 ***/
/***输入:coor_y   方块当前头(右下角第一个方块)的坐标         ***/
/***************************************************************/
int new_back_y_check(int block_type, int dir_type, int coor_y)
{
	if(coor_y > 19)
	{
		coor_y = 19;
	}

	switch (block_type)
	{
		case 1://长条
			if(0 == dir_type)//横向
			{
				if(coor_y - 3 < 0)
				{
					coor_y = 3;
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(coor_y < 0)
				{
					coor_y = 0;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 2://2型
			if(0 == dir_type)//横向
			{
				if(coor_y - 2 < 0)
				{
					coor_y = 2;
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(coor_y - 1 < 0)
				{
					coor_y = 1;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 3://7型
			if(0 == dir_type)//横向
			{
				if(coor_y - 2 < 0)
				{
					coor_y = 2;
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(coor_y - 1 < 0)
				{
					coor_y = 1;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 4://田型
			if((0 == dir_type) || (1 == dir_type))//横向
			{
				if(coor_y - 1 < 0)
				{
					coor_y = 1;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		default:
			printf("block_type is  error!\n");
			break;
	}

	return coor_y;
}
/*
检查方块是否触到下面已经停止的方块，触碰到就返回ERROR
*/
/***************************************************************/
/****检查方块是否触到下面已经停止的方块，触碰到就返回ERROR   ***/
/***输入:block_type 方块类型  1：长条  2：2型  3:7型  4：田型***/
/***输入:dir_type 方向类型  0：横向  1：竖向                 ***/
/***输入:coor_x coor_y   方块当前头的坐标                    ***/
/***初始coor_x,coor_y为一个方块的最右下的一个方块的坐标      ***/
/***************************************************************/
int block_move_check_x(int block_type, int dir_type, int coor_x, int coor_y)
{
	int ret = OK;
	int x = 0, y = 0;

	switch (block_type)
	{
		case 1://长条
			if(0 == dir_type)//横向
			{
				for(y = coor_y; y >= coor_y - 3; y--)
				{
					if(NODE_INT == back[coor_x + 1][y])
					{
						ret = ERROR;
						break;
					}
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 2://2型
			if(0 == dir_type)//横向
			{
				for(y = coor_y; y >= coor_y - 1; y--)
				{
					if(NODE_INT == back[coor_x + 1][coor_y])
					{
						ret = ERROR;
						break;
					}
				}

				coor_x--;
				coor_y =coor_y - 2 ;

				if(NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}

				coor_x--;
				coor_y--;

				if(NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 3://7型
			if(0 == dir_type)//横向
			{
				for(y = coor_y; y >= coor_y - 2; y--)
				{
					if(NODE_INT == back[coor_x + 1][y])
					{
						ret = ERROR;
						break;
					}
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}

				coor_x = coor_x - 2;
				coor_y--;

				if(NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 4://田型
			if((0 == dir_type) || (1 == dir_type))//横向
			{
				for(y = coor_y; y >= coor_y - 1; y--)
				{
					if(NODE_INT == back[coor_x + 1][y])
					{
						ret = ERROR;
						break;
					}
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		default:
			printf("block_type is  error!\n");
			break;
	}

	return ret;
}

/***************************************************************/
/****检查方块在向下移动的过程中是否触到左右的方块            ***/
/***输入:block_type 方块类型  1：长条  2：2型  3:7型  4：田型***/
/***输入:dir_type 方向类型  0：横向  1：竖向                 ***/
/***输入:coor_x coor_y   方块当前头（右下）的坐标            ***/
/***输入:dir_block    当前方块的移动方向（左右）   			 ***/
/***************************************************************/
int block_move_check_y(int block_type, int dir_type, int coor_x, int coor_y, int dir_block)
{
	int x = 0, y = 0;
	int ret = OK;

	switch (block_type)
	{
		case 1://长条
			if(0 == dir_type)//横向
			{
				if(1 == dir_block)//右移
				{
					if(NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
					}
				}
				else if(0 == dir_block)//左移
				{
					if(NODE_INT == back[coor_x][coor_y - 3])
					{
						ret = ERROR;
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else if(1 == dir_type)//竖向
			{
				//当长条为竖向时，不用判断长条是往左移还是往右移
				for(x = coor_x; x >= coor_x - 3; x--)
				{
					if(NODE_INT == back[x][coor_y])
					{
						ret = ERROR;
						break;
					}
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 2://2型
			if(0 == dir_type)//横向
			{
				if(1 == dir_block)//右移
				{
					if(NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
						break;
					}

					if(NODE_INT == back[coor_x - 1][coor_y - 1])
					{
						ret = ERROR;
					}
				}
				else if(0 == dir_block)//左移
				{
					if(NODE_INT == back[coor_x][coor_y - 1])
					{
						ret = ERROR;
						break;
					}

					if(NODE_INT == back[coor_x - 1][coor_y - 2])
					{
						ret = ERROR;
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(1 == dir_block)//右移
				{
					for(x = coor_x; x >= coor_x - 1; x--)
					{
						if(NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}

					if(NODE_INT == back[coor_x - 2][coor_y - 1])
					{
						ret = ERROR;
					}
				}
				else if(0 == dir_block)//左移
				{
					if(NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
						break;
					}

					coor_x--;
					coor_y--;

					for(x = coor_x; x >= coor_x - 1; x--)
					{
						if(NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 3://7型
			if(0 == dir_type)//横向
			{
				if(1 == dir_block)//右移
				{
					if(NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
						break;
					}

					if(NODE_INT == back[coor_x - 1][coor_y - 2])
					{
						ret = ERROR;
					}
				}
				else if(0 == dir_block)//左移
				{
					if(NODE_INT == back[coor_x][coor_y - 2])
					{
						ret = ERROR;
						break;
					}

					if(NODE_INT == back[coor_x - 1][coor_y - 2])
					{
						ret = ERROR;
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else if(1 == dir_type)//竖向
			{
				if(1 == dir_block)//右移
				{
					for(x = coor_x; (x >= coor_x - 2) && (x >= 0); x--)
					{
						if(NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}
				}
				else if(0 == dir_block)//左移
				{
					for(x = coor_x - 1; (x >= coor_x - 2) && (x >= 0); x--)
					{
						if(NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}

					coor_x = coor_x - 2;
					coor_y--;

					if(NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 4://田型
			if((0 == dir_type) || (1 == dir_type))//横向
			{
				if(1 == dir_block)//右移
				{
					for(x = coor_x; x >= coor_x - 1; x--)
					{
						if(NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}
				}
				else if(0 == dir_block)//左移
				{
					for(x = coor_x; x >= coor_x - 1; x--)
					{
						if(NODE_INT == back[x][coor_y - 1])
						{
							ret = ERROR;
							break;
						}
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		default:
			printf("block_type is  error!\n");
			break;
	}

	return ret;
}

void block_clear_x(int row)//消除某一行
{
	int x = 0, y = 0;

	char back_replace[20][30] = {0};//替代back

	memcpy(back_replace, back, sizeof(back));//将back暂存到back_replace中

	for(x = 0; x <= row; x++)
	{
		for(y = 0; y < 20; y++)
		{
			back[x][y] = BACK;//初始化未背景色
		}
	}
	for(x = row; x >= 1; x--)
	{
		for(y = 0; y < 20; y++)
		{
			back[x][y] = back_replace[x - 1][y];//消除一行，方块下沉
		}
	}
	set_windows_pos(0, 0);//移动windows的光标
	for(x = 0; x < 20; x++)
	{
		for(y = 0; y < 20; y++)
		{
			printf("%c", back[x][y]);
		}

		printf("\n");
	}
}
/*
检查是否消行并且进行计分
*/
int block_clear_sort(void)
{
	int x = 0, y = 0;
	int ret = ERROR;
	int flag = 0;

	for(x = 19; x >= 0; x--)//行
	{
		flag = 0;

		for(y = 0; y < 20; y++)
		{
			if(NODE_INT == back[x][y])
			{
				flag++;//一行的块计数
			}

			if(20 == flag)//表示一行有20个方块
			{
				block_clear_x(x);//消行
				score++;//加分
				ret = OK;
			}
		}
	}

	return ret;
}


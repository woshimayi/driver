/***************************************************************/
/*����˹�����ʵ��
* ����VC 6.0  �������Ӽ�������
* ��ʵ�ֵĹ��ܣ�
* 1�������Ĺ滮������ͼ������ʾ
* 2�����ַ���ʵ�������ƶ����¼����١��ϼ����Σ����ֱ��Σ�����
* 3�����䷽�����ڼ���������������ļ��
* 4���������һ�е��������ӷֵĹ���
* 5��Ԥ�ⷽ��Ĺ���
* 6������set_windows_pos���������������
* δ�����ȱ�ݻ��ߴ�����ڵ�����
* 1��Ԥ�ⷽ�鴦Ī������ĳ��ֱ�����
* 2�����������̫�ߣ��ӿڷ�װ���ǲ�����
* 2017.3.22
* ��Ȩ��ͨμ������Сѧ���꼶һ�����
*/
/***************************************************************/
#include<stdio.h>
#include<windows.h>
#include<conio.h>
#include<string.h>

#define BACK				176//����ͼ��
#define BACK_INT            -80//������������ʾ
#define FRAME               178//�ָ��ͼ��
#define NODE				219//����ͼ��
#define NODE_INT            -37//�����������ʾ
#define ERROR               -1
#define OK                  0

int score = 0;//�Ʒ�
static int time = 500;//ʱ�䣬��ʼΪ500����
char back[20][30] = {0};//��ֱΪx�ᣬ����Ϊy�ᣬԭ�������Ͻ�
int block_type = 0;//��������

void backgroud_init(void);//���汳���ĳ�ʼ��
void set_windows_pos(int i, int j);//ָ�����λ��
void block_display(int block_type, int dir_type, int coor_x, int coor_y, int color_type);//��ʾ������������
void time_add(void);//���ٺ���
void block_type_change(void);//�������ͱ仯
int  block_move_check_x(int block_type, int dir_type, int coor_x, int coor_y);//��ⷽ���Ƿ񴥵������Ѿ�ֹͣ�ķ���
int  block_move_check_y(int block_type, int dir_type, int coor_x, int coor_y,
                        int dir_block);//��ⷽ���Ƿ��������ҵķ���
int new_back_y_check(int block_type, int dir_type, int coor_y);//��ⷽ���Ƿ�����ǽ��
int block_clear_sort(void);//����Ƿ���һ�У���Ҫ�����÷�
void block_clear_x(int row);//����һ�����У����Ƿ��飩

int main(void)
{
	int c = 0;
	int new_dir_type = 0;//�µķ��鷽��
	int befor_dir_type = 0;//�ɵķ��鷽��
	int new_back_x = 0;//�µķ���ͷ����x
	int new_back_y = 0;//�µķ���ͷ����y
	int befor_back_x = 0;//�ɵķ���ͷ����x
	int befor_back_y = -1;//�ɵķ���ͷ����y
	int block_dir = -1;//������ƶ�����

	backgroud_init();//��������ʾ

	while (1)
	{

		block_type_change();//�������ͱ仯
		new_back_y = 8;//ÿ�η������˳���ʱ����y���궼Ϊ8
		time = 500;

		//��������������������룬flush��������

		for (new_back_x = 0; new_back_x < 20; new_back_x++)
		{
			befor_dir_type = new_dir_type;
			befor_back_x = new_back_x - 1;
			befor_back_y = new_back_y;
			block_dir = -1;//����ķ��������ʼΪ-1

			block_display(block_type, 0, 17, 26, NODE);//����Ԥ������ķ���
			if (kbhit()) //���������
			{
				c = getch();
				if (c > 0)
				{
					switch (c)
					{
						case 119://��
							if (0 == new_dir_type)
							{
								new_dir_type = 1;//����
							}
							else
							{
								new_dir_type = 0;//����
							}
							break;
						case 97://��
							new_back_y--;
							block_dir = 0;
							break;
						case 100://��
							new_back_y++;
							block_dir = 1;
							break;
						case 115://��
							time_add();//����
							break;
						default://ESC
							break;
					}
				}
			}

			new_back_y = new_back_y_check(block_type, new_dir_type, new_back_y);//����Ƿ��������ұ�

			block_display(block_type, befor_dir_type, befor_back_x, befor_back_y, BACK);//ԭ���ķ���λ�������

			if (-1 != block_dir) //�����ƶ��˷���
			{
				if (ERROR == block_move_check_y(block_type, new_dir_type, new_back_x, new_back_y,
				                                block_dir)) //����ƶ��ķ����Ǵ��������ҵķ���
				{
					new_back_y = befor_back_y;
				}
			}

			block_display(block_type, new_dir_type, new_back_x, new_back_y, NODE);//����һ������λ��

			if (ERROR == block_move_check_x(block_type, new_dir_type, new_back_x, new_back_y)) //����ƶ��ķ����Ƿ񴥷�������ķ���
			{
				break;
			}

			Sleep(time);
		}

		block_display(block_type, 0, 17, 26, BACK);//���Ԥ������ķ���

		if (OK == block_clear_sort()) //������淽���Ƿ�ȹ��÷֣��ܵ÷��������÷�
		{
			set_windows_pos(8, 22);//���µ÷�
			printf("%d", score);
		}
	}

	return 0;
}

/***************************************************************/
/***  				���汳���ĳ�ʼ��                         ***/
/***����ԭ�������Ͻǣ�����Ϊx�ᣬ����Ϊy�ᡣy(0 - 19)Ϊ������***/
/***��20 - 30 Ϊ�Ʒ����򼰷���Ԥ����ʾ����                 ***/
/***************************************************************/
void backgroud_init(void)
{
	int x = 0, y = 0;

	for (x = 0; x < 20; x++)
	{
		for (y = 0; y < 20; y++)
		{
			back[x][y] = BACK;
		}
	}
	for (x = 0; x < 20; x++)
	{
		for (y = 20; y < 30; y++)
		{
			if ((0 == x) || (4 == x) || (10 == x) || (19 == x))
			{
				back[x][y] = FRAME;
			}
			if ((20 == y) || (29 == y))
			{
				back[x][y] = FRAME;
			}
		}
	}
	//����ͼ������ʾ
	for (x = 0; x < 20; x++)
	{
		for (y = 0; y < 30; y++)
		{
			printf("%c", back[x][y]);
		}

		printf("\n");
	}
	//��ʾTETRIS
	set_windows_pos(2, 22);//�ƶ�windows�Ĺ��
	printf("TETRIS\n");
	//��ʾ����
	set_windows_pos(6, 22);//�ƶ�windows�Ĺ��
	printf("SORT\n");
	set_windows_pos(8, 22);
	printf("%d\n", score);
	//��ʾ��һ��Ҫ���ֵķ���
	set_windows_pos(12, 22);//�ƶ�windows�Ĺ��
	printf("EXPECT\n");
}
/***************************************************************/
/***  		����windows��꣬������TC�е�gotoxy              ***/
/***i,j ΪҪ�����x,y����									 ***/
/***************************************************************/
void set_windows_pos(int i, int j)//����windows��꣬������TC�е�gotoxy
{
	/*if((0 > i) || (0 > j))
	{
		return ERROR;
	}*/

	/*windows�ĺ�����x�ᣬ����������Ƶ���������X��
	 ��������ת��*/
	COORD pos = {j, i};
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hOut, pos);
}

/***************************************************************/
/***********************����/���Ϸ���***************************/
/***����:block_type ��������  1������  2��2��  3:7��  4������***/
/***����:dir_type ��������  0������  1������                 ***/
/***����:coor_x coor_y   ���鵱ǰͷ������                    ***/
/***����:color_type    ͼ������   BACK ����ɫ  NODE ����ɫ   ***/
/***��ʼcoor_x,coor_yΪһ������������µ�һ�����������      ***/
/***************************************************************/
void block_display(int block_type, int dir_type, int coor_x, int coor_y, int color_type)
{
	int x = 0, y = 0;

	switch (block_type)
	{
		case 1://����
			if (0 == dir_type) //����
			{
				for (y = coor_y; y >= (coor_y - 3); y--)
				{
					back[coor_x][y] = color_type;//����ɫ
					set_windows_pos(coor_x, y);//�ƶ�windows�Ĺ��
					printf("%c", back[coor_x][y]);
				}
			}
			else if (1 == dir_type) //����
			{
				for (x = coor_x; (x >= (coor_x - 3)) && (x >= 0); x--)
				{
					back[x][coor_y] = color_type;//����ɫ
					set_windows_pos(x, coor_y);//�ƶ�windows�Ĺ��
					printf("%c", back[x][coor_y]);
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 2://2��
			if (0 == dir_type) //����
			{
				for (y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//����ɫ
					set_windows_pos(coor_x, y);//�ƶ�windows�Ĺ��
					printf("%c", back[coor_x][y]);
				}

				coor_x--;
				coor_y--;

				if (coor_x < 0)
				{
					return;
				}

				for (y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//����ɫ
					set_windows_pos(coor_x, y);//�ƶ�windows�Ĺ��
					printf("%c", back[coor_x][y]);
				}
			}
			else if (1 == dir_type) //����
			{
				for (x = coor_x; (x >= coor_x - 1) && (x >= 0); x--)
				{
					back[x][coor_y] = color_type;//����ɫ
					set_windows_pos(x, coor_y);//�ƶ�windows�Ĺ��
					printf("%c", back[x][coor_y]);
				}

				coor_x--;
				coor_y--;

				if (coor_x < 0)
				{
					return;
				}

				for (x = coor_x; x >= coor_x - 1; x--)
				{
					back[x][coor_y] = color_type;//����ɫ
					set_windows_pos(x, coor_y);//�ƶ�windows�Ĺ��
					printf("%c", back[x][coor_y]);
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 3://7��
			if (0 == dir_type) //����
			{
				for (y = coor_y; y >= coor_y - 2; y--)
				{
					back[coor_x][y] = color_type;//����ɫ
					set_windows_pos(coor_x, y);//�ƶ�windows�Ĺ��
					printf("%c", back[coor_x][y]);
				}

				coor_x--;
				coor_y = coor_y - 2;

				if (coor_x < 0)
				{
					return;
				}

				back[coor_x][coor_y] = color_type;//����ɫ
				set_windows_pos(coor_x, coor_y);//�ƶ�windows�Ĺ��
				printf("%c", back[coor_x][coor_y]);
			}
			else if (1 == dir_type) //����
			{
				for (x = coor_x; (x >= coor_x - 2) && (x >= 0); x--)
				{
					back[x][coor_y] = color_type;//����ɫ
					set_windows_pos(x, coor_y);//�ƶ�windows�Ĺ��
					printf("%c", back[x][coor_y]);
				}

				coor_x = coor_x - 2;
				coor_y--;

				if (coor_x < 0)
				{
					return;
				}
				back[coor_x][coor_y] = color_type;//����ɫ
				set_windows_pos(coor_x, coor_y);//�ƶ�windows�Ĺ��
				printf("%c", back[coor_x][coor_y]);

			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 4://����
			if ((0 == dir_type) || (1 == dir_type)) //����
			{
				for (y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//����ɫ
					set_windows_pos(coor_x, y);//�ƶ�windows�Ĺ��
					printf("%c", back[coor_x][y]);
				}

				coor_x--;

				if (coor_x < 0)
				{
					return;
				}

				for (y = coor_y; y >= coor_y - 1; y--)
				{
					back[coor_x][y] = color_type;//����ɫ
					set_windows_pos(coor_x, y);//�ƶ�windows�Ĺ��
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

void time_add(void)//���ٺ���
{
	if (500 == time)
	{
		time = 100;//���ٵ�100����
	}
	else if (100 == time)
	{
		time = 500;
	}
	else
	{
		;//��ʱ����
	}
}

void block_type_change(void)//�������ͱ仯
{
	block_type++;

	if (block_type > 4)
	{
		block_type = 1;
	}
}

/***************************************************************/
/*******************����y����ײǽ���***************************/
/***����:block_type ��������  1������  2��2��  3:7��  4������***/
/***����:dir_type ��������  0������  1������                 ***/
/***����:coor_y   ���鵱ǰͷ(���½ǵ�һ������)������         ***/
/***************************************************************/
int new_back_y_check(int block_type, int dir_type, int coor_y)
{
	if (coor_y > 19)
	{
		coor_y = 19;
	}

	switch (block_type)
	{
		case 1://����
			if (0 == dir_type) //����
			{
				if (coor_y - 3 < 0)
				{
					coor_y = 3;
				}
			}
			else if (1 == dir_type) //����
			{
				if (coor_y < 0)
				{
					coor_y = 0;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 2://2��
			if (0 == dir_type) //����
			{
				if (coor_y - 2 < 0)
				{
					coor_y = 2;
				}
			}
			else if (1 == dir_type) //����
			{
				if (coor_y - 1 < 0)
				{
					coor_y = 1;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 3://7��
			if (0 == dir_type) //����
			{
				if (coor_y - 2 < 0)
				{
					coor_y = 2;
				}
			}
			else if (1 == dir_type) //����
			{
				if (coor_y - 1 < 0)
				{
					coor_y = 1;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 4://����
			if ((0 == dir_type) || (1 == dir_type)) //����
			{
				if (coor_y - 1 < 0)
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
��鷽���Ƿ񴥵������Ѿ�ֹͣ�ķ��飬�������ͷ���ERROR
*/
/***************************************************************/
/****��鷽���Ƿ񴥵������Ѿ�ֹͣ�ķ��飬�������ͷ���ERROR   ***/
/***����:block_type ��������  1������  2��2��  3:7��  4������***/
/***����:dir_type ��������  0������  1������                 ***/
/***����:coor_x coor_y   ���鵱ǰͷ������                    ***/
/***��ʼcoor_x,coor_yΪһ������������µ�һ�����������      ***/
/***************************************************************/
int block_move_check_x(int block_type, int dir_type, int coor_x, int coor_y)
{
	int ret = OK;
	int x = 0, y = 0;

	switch (block_type)
	{
		case 1://����
			if (0 == dir_type) //����
			{
				for (y = coor_y; y >= coor_y - 3; y--)
				{
					if (NODE_INT == back[coor_x + 1][y])
					{
						ret = ERROR;
						break;
					}
				}
			}
			else if (1 == dir_type) //����
			{
				if (NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 2://2��
			if (0 == dir_type) //����
			{
				for (y = coor_y; y >= coor_y - 1; y--)
				{
					if (NODE_INT == back[coor_x + 1][coor_y])
					{
						ret = ERROR;
						break;
					}
				}

				coor_x--;
				coor_y = coor_y - 2 ;

				if (NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else if (1 == dir_type) //����
			{
				if (NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}

				coor_x--;
				coor_y--;

				if (NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 3://7��
			if (0 == dir_type) //����
			{
				for (y = coor_y; y >= coor_y - 2; y--)
				{
					if (NODE_INT == back[coor_x + 1][y])
					{
						ret = ERROR;
						break;
					}
				}
			}
			else if (1 == dir_type) //����
			{
				if (NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}

				coor_x = coor_x - 2;
				coor_y--;

				if (NODE_INT == back[coor_x + 1][coor_y])
				{
					ret = ERROR;
				}
			}
			else
			{
				printf("dir_type is error!\n");
			}
			break;
		case 4://����
			if ((0 == dir_type) || (1 == dir_type)) //����
			{
				for (y = coor_y; y >= coor_y - 1; y--)
				{
					if (NODE_INT == back[coor_x + 1][y])
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
/****��鷽���������ƶ��Ĺ������Ƿ񴥵����ҵķ���            ***/
/***����:block_type ��������  1������  2��2��  3:7��  4������***/
/***����:dir_type ��������  0������  1������                 ***/
/***����:coor_x coor_y   ���鵱ǰͷ�����£�������            ***/
/***����:dir_block    ��ǰ������ƶ��������ң�   			 ***/
/***************************************************************/
int block_move_check_y(int block_type, int dir_type, int coor_x, int coor_y, int dir_block)
{
	int x = 0, y = 0;
	int ret = OK;

	switch (block_type)
	{
		case 1://����
			if (0 == dir_type) //����
			{
				if (1 == dir_block) //����
				{
					if (NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
					}
				}
				else if (0 == dir_block) //����
				{
					if (NODE_INT == back[coor_x][coor_y - 3])
					{
						ret = ERROR;
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else if (1 == dir_type) //����
			{
				//������Ϊ����ʱ�������жϳ����������ƻ���������
				for (x = coor_x; x >= coor_x - 3; x--)
				{
					if (NODE_INT == back[x][coor_y])
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
		case 2://2��
			if (0 == dir_type) //����
			{
				if (1 == dir_block) //����
				{
					if (NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
						break;
					}

					if (NODE_INT == back[coor_x - 1][coor_y - 1])
					{
						ret = ERROR;
					}
				}
				else if (0 == dir_block) //����
				{
					if (NODE_INT == back[coor_x][coor_y - 1])
					{
						ret = ERROR;
						break;
					}

					if (NODE_INT == back[coor_x - 1][coor_y - 2])
					{
						ret = ERROR;
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else if (1 == dir_type) //����
			{
				if (1 == dir_block) //����
				{
					for (x = coor_x; x >= coor_x - 1; x--)
					{
						if (NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}

					if (NODE_INT == back[coor_x - 2][coor_y - 1])
					{
						ret = ERROR;
					}
				}
				else if (0 == dir_block) //����
				{
					if (NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
						break;
					}

					coor_x--;
					coor_y--;

					for (x = coor_x; x >= coor_x - 1; x--)
					{
						if (NODE_INT == back[x][coor_y])
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
		case 3://7��
			if (0 == dir_type) //����
			{
				if (1 == dir_block) //����
				{
					if (NODE_INT == back[coor_x][coor_y])
					{
						ret = ERROR;
						break;
					}

					if (NODE_INT == back[coor_x - 1][coor_y - 2])
					{
						ret = ERROR;
					}
				}
				else if (0 == dir_block) //����
				{
					if (NODE_INT == back[coor_x][coor_y - 2])
					{
						ret = ERROR;
						break;
					}

					if (NODE_INT == back[coor_x - 1][coor_y - 2])
					{
						ret = ERROR;
					}
				}
				else
				{
					printf("dir_block is error!");
				}
			}
			else if (1 == dir_type) //����
			{
				if (1 == dir_block) //����
				{
					for (x = coor_x; (x >= coor_x - 2) && (x >= 0); x--)
					{
						if (NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}
				}
				else if (0 == dir_block) //����
				{
					for (x = coor_x - 1; (x >= coor_x - 2) && (x >= 0); x--)
					{
						if (NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}

					coor_x = coor_x - 2;
					coor_y--;

					if (NODE_INT == back[coor_x][coor_y])
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
		case 4://����
			if ((0 == dir_type) || (1 == dir_type)) //����
			{
				if (1 == dir_block) //����
				{
					for (x = coor_x; x >= coor_x - 1; x--)
					{
						if (NODE_INT == back[x][coor_y])
						{
							ret = ERROR;
							break;
						}
					}
				}
				else if (0 == dir_block) //����
				{
					for (x = coor_x; x >= coor_x - 1; x--)
					{
						if (NODE_INT == back[x][coor_y - 1])
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

void block_clear_x(int row)//����ĳһ��
{
	int x = 0, y = 0;

	char back_replace[20][30] = {0};//���back

	memcpy(back_replace, back, sizeof(back));//��back�ݴ浽back_replace��

	for (x = 0; x <= row; x++)
	{
		for (y = 0; y < 20; y++)
		{
			back[x][y] = BACK;//��ʼ��δ����ɫ
		}
	}
	for (x = row; x >= 1; x--)
	{
		for (y = 0; y < 20; y++)
		{
			back[x][y] = back_replace[x - 1][y];//����һ�У������³�
		}
	}
	set_windows_pos(0, 0);//�ƶ�windows�Ĺ��
	for (x = 0; x < 20; x++)
	{
		for (y = 0; y < 20; y++)
		{
			printf("%c", back[x][y]);
		}

		printf("\n");
	}
}
/*
����Ƿ����в��ҽ��мƷ�
*/
int block_clear_sort(void)
{
	int x = 0, y = 0;
	int ret = ERROR;
	int flag = 0;

	for (x = 19; x >= 0; x--) //��
	{
		flag = 0;

		for (y = 0; y < 20; y++)
		{
			if (NODE_INT == back[x][y])
			{
				flag++;//һ�еĿ����
			}

			if (20 == flag) //��ʾһ����20������
			{
				block_clear_x(x);//����
				score++;//�ӷ�
				ret = OK;
			}
		}
	}

	return ret;
}


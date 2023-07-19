/*
 * @*************************************:
 * @FilePath: /user/C/tools/ncuress_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-07-19 16:51:08
 * @LastEditors: dof
 * @LastEditTime: 2023-07-19 16:58:29
 * @Descripttion:
 * @**************************************:
 */

#include <ncurses.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

char *options[] = {
	"Option 1",
	"Option 2",
	"Option 3"};

int main()
{
	initscr();			  // 初始化Ncurses
	cbreak();			  // 禁用行缓冲
	noecho();			  // 禁用回显
	keypad(stdscr, TRUE); // 启用键盘功能

	int option_count = ARRAY_SIZE(options);
	int current_option = 0;

	int max_row, max_col;
	getmaxyx(stdscr, max_row, max_col); // 获取终端屏幕大小

	while (1)
	{
		clear(); // 清空屏幕

		int start_row = (max_row - option_count) / 2; // 计算居中位置

		// 显示菜单选项
		for (int i = 0; i < option_count; i++)
		{
			if (i == current_option)
			{
				attron(A_REVERSE); // 选中项反色显示
			}

			int option_len = strlen(options[i]);
			int start_col = (max_col - option_len) / 2; // 计算选项的起始列位置

			mvprintw(start_row + i, start_col, options[i]);

			if (i == current_option)
			{
				attroff(A_REVERSE); // 恢复正常显示
			}
		}

		refresh(); // 刷新屏幕

		int ch = getch(); // 读取用户输入

		switch (ch)
		{
		case KEY_UP:
			current_option--;
			if (current_option < 0)
			{
				current_option = option_count - 1;
			}
			break;

		case KEY_DOWN:
			current_option++;
			if (current_option >= option_count)
			{
				current_option = 0;
			}
			break;

		case '\n':
			// 用户按下回车键，执行选中的选项
			clear();
			mvprintw(max_row - 1, 0, "Selected Option: %s", options[current_option]);
			refresh();
			getch();
			endwin();
			return 0;

		default:
			break;
		}
	}

	endwin(); // 关闭Ncurses
	return 0;
}
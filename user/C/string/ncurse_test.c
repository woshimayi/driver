/*
* sudo apt-get install libncurses5-dev
* gcc  ncurse_test.c -lncurses
* ./a.out
*
*/


#include <string.h>
#include <ncurses.h>

int main(int argc,char* argv[]){
	initscr();
	raw();
	noecho();
	curs_set(0);

	char* c = "Hello, World!";

	mvprintw(LINES/2,(COLS-strlen(c))/2,c);
	mvprintw(20, 20, "asdasdasdasdasd");
	refresh();

	getch();
	endwin();

	return 0;
}

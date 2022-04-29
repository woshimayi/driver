#include <curses.h>

int main()
{
    /*  初始化屏幕，使之进入curses工作模式      */
    initscr();

    /*  在最外围画一个方框  */
    box(stdscr, ACS_VLINE, ACS_HLINE); 

    /*  将光标移到屏幕中间, LINES代表当前屏幕最大行数，COLS代表当前屏幕最大列数  */
    move(LINES/2, COLS/2);

    /*  在stdscr上打印"Hello, World"    */
    waddstr(stdscr, "Hello, world!");
    getch();

    /*  刷新    */
    refresh();

    move(LINES/2, COLS/2);
    waddstr(stdscr, "Hello, CSDN!");
    getch();
    /*  结束curses工作模式，恢复原来的屏幕  */
    endwin();

    return 0;
}
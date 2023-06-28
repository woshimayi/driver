/*
 * @*************************************: 
 * @FilePath: /user/C/software_engineering/call_struct_1.c
 * @version: 
 * @Author: dof
 * @Date: 2023-06-21 13:39:07
 * @LastEditors: dof
 * @LastEditTime: 2023-06-21 13:40:17
 * @Descripttion: 
 * @**************************************: 
 */


/* 定义封装函数结构体由外部调用*/
typedef struct {
    int x;
    int y;
    void (*move_up)(int steps);
    void (*move_down)(int steps);
    void (*move_left)(int steps);
    void (*move_right)(int steps);
} Point;

// 定义结构体中的函数
void move_up(int steps) {
    // 向上移动steps个单位
    // ...
	printf("up %4d\n", steps);
}

void move_down(int steps) {
    // 向下移动steps个单位
    // ...
	printf("down %4d\n", steps);
}

void move_left(int steps) {
    // 向左移动steps个单位
    // ...
	printf("left %4d\n", steps);
}

void move_right(int steps) {
    // 向右移动steps个单位
    // ...
	printf("right %4d\n", steps);
}

int main() {
    // 初始化结构体
    Point point = {
        .x = 0,
        .y = 0,
        .move_up = move_up,
        .move_down = move_down,
        .move_left = move_left,
        .move_right = move_right
    };

    // 调用结构体中的函数
    point.move_up(10);
    point.move_right(5);

    return 0;
}
/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/design_pattern/status_status_2.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-20 13:34:15
 * @LastEditors: dof
 * @LastEditTime: 2023-01-20 13:35:58
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>

/***********************************************
1、定义状态接口，以MP3的状态接口为例，每种状态下都可能发生
两种按键动作。
************************************************/
typedef struct State
{
	void (*stop)();
	void (*palyOrPause)();
} State;

/***********************************************
2、定义系统当前状态指针，保存系统的当前状态
************************************************/
State *pCurrentState;

/***********************************************
3、定义具体状态，根据状态迁移图来实现具体功能和状态切换。
************************************************/
void ignore();
void startPlay();
void stopPlay();
void pausePlay();
void resumePlay();

void onStop();
void onPlayOrPause();

// 空闲状态时，stop键操作无效，play/pause会开始播放音乐
State IDLE = {
	ignore,
	startPlay};

// 播放状态时，stop键会停止播放音乐，play/pause会暂停播放音乐
State PLAY = {
	stopPlay,
	pausePlay};

// 暂停状态时，stop键会停止播放音乐，play/pause会恢复播放音乐
State PAUSE = {
	stopPlay,
	resumePlay};

void ignore()
{
	// 空函数，不进行操作
}

void startPlay()
{
	// 实现具体功能
	printf("开始播放音乐\n");
	// 进入播放状态
	pCurrentState = &PLAY;
}

void stopPlay()
{
	// 实现具体功能
	printf("停止播放音乐\n");
	// 进入空闲状态
	pCurrentState = &IDLE;
}

void pausePlay()
{
	// 实现具体功能
	printf("暂停播放音乐\n");
	// 进入暂停状态
	pCurrentState = &PAUSE;
}

void resumePlay()
{
	// 实现具体功能
	printf("恢复播放音乐\n");
	// 进入播放状态
	pCurrentState = &PLAY;
}

State context = {
	onStop,
	onPlayOrPause};

void onStop()
{
	pCurrentState->stop();
}

void onPlayOrPause()
{
	pCurrentState->palyOrPause();
}

void init()
{
	pCurrentState = &IDLE;
}

void main()
{
	init();
	context.palyOrPause(); // 播放
	context.palyOrPause(); // 暂停
	context.palyOrPause(); // 播放
	context.stop();		   // 停止
}
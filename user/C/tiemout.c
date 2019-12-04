#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <setjmp.h>

jmp_buf j;

/**
 * 时间中断函数
 */
void PASCAL OneMilliSecondProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2) {
    printf("Timout!\n");
    longjmp(j,1);
}

int longTimeFunction(){
	while (1) {
        printf("operating...\n");
        Sleep(1000);
    }

    return 0;
}


int main(){
    HANDLE hHandle;

    UINT wTimerRes_1ms;//定义时间间隔
    UINT wAccuracy; //定义分辨率
    UINT TimerID_1ms; //定义定时器句柄
    wTimerRes_1ms = 5000;
    if((TimerID_1ms = timeSetEvent(
                          wTimerRes_1ms,
                          wAccuracy,
                          (LPTIMECALLBACK)OneMilliSecondProc, // 回调函数
                          (DWORD)(1), // 用户传送到回调函数的数据；
                          TIME_PERIODIC//周期调用定时处理函数
                      )) == 0) {
        printf("start!!!!!!!!!!!\n");
    } else {
        printf("end!!!!!!!!!!!\n");
    }

    int temp = 0;
    if(setjmp(j) == 0){
    	temp = longTimeFunction();
    }else{
    	printf("xxxxxx...\n");
    	temp = -1;
    }
	
    printf("%d\n", temp);
    
    return 0;
}

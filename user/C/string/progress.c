/*
 * @*************************************: 
 * @FilePath: /user/C/string/progress.c
 * @version: 
 * @Author: dof
 * @Date: 2022-09-23 13:15:41
 * @LastEditors: dof
 * @LastEditTime: 2022-09-23 13:16:55
 * @Descripttion: 进度条
 * @**************************************: 
 */

#include <stdio.h>    
#include <string.h>    
#include <unistd.h>    
    
void ProcBar()    
{    
  int i = 0;    
  char proc[102];  
  memset(proc, '\0', sizeof(proc));    
    
  while(i <= 100)    
  {  
   //C语言格式控制时默认右对齐，所以要在前面加-变成左对齐    
    printf("[%-100s] [%d%%]\r", proc, i);                                                                                                                                  
    fflush(stdout);//刷新屏幕打印  
    proc[i] = '#';  
    usleep(100000);//以微秒为单位的sleep  
    i++;  
  }  
  printf("\n");  
}                                                                                                                                                   
                                                                                                                                             
int main()                                                                                                                                   
{                                                                                                                                            
  ProcBar();                                                                                                                              
  return 0;                                                                                                                              
}
#include<stdio.h>
#include <stdlib.h>

#ifdef __WIN32
#include <conio.h>
#endif // __WIN32

#ifdef linux
#include <curses.h>
#endif

int main()
  {  
       int ch1=0;
       int ch2=0;

    //   #ifdef __WIN32 
       while (1)
        {
          if (ch1=getch())
          { 
             ch2=getch();//第一次调用getch()，返回值224
             switch (ch2)//第二次调用getch()
             {
             case 72: printf("The key you Pressed is : ↑ \n");break;  
             case 80: printf("The key you Pressed is : ↓ \n");break; 
             case 75: printf("The key you Pressed is : ← \n");break;
             case 77: printf("The key you Pressed is : → \n");break;                                   
             default: printf("No direction keys detected \n");break;
                 break;
             }
          }
        }
    // #endif // __WIN32

    // #ifdef linux
    // system("stty -echo");
    // int c = 0;
    // while (1)
    // {
    //     if (c=getchar())
    //     {
    //         system("stty echo");
    //         printf("You have inputed:%c \n",c);
    //     }

    // }
    // #endif // linux
       return 0;
    }

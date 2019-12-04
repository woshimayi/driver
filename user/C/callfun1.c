#include <stdio.h>  

int add_ret(int a , int b)  
{  
	printf("11111111");
    return a+b ;  
}  
  
int add(int a , int b , int (*add_value)())  
{  
    (*add_value);
    return 0;
}  
  
int main(void)  
{  
    int sum = add(3,4,add_ret);  
    printf("sum:%d\n",sum);  
    printf("=\r");
    printf("===\r");
    printf("========\r");
    return 0 ;  
}   
  

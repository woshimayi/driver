#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char *strrpl(char * s, const char * s1, const char * s2)
{
	char * ptr;
	
	while(ptr =  strstr(s, s1))
	{
		memmove(ptr+strlen(s2), ptr+strlen(s1), strlen(ptr)-strlen(s1)+1);
		memcpy(ptr, &s2[0], strlen(s2));
	}
	
	return s;
}


//#define REPLACE(s) \
//	if (strchr(s, '+')) \
//    { \
//    	strrpl(s, "+", "-"); \
//    	return s; \
//	} \
//	else if (strchr(s, '-')) \
//	{ \
//    	strrpl(s, "-", "+"); \
//    	return s; \
//	}

int main(int argc, const char *argv[])
{
#if 0	
	char date[128] = "04/29/2020";
	int M = 0;
	int D = 0;
	int Y = 0;
	
	sscanf(date, "%d/%d/%d", &M, &D, &Y);
	printf("%d %d %d \n", M, D, Y);
		
    strrpl(date, "/", "-");
    printf("date = %s", date);
#endif    

    char zone[128] = "GMT+9";
    
    if (strchr(zone, '+'))
    {
    	strrpl(zone, "+", "-");
    	printf("+++++++++ %s\n", strrpl(zone, "+", "-"));
	}
	else if (strchr(zone, '-'))
	{
    	strrpl(zone, "-", "+");
    	printf("--------- %s\n", strrpl(zone, "+", "-"));
	}
	
    printf("date = %s", zone);
    
       
    
    

	return 0;
}


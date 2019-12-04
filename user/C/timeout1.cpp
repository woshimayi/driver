#include <stdio.h>
#include <sys/time.h>
 
 
int timeout(int timeout, char sec)
{
	long long tmpTime = 0;
	if (timeout == 0)
	{
		return 0;
	}
	
	
	switch(sec)
	{
		case 'u':
			tmpTime = timeout;
			break;
		case 'm':
			tmpTime = timeout*1000;
			break;
		default:
			tmpTime = timeout*1000000;
	}
	
	float time_use=0;
	struct timeval start;
	struct timeval end;//struct timezone tz; 
	gettimeofday(&start,NULL); //gettimeofday(&start,&tz);
	while(1)
	{
		gettimeofday(&end,NULL);
		time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//Î¢Ãë
 		printf("time_use = %f\n", time_use);
		if(time_use>=tmpTime)
		{
			printf("time is enough!\n");
			return 1;
		}
		else
		{
//			break;
		}
	}
	return 0;
}

int main()
{
	int ret = -1;
	
	ret = timeout(12, 'u');
	if (ret == 0)
	{
		
	}
	return 0;	
}


/*
	timer 定时器
 */


int check_stad_timer_init()
{
	struct itimerval tick;
   
	signal(SIGALRM, check_stad_timer);
	memset(&tick, 0, sizeof(tick));

	//Timeout to run first time
	tick.it_value.tv_sec = 5;
	tick.it_value.tv_usec = 0;

	tick.it_interval.tv_sec = 5;  
       tick.it_interval.tv_usec = 0; 

	if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	printf("Set timer failed!\n");

        while(1)
        {
            sleep(15);
        }
        return 0;
}

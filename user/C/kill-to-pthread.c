#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

/**
 * 用户自定义signal
 */
#define   SIG_MY_MSG   SIGUSR1+100  


#define PP printf("%s %d\n", __FUNCTION__, __LINE__);

/**
 * [main 向其他的进程发送signal USR1]
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char * argv[])
{
	char buf[128] = {0};
	int pid;
	PP
	setenv("POT_NTP", "2020", 1);
PP
	snprintf(buf, sizeof(buf), "%s", getenv("UHTTPD_PID"));
	printf("buf = %s\n", buf);
	if ('\0' == buf[0])
	{
		printf("buf = %s\n", buf);
		return -1;
	}
	pid = atoi(buf);
	PP
	printf("main buf %s %d\n", buf, pid);

	kill(pid, SIGALRM);
	PP
	printf("end \n");
	return 0;
}
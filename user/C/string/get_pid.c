/*
 * @*************************************: 
 * @FilePath: /user/C/get_pid.c
 * @version: 
 * @Author: dof
 * @Date: 2023-07-25 09:56:27
 * @LastEditors: dof
 * @LastEditTime: 2023-07-25 10:01:39
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int hgs_process_in_whitelist(int pid, char * exeWhiteList[], char * exePath)
{
	char procPath[256];
	int len;
	int i;

	snprintf(procPath, sizeof(procPath), "/proc/%d/exe", pid);
	len = readlink(procPath, exePath, 256);
	if(len <= 0)
	{
		return 0;
	}
	
	exePath[len] = '\0'; 

	for(i = 0;exeWhiteList[i];i++)
	{
		if(strstr(exePath, exeWhiteList[i]))
		{
			return 1;
		}
	}

	return 0;
}

int hgs_register_reboot()
{
	// int fd;
	char * whiteListExe[] = {"telnetd", "consoled", "hghttpd", "mdmd", "miscd", "tr69c", "omcid", "hgsloader", "get_pid", NULL};
	char exePath[256];
	// unsigned long uptime = (unsigned long)hgs_get_uptime_secs();
	
	if(!hgs_process_in_whitelist(getpid(), whiteListExe, exePath))
	{
		printf("process [%s] try to register reboot invalidly at uptime\n", exePath);

		return -1;
	}
	
	    
    // fd = open(FILE_REGISTER_REBOOT, O_WRONLY | O_CREAT, 0644);
    // if (fd == -1) {
    //     printf("Error: Unable to create file %s\n", FILE_REGISTER_REBOOT);
    //     return -2;
    // }
    // close(fd);
	
	printf("process [%s] try to register reboot successfully at uptime\n", exePath);
	system("sleep 15 && echo force reboot && reboot -f &");

	return 0;
}


int main(int argc, char const *argv[])
{
	hgs_register_reboot();
	return 0;
}

#include <stdio.h>

#include <unistd.h>

int get_exe_path(char *buf, int count)

{

	return 0;
}


int main(int argc, char **argv)
{
	char path[1024];
	int rslt = readlink("/proc/self/exe", path, 1023);
	if (rslt < 0 || (rslt >= 1023))
	{
		return 0;
	}
	path[rslt] = '\0';
	printf("path = %s\n", path);

	return 0;
}

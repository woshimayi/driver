#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "../include/vos_log.h"

int main(int argc, char * argv[])
{

	vos_print("asd\n");
	vos_error("asd\n");
	vos_notic("ads\n");
	vos_debug("ads\n");
	return 0;
}

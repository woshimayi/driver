/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

static int do_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("my cmd\n");
	return 0;
}

U_BOOT_CMD(
	my_cmd,	CONFIG_SYS_MAXARGS,	1,	do_cmd,
	"print my cmd",
	"\n"
	"	- print my cmd\n"
	"my command ...\n"
	"	- print 'my cmd'"
);


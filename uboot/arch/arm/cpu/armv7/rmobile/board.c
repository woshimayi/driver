/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>

int checkboard(void)
{
	printf("Board: %s %s %d\n", sysinfo.board_string, __FUNCTION__, __LINE__);
	return 0;
}

/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * (C) Copyright 2004 Texas Insturments
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

__weak void reset_misc(void)
{
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	puts("resetting ...\n");

	udelay(50000); /* wait 50 ms */

	disable_interrupts();

	reset_misc();
	reset_cpu(0);

	/*NOTREACHED*/
	return 0;
}


sudo apt install nfs-common 
sudo apt install nfs-kernel-server

在
sudo vim /etc/export


                    root=/dev/nfs    nfsroot=[<server-ip>:]<root-dir>[,<nfs-options>] ip=<client-ip>:<server-ip>:<gw-ip>:<netmask>:<hostname>:<device>:<autoconf>:<dns0-ip>:<dns1-ip>
// setenv bootargs 'console=ttymxc0,115200 root=/dev/nfs rw nfsroot=192.168.0.100:/home/zs/linux/nfs/rootfs  ip=192.168.0.1:192.168.0.100:192.168.0.1:255.255.255.0::eth0:off'
// setenv bootcmd 'nfs 80800000 192.168.0.100:/home/zs/linux/nfs/zImage;nfs 83000000 192.168.0.100:/home/zs/linux/nfs/imx6ull-14x14-evk-my-nand.dtb;bootz 80800000 - 83000000;'
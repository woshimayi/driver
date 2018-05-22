/*
 * a simple char device driver: globalmem without mutex
 *
 * Copyright (C) 2014 Barry Song  (baohua@kernel.org)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/module.h>
#include <linux/init.h>


int init_module(void)
{
	printk(KERN_ALERT"hello world\n");
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_ALERT"GOODbye world\n");
}

MODULE_LICENSE("GPL v2");

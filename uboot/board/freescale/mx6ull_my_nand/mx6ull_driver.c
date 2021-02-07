/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include "mx6_common.h"
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/gpio.h>



static iomux_v3_cfg_t const leds_pads[] =
{
    CONFIG_LED1_IOMUXC | MUX_PAD_CTRL(NO_PAD_CTRL),
    CONFIG_LED2_IOMUXC | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int imx6_light_up_led1(void)
{
    gpio_direction_output(CONFIG_LED1, 0);
    return 0;
}

int imx6_light_up_led2(void)
{
    gpio_direction_output(CONFIG_LED2, 0);
    return 0;
}


static int do_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    printf("my cmd\n");

    //
    //	imx_iomux_v3_setup_multiple_pads(leds_pads, ARRAY_SIZE(leds_pads));
    //
    //	imx_iomux_v3_setup_multiple_pads(iox_pads, ARRAY_SIZE(iox_pads));
    //
    //
    //    while (1)
    //    {
    //
    //        imx6_light_up_led1();
    //        imx6_light_up_led2();
    //    }


    return 0;
}

/*
 * jstest.c  Version 1.2
 *
 * Copyright (c) 1996-1999 Vojtech Pavlik
 *
 * Sponsored by SuSE
 */

/*
 * This program can be used to test all the features of the Linux
 * joystick API, including non-blocking and select() access, as
 * well as version 0.x compatibility mode. It is also intended to
 * serve as an example implementation for those who wish to learn
 * how to write their own joystick using applications.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#define _DEFAULT_SOURCE

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <linux/input.h>
#include <linux/joystick.h>

#include "axbtnmap.h"

char *axis_names[ABS_MAX + 1] =
{
    "X", "Y", "Z", "Rx", "Ry", "Rz", "Throttle", "Rudder",
    "Wheel", "Gas", "Brake", "?", "?", "?", "?", "?",
    "Hat0X", "Hat0Y", "Hat1X", "Hat1Y", "Hat2X", "Hat2Y", "Hat3X", "Hat3Y",
    "?", "?", "?", "?", "?", "?", "?",
};

/* These must match the constants in include/uapi/linux/input.h */
char *button_names[KEY_MAX - BTN_MISC + 1] =
{
    /* BTN_0, 0x100, to BTN_9, 0x109 */
    "Btn0", "Btn1", "Btn2", "Btn3", "Btn4", "Btn5", "Btn6", "Btn7", "Btn8", "Btn9",
    /* 0x10a to 0x10f */
    "?", "?", "?", "?", "?", "?",
    /* BTN_LEFT, 0x110, to BTN_TASK, 0x117 */
    "LeftBtn", "RightBtn", "MiddleBtn", "SideBtn", "ExtraBtn", "ForwardBtn", "BackBtn", "TaskBtn",
    /* 0x118 to 0x11f */
    "?", "?", "?", "?", "?", "?", "?", "?",
    /* BTN_TRIGGER, 0x120, to BTN_PINKIE, 0x125 */
    "Trigger", "ThumbBtn", "ThumbBtn2", "TopBtn", "TopBtn2", "PinkieBtn",
    /* BTN_BASE, 0x126, to BASE6, 0x12b */
    "BaseBtn", "BaseBtn2", "BaseBtn3", "BaseBtn4", "BaseBtn5", "BaseBtn6",
    /* 0x12c to 0x12e */
    "?", "?", "?",
    /* BTN_DEAD, 0x12f */
    "BtnDead",
    /* BTN_A, 0x130, to BTN_TR2, 0x139 */
    "BtnA", "BtnB", "BtnC", "BtnX", "BtnY", "BtnZ", "BtnTL", "BtnTR", "BtnTL2", "BtnTR2",
    /* BTN_SELECT, 0x13a, to BTN_THUMBR, 0x13e */
    "BtnSelect", "BtnStart", "BtnMode", "BtnThumbL", "BtnThumbR",
    /* 0x13f */
    "?",
    /* Skip the BTN_DIGI range, 0x140 to 0x14f */
    "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
    /* BTN_WHEEL / BTN_GEAR_DOWN, 0x150, to BTN_GEAR_UP, 0x151 */
    "WheelBtn", "Gear up",
};

#define NAME_LENGTH 128

int main(int argc, char **argv)
{
    int fd, i;
    unsigned char axes = 2;
    unsigned char buttons = 2;
    int version = 0x000800;
    char name[NAME_LENGTH] = "Unknown";
    uint16_t btnmap[BTNMAP_SIZE];
    uint8_t axmap[AXMAP_SIZE];
    int btnmapok = 1;

    if (argc < 2 || argc > 3 || !strcmp("--help", argv[1]))
    {
        puts("");
        puts("Usage: jstest [<mode>] <device>");
        puts("");
        puts("Modes:");
        puts("  --normal           One-line mode showing immediate status");
        puts("  --old              Same as --normal, using 0.x interface");
        puts("  --event            Prints events as they come in");
        puts("  --nonblock         Same as --event, in nonblocking mode");
        puts("  --select           Same as --event, using select() call");
        puts("");
        return 1;
    }
    if ((fd = open(argv[argc - 1], O_RDONLY)) < 0)
    {
        perror("jstest");
        return 1;
    }

    ioctl(fd, JSIOCGVERSION, &version);
    ioctl(fd, JSIOCGAXES, &axes);
    ioctl(fd, JSIOCGBUTTONS, &buttons);
    ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);

    getaxmap(fd, axmap);
    getbtnmap(fd, btnmap);

    printf("Driver version is %d.%d.%d.\n",
           version >> 16, (version >> 8) & 0xff, version & 0xff);

    /* Determine whether the button map is usable. */
    for (i = 0; btnmapok && i < buttons; i++)
    {
        if (btnmap[i] < BTN_MISC || btnmap[i] > KEY_MAX)
        {
            btnmapok = 0;
            break;
        }
    }
    if (!btnmapok)
    {
        /* btnmap out of range for names. Don't print any. */
        puts("jstest is not fully compatible with your kernel. Unable to retrieve button map!");
        printf("Joystick (%s) has %d axes ", name, axes);
        printf("and %d buttons.\n", buttons);
    }
    else
    {
        printf("Joystick (%s) has %d axes (", name, axes);
        for (i = 0; i < axes; i++)
            printf("%s%s", i > 0 ? ", " : "", axis_names[axmap[i]]);
        puts(")");

        printf("and %d buttons (", buttons);
        for (i = 0; i < buttons; i++)
        {
            printf("%s%s", i > 0 ? ", " : "", button_names[btnmap[i] - BTN_MISC]);
        }
        puts(").");
    }

    printf("Testing ... (interrupt to exit)\n");

    /*
     * Old (0.x) interface.
     */

    if ((argc == 2 && version < 0x010000) || !strcmp("--old", argv[1]))
    {

        struct JS_DATA_TYPE js;

        while (1)
        {

            if (read(fd, &js, JS_RETURN) != JS_RETURN)
            {
                perror("\njstest: error reading");
                return 1;
            }

            printf("Axes: X:%3d Y:%3d Buttons: A:%s B:%s\r",
                   js.x, js.y, (js.buttons & 1) ? "on " : "off", (js.buttons & 2) ? "on " : "off");

            fflush(stdout);

            usleep(10000);
        }
    }

    /*
     * Event interface, single line readout.
     */

    if (argc == 2 || !strcmp("--normal", argv[1]))
    {

        int *axis;
        char *button;
        int i;
        struct js_event js;

        axis = calloc(axes, sizeof(int));
        button = calloc(buttons, sizeof(char));

        while (1)
        {
            if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event))
            {
                perror("\njstest: error reading");
                return 1;
            }

            switch (js.type & ~JS_EVENT_INIT)
            {
                case JS_EVENT_BUTTON:
                    button[js.number] = js.value;
                    break;
                case JS_EVENT_AXIS:
                    axis[js.number] = js.value;
                    break;
            }

            printf("\r");

            if (axes)
            {
                printf("Axes: ");
                for (i = 0; i < axes; i++)
                    printf("%2d:%6d ", i, axis[i]);
            }

            if (buttons)
            {
                printf("Buttons: ");
                for (i = 0; i < buttons; i++)
                    printf("%2d:%s ", i, button[i] ? "on " : "off");
            }

            fflush(stdout);
        }
    }


    /*
     * Event interface, events being printed.
     */

    if (!strcmp("--event", argv[1]))
    {

        struct js_event js;

        while (1)
        {
            if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event))
            {
                perror("\njstest: error reading");
                return 1;
            }

            printf("Event: type %d, time %d, number %d, value %d\n",
                   js.type, js.time, js.number, js.value);

            fflush(stdout);
        }
    }

    /*
     * Reading in nonblocking mode.
     */

    if (!strcmp("--nonblock", argv[1]))
    {

        struct js_event js;

        fcntl(fd, F_SETFL, O_NONBLOCK);

        while (1)
        {

            while (read(fd, &js, sizeof(struct js_event)) == sizeof(struct js_event))
            {
                printf("Event: type %d, time %d, number %d, value %d\n",
                       js.type, js.time, js.number, js.value);
            }

            if (errno != EAGAIN)
            {
                perror("\njstest: error reading");
                return 1;
            }

            usleep(10000);
        }
    }

    /*
     * Using select() on joystick fd.
     */

    if (!strcmp("--select", argv[1]))
    {

        struct js_event js;
        fd_set set;


        while (1)
        {

            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            FD_ZERO(&set);
            FD_SET(fd, &set);

            if (select(fd + 1, &set, NULL, NULL, &tv))
            {

                if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event))
                {
                    perror("\njstest: error reading");
                    return 1;
                }

                printf("Event: type %d, time %d, number %d, value %d\n",
                       js.type, js.time, js.number, js.value);

            }

        }
    }

    printf("jstest: unknown mode: %s\n", argv[1]);
    return -1;
}

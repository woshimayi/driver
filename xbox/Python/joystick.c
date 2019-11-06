#include "joystick.h"


int main(void)
{
    int xbox_fd, len = -1;
    xbox_map_t map;
    xbox_init(xbox_fd, &map);

    while (1)
    {
        len = xbox_map_read(xbox_fd, &map);
        if (len < 0)
        {
            usleep(10 * 1000);
            continue;
        }

        printf("\rTime:%8d A:%d B:%d X:%d Y:%d LB:%d RB:%d start:%d back:%d home:%d LO:%d RO:%d XX:%-6d YY:%-6d LX:%-6d LY:%-6d RX:%-6d RY:%-6d LT:%-6d RT:%-6d",
               map.time, map.a, map.b, map.x, map.y, map.lb, map.rb, map.start, map.back, map.home, map.lo, map.ro,
               map.xx, map.yy, map.lx, map.ly, map.rx, map.ry, map.lt, map.rt);
        fflush(stdout);
    }

    xbox_close(xbox_fd);
    return 0;
}


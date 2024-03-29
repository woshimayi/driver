#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/joystick.h>

#define XBOX_TYPE_BUTTON    0x01
#define XBOX_TYPE_AXIS      0x02

#define XBOX_BUTTON_A       0x00
#define XBOX_BUTTON_B       0x01
#define XBOX_BUTTON_X       0x02
#define XBOX_BUTTON_Y       0x03
#define XBOX_BUTTON_LB      0x04
#define XBOX_BUTTON_RB      0x05
#define XBOX_BUTTON_START   0x06
#define XBOX_BUTTON_BACK    0x07
#define XBOX_BUTTON_HOME    0x08
#define XBOX_BUTTON_LO      0x09    /* 左摇杆按键 */
#define XBOX_BUTTON_RO      0x0a    /* 右摇杆按键 */

#define XBOX_BUTTON_ON      0x01
#define XBOX_BUTTON_OFF     0x00

#define XBOX_AXIS_LX        0x00    /* 左摇杆X轴 */
#define XBOX_AXIS_LY        0x01    /* 左摇杆Y轴 */
#define XBOX_AXIS_RX        0x03    /* 右摇杆X轴 */
#define XBOX_AXIS_RY        0x04    /* 右摇杆Y轴 */
#define XBOX_AXIS_LT        0x02
#define XBOX_AXIS_RT        0x05
#define XBOX_AXIS_XX        0x06    /* 方向键X轴 */
#define XBOX_AXIS_YY        0x07    /* 方向键Y轴 */

#define XBOX_AXIS_VAL_UP    -32767
#define XBOX_AXIS_VAL_DOWN   32767
#define XBOX_AXIS_VAL_LEFT  -32767
#define XBOX_AXIS_VAL_RIGHT  32767

#define XBOX_AXIS_VAL_MIN   -32767
#define XBOX_AXIS_VAL_MAX    32767
#define XBOX_AXIS_VAL_MID    0x00

typedef struct xbox_map
{
    int     time;
    int     a;
    int     b;
    int     x;
    int     y;
    int     lb;
    int     rb;
    int     start;
    int     back;
    int     home;
    int     lo;
    int     ro;

    int     lx;
    int     ly;
    int     rx;
    int     ry;
    int     lt;
    int     rt;
    int     xx;
    int     yy;

} xbox_map_t;


int xbox_open(const char *file_name)
{
    int xbox_fd;
    int i4_open_flags = O_RDONLY;
    i4_open_flags |= O_NONBLOCK; // 使用非阻塞模式，否则无法进行顺利读取数据

    xbox_fd = open(file_name, i4_open_flags);
    if (xbox_fd < 0)
    {
        perror("open");
        return -1;
    }

    return xbox_fd;
}

int xbox_map_read(int xbox_fd, xbox_map_t *map)
{
    int len, type, number, value;
    struct js_event js;

    len = read(xbox_fd, &js, sizeof(struct js_event));
    if (len < 0)
    {
        perror("read");
        return -1;
    }

    type = js.type;
    number = js.number;
    value = js.value;

    map->time = js.time;

    if (type == JS_EVENT_BUTTON)
    {
        switch (number)
        {
            case XBOX_BUTTON_A:
                printf("XBOX_BUTTON_A\n");
                map->a = value;
                break;

            case XBOX_BUTTON_B:
                printf("XBOX_BUTTON_B\n");
                map->b = value;
                break;

            case XBOX_BUTTON_X:
                printf("XBOX_BUTTON_X\n");
                map->x = value;
                break;

            case XBOX_BUTTON_Y:
                printf("XBOX_BUTTON_Y\n");
                map->y = value;
                break;

            case XBOX_BUTTON_LB:
                printf("XBOX_BUTTON_LB\n");
                map->lb = value;
                break;

            case XBOX_BUTTON_RB:
                printf("XBOX_BUTTON_RB\n");
                map->rb = value;
                break;

            case XBOX_BUTTON_START:
                printf("XBOX_BUTTON_START\n");
                map->start = value;
                break;

            case XBOX_BUTTON_BACK:
                printf("XBOX_BUTTON_BACK\n");
                map->back = value;
                break;

            case XBOX_BUTTON_HOME:
                printf("XBOX_BUTTON_HOME\n");
                map->home = value;
                break;

            case XBOX_BUTTON_LO:
                printf("XBOX_BUTTON_LO\n");
                map->lo = value;
                break;

            case XBOX_BUTTON_RO:
                printf("XBOX_BUTTON_RO\n");
                map->ro = value;
                break;

            default:
                break;
        }
    }
    else if (type == JS_EVENT_AXIS)
    {
        switch (number)
        {
            case XBOX_AXIS_LX:
                printf("XBOX_AXIS_LX\n");
                map->lx = value;
                break;

            case XBOX_AXIS_LY:
                printf("XBOX_AXIS_LY\n");
                map->ly = value;
                break;

            case XBOX_AXIS_RX:
                printf("XBOX_AXIS_RX\n");
                map->rx = value;
                break;

            case XBOX_AXIS_RY:
                printf("XBOX_AXIS_RY\n");
                map->ry = value;
                break;

            case XBOX_AXIS_LT:
                printf("XBOX_AXIS_LT\n");
                map->lt = value;
                break;

            case XBOX_AXIS_RT:
                printf("XBOX_AXIS_RT\n");
                map->rt = value;
                break;

            case XBOX_AXIS_XX:
                printf("XBOX_AXIS_XX\n");
                map->xx = value;
                break;

            case XBOX_AXIS_YY:
                printf("XBOX_AXIS_YY\n");
                map->yy = value;
                break;

            default:
                break;
        }
    }
    else
    {
        /* Init do nothing */
    }

    printf("\rTime:%8d A:%d B:%d X:%d Y:%d LB:%d RB:%d start:%d back:%d home:%d LO:%d RO:%d XX:%-6d YY:%-6d LX:%-6d LY:%-6d RX:%-6d RY:%-6d LT:%-6d RT:%-6d",
           map->time, map->a, map->b, map->x, map->y, map->lb, map->rb, map->start, map->back, map->home, map->lo, map->ro,
           map->xx, map->yy, map->lx, map->ly, map->rx, map->ry, map->lt, map->rt);

    return len;
}


int xbox_init(int xbox_fd, xbox_map_t *map)
{
    int len, type;

    memset(map, 0, sizeof(xbox_map_t));

    xbox_fd = xbox_open("/dev/input/js0");
    if (xbox_fd < 0)
    {
        return -1;
    }
}


void xbox_close(int xbox_fd)
{
    close(xbox_fd);
    return;
}


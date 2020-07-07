#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


//t == 0 为当前时间
//t != 0 位标准时间加t

/**
 * [oalTms_getXSIDateTime 获取从1970之后指定的时间]
 * @param  t      [1970 之后的秒数]
 * @param  buf    [description]
 * @param  bufLen [description]
 * @return        [description]
 */
int oalTms_getXSIDateTime(int t, char *buf, int bufLen)
{
    int          c;
    time_t       now;
    struct tm   *tmp;

    if (t == 0)
    {
        now = time(NULL);
    }
    else
    {
        now = t;
    }

    tmp = localtime(&now);
    memset(buf, 0, bufLen);
    c = strftime(buf, bufLen, "%Y-%m-%dT%H:%M:%S%z", tmp);
    if ((c == 0) || (c + 1 > bufLen))
    {
        /* buf was not long enough */
        return 2;
    }

    /* fix missing : in time-zone offset-- change -500 to -5:00 */
    buf[c + 1] = '\0';
    buf[c] = buf[c - 1];
    buf[c - 1] = buf[c - 2];
    buf[c - 2] = ':';
    printf("buf = %s\n", buf);
    return 0;
}


int main()
{
    int t = 0;
    char buf[100] = {0};
    int bufLen = 100;
    int ret = -1;
    ret = oalTms_getXSIDateTime(10, buf, bufLen);
    printf("%s\n", buf);
    printf("ret = %d\n", ret);
    return 0;
}





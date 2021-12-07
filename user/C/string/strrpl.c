#include <stdio.h>
#include <string.h>
#include <stdlib.h>



typedef struct
{
    char timeZones[128];
    char TimeZone[32];
    char firstServer[128];
    char secondServer[128];
    char zoneName[128];
} ZoneTime_T;


#define   TIMESERVER_A   "time-a.netgear.com"
#define   TIMESERVER_B   "time-b.netgear.com"
#define   TIMESERVER_C   "time-c.netgear.com"
#define   TIMESERVER_D   "time-d.netgear.com"
#define   TIMESERVER_E   "time-e.netgear.com"
#define   TIMESERVER_F   "time-f.netgear.com"
#define   TIMESERVER_G   "time-g.netgear.com"
#define   TIMESERVER_H   "time-h.netgear.com"


char *strrpl(char *s, const char *s1, const char *s2)
{
    char *ptr;

    while (ptr =  strstr(s, s1))
    {
        memmove(ptr + strlen(s2), ptr + strlen(s1), strlen(ptr) - strlen(s1) + 1);
        memcpy(ptr, &s2[0], strlen(s2));
    }

    return s;
}


//#define REPLACE(s) \
//	if (strchr(s, '+')) \
//    { \
//    	strrpl(s, "+", "-"); \
//    	return s; \
//	} \
//	else if (strchr(s, '-')) \
//	{ \
//    	strrpl(s, "-", "+"); \
//    	return s; \
//	}

int main(int argc, const char *argv[])
{
#if 0
    char date[128] = "04/29/2020";
    int M = 0;
    int D = 0;
    int Y = 0;

    sscanf(date, "%d/%d/%d", &M, &D, &Y);
    printf("%d %d %d \n", M, D, Y);

    strrpl(date, "/", "-");
    printf("date = %s", date);

    char zone[128] = "GMT+9";

    if (strchr(zone, '+'))
    {
        strrpl(zone, "+", "-");
        printf("+++++++++ %s\n", strrpl(zone, "+", "-"));
    }
    else if (strchr(zone, '-'))
    {
        strrpl(zone, "-", "+");
        printf("--------- %s\n", strrpl(zone, "+", "-"));
    }

    printf("date = %s", zone);
#endif

    //    ZoneTime_T zoneTime[] =
    //	{
    //	    {"International Date Line West",                                        "-12", TIMESERVER_C, TIMESERVER_D, NULL},
    //	    {"Midway Island, Samoa",                                                "-11", TIMESERVER_C, TIMESERVER_D, NULL},
    //	    {"Hawaii",                                                              "-10", TIMESERVER_C, TIMESERVER_D, 0},
    //	    {"Alaska",                                                               "-9", TIMESERVER_B, TIMESERVER_C, 0},
    //	    {"Pacific Time, Tijuana",                                                "-8", TIMESERVER_B, TIMESERVER_C, "PST8PDT,M3.2.0,M11.1.0"},
    //	    {"Arizona, Chihuahua, La Paz, Mazatlan",                                 "-7", TIMESERVER_B, TIMESERVER_C, "MST7MDT,M4.1.0,M10.5.0"},
    //	};
    //
    //	if (NULL == zoneTime[2].zoneName)
    //	{
    //		printf("%s\n", zoneTime[2].zoneName);
    //		printf("%s\n", zoneTime[3].zoneName);
    //		printf("%s\n", zoneTime[4].zoneName);
    //	}


#define _LINE_LENGTH 128

    FILE *file;
    char line[_LINE_LENGTH];
    file = popen("date +%", "r");
    if (NULL != file)
    {
        while (fgets(line, _LINE_LENGTH, file) != NULL)
        {
            printf("line=%s\n", line);
        }
    }
    else
    {
        return 1;
    }
    pclose(file);






    return 0;
}


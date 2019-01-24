#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <regex.h>
#include <string.h>
#include <ctype.h>


#ifdef linux
int is_valid_mac_addr(char* mac)
{
	int status = 0;
	const char * pattern = "^([A-Fa-f0-9]{2}[-,:]){5}[A-Fa-f0-9]{2}$";
	const int cflags = REG_EXTENDED | REG_NEWLINE;

	char ebuf[128] = {0};
	regmatch_t pmatch[1] = {{0}};
	int nmatch = 10;
	regex_t reg;


	status = regcomp(&reg, pattern, cflags);//编译正则模式
	if(status != 0) {
		regerror(status, &reg, ebuf, sizeof(ebuf));
		printf("%s:%d regcomp fail: %s , pattern '%s' \n", __FUNCTION__, __LINE__, ebuf, pattern);
		goto failed;
	}

	status = regexec(&reg, mac, nmatch, pmatch,0);//执行正则表达式和缓存的比较,
	if(status != 0) {
		regerror(status, &reg, ebuf, sizeof(ebuf));
		printf("%s:%d regexec fail: %s , mac:\"%s\" \n", __FUNCTION__, __LINE__, ebuf, mac);
		goto failed;
	}

	printf("[%s]:%d match success.\n", __FUNCTION__, __LINE__);
	regfree(&reg);
	return 0;

failed:
	regfree(&reg);
	return -1;
}
#endif // linux


int is_valid_mac(const char *mac)
{
    int len = strlen(mac);
    int ret = 0;
    int num_count = 0;
    int colon_count = 0;
    const char *start = mac;
    const char *end = mac + len - 1;

    if (mac == NULL || strlen(mac) < 17)
    {
        return -1;
    }

    while (end > start && isspace(*end))
    {
        --end;
    }

    while (start < end && isspace(*start))
    {
        ++start;
    }

    if ((end - start) != 16 || isxdigit(*start) == 0 || isxdigit(*end) == 0)
    {
        return -1;
    }

    while (start <= end)
    {
        while (start <= end && isxdigit(*start))
        {
            ++start;
            if (++num_count > 2)
            {
                ret = -1;
                break;
            }
        }

        if (start <= end )
        {
            if (*start == ':' && num_count == 2)
            {
                ++colon_count;
                num_count = 0;
                ++start;
            }
            else
            {
                ret = -1;
                break;
            }
        }
        else
        {
            break;
        }
    }

    if (ret == 0 && colon_count != 5)
    {
        ret = -1;
    }

    return ret;
}

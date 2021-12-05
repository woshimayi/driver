#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if 1
int main(int argc, char const *argv[])
{
    char        line[128];
	FILE        *fpin;

    char active[] = "bs /b/e egress_tm/dir=us,index=1 queue_stat | awk -F '=' '{print $7}' | cut -d '}' -f 1";
    if ((fpin = popen(active, "r")) == NULL)
        perror("popen error");

	for (; ;)
	{
		fputs("prompt> ", stdout);
		fflush(stdout);
		if (fgets(line, 128, fpin) == NULL) /* read from pipe */
        {
		    break;
        }
     
        printf("line = %s\n", line);
	}

	if (pclose(fpin) == -1)
		perror("pclose error");

	putchar('\n');
	exit(0);
    return 0;
}
#endif

#if 0
// #include <ctype.h>
#include <sys/types.h>
#include <regex.h>


int main(void)
{

    int i;

    char ebuff[256];

    int ret;

    int cflags;

    regex_t reg;

    regmatch_t rm[5];

    char *part_str = NULL;

    cflags = REG_EXTENDED | REG_ICASE;

    // char *test_str = "queue_stat[{channel={tcont/index=1},queue_id=0}] : {passed={packets=679,bytes=58223},discarded={packets=0,bytes=0}}";
    // char *reg_str = "(queue_id).(*)";

    // char *test_str = "Hello, World=";
    // char *reg_str = "(.*?)(\1)(.*?)(\1)";
    char *test_str = "123-345-567=";
    // char *reg_str = "\d\d\d-\d\d\d-\d\d\d";
    // char *reg_str = "\d\d\d\D\d\d\d\D\d\d\d";
    // char *reg_str = "\d\d\d.\d\d\d.\d\d\d";
    char *reg_str = "(\d)\d(\1)";


    // char *test1 = strstr(test_str, "queue_id");
    // printf("zzzzz = %s\n", test1);

    ret = regcomp(&reg, reg_str, cflags);

    if (ret)
    {
        regerror(ret, &reg, ebuff, 256);
        fprintf(stderr, "%s\n", ebuff);
        goto end;
    }

    ret = regexec(&reg, reg_str,5, rm, 0);

    if (ret)

    {

        regerror(ret, &reg, ebuff, 256);

        fprintf(stderr, "%s\n", ebuff);

        goto end;
    }

    regerror(ret, &reg, ebuff, 256);
    fprintf(stderr, "result is:\n%s\n\n", ebuff);

    for (i = 0; i < 5; i++)
    {
        if (rm[i].rm_so > -1)
        {

            part_str = strndup(test_str + rm[i].rm_so, rm[i].rm_eo - rm[i].rm_so);

            fprintf(stderr, "----- %s\n", part_str);

            free(part_str);

            part_str = NULL;
        }
    }

end:

    regfree(&reg);

    return 0;
}
#endif
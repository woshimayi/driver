#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *strsplit(char **stringp, const char *delim)
{
	char *start = *stringp;
	char *p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	printf("1 p = %s\n", p);
	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}
	printf("2 p = %s stringp = %s\n", p, stringp);

	return start;
}

void rmc_data_parse(char *rmc_data)
{
	unsigned char times = 0;
	char *p;
	char *s = strdup(rmc_data);

	p = strsplit(&s, ",");
	while (p)
	{
		switch (times)
		{
			case 1:   // UTC
				printf("UTC = %s\n", p);
				break;
			case 2:   // pos status
				printf("pos = %s\n", p);
				break;
			case 3:   // lat
				printf("lat = %s\n", p);
				break;
			case 4:   // lat dir
				printf("lat dir = %s\n", p);
				break;
			case 5:   // lon
				printf("lon = %s\n", p);
				break;
			case 6:   // lon dir
				printf("lon dir = %s\n", p);
				break;
			case 7:   // speen Kn
				printf("speen Kn = %s\n", p);
				break;
			case 8:   // track true
				printf("track true = %s\n", p);
				break;
			case 9:   // date
				printf("date = %s\n", p);
				break;
			case 10:  // mag var
				printf("mag var = %s\n", p);
				break;
			case 11:  // var dir
				printf("var dir = %s\n", p);
				break;
			case 14:  // mode ind
				printf("mode ind = %s\n", p);
				break;
			default:
				break;
		}
		p = strsplit(&s, ",");
		times++;
	}
	free(s);
}

int main(int argc, const char *argv[])
{
	char gps_data[] = "$GNRMC,013300.00,A,2240.84105,N,11402.70763,E,0.007,,220319,,,D*69\r\n";
	char *p;
	//	rmc_data_parse(gps_data);

	printf("gps = %s\n", gps_data);
	//	p = strpbrk(gps_data, ",");
	//	char **start = &gps_data;
	//	while ((p+1) != '\0')
	//	{
	//		*p = '\0';
	//		printf("p = %s start = %s\n", p, *start);
	//		p = p+1;
	//		*start++ = p;
	//		p = strpbrk(p, ",");
	//	}


	//	char *tok;
	//	tok = strtok(gps_data, ",");
	//	while(tok)
	//	{
	//		printf("tok = %s\n", tok);
	//		tok = strtok(NULL, ",");
	//	}


	char *tok;
	char *ptr;
	tok = strtok_r(gps_data, ",", &ptr);
	while (tok)
	{
		printf("tok = %s ptr = %s\n", tok, ptr);
		tok = strtok_r(NULL, ",", &ptr);
	}






	return 0;
}


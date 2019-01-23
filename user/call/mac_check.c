#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
 
 
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
		fprintf(stderr, "%s regcomp fail: %s , pattern '%s' \n", __FUNCTION__, ebuf, pattern);
		goto failed;
	}
 
	status = regexec(&reg, mac, nmatch, pmatch,0);//执行正则表达式和缓存的比较,
	if(status != 0) {
		regerror(status, &reg, ebuf, sizeof(ebuf));
		fprintf(stderr, "%s regexec fail: %s , mac:\"%s\" \n", __FUNCTION__, ebuf, mac);
		goto failed;
	}
 
//	printf("[%s] match success.\n", __FUNCTION__);
	regfree(&reg);
	return 0;
 
failed:
	regfree(&reg);
	return -1;
}


int main(int argc, char *argv[])
{
	printf("%d\n", is_valid_mac_addr(argv[1]));
	
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	char *str = "<LANDeviceNumberOfEntries>2</LANDeviceNumberOfEntries>";
	printf("%c\n", (char *)strspn(str, "<LAN"));
	printf("%d\n", strspn(str, "/-"));
	printf("%d\n", strspn(str, "1234567890"));
	return 0;
}

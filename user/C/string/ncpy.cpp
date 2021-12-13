#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
	char     name[40] = "3_OTHER_R_VID_1003";
	char     *name1 = NULL;
	printf("name1 = %s %d %s\n", name1, sizeof(name), &name);
	//	name1 = strdup(name);
	strncpy(name1, name, strlen(name) + 1);
	//
	//	name1 = name;
	//	strcpy(name1, name);

	printf("name1 = %s %d %s\n", name1, sizeof(name), name);

	return 0;
}


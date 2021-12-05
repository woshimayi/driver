#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

int  main()
{
	int i = 0;
	char fileName[64] = {0}, *ch = "Dateline Standard Time.Eniwetok, Kwajalein";
	while (*ch != '\0' && *ch != ',')
	{
		if (*ch != ' ')
			fileName[i] = *ch;
		else
			fileName[i] = '_';
		ch ++;
		i ++;
	}
	printf("filename = %s\n", fileName);
	
	return 0;
}

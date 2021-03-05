/*ini.h*/
#ifndef INI_H
#define INI_H

#include <stdio.h>
#include <string.h>

int GetIniKeyString(char *title, char *key, char *filename, char *buf);

int PutIniKeyString(char *title, char *key, char *val, char *filename);

#endif /*INI_H*/
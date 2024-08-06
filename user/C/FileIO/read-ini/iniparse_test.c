#include <stdio.h>
#include "iniparser.h"
#define CONFIG_NAME "Config.ini"

void Create_Default_InI_File(void)
{
	FILE *Default_ini = NULL ;
	Default_ini = fopen(CONFIG_NAME, "w");
	fprintf(Default_ini,
	        "[Setting]\n"
	        "INIT_FLAG=0;\n"
	        "VOLUME=1;\n"
	        "LANGUAGE=1;\n"
	       );
	fclose(Default_ini);
}

int main(void)
{
	int Init_flag ;
	int Volume_flag ;
	int English_flag ;
	/*1、创建一个默认的Config.ini文件*/
	Create_Default_InI_File();
	/*2、解析Config.ini文件获得参数*/
	dictionary  *Config_ini = NULL;
	Config_ini = iniparser_load(CONFIG_NAME);
	if (NULL == Config_ini)
	{
		printf("cannot parse %s file\n", CONFIG_NAME);
		return -1 ;
	}
	iniparser_dump(Config_ini, stderr);
	Init_flag = iniparser_getint(Config_ini, "Setting:INIT_FLAG", -1);
	Volume_flag = iniparser_getint(Config_ini, "Setting:VOLUME", -1);
	English_flag = iniparser_getint(Config_ini, "Setting:LANGUAGE", -1);
	printf("Init_flag:%d\n", Init_flag);
	printf("Volume_flag:%d\n", Volume_flag);
	printf("English_flag:%d\n", English_flag);
	iniparser_set(Config_ini, "Setting:INIT_FLAG", "1");
	Init_flag = iniparser_getint(Config_ini, "Setting:INIT_FLAG", -1);
	Volume_flag = iniparser_getint(Config_ini, "Setting:VOLUME", -1);
	English_flag = iniparser_getint(Config_ini, "Setting:LANGUAGE", -1);
	printf("Init_flag:%d\n", Init_flag);
	printf("Volume_flag:%d\n", Volume_flag);
	printf("English_flag:%d\n", English_flag);
	iniparser_freedict(Config_ini);
	return 0;
}
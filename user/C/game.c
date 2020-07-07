#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * CodinGame planet is being attacked by slimy insectoid aliens.
 * <---
 * Hint:To protect the planet, you can implement the pseudo-code provided in the statement, below the player.
 **/

typedef  struct _image_head
{
    char model[32];        /*model name*/
    char region[32];       /*region*/
    char version[64];      /*version*/
	char dateTime[64];     /*date*/
    char reserved[320];    /*reserved space, if add struct member, please adjust this reserved size to keep the head total size is 512 bytes*/
} IMAGE_HEAD;


#define PP printf("%s:%d\n", __FUNCTION__, __LINE__);


int main()
{

    // game loop
     // name of enemy 1
    char enemy_1[257] = "model= ac6100";
    char str[128] = {0};
    sscanf(enemy_1, "%*s %s", str);
    printf("str = %s\n", str);
//    printf("%s\n", strchr(enemy_1, 'a'));


    char value[1024] = {0};
    char newversion[128] = {0};
    FILE *fs = NULL;
    char *find;
    IMAGE_HEAD head = {0};

    fs = fopen("./upg_new_version", "r");
    if (NULL == fs)
    {
      return;
    }
    while (fgets(newversion, sizeof(newversion), fs))
    {
//        printf("newversion: %s\n", newversion);
        if (strstr( newversion, "model"))
        {
        	PP
            sscanf(newversion, "%*s %s", head.model);
        }
        else if (strstr(newversion, "region"))
        {
        	PP
            sscanf(newversion, "%*s %s", head.region);
        }
        else if (strstr(newversion, "version"))
        {
        	PP
            sscanf(newversion, "%*s %s", head.version);
        }
        else if (strstr(newversion, "dateTime"))
        {
        	PP
            sscanf(newversion, "%*s %s", head.dateTime);
        }
    }
    fclose(fs);
    printf("model = %s, region = %s, version = %s, dateTime = %s\n", head.model, head.region, head.version, head.dateTime);

    printf("name of the enemy\n");

    return 0;
}

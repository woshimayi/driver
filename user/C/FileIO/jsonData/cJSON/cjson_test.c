/*
 * @*************************************:
 * @FilePath: /user/C/fileIO/cjson_test.c
 * @version:
 * @Author: dof
 * @Date: 2024-04-08 14:35:32
 * @LastEditors: dof
 * @LastEditTime: 2024-04-08 14:40:35
 * @Descripttion:
 * @**************************************:
 */

#include <stddef.h>
#include "cJSON.h"
#include <stdio.h>


int get_object_size(cJSON *object) {
    int size = 0;
    cJSON *child = object->child;
    while (child) {
      size++;
      child = child->next;
    }
    return size;
  }

int main(int argc, char const *argv[])
{

    // if (access(data.json, F_OK))
    // {
    //     printf("file no exits");
    //     return 0;
    // }

    // FILE *fp = fopen("data.json", "r");
    // if (fp == NULL)
    // {
    //     perror("fopen");
    //     return 1;
    // }

    // char *buffer = malloc(1024);
    // size_t n = fread(buffer, 1, 1024, fp);
    // if (n == 0)
    // {
    //     perror("fread");
    //     return 1;
    // }

    // cJSON *root = cJSON_Parse(buffer);
    // if (root == NULL)
    // {
    //     fprintf(stderr, "cJSON_Parse error\n");
    //     return 1;
    // }

    // cJSON *item = cJSON_GetObjectItem(root, "name");
    // if (item == NULL)
    // {
    //     fprintf(stderr, "cJSON_GetObjectItem error\n");
    //     return 1;
    // }

    // const char *name = cJSON_GetStringValue(item);
    // printf("name: %s\n", name);

    // cJSON_Delete(root);


    const char *json_string = "{\"name\": \"John\", \"age\": 30, \"city\": \"New York\"}";
    cJSON *json = cJSON_Parse(json_string);
  
    if (json) {
      if (json->type == cJSON_Object) {
        int size = get_object_size(json);
        printf("Object size: %d\n", size);
      }
      cJSON_Delete(json);
    }

    return 0;
}

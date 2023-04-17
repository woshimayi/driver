/*
 * @*************************************: 
 * @FilePath: /user/C/string/strcpy_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-02-09 18:05:56
 * @LastEditors: dof
 * @LastEditTime: 2023-02-14 19:38:29
 * @Descripttion: 
 * @**************************************: 
 */


// #include <stdio.h>
// #include <string.h>

// int main(int argc, char const *argv[])
// {
// 	char str[128] = {0};
// 	void *buf;
// 	char *p;
// 	int len = strlen(str);
// 	buf = malloc(len + 1);
// 	/* code */
// 	strncpy(buf, str, len);
// 	printf("zz\n");
// 	free(buf);
// 	snprintf(str, "%s,%s", buf, p);
// 	return 0;
// }



#include<stdio.h>

#pragma pack(1)
struct tree
{
  int height;
  int age;
  char tag;
};
#pragma pack() 

int main()
{
  char buffer[512];
  char *tmp_ptr = NULL;
  struct tree *t_ptr = NULL;
  char *t_ptr_new = NULL; 

  tmp_ptr = buffer;
//   t_ptr = (struct tree *) tmp_ptr;
  t_ptr = (char *)tmp_ptr;
  t_ptr_new = (char *)((char *)t_ptr + 1);

  printf("t_ptr_new point to buffer[%ld]\n", t_ptr_new - tmp_ptr);
  
  return 0;
}

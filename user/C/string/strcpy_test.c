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

#include <stdio.h>
#include <string.h>

#if 1
int main(int argc, char const *argv[])
{
  char str[128] = "1234567890";
  void *buf;
  char *p;
  int len = strlen(str);
  buf = malloc(12);
  /* code */
  strncpy(buf, str, 12);              // strncpy  按照str 的 '\0' 结束字符串的复制
  printf("zz = %s\n", buf);
  strncpy(buf, str, 3);
  printf("zz = %s\n", buf);
  snprintf(buf, 4, "%s", str);        // snprintf  按照buf sizeof 最后一个长度或者'\0' 结束字符串的复制, 并会再最后一个字符上赋值为'\0'
  printf("zz = %s\n", buf);
  free(buf);
  snprintf(str, "%s,%s", buf, p);
  return 0;
}

#else
#include <stdio.h>

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
#endif
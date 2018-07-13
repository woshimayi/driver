#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include "vos_log.h"

int Test2(void * num);
int Test3(void * num);
int Test4(void * num);

void Caller2(void* n, int (*ptr)());
#endif // _FUNCTION_H_

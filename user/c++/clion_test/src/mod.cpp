//
// Created by root on 25-5-13.
//

#include "mod.h"
#include <stdio.h>

mod::mod(/* args */)
{
}

mod::~mod()
{
}


void mod::dump() {
    printf("[%s:%d]\n", __func__, __LINE__);
}
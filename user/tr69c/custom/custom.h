#ifndef CUSTOM_H
#define CUSTOM_H

#include "protocol.h"
#include "RPCState.h"


void TR69C_buildWlanStatsCustom(tProtoCtx *pc, int *bufsz, int *paramNum);

void TR69C_buildVlanCustom(tProtoCtx *pc, int *bufsz, int *paramNum);

#endif

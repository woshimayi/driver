#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "fwk.h"
#include "RPCState.h"
#include "xmlTables.h"
#include "cmc_api.h"
#include "../main/httpProto.h"
#include "../main/informer.h"
#include "../main/httpDownload.h"
#include "../bcmLibIF/bcmWrapper.h"
#include "../inc/utils.h"
#include "tr69c_api.h"
#include "hal_util_sys.h"


extern HttpTask  simHttpTask;
extern RPCAction *simRpcAction;

UBOOL8 g_writeLog = FALSE;


RPCAction *tr69_initSimulate()
{
    HttpTask *ht = &simHttpTask;
    int fd = -1;

    if (simRpcAction != NULL)
    {
        vosLog_debug("simRpcAction is not NULL");
        freeRPCAction(simRpcAction);
    }

    simRpcAction = newRPCAction();
    if ((ht->wio = (tWgetInternal *)VOS_MALLOC_FLAGS(sizeof(tWgetInternal), ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_debug("allocate memory failed");
        return NULL;
    }

    fd = open(UTIL_CLI_TR69_RESULT_FILE, O_CREAT | O_RDWR);
    if (fd != -1)
    {
        if (g_writeLog)
        {
            ht->wio->pc = proto_NewCtx(fd);
        }
        else
        {
            ht->wio->pc = proto_NewCtx(1);
        }
    }

    return simRpcAction;
}


void tr69_clearSimulate()
{
    HttpTask *ht = &simHttpTask;

    if (simRpcAction != NULL)
    {
        freeRPCAction(simRpcAction);
    }

    simRpcAction = NULL;
    if (ht->wio != NULL)
    {
        if (ht->wio->pc != NULL)
        {
            if (1 != ht->wio->pc->fd)
            {
                close(ht->wio->pc->fd);
            }

            VOS_FREE(ht->wio->pc);
            ht->wio->pc = NULL;
        }

        VOS_FREE(ht->wio);
    }
}


void tr69_dumpToFile(FILE *file)
{
    FILE *fp = NULL;
    UINT32 len = 0;
    UINT32 count = 0;
    char *buf = NULL;
    struct stat stbuf;

    if (stat(UTIL_CLI_TR69_RESULT_FILE, &stbuf) < 0)
    {
        vosLog_error("The file is not exist");
        return;
    }

    len = (unsigned long)stbuf.st_size;
    fp = fopen(UTIL_CLI_TR69_RESULT_FILE, "r");
    if (NULL == fp)
    {
        vosLog_error("open file logTr69 filed");
        return;
    }

    buf = VOS_MALLOC_FLAGS(len + 1, ALLOC_ZEROIZE);
    if (NULL == buf)
    {
        vosLog_error("allocate memory failed");
        fclose(fp);
        return;
    }

    count = fread(buf, len, 1, fp);
    if (count != 1)
    {
        buf[len] = '\0';
        fclose(fp);
        VOS_FREE(buf);
        return;
    }
    else
    {
        vosLog_debug("len =%d", len);
        count = fwrite(buf, len, 1, file);
        if (count != 1)
        {
            vosLog_error("write failed");
        }
    }

    fclose(fp);
    VOS_FREE(buf);
}


VOS_RET_E TR69_processEnableShowLog(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char enableSim[BUFLEN_16] = {0};

    if (util_strcmp(argv[0], "enable"))
    {
        g_writeLog = FALSE;
    }
    else
    {
        g_writeLog = TRUE;
    }

    UTIL_SNPRINTF(enableSim, sizeof(enableSim), "%d", g_writeLog);
    ret = HAL_sysSetTr69cData("tr69c_simulate", enableSim, sizeof(enableSim));
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("save tr69c_simulate failed(ret=%d)", ret);
    }

    vosLog_debug("g_writeLog = %d", g_writeLog);
    return ret;
}


VOS_RET_E TR69_processClearLog(int argc, const char **argv, FILE *file)
{
    FILE *fd = NULL;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if ((fd = fopen(UTIL_CLI_TR69_RESULT_FILE, "w")) != NULL)
    {
        fclose(fd);
    }

    return ret;
}


VOS_RET_E TR69_processShowLog(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    vosLog_debug("show soap");
    tr69_dumpToFile(file);

    return ret;
}


VOS_RET_E TR69_processRemoteGetValue(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char buf[512] = {0};
    ParamItem *p = NULL;

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    simRpcAction->rpcMethod = rpcGetParameterValues;
    UTIL_SNPRINTF(buf, sizeof(buf), "%s.%s", argv[0], argv[1]);

    simRpcAction->ID = VOS_STRDUP(itoa(rand()));
    if (simRpcAction)
    {
        p = (ParamItem *)VOS_MALLOC_FLAGS(sizeof(ParamItem), 0);
        if (p)
        {
            p->pname = p->pvalue = p->pOrigValue = NULL;
            p->next = simRpcAction->ud.pItem;
            simRpcAction->ud.pItem = p;
            p->pname = VOS_STRDUP(buf);
        }
    }

    if (eRPCRunOK != runRPC())
    {
        vosLog_error("failed");
    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteSetValue(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ParamItem *p = NULL;
    char buf[512] = {0};

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    simRpcAction->rpcMethod = rpcSetParameterValues;
    simRpcAction->ID = VOS_STRDUP(itoa(rand()));

    UTIL_SNPRINTF(buf, sizeof(buf), "%s.%s", argv[0], argv[1]);

    if (simRpcAction)
    {
        p = (ParamItem *)VOS_MALLOC_FLAGS(sizeof(ParamItem), 0);
        if (p)
        {
            p->pname = p->pvalue = p->pOrigValue = NULL;
            p->next = simRpcAction->ud.pItem;
            simRpcAction->ud.pItem = p;
            p->pname = VOS_STRDUP(buf);
            p->pvalue = VOS_STRDUP(argv[2]);
        }
        else
        {
            vosLog_error("alloc memory failed");
        }
    }

    if (runRPC() != eRPCRunOK)
    {
        vosLog_error("failed");
    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteAddObj(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    simRpcAction->ID = VOS_STRDUP(itoa(rand()));
    simRpcAction->rpcMethod = rpcAddObject;
    simRpcAction->ud.addDelObjectReq.objectName = VOS_STRDUP(argv[0]);

    if (runRPC() != eRPCRunOK)
    {
        vosLog_error("failed");
    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteDelObj(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char buf[512] = {0};

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    UTIL_SNPRINTF(buf, sizeof(buf), "%s.", argv[0]);
    simRpcAction->ID = VOS_STRDUP(itoa(rand()));
    simRpcAction->rpcMethod = rpcDeleteObject;
    simRpcAction->ud.addDelObjectReq.objectName = VOS_STRDUP(buf);
    if (runRPC() != eRPCRunOK)
    {
        vosLog_error("failed");
    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteReset(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    simRpcAction->rpcMethod = rpcFactoryReset;
    if (runRPC() != eRPCRunOK)
    {
        vosLog_error("failed");
    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteReboot(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    simRpcAction->rpcMethod = rpcReboot;
    if (runRPC() != eRPCRunOK)
    {
        vosLog_error("failed");
    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteGetName(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char buf[512] = {0};

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    UTIL_SNPRINTF(buf, sizeof(buf), "%s.", argv[0]);
    simRpcAction->ID = VOS_STRDUP(itoa(rand()));
    simRpcAction->rpcMethod = rpcGetParameterNames;
    simRpcAction->ud.paramNamesReq.parameterPath = VOS_STRDUP(buf);
    simRpcAction->ud.paramNamesReq.nextLevel = 1;

    if (runRPC() != eRPCRunOK)
    {
        vosLog_error("failed");
    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteGetAttributes(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char buf[512] = {0};
    ParamItem *p = NULL;

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    UTIL_SNPRINTF(buf, sizeof(buf), "%s.%s", argv[0], argv[1]);
    if (simRpcAction)
    {
        simRpcAction->ID = VOS_STRDUP(itoa(rand()));
        simRpcAction->rpcMethod = rpcGetParameterAttributes;
        p = (ParamItem *)VOS_MALLOC_FLAGS(sizeof(ParamItem), 0);
        if (p)
        {
            p->pname = p->pvalue = p->pOrigValue = NULL;
            p->next = simRpcAction->ud.pItem;
            simRpcAction->ud.pItem = p;
            p->pname = VOS_STRDUP(buf);
        }
    }

    if (eRPCRunOK != runRPC())
    {
        vosLog_error("failed");

    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


VOS_RET_E TR69_processRemoteSetAttributes(int argc, const char **argv, FILE *file)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    AttributeItem *p = NULL;
    char buf[512] = {0};

    simRpcAction = tr69_initSimulate();
    if (NULL == simRpcAction)
    {
        vosLog_error("There is a connection or allocate failed, Please try again later");
        return VOS_RET_INTERNAL_ERROR;
    }

    UTIL_SNPRINTF(buf, sizeof(buf), "%s.%s", argv[0], argv[1]);
    if (simRpcAction)
    {
        simRpcAction->rpcMethod = rpcSetParameterAttributes;
        p = (AttributeItem *)VOS_MALLOC_FLAGS(sizeof(AttributeItem), ALLOC_ZEROIZE);
        if (p)
        {
            p->next = simRpcAction->ud.aItem;
            simRpcAction->ud.aItem = p;
            p->pname = VOS_STRDUP(buf);
            p->notification = atoi(argv[2]);
            p->chgNotify = 1;
            p->chgAccess = 0;
            p->subAccess = 0;
        }
    }

    if (eRPCRunOK != runRPC())
    {
        vosLog_error("failed");

    }
    else
    {
        tr69_dumpToFile(file);
    }

    tr69_clearSimulate();
    return ret;
}


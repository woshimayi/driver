#include "fwk.h"
#include "cmc_api.h"
#include "mdm.h"
#include "emdm.h"
#include "custom.h"


void TR69C_buildWlanStatsCustom(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_WLAN_STATISTICS_T wlanStat;
    char paramPath[BUFLEN_128] = {0};
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
 
    memset(&wlanStat, 0, sizeof(wlanStat));
    
    while(VOS_RET_SUCCESS == CMC_wlanGetNextIntfStats(&wlanStat, &iidStack))
    {
        if (wlanStat.showToAcs)
        {
            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN_STATS, &iidStack, paramPath, sizeof(paramPath), "errorsSent");
            TR69C_writePathAndUintValue(paramPath, wlanStat.totalTxErrors, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN_STATS, &iidStack, paramPath, sizeof(paramPath), "errorsReceived");
            TR69C_writePathAndUintValue(paramPath, wlanStat.totalRxErrors, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN_STATS, &iidStack, paramPath, sizeof(paramPath), "discardPacketsSent");
            TR69C_writePathAndUintValue(paramPath, wlanStat.totalTxDrops, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN, &iidStack, paramPath, sizeof(paramPath), "totalBytesReceived");
            TR69C_writePathAndUintValue(paramPath, wlanStat.totalPacketsReceived, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN, &iidStack, paramPath, sizeof(paramPath), "totalBytesSent");
            TR69C_writePathAndUintValue(paramPath, wlanStat.totalBytesSent, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN, &iidStack, paramPath, sizeof(paramPath), "totalPacketsReceived");
            TR69C_writePathAndUintValue(paramPath, wlanStat.totalPacketsReceived, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN, &iidStack, paramPath, sizeof(paramPath), "totalPacketsSent");
            TR69C_writePathAndUintValue(paramPath, wlanStat.totalPacketsSent, pc, bufsz, paramNum);
        }
        
        memset(&wlanStat, 0, sizeof(wlanStat));
    }
}


void TR69C_buildVlanCustom(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    UINT32 vlan = 0;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK; 
    char paramPath[BUFLEN_128] = {0};
    VOS_RET_E  ret = VOS_RET_SUCCESS;

    while (VOS_RET_SUCCESS == CMC_wanGetVlan(&vlan, &iidStack))  
    {
        ret = CMC_phlGetPathByDesc(EMDMOID_WAN_EPON_LINK_CFG, &iidStack, paramPath, sizeof(paramPath), "VLANIDMark");
        if (VOS_RET_SUCCESS != ret)
        {
            vosLog_error("CMC_phlGetPathByDesc failed, ret = %d", ret);
            return;
        }

        TR69C_writePathAndUintValue(paramPath, vlan, pc, bufsz, paramNum);
    } 
}


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <string.h>
#include <syslog.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/un.h> 
#include <dirent.h>

#include "fwk.h"
#include "vos_msg.h"
#include "vos_mem.h"
#include "cmc_api.h"
#include "ctMiddleware.h"
//#define CPEVARNAMEINSTANCE
//#include "../inc/tr69cdefs.h"
//#undef CPEVARNAMEINSTANCE
#include "../inc/appdefs.h"
#include "../inc/utils.h"
#include "../SOAPParser/RPCState.h"
#include "bcmWrapper.h"
#include "event.h"
#include "mdm.h"

//upload, download routine.
extern void downloadStart( void *handle);
extern void uploadStart( void *handle);

//framework & rpc
#if 0
extern TRxObjNode  *findGWParameter(const char *pstr);
extern void saveNotificationAttributes(void);
InstanceDope *findDopeInstance( TRxObjNode *n );
#endif

//event
typedef void (*EventHandler)(void*);


extern int ctmdw_getAllITMSNotifications(void *pc);
extern int ctmdw_getAllMDWNotifications(void *pc);


int g_ctmdwFd = -1;
int enblCTMiddleware = CTMDW_MODE_1;
int enblResetDefault = 0;
int g_twCodeValue = -1;
CTMDW_STATE g_ctmdwState = CTMDW_STATE_NOTINIT;
CTMDW_DOWNLOAD_STATE g_downloadState = CTMDW_DOWNLOAD_STATE_IDLE;
CTMDW_DIAGNOSTIC_STATE g_diagState = CTMDW_DIAGNOSTIC_NONE;
static UBOOL8  ctmdw_saveConfigFlag = FALSE;   /* save config on ctmdw_client_handler */


/**********************************************************************************/
/*********************  CT Middelware client socket wrapper ***********************/
/**********************************************************************************/
static int init_ctmdw_socket(void)
{
    int ctmdw_fd;
    
#ifdef CTMDW_STREAM
    ctmdw_fd = socket(AF_UNIX, SOCK_STREAM, 0);
#else
    ctmdw_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
#endif
    return ctmdw_fd;
}

#ifdef CTMDW_STREAM
static int connect_ctmdw_server(char* server)
{
   int ret = 0;
   struct sockaddr_un ctmdw_svr;

   ctmdw_svr.sun_family = AF_UNIX;
   UTIL_STRNCPY(ctmdw_svr.sun_path, server, sizeof(ctmdw_svr.sun_path));

   if ((ret = connect(g_ctmdwFd, (struct sockaddr *)&ctmdw_svr, 
               sizeof(ctmdw_svr.sun_family) + util_strlen(ctmdw_svr.sun_path))) < 0) {
    CTMDW_DEBUG("ctmdw: connect_ctmdw_server:....failed %d.\n", errno);
   }

   return ret;
}

static int sendto_ctmdw_server(char* buf, int bufLen)
{
   int cnt;
   
   cnt = send(g_ctmdwFd, buf, bufLen,0);
   CTMDW_DEBUG("ctmdw: sendto_ctmdw_server: send....%d\n", cnt);
   return cnt;
}
#else
static int sendto_ctmdw_server(char* buf, int bufLen)
{
   int cnt;
   struct sockaddr_un ctmdw_svr;

   ctmdw_svr.sun_family = AF_UNIX;
   UTIL_STRNCPY(ctmdw_svr.sun_path, CTMDW_UNIX_SOCK, sizeof(ctmdw_svr.sun_path));

   cnt = sendto(g_ctmdwFd, buf, bufLen, 0, &ctmdw_svr, 
                sizeof(ctmdw_svr.sun_family) + util_strlen(ctmdw_svr.sun_path));
   CTMDW_DEBUG("ctmdw: sendto_ctmdw_server: send....%d\n", cnt);
   return cnt;
}
#endif

static int recvfrom_ctmdw_server(char* buf, int bufLen)
{
   int cnt;
   
   cnt = recv(g_ctmdwFd, buf, bufLen, 0);
   CTMDW_DEBUG("ctmdw: recvfrom_ctmdw_server: recv....%d\n", cnt);
   return cnt;
}

/* save Configuration routine */
void ctmdw_saveConfigurations(void)
{
   if (ctmdw_saveConfigFlag)
   {
      wrapperSaveConfigurations();
      ctmdw_saveConfigFlag = FALSE;
   }
}

/**********************************************************************************/
/*********************  CT Middelware client object parser  ***********************/
/**********************************************************************************/
unsigned int calcStrElmtNum(char* elmtsStr)
{
    int i = 0;
    char *r;
    int numOfElmts = 0;
    int elmtsStrLen = 0;

    if(elmtsStr == NULL)
    {
        return numOfElmts;
    }

    elmtsStrLen = util_strlen(elmtsStr);

    if(elmtsStrLen > 0)
    {
        numOfElmts = 1;
        r = elmtsStr;
        for(i = 0; i < elmtsStrLen; i++)
        {
            if (*r == '&')
                numOfElmts++;

            r++;
        }
    }
    
    return numOfElmts++;
}


static CTMDW_STATUS validateOptCode(int optCode)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   if(optCode < CTMDW_OPTCODE_REGISTER || optCode > CTMDW_OPTCODE_SETDEFAULT_RET)
   {
      ret = CTMDW_STATUS_ERR_FMT;
   }
   return ret;
}

static CTMDW_STATUS validateType(int type)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   if(type != CTMDW_TYPE_MODNAME && type != CTMDW_TYPE_PARAMETERNAMES &&
      type != CTMDW_TYPE_PARAVALUES && type != CTMDW_TYPE_RETCODE &&
      type != CTMDW_TYPE_PARAATTRIBUTES && type != CTMDW_TYPE_TW && 
      type != CTMDW_TYPE_INSTANCE && type != CTMDW_TYPE_OPERATION &&
      type != CTMDW_TYPE_OBJECT && type != CTMDW_TYPE_PARALIST &&
      type != CTMDW_TYPE_OPTION )
   {
      ret = CTMDW_STATUS_ERR_FMT;
   }
   
   return ret;
}

char mappingPrefix[] = "InternetGatewayDevice.";

MAPCTTOTR69TABLE mappingTableCtNameToTr69L1[] = {
   {"WAN",         "WANDevice.1.WANConnectionDevice."},
   {"WLAN",        "LANDevice.1.WLANConfiguration."},
   {"USB",         "LANDevice.1.LANUSBInterfaceConfig."},
   {"LAN",         "LANDevice."},
   {"Alarm",       "DeviceInfo.X_CT-COM_Alarm"},
   {"Algability",       "DeviceInfo.X_CT-COM_Algability"},
   {"Monitor",     "DeviceInfo.X_CT-COM_Monitor"},
   {"RestoreEnable",      "DeviceInfo.X_CT-COM_Restore.Enable"},
   {"Syslog","DeviceInfo.X_CT-COM_Syslog"},
   {"CTAccount",   "DeviceInfo.X_CT-COM_TeleComAccount"},
   {"Portal",      "DeviceInfo.X_CT-COM_PortalManagement"},
   {"ALG",         "DeviceInfo.X_CT-COM_ALGAbility"},
   {"MWBAND",      "Services.X_CT-COM_MWBAND"},
   {"ServiceManage","DeviceInfo.X_CT-COM_ServiceManage"},
   {"IGMP.Enable",      "Services.X_CT-COM_IPTV.IGMPEnable"},
   {"IGMP.STBNumber",        "Services.X_CT-COM_IPTV.STBNumber"},
   {"ITMSURL",     "ManagementServer.URL"},
   {"MWSURL",      "DeviceInfo.X_CT-COM_MiddlewareMgt.MiddlewareURL"},
   {"CTMgtIPAddress", "ManagementServer.CTMgtIPAddress"},
   {"MgtDNS",      "ManagementServer.MgtDNS"},
   {"ReConnectEnable", "DeviceInfo.X_CT-COM_ReConnect.Enable"},
   {"CTUserIPAddress1","ManagementServer.CTUserIPAddress.1.CTUserIPAddress"},
   {"CTUserIPAddress2","ManagementServer.CTUserIPAddress.2.CTUserIPAddress"},
   {"CTUserIPAddress3","ManagementServer.CTUserIPAddress.3.CTUserIPAddress"},
   {"CTUserIPAddress4","ManagementServer.CTUserIPAddress.4.CTUserIPAddress"},
   {"CTUserIPAddress5","ManagementServer.CTUserIPAddress.5.CTUserIPAddress"},
   {"CTUserIPAddress6","ManagementServer.CTUserIPAddress.6.CTUserIPAddress"},
   {"CTUserIPAddress7","ManagementServer.CTUserIPAddress.7.CTUserIPAddress"},
   {"CTUserIPAddress8","ManagementServer.CTUserIPAddress.8.CTUserIPAddress"},
   {"InternetPvc", "ManagementServer.InternetPvc"}
};
int numOfL1MTableEntries = sizeof(mappingTableCtNameToTr69L1)/sizeof(MAPCTTOTR69TABLE);

MAPCTTOTR69TABLE mappingTableCtNameToTr69L2[] = {
   {"PPP",         "WANPPPConnection."},
   {"DSLConfig",   "WANDSLLinkConfig"},
   {"SSIDHide",   "X_CT-COM_SSIDHide"},
   {"Powerlevel",   "X_CT-COM_Powerlevel"},
   {"PowerValue",   "X_CT-COM_PowerValue"},
   {"APModuleEnable",   "X_CT-COM_APModuleEnable"},
   {"AssociatedDevice",  "AssociatedDevice."}, 
   {"WEPKey",  "WEPKey."},
   {"PreSharedKey",  "PreSharedKey."},
   {"LANHostConfigManagement.STBMinAddr",       "LANHostConfigManagement.X_CT-COM_STB-MinAddress"},
   {"LANHostConfigManagement.STBMaxAddr",       "LANHostConfigManagement.X_CT-COM_STB-MaxAddress"},
   {"LANHostConfigManagement.PhoneMinAddr",     "LANHostConfigManagement.X_CT-COM_Phone-MinAddress"},
   {"LANHostConfigManagement.PhoneMaxAddr",     "LANHostConfigManagement.X_CT-COM_Phone-MaxAddress"},
   {"LANHostConfigManagement.CameraMinAddr",    "LANHostConfigManagement.X_CT-COM_Camera-MinAddress"},
   {"LANHostConfigManagement.CameraMaxAddr",    "LANHostConfigManagement.X_CT-COM_Camera-MaxAddress"},
   {"LANHostConfigManagement.ComputerMinAddr",  "LANHostConfigManagement.X_CT-COM_Computer-MinAddress"},
   {"LANHostConfigManagement.ComputerMaxAddr",  "LANHostConfigManagement.X_CT-COM_Computer-MaxAddress"},
};
int numOfL2MTableEntries = sizeof(mappingTableCtNameToTr69L2)/sizeof(MAPCTTOTR69TABLE);

MAPCTTOTR69TABLE mappingTableCtNameToTr69L3[] = {
   {"PortMapping",      "PortMapping."},
   {"LanInterface",     "X_CT-COM_LanInterface"},
   {"ServiceList",      "X_CT-COM_ServiceList"},
   {"ProxyEnable",      "X_CT-COM_ProxyEnable"},
   {"ProxyMaxUser",     "X_CT-COM_MAXUser"},
   {"SSIDHide",         "X_CT-COM_SSIDHide"},
   {"Powerlevel",       "X_CT-COM_Powerlevel"},
   {"PowerValue",       "X_CT-COM_PowerValue"},
   {"APModuleEnable",   "X_CT-COM_APModuleEnable"},
};

int numOfL3MTableEntries = sizeof(mappingTableCtNameToTr69L3)/sizeof(MAPCTTOTR69TABLE);

MAPCTTOTR69TABLE mappingTableCtNameToTr69L4[] = {
   {"Enabled",         "PortMappingEnabled"},
   {"LeaseDuration",   "PortMappingLeaseDuration"},
   {"Protocol",        "PortMappingProtocol"},
   {"Description",     "PortMappingDescription"},
};

int numOfL4MTableEntries = sizeof(mappingTableCtNameToTr69L4)/sizeof(MAPCTTOTR69TABLE);

static CTMDW_STATUS mappingCTNameToTR69Name(char* pPName, char* paraNameMapped, UINT32 paraNamelen)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   char *pStr = NULL, *pStr2 = NULL, *pStr3 = NULL, *pStr4= NULL;
   int paraMappedOffset = 0;
   int i = 0, len = 0;
   char bL1Matched = FALSE;
   char bL2Matched = FALSE;
   char bL3Matched = FALSE;
   if(pPName == NULL || paraNameMapped == NULL)
   {
      return ret = CTMDW_STATUS_ERR_PARAM;
   }

   //Copy "InternetGatewayDevice."
   UTIL_STRNCPY(paraNameMapped, mappingPrefix, paraNamelen);
   paraMappedOffset += util_strlen(mappingPrefix);

   //Compare first Level
   for(i = 0; i < numOfL1MTableEntries; i++)
   {

      len = util_strlen(mappingTableCtNameToTr69L1[i].ctName);
      if (!strncmp(pPName, mappingTableCtNameToTr69L1[i].ctName, len))
      {
         bL1Matched = TRUE;
         UTIL_STRNCAT(paraNameMapped, mappingTableCtNameToTr69L1[i].tr69Name, paraNamelen); 
         pStr = pPName + len;
         paraMappedOffset += util_strlen(mappingTableCtNameToTr69L1[i].tr69Name);
         
         break;
      }
   }

   CTMDW_DEBUG("ctmdw: mappingCTNameToTR69Name L1 %d %s %d\n", bL1Matched, paraNameMapped, paraMappedOffset);

   if (!bL1Matched)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for(i = 0; i < numOfL2MTableEntries; i++)
   {
      /* For WEPKeyIndex ºÍWEPKey.{i}. ³åÍ»ÎÊÌâ¡£houweilin 20100305*/
      if(NULL != strstr(pStr,"WEPKeyIndex"))
         break;
      if ((pStr2 = strstr(pStr, mappingTableCtNameToTr69L2[i].ctName)) != NULL)
      {
         CTMDW_DEBUG("ctmdw: mappingCTNameToTR69Name L2 %s\n", pStr2);
         bL2Matched = TRUE;
         memcpy(paraNameMapped + paraMappedOffset, pStr, pStr2-pStr);
         paraMappedOffset += pStr2 - pStr;
         paraNameMapped[paraMappedOffset] = 0;

         UTIL_STRNCAT(paraNameMapped, mappingTableCtNameToTr69L2[i].tr69Name, paraNamelen);
         paraMappedOffset += util_strlen(mappingTableCtNameToTr69L2[i].tr69Name);
         pStr = pStr2 + util_strlen(mappingTableCtNameToTr69L2[i].ctName);
         break;
      }
   }

   if(bL2Matched)
   {
      for(i = 0; i < numOfL3MTableEntries; i++)
      {
         if ((pStr3 = strstr(pStr, mappingTableCtNameToTr69L3[i].ctName)) != NULL)
         {
            CTMDW_DEBUG("ctmdw: mappingCTNameToTR69Name L3 %s\n", pStr3);
            bL3Matched = TRUE;
            memcpy(paraNameMapped + paraMappedOffset, pStr, pStr3-pStr);
            paraMappedOffset += pStr3 - pStr;
            paraNameMapped[paraMappedOffset] = 0;

            UTIL_STRNCAT(paraNameMapped, mappingTableCtNameToTr69L3[i].tr69Name, paraNamelen);
            paraMappedOffset += util_strlen(mappingTableCtNameToTr69L3[i].tr69Name);
            pStr = pStr3 + util_strlen(mappingTableCtNameToTr69L3[i].ctName);
            break;
         }
      }

      if(bL3Matched)
      {
         for(i = 0; i < numOfL4MTableEntries; i++)
         {
            if ((pStr4 = strstr(pStr, mappingTableCtNameToTr69L4[i].ctName)) != NULL)
            {
               CTMDW_DEBUG("ctmdw: mappingCTNameToTR69Name L4 %s\n", pStr4);
               memcpy(paraNameMapped + paraMappedOffset, pStr, pStr4-pStr);
               paraMappedOffset += pStr4 - pStr;
               paraNameMapped[paraMappedOffset] = 0;

               UTIL_STRNCAT(paraNameMapped, mappingTableCtNameToTr69L4[i].tr69Name, paraNamelen);
               paraMappedOffset += util_strlen(mappingTableCtNameToTr69L4[i].tr69Name);
               pStr = pStr4 + util_strlen(mappingTableCtNameToTr69L4[i].ctName);
               break;
            }
         }
      }
   }

   UTIL_STRNCAT(paraNameMapped, pStr, paraNamelen); 
   CTMDW_DEBUG("ctmdw: mappingCTNameToTR69Name %s\n", paraNameMapped);
   
   return ret;
}

CTMDW_STATUS mappingTR69NameToCTName(char* paraNameMapped, char* pPName, UINT32 nameLen)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           *pStr = NULL, *pStr2 = NULL, *pStr3 = NULL, *pStr4 = NULL;
   char           *pParaMName = NULL;
   int            paraPNameOffset = 0;
   int            i = 0, len = 0;
   char           bL1Matched = FALSE;
   char           bL2Matched = FALSE;
   char           bL3Matched = FALSE;
   
   if(pPName == NULL || paraNameMapped == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   //skip "InternetGatewayDevice."
   pParaMName = paraNameMapped + util_strlen(mappingPrefix);

   //Compare first Level
   for(i = 0; i < numOfL1MTableEntries; i++)
   {

      len = util_strlen(mappingTableCtNameToTr69L1[i].tr69Name);
      if (!strncmp(pParaMName, mappingTableCtNameToTr69L1[i].tr69Name, len))
      {
         bL1Matched = TRUE;
         UTIL_STRNCAT(pPName, mappingTableCtNameToTr69L1[i].ctName, nameLen); 
         pStr = pParaMName + len;
         paraPNameOffset += util_strlen(mappingTableCtNameToTr69L1[i].ctName);
         
         break;
      }
   }

   CTMDW_DEBUG("ctmdw: mappingTR69NameToCTName L1 %d %s %d\n", bL1Matched, pPName, paraPNameOffset);

   if (!bL1Matched)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for(i = 0; i < numOfL2MTableEntries; i++)
   {
      if ((pStr2 = strstr(pStr, mappingTableCtNameToTr69L2[i].tr69Name)) != NULL)
      {
         CTMDW_DEBUG("ctmdw: mappingTR69NameToCTName %s\n", pStr2);
         bL2Matched = TRUE;
         memcpy(pPName + paraPNameOffset, pStr, pStr2-pStr);
         paraPNameOffset += pStr2 - pStr;
         pPName[paraPNameOffset] = 0;

         UTIL_STRNCAT(pPName, mappingTableCtNameToTr69L2[i].ctName, nameLen);
         paraPNameOffset += util_strlen(mappingTableCtNameToTr69L2[i].ctName);
         pStr = pStr2 + util_strlen(mappingTableCtNameToTr69L2[i].tr69Name);
         break;
      }
   }

   if(bL2Matched)
   {
      for(i = 0; i < numOfL3MTableEntries; i++)
      {
         if ((pStr3 = strstr(pStr, mappingTableCtNameToTr69L3[i].tr69Name)) != NULL)
         {
            bL3Matched = TRUE;
            CTMDW_DEBUG("ctmdw: mappingTR69NameToCTName %s\n", pStr3);
            memcpy(pPName + paraPNameOffset, pStr, pStr3-pStr);
            paraPNameOffset += pStr3 - pStr;
            pPName[paraPNameOffset] = 0;
   
            UTIL_STRNCAT(pPName, mappingTableCtNameToTr69L3[i].ctName, nameLen);
            paraPNameOffset += util_strlen(mappingTableCtNameToTr69L3[i].ctName);
            pStr = pStr3 + util_strlen(mappingTableCtNameToTr69L3[i].tr69Name);
            break;
         }
      }

      if(bL3Matched)
      {
         for(i = 0; i < numOfL4MTableEntries; i++)
         {
            if ((pStr4 = strstr(pStr, mappingTableCtNameToTr69L4[i].tr69Name)) != NULL)
            {
               CTMDW_DEBUG("ctmdw: mappingTR69NameToCTName %s\n", pStr4);
               memcpy(pPName + paraPNameOffset, pStr, pStr4-pStr);
               paraPNameOffset += pStr4 - pStr;
               pPName[paraPNameOffset] = 0;
      
               UTIL_STRNCAT(pPName, mappingTableCtNameToTr69L4[i].ctName, nameLen);
               paraPNameOffset += util_strlen(mappingTableCtNameToTr69L4[i].ctName);
               pStr = pStr4 + util_strlen(mappingTableCtNameToTr69L4[i].tr69Name);
               break;
            }
         }        
      }
   }

   UTIL_STRNCAT(pPName, pStr, nameLen); 
   CTMDW_DEBUG("ctmdw: mappingTR69NameToCTName %s\n", pPName);
   
   return ret;
}

static CTMDW_STATUS parseCTMDWParaNames(PCTMDW_TLV tlv)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           origVal[CTMDW_PACKET_MAXLEN];
   char           paraNameMapped[CTMDW_PARAM_MAXLEN];
   char           *pPName;
   char           *pToken;
   int            numOfParam = 0;
   int            i;

   if(tlv == NULL || tlv->length == 0 || tlv->value == NULL ||
      tlv->type != CTMDW_TYPE_PARAMETERNAMES) 
      return (ret = CTMDW_STATUS_ERR_FMT);
   
   CTMDW_DEBUG("ctmdw: parseCTMDWParaNames param...ok!\n");

   strncpy(origVal, tlv->value, tlv->length);
   origVal[tlv->length] = '\0';
   numOfParam = (int)calcStrElmtNum(origVal);
   tlv->numofVal = numOfParam;
   
   tlv->valLst = (PCTMDW_VAL)VOS_MALLOC(sizeof(CTMDW_VAL) * numOfParam);
   if(tlv->valLst == NULL)
      return (ret = CTMDW_STATUS_ERR_NOMEM);
   else
      memset(tlv->valLst, 0, sizeof(CTMDW_VAL) * numOfParam);

   CTMDW_DEBUG("ctmdw: parseCTMDWParaNames valLst %p, numOfParam %d origVal %s!\n", 
                tlv->valLst, numOfParam, origVal);

   pPName = pToken = origVal;
   for(i = 0; i < numOfParam; i++)
   {
      //token: param
      pToken = strtok(pToken, CTMDW_PARAM_DELIM);
      CTMDW_DEBUG("ctmdw: parseCTMDWParaNames step1 %p\n", pToken);
      if(pToken != NULL)
      {
         pPName = pToken;
         pToken += util_strlen(pToken) + 1;
      }
      
      ret = mappingCTNameToTR69Name(pPName, paraNameMapped, sizeof(paraNameMapped));
      CTMDW_DEBUG("ctmdw: parseCTMDWParaNames get param...%s %s!\n", pPName, paraNameMapped);
      if(ret != CTMDW_STATUS_OK)
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(pPName) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }

         UTIL_STRNCPY(tlv->valLst[i].name, pPName, util_strlen(pPName) + 1);
      }
      else
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(paraNameMapped) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }

         UTIL_STRNCPY(tlv->valLst[i].name, paraNameMapped, util_strlen(paraNameMapped) + 1);
         
      }

      CTMDW_DEBUG("ctmdw: parseCTMDWParaNames value[%d].name = %s!\n", i, tlv->valLst[i].name);

      ret = CTMDW_STATUS_OK;      
   }
   
   return ret;
}

static CTMDW_STATUS parseCTMDWParaNamesValues(PCTMDW_TLV tlv)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           origVal[CTMDW_PACKET_MAXLEN];
   char           paraNameMapped[CTMDW_PARAM_MAXLEN];
   char           *pPName = NULL;
   char           *pValue = NULL;
   char           *pToken = NULL;
   int            numOfParam = 0;
   int            i;

   if(tlv == NULL || tlv->length == 0 || tlv->value == NULL ||
      tlv->type != CTMDW_TYPE_PARAVALUES) 
      return (ret = CTMDW_STATUS_ERR_FMT);
   
   CTMDW_DEBUG("ctmdw: parseCTMDWParaNamesValues param...ok!\n");

   strncpy(origVal, tlv->value, tlv->length);
   origVal[tlv->length] = '\0';
   numOfParam = (int)calcStrElmtNum(origVal);
   tlv->numofVal = numOfParam;
   
   tlv->valLst = (PCTMDW_VAL)VOS_MALLOC(sizeof(CTMDW_VAL) * numOfParam);
   if(tlv->valLst == NULL)
      return (ret = CTMDW_STATUS_ERR_NOMEM);
   else
      memset(tlv->valLst, 0, sizeof(CTMDW_VAL) * numOfParam);

   CTMDW_DEBUG("ctmdw: parseCTMDWParaNamesValues valLst %p, numOfParam %d origVal %s!\n", 
                tlv->valLst, numOfParam, origVal);

   pPName = pToken = origVal;
   for(i = 0; i < numOfParam; i++)
   {
      //token: param=value
      pToken = strtok(pToken, CTMDW_PARAM_DELIM);
      CTMDW_DEBUG("ctmdw: parseCTMDWParaNamesValues %p\n", pToken);
      if(pToken != NULL)
      {
         if(strstr(pToken, CTMDW_VALUE_DELIM) == NULL)
         {
            //No value
            tlv->actRetVal = CTMDW_ACTION_ERR_FMT;
            tlv->valLst[i].actRetVal = CTMDW_ACTION_ERR_FMT;
            break;
         }
         pToken = strtok(pToken, CTMDW_VALUE_DELIM);
         pPName = pToken;
         pToken += util_strlen(pToken) + 1;
         pValue = pToken;
         pToken += util_strlen(pToken) + 1;
      }

      ret = mappingCTNameToTR69Name(pPName, paraNameMapped, sizeof(paraNameMapped));
      CTMDW_DEBUG("ctmdw: parseCTMDWParaNamesValues get param... ret %d %s %s %s!\n", ret, pPName, pValue, paraNameMapped);

      if(ret != CTMDW_STATUS_OK)
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(pPName) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }
         UTIL_STRNCPY(tlv->valLst[i].name, pPName, util_strlen(pPName) + 1);
      }
      else
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(paraNameMapped) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }
         UTIL_STRNCPY(tlv->valLst[i].name, paraNameMapped, util_strlen(paraNameMapped) + 1);
      }

      tlv->valLst[i].value = VOS_MALLOC(util_strlen(pValue) + 1);
      if(tlv->valLst[i].value == NULL)
      {
         tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
         tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
         ret = CTMDW_STATUS_ERR_NOMEM;
         break;
      }
      UTIL_STRNCPY(tlv->valLst[i].value, pValue, util_strlen(pValue) + 1);

      ret = CTMDW_STATUS_OK; 
      
   }
   
   return ret;
}

static CTMDW_STATUS parseCTMDWParaAttrValues(PCTMDW_TLV tlv, char* twType)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           origVal[CTMDW_PACKET_MAXLEN];
   char           paraNameMapped[CTMDW_PARAM_MAXLEN];
   char           *pPName = NULL;
   char           *pValue = NULL;
   char           *pToken = NULL;
   int            numOfParam = 0;
   int            i;

   if(tlv == NULL || tlv->length == 0 || tlv->value == NULL ||
      tlv->type != CTMDW_TYPE_PARAATTRIBUTES) 
      return (ret = CTMDW_STATUS_ERR_FMT);
   
   CTMDW_DEBUG("ctmdw: parseCTMDWParaAttrValues param...ok!\n");

   strncpy(origVal, tlv->value, tlv->length);
   origVal[tlv->length] = '\0';
   numOfParam = calcStrElmtNum(origVal);
   tlv->numofVal = numOfParam;
   
   tlv->valLst = (PCTMDW_VAL)VOS_MALLOC(sizeof(CTMDW_VAL) * numOfParam);
   if(tlv->valLst == NULL)
      return (ret = CTMDW_STATUS_ERR_NOMEM);
   else
      memset(tlv->valLst, 0, sizeof(CTMDW_VAL) * numOfParam);

   CTMDW_DEBUG("ctmdw: parseCTMDWParaAttrValues valLst %p, numOfParam %d origVal %s!\n", 
                tlv->valLst, numOfParam, origVal);

   pPName = pToken = origVal;
   for(i = 0; i < numOfParam; i++)
   {
      //token: param=value
      pToken = strtok(pToken, CTMDW_PARAM_DELIM);
      CTMDW_DEBUG("ctmdw: parseCTMDWParaAttrValues %p\n", pToken);
      if(pToken != NULL)
      {
         if(strstr(pToken, CTMDW_VALUE_DELIM) == NULL)
         {
            //No value
            tlv->actRetVal = CTMDW_ACTION_ERR_FMT;
            tlv->valLst[i].actRetVal = CTMDW_ACTION_ERR_FMT;
            break;
         }
         pToken = strtok(pToken, CTMDW_VALUE_DELIM);
         pPName = pToken;
         pToken += util_strlen(pToken) + 1;
         pValue = pToken;
         pToken += util_strlen(pToken) + 1;
      }

      if ((!strncmp(twType, CTMDW_TW_MDW, util_strlen(CTMDW_TW_MDW)) && strcmp(pValue, CTMDW_ATTRIBUTES_MDW_NOINFORM) && strcmp(pValue, CTMDW_ATTRIBUTES_MDW_INFORM)) ||
         (!strncmp(twType, CTMDW_TW_TR69, util_strlen(CTMDW_TW_TR69)) && strcmp(pValue, CTMDW_ATTRIBUTES_ITMS_NOINFORM_READABLE) && strcmp(pValue, CTMDW_ATTRIBUTES_ITMS_REBOOTINFORM_READABLE) && strcmp(pValue, CTMDW_ATTRIBUTES_ITMS_INFORM_READABLE) && 
          strcmp(pValue, CTMDW_ATTRIBUTES_ITMS_NOINFORM_WRITABLE) && strcmp(pValue, CTMDW_ATTRIBUTES_ITMS_REBOOTINFORM_WRITABLE) && strcmp(pValue, CTMDW_ATTRIBUTES_ITMS_INFORM_WRITABLE) ))
         return ret = CTMDW_STATUS_ERR_PARAM;
         

      ret = mappingCTNameToTR69Name(pPName, paraNameMapped, sizeof(paraNameMapped));
      CTMDW_DEBUG("ctmdw: parseCTMDWParaAttrValues get param... %s %s %s %s!\n", twType, pPName, pValue, paraNameMapped);

      if(ret != CTMDW_STATUS_OK)
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(pPName) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }
         UTIL_STRNCPY(tlv->valLst[i].name, pPName, util_strlen(pPName) + 1);
      }
      else
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(paraNameMapped) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }
         UTIL_STRNCPY(tlv->valLst[i].name, paraNameMapped, util_strlen(paraNameMapped) + 1);
      }

      tlv->valLst[i].value = VOS_MALLOC(util_strlen(pValue) + 1);
      if(tlv->valLst[i].value == NULL)
      {
         tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
         tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
         ret = CTMDW_STATUS_ERR_NOMEM;
         break;
      }
      
      UTIL_STRNCPY(tlv->valLst[i].value, pValue, util_strlen(pValue) + 1);
      ret = CTMDW_STATUS_OK;
   }
   
   return ret;
}

static CTMDW_STATUS parseCTMDWParaTW(PCTMDW_TLV tlv)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;

   if(tlv == NULL || tlv->length == 0 || tlv->value == NULL ||
      tlv->type != CTMDW_TYPE_TW) 
      return (ret = CTMDW_STATUS_ERR_FMT);
   
   CTMDW_DEBUG("ctmdw: parseCTMDWParaTW param...ok!\n");

   if(strncmp(tlv->value, CTMDW_TW_WEBGUI, util_strlen(CTMDW_TW_WEBGUI)) && 
      strncmp(tlv->value, CTMDW_TW_TR69, util_strlen(CTMDW_TW_TR69)) && 
      strncmp(tlv->value, CTMDW_TW_MDW, util_strlen(CTMDW_TW_MDW)))
      ret = CTMDW_STATUS_ERR_FMT;
   
   return ret;
}

static CTMDW_STATUS parseCTMDWObject(PCTMDW_TLV tlv)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           origVal[CTMDW_PACKET_MAXLEN];
   char           paraNameMapped[CTMDW_PARAM_MAXLEN];
   char           *pPName;
   char           *pToken;
   int            numOfParam = 0;
   int            i;
   
   if(tlv == NULL || tlv->length == 0 || tlv->value == NULL ||
      tlv->type != CTMDW_TYPE_OBJECT) 
      return (ret = CTMDW_STATUS_ERR_FMT);
   
   CTMDW_DEBUG("ctmdw: parseCTMDWObject param...ok!\n");

   strncpy(origVal, tlv->value, tlv->length);
   origVal[tlv->length] = '\0';
   numOfParam = calcStrElmtNum(origVal);
   tlv->numofVal = numOfParam;
   
   tlv->valLst = (PCTMDW_VAL)VOS_MALLOC(sizeof(CTMDW_VAL) * numOfParam);
   if(tlv->valLst == NULL)
      return (ret = CTMDW_STATUS_ERR_NOMEM);
   else
      memset(tlv->valLst, 0, sizeof(CTMDW_VAL) * numOfParam);

   CTMDW_DEBUG("ctmdw: parseCTMDWObject valLst %p, numOfParam %d origVal %s!\n", 
                tlv->valLst, numOfParam, origVal);

   pPName = pToken = origVal;
   for(i = 0; i < numOfParam; i++)
   {
      //token: param
      pToken = strtok(pToken, CTMDW_PARAM_DELIM);
      CTMDW_DEBUG("ctmdw: parseCTMDWObject step1 %p\n", pToken);
      if(pToken != NULL)
      {
         pPName = pToken;
         pToken += util_strlen(pToken) + 1;
      }
      
      ret = mappingCTNameToTR69Name(pPName, paraNameMapped, sizeof(paraNameMapped));
      CTMDW_DEBUG("ctmdw: parseCTMDWObject get param...%s %s!ret = %d\n", pPName, paraNameMapped, ret);
      if(ret != CTMDW_STATUS_OK)
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(pPName) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }

         UTIL_STRNCPY(tlv->valLst[i].name, pPName, util_strlen(pPName) + 1);    
      }
      else
      {
         tlv->valLst[i].name = VOS_MALLOC(util_strlen(paraNameMapped) + 1);
         if(tlv->valLst[i].name == NULL)
         {
            tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
            tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
            ret = CTMDW_STATUS_ERR_NOMEM;
            break;
         }

         UTIL_STRNCPY(tlv->valLst[i].name, paraNameMapped, util_strlen(paraNameMapped) + 1);
         
      }

      ret = CTMDW_STATUS_OK;      
   }
   
   return ret;
}

static CTMDW_STATUS parseCTMDWDownload(PCTMDW_TLV tlv)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           origVal[CTMDW_PACKET_MAXLEN];
   char           *pPName = NULL;
   char           *pValue = NULL;
   char           *pToken = NULL;
   int            numOfParam = 0;
   int            i;

   if(tlv == NULL || tlv->length == 0 || tlv->value == NULL ||
      tlv->type != CTMDW_TYPE_PARAVALUES) 
      return (ret = CTMDW_STATUS_ERR_FMT);
   
   CTMDW_DEBUG("ctmdw: parseCTMDWDownload param...ok!\n");

   strncpy(origVal, tlv->value, tlv->length);
   origVal[tlv->length] = '\0';
   numOfParam = calcStrElmtNum(origVal);
   tlv->numofVal = numOfParam;
   
   tlv->valLst = (PCTMDW_VAL)VOS_MALLOC(sizeof(CTMDW_VAL) * numOfParam);
   if(tlv->valLst == NULL)
      return (ret = CTMDW_STATUS_ERR_NOMEM);
   else
      memset(tlv->valLst, 0, sizeof(CTMDW_VAL) * numOfParam);

   CTMDW_DEBUG("ctmdw: parseCTMDWDownload valLst %p, numOfParam %d origVal %s!\n", 
                tlv->valLst, numOfParam, origVal);

   pPName = pToken = origVal;
   for(i = 0; i < numOfParam; i++)
   {
      //token: param=value
      pToken = strtok(pToken, CTMDW_PARAM_DELIM);
      CTMDW_DEBUG("ctmdw: parseCTMDWParaAttrValues %p\n", pToken);
      if(pToken != NULL)
      {
         if(strstr(pToken, CTMDW_VALUE_DELIM) == NULL)
         {
            //No value
            tlv->actRetVal = CTMDW_ACTION_ERR_FMT;
            tlv->valLst[i].actRetVal = CTMDW_ACTION_ERR_FMT;
            break;
         }
         pToken = strtok(pToken, CTMDW_VALUE_DELIM);
         pPName = pToken;
         pToken += util_strlen(pToken) + 1;
         pValue = pToken;
         pToken += util_strlen(pToken) + 1;
      }

      if(strcmp(pPName, CTMDW_DOWNLOAD_COMMANDKEY) && strcmp(pPName, CTMDW_DOWNLOAD_DELAYSECONDS) &&
         strcmp(pPName, CTMDW_DOWNLOAD_FAILUREURL) && strcmp(pPName, CTMDW_DOWNLOAD_FILESIZE) &&
         strcmp(pPName, CTMDW_DOWNLOAD_FILETYPE) && strcmp(pPName, CTMDW_DOWNLOAD_PASSWORD) &&
         strcmp(pPName, CTMDW_DOWNLOAD_SUCCESSURL) && strcmp(pPName, CTMDW_DOWNLOAD_TARGETFILENAME)&&
         strcmp(pPName, CTMDW_DOWNLOAD_URL) && strcmp(pPName, CTMDW_DOWNLOAD_USERNAME))
         return ret = CTMDW_STATUS_ERR_PARAM;
         
      CTMDW_DEBUG("ctmdw: parseCTMDWDownload get param...%s %s \n", pPName, pValue);

      if(ret != CTMDW_STATUS_OK)
      {
         tlv->actRetVal = CTMDW_ACTION_ERR_FMT;
         tlv->valLst[i].actRetVal = CTMDW_ACTION_ERR_FMT;
         break;
      }
      
      tlv->valLst[i].name = VOS_MALLOC(util_strlen(pPName) + 1);
      if(tlv->valLst[i].name == NULL)
      {
         tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
         tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
         ret = CTMDW_STATUS_ERR_NOMEM;
         break;
      }

      tlv->valLst[i].value = VOS_MALLOC(util_strlen(pValue) + 1);
      if(tlv->valLst[i].value == NULL)
      {
         tlv->actRetVal = CTMDW_STATUS_ERR_NOMEM;
         tlv->valLst[i].actRetVal = CTMDW_STATUS_ERR_NOMEM;
         ret = CTMDW_STATUS_ERR_NOMEM;
         break;
      }

      UTIL_STRNCPY(tlv->valLst[i].name, pPName, util_strlen(pPName) + 1);
      UTIL_STRNCPY(tlv->valLst[i].value, pValue, util_strlen(pValue) + 1);
   }
   
   return ret;
}

static CTMDW_STATUS parseCTMDWParaRetCode(PCTMDW_TLV tlv)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;

   if(tlv == NULL || tlv->length == 0 || tlv->value == NULL ||
      tlv->type != CTMDW_TYPE_RETCODE) 
      return (ret = CTMDW_STATUS_ERR_FMT);
   
   CTMDW_DEBUG("ctmdw: parseCTMDWParaRetCode param...ok!\n");

   if(strncmp(tlv->value, CTMDW_RETCODE_OK, util_strlen(CTMDW_RETCODE_OK)) && 
      strncmp(tlv->value, CTMDW_RETCODE_ERR, util_strlen(CTMDW_RETCODE_ERR)))
      ret = CTMDW_STATUS_ERR_FMT;
   
   return ret;
}

static CTMDW_STATUS parseCTMDWMsgAdvance(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_ERR_FMT;
   int            i;

   CTMDW_DEBUG("ctmdw: parseCTMDWMsgAdvance opcode %d\n", ctmdw_msg->opcode);

   switch(ctmdw_msg->opcode)
   {  
      // zero TLV
      case CTMDW_OPTCODE_FILEGET:
      case CTMDW_OPTCODE_REBOOT:
      case CTMDW_OPTCODE_SETDEFAULT:
         if(ctmdw_msg->numOfTLV == 0)
            ret = CTMDW_STATUS_OK;
         break;
      // one TLV with type 7(retcode)
      case CTMDW_OPTCODE_REGISTER_OK:
      case CTMDW_OPTCODE_PARAINFORM_RET:
      case CTMDW_OPTCODE_PARACHANGEINFORM_RET:
         if(ctmdw_msg->numOfTLV == 1 && ctmdw_msg->tvlLst->type == CTMDW_TYPE_RETCODE)
         {
            ret = parseCTMDWParaRetCode(ctmdw_msg->tvlLst);
         }
         break;
      // one TLV with type 4(parameterNames)
      case CTMDW_OPTCODE_PARAMETERGET:
         CTMDW_DEBUG("ctmdw: parseCTMDWMsgAdvance %d %p\n", 
                      ctmdw_msg->numOfTLV, ctmdw_msg->tvlLst);
         
         if(ctmdw_msg->numOfTLV == 1 && ctmdw_msg->tvlLst != NULL)
         {
            if(ctmdw_msg->tvlLst->type == CTMDW_TYPE_PARAMETERNAMES)
            {
               //Parse Para Names
               ret = parseCTMDWParaNames(ctmdw_msg->tvlLst);
            }
         }
         break;
      // one TLV with type 4(parameterNames), one TLV with type 9(TW)
      case CTMDW_OPTCODE_PARAATTRIBUTEGET:
         CTMDW_DEBUG("ctmdw: parseCTMDWMsgAdvance %d %p\n", 
                      ctmdw_msg->numOfTLV, ctmdw_msg->tvlLst);
         if(ctmdw_msg->numOfTLV == 2 && ctmdw_msg->tvlLst != NULL)
         {
            if ((ctmdw_msg->tvlLst[0].type == CTMDW_TYPE_PARAMETERNAMES && ctmdw_msg->tvlLst[1].type == CTMDW_TYPE_TW) ||
               (ctmdw_msg->tvlLst[1].type == CTMDW_TYPE_PARAMETERNAMES && ctmdw_msg->tvlLst[0].type == CTMDW_TYPE_TW))
            {
               
               for(i = 0; i < ctmdw_msg->numOfTLV; i++)
               {
                  if(ctmdw_msg->tvlLst[i].type == CTMDW_TYPE_PARAMETERNAMES)
                     ret = parseCTMDWParaNames(ctmdw_msg->tvlLst + i);
                  else
                     ret = parseCTMDWParaTW(ctmdw_msg->tvlLst + i);

                  if(ret != CTMDW_STATUS_OK)
                     break;
               }
            }
         }
         break;
      // one TLV with type 8(attributes value), one TLV with type 9(TW)
      case CTMDW_OPTCODE_PARAATTRIBUTESET: 
         if(ctmdw_msg->numOfTLV == 2 && ctmdw_msg->tvlLst != NULL)
         {
            if(ctmdw_msg->tvlLst[0].type == CTMDW_TYPE_PARAATTRIBUTES && ctmdw_msg->tvlLst[1].type == CTMDW_TYPE_TW)
            {        
                 ret = parseCTMDWParaTW(ctmdw_msg->tvlLst + 1);
                 if(ret != CTMDW_STATUS_OK)
                     break;

                 ret = parseCTMDWParaAttrValues(ctmdw_msg->tvlLst, ctmdw_msg->tvlLst[1].value);
                 if(ret != CTMDW_STATUS_OK)
                     break;
            }
            else if(ctmdw_msg->tvlLst[1].type == CTMDW_TYPE_PARAATTRIBUTES && ctmdw_msg->tvlLst[0].type == CTMDW_TYPE_TW)
            {
                 ret = parseCTMDWParaTW(ctmdw_msg->tvlLst);
                 if(ret != CTMDW_STATUS_OK)
                     break;

                 ret = parseCTMDWParaAttrValues(ctmdw_msg->tvlLst + 1, ctmdw_msg->tvlLst[0].value);
                 if(ret != CTMDW_STATUS_OK)
                     break;       
            }
         }
         break;

      // one TLV with type 5(PARA VALUES), one TLV with type 9(TW)
      case CTMDW_OPTCODE_PARAMETERSET:
         if(ctmdw_msg->numOfTLV == 2 && ctmdw_msg->tvlLst != NULL)
         {
            if ((ctmdw_msg->tvlLst[0].type == CTMDW_TYPE_PARAVALUES && ctmdw_msg->tvlLst[1].type == CTMDW_TYPE_TW) ||
               (ctmdw_msg->tvlLst[1].type == CTMDW_TYPE_PARAVALUES && ctmdw_msg->tvlLst[0].type == CTMDW_TYPE_TW))
            {
               
               for(i = 0; i < ctmdw_msg->numOfTLV; i++)
               {
                  if(ctmdw_msg->tvlLst[i].type == CTMDW_TYPE_PARAVALUES)
                     ret = parseCTMDWParaNamesValues(ctmdw_msg->tvlLst + i);
                  else
                     ret = parseCTMDWParaTW(ctmdw_msg->tvlLst + i);

                  if(ret != CTMDW_STATUS_OK)
                     break;
               }
            }
         }
         break;
      case CTMDW_OPTCODE_ADDOBJECT:
      case CTMDW_OPTCODE_DELETEOBJECT:
         if(ctmdw_msg->numOfTLV == 1 && ctmdw_msg->tvlLst != NULL)
         {
            ret = parseCTMDWObject(ctmdw_msg->tvlLst);
         }
         break;
      case CTMDW_OPTCODE_GETPARANAMES:
         if(ctmdw_msg->numOfTLV == 1 && ctmdw_msg->tvlLst != NULL)
         {
            ret = parseCTMDWObject(ctmdw_msg->tvlLst);
         }
         break;
   
      case CTMDW_OPTCODE_DOWNLOAD:
      case CTMDW_OPTCODE_UPLOAD:
         if(ctmdw_msg->numOfTLV == 1 && ctmdw_msg->tvlLst != NULL &&
            ctmdw_msg->tvlLst[0].type == CTMDW_TYPE_PARAVALUES)
         {
            ret = parseCTMDWDownload(ctmdw_msg->tvlLst);
         }
         break;
      default:
         break;
   }
  
   return ret;
}


static CTMDW_STATUS parseCTMDWMsgGeneric(char* buf, int bufLen, PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char*          pPkt = NULL;
   int            pktOffset = 0;
   int            optCode = -1;
   int            numOfTLV = -1;
   PCTMDW_TLV     pTLV = NULL;
   int            i;

   g_twCodeValue = -1;

   if(ctmdw_msg == NULL || buf == NULL || bufLen <= 0)
      return ret = CTMDW_STATUS_ERR_PARAM;

   CTMDW_DEBUG("ctmdw: parseCTMDWMsgGeneric params...ok!\n");

   pPkt = buf;
   pktOffset = 0;

   //optCode
   optCode = *((unsigned char *)pPkt);
   if ((ret = validateOptCode(optCode)) != CTMDW_STATUS_OK)
      return ret;
   else
      ctmdw_msg->opcode = optCode;
   CTMDW_DEBUG("ctmdw: parseCTMDWMsgGeneric opt %d\n", optCode);
   pktOffset++;
   pPkt = buf + pktOffset;

   
   //numOfTLV
   numOfTLV = *((unsigned char *)pPkt);
   ctmdw_msg->numOfTLV = numOfTLV;
   CTMDW_DEBUG("ctmdw: parseCTMDWMsgGeneric numOfTLV %d\n", numOfTLV);
   pktOffset ++;
   pPkt = buf + pktOffset;

   if(pktOffset > bufLen)
      return ret = CTMDW_STATUS_ERR_FMT;

   //TLV
   ctmdw_msg->tvlLst = VOS_MALLOC(sizeof(CTMDW_TLV) * numOfTLV);
   if(ctmdw_msg->tvlLst == NULL && numOfTLV != 0)
      return ret = CTMDW_STATUS_ERR_NOMEM;
   else if ( numOfTLV != 0 )
      memset(ctmdw_msg->tvlLst, 0, sizeof(CTMDW_TLV) * numOfTLV);

   char dbgVal[512] = {0};
   for(i = 0; i < numOfTLV; i++)
   {  
      pTLV = ctmdw_msg->tvlLst + i;
      //TLV type
      pTLV->type = *((unsigned char *)pPkt);
      if ((ret = validateType(pTLV->type)) != CTMDW_STATUS_OK)
         break;    
      pktOffset++;
      pPkt = buf + pktOffset;

      //TLV len
      pTLV->length = ntohs(*((unsigned short *)pPkt));
      pktOffset += 2;
      pPkt = buf + pktOffset;

      //TLV value
      pTLV->value = pPkt;
      pktOffset += pTLV->length;
      pPkt = buf + pktOffset;

      //dump information
      strncpy(dbgVal, pTLV->value, pTLV->length);
      dbgVal[pTLV->length] = 0;
      CTMDW_DEBUG("ctmdw: parseCTMDWMsgGeneric...TLV type %d, len %d, value %s\n",
                  pTLV->type, pTLV->length, dbgVal);
      if(pTLV->type==CTMDW_TYPE_TW)
      {
         g_twCodeValue = atoi(dbgVal);
      }
      //OK as default
      pTLV->actRetVal = CTMDW_ACTION_OK;

      //TLV check boundary
      if(pktOffset > bufLen)
      {
         ret = CTMDW_STATUS_ERR_FMT;
         break;
      }
   }

   if(pktOffset != bufLen)
      return ret = CTMDW_STATUS_ERR_FMT;

   return ret;
}

/**********************************************************************************/
/*********************  CT Middelware client object handler ***********************/
/**********************************************************************************/
static PCTMDW_MSG alloc_ctmdwmsg(void)
{
   return (PCTMDW_MSG)VOS_MALLOC(sizeof(CTMDW_MSG));
}

static void free_ctmdwmsg(PCTMDW_MSG ctmdw_msg)
{
   int i, j; 
   PCTMDW_TLV tlv;
   PCTMDW_VAL val;
   
   if(ctmdw_msg == NULL)
      return;

   if(ctmdw_msg->tvlLst != NULL)
   {
      //Free all TLV
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {
         tlv = ctmdw_msg->tvlLst + i;

         if(tlv->valLst != NULL)
         {
            //Free all VAL for each TLV
            for( j = 0; j < tlv->numofVal; j++)
            {  
               val = tlv->valLst + j;
               //Free name and value of each VAL
               if(val->name != NULL)
                  VOS_FREE(val->name);
               if(val->value != NULL)
                  VOS_FREE(val->value);
            }
            VOS_FREE(tlv->valLst);
         }

         if(tlv->rvalue != NULL)
         {
            VOS_FREE(tlv->rvalue);
         }
      }
      VOS_FREE(ctmdw_msg->tvlLst);
   }

   VOS_FREE(ctmdw_msg);

   return;
}

static CTMDW_STATUS ctmdw_buildReturnValueSuccessNames(PCTMDW_TLV tlv)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   PCTMDW_VAL val;
   int i;
   char buf[CTMDW_PACKET_MAXLEN];
   char bFirst = TRUE;
   
   if(tlv == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   buf[0] = 0;

   for( i = 0; i < tlv->numofVal; i++)
   {  
      val = tlv->valLst + i;

      if ( val->actRetVal != CTMDW_ACTION_OK && val->actRetVal != CTMDW_ACTION_OK_REBOOT)
         continue;

      if ( !bFirst )
         UTIL_STRNCAT(buf, CTMDW_PARAM_DELIM, sizeof(buf));

      if(mappingTR69NameToCTName(val->name, buf, sizeof(buf)) != CTMDW_STATUS_OK)
         UTIL_STRNCAT(buf, val->name, sizeof(buf));

      bFirst = FALSE;
   }

   tlv->rvalue = VOS_MALLOC(util_strlen(buf) + 1);
   if(tlv->rvalue == NULL)
      return ret = CTMDW_STATUS_ERR_NOMEM;
   UTIL_STRNCPY(tlv->rvalue, buf, util_strlen(buf) + 1);

   CTMDW_DEBUG("ctmdw: ctmdw_buildReturnValueSuccessNames name %s\n", tlv->rvalue);

   return ret;
}

static CTMDW_STATUS ctmdw_buildReturnValue(PCTMDW_TLV tlv)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   PCTMDW_VAL val;
   int i;
   char buf[CTMDW_PACKET_MAXLEN];
   char param[CTMDW_PARAM_MAXLEN];
   char bOutOfRange = FALSE;
   
   if(tlv == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   buf[0] = 0;

   for( i = 0; i < tlv->numofVal; i++)
   {  
      val = tlv->valLst + i;
      param[0] = 0;
      if ( i != 0 )
         UTIL_STRNCAT(buf, CTMDW_PARAM_DELIM, sizeof(buf));

      if(mappingTR69NameToCTName(val->name, param, sizeof(param)) != CTMDW_STATUS_OK)
         UTIL_STRNCAT(param, val->name, sizeof(param));
      UTIL_STRNCAT(param, "=", sizeof(param));
      UTIL_STRNCAT(param, val->value, sizeof(param));

      if ((util_strlen(buf) + util_strlen(param)) > CTMDW_PACKET_MAXLEN)
      {
         bOutOfRange = TRUE;
         break;
      }
      else
         UTIL_STRNCAT(buf, param, sizeof(buf));
   }

   if(bOutOfRange)
   {
      tlv->actRetVal = CTMDW_ACTION_ERR_OUTOFRANGE;
      return ret = CTMDW_STATUS_ERR_PARAM;
   }

   tlv->rvalue = VOS_MALLOC(util_strlen(buf) + 1);
   if(tlv->rvalue == NULL)
      return ret = CTMDW_STATUS_ERR_NOMEM;
   UTIL_STRNCPY(tlv->rvalue, buf, util_strlen(buf) + 1);

   CTMDW_DEBUG("ctmdw: ctmdw_buildReturnValue name %s\n", tlv->rvalue);

   return ret;
}

static CTMDW_STATUS ctmdw_doParameterAttrSetSimple(char* path, char* value, int twValue)
{
    CTMDW_STATUS ret = CTMDW_STATUS_OK;
    CMC_PHL_SET_PARAM_ATTR_T *pSetParamAttr = NULL;
    
    pSetParamAttr = VOS_MALLOC_FLAGS(sizeof(CMC_PHL_SET_PARAM_ATTR_T), ALLOC_ZEROIZE);
    memset(pSetParamAttr, 0, sizeof(CMC_PHL_SET_PARAM_ATTR_T));

    if (pSetParamAttr == NULL)
    {
        CTMDW_DEBUG("ctmdw_doParameterAttrSetSimple: malloc failed\n");
        ret = CTMDW_STATUS_ERR_NOMEM;
    }
    else
    {
        if ((path!=NULL) && (util_strlen(path)!=0)) 
        {      
            CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSetSimple name %s\n", path);

            UTIL_STRNCPY(pSetParamAttr->paramPath, path, sizeof(pSetParamAttr->paramPath));
            pSetParamAttr->notificationChange = 1;
            pSetParamAttr->notification = atoi(value);
            pSetParamAttr->accessBitMaskChange = 0;
            pSetParamAttr->accessBitMask = NDA_ACCESS_TR69C;

            ret = CMC_phlSetMdwParamAttrList(pSetParamAttr, 1, twValue);
            if ( CTMDW_STATUS_OK == ret )
            {
                ctmdw_saveConfigFlag = TRUE;
                CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSetSimple Successful %s = %s\n", path, value);
            }
            else
            {
                CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSetSimple Failed %s = %s\n", path, value);
            }
        }
        else
        {
            CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSetSimple : name is NULL !!!\n");
        }
    }

    /* free pSetParamAttrList buffer */
    VOS_MEM_FREE_BUF_AND_NULL_PTR(pSetParamAttr);

    return ret;
}  

static CTMDW_STATUS ctmdw_doParameterAttrSet(PCTMDW_MSG ctmdw_msg)
{
    CTMDW_STATUS ret = CTMDW_STATUS_OK;
    PCTMDW_TLV tlv;
    PCTMDW_VAL val;
    int i = 0;
    int j = 0;
    UINT32 twValue = 0;
    UBOOL8 bOkOnce = FALSE;
    UBOOL8 bErrOnce = FALSE;

    if(ctmdw_msg == NULL)
    return ret;

    /*Get T/M/W value first*/
    twValue = g_twCodeValue;
    if(ctmdw_msg->tvlLst != NULL)
    {
        CMC_PHL_SET_PARAM_ATTR_T *pSetParamAttr = NULL;
        pSetParamAttr = VOS_MALLOC_FLAGS(sizeof(CMC_PHL_SET_PARAM_ATTR_T), ALLOC_ZEROIZE);
        memset(pSetParamAttr, 0, sizeof(CMC_PHL_SET_PARAM_ATTR_T));

        if (pSetParamAttr == NULL)
        {
            CTMDW_DEBUG("ctmdw_doParameterAttrSet: malloc failed\n");
            ret = CTMDW_STATUS_ERR_NOMEM;
        }
        else
        {
            for( i = 0; i < ctmdw_msg->numOfTLV; i++)
            {
                tlv = ctmdw_msg->tvlLst + i;

                if(tlv->type == CTMDW_TYPE_PARAATTRIBUTES && tlv->valLst != NULL)
                {
                    for( j = 0; j < tlv->numofVal; j++)
                    {  
                        val = tlv->valLst + j;
                        /* Please use name to retrieve value */
                        CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSet name %s\n", val->name);

                        const char  *pp = val->name;

                        if ((pp!=NULL) && (util_strlen(pp)!=0)) 
                        {
                            UTIL_STRNCPY(pSetParamAttr->paramPath, pp,sizeof(pSetParamAttr->paramPath));
                            pSetParamAttr->notificationChange = 1;
                            pSetParamAttr->notification = atoi(val->value);
                            pSetParamAttr->accessBitMaskChange = 0;
                            pSetParamAttr->accessBitMask = NDA_ACCESS_TR69C;

                            ret = CMC_phlSetMdwParamAttrList(pSetParamAttr, 1, twValue);

                            if ( CTMDW_STATUS_OK == ret )
                            {
                                ctmdw_saveConfigFlag = TRUE;
                                CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSet Successful %s = %s\n", val->name, val->value);
                                val->actRetVal = CTMDW_ACTION_OK;
                                bOkOnce = TRUE;
                            }
                            else
                            {
                                CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSet Failed %s = %s\n", val->name, val->value);
                                val->actRetVal = CTMDW_ACTION_ERR_GEN;
                                bErrOnce = TRUE;
                            }   
                        }
                        else
                        {
                            val->actRetVal = CTMDW_ACTION_ERR_GEN;
                            bErrOnce = TRUE;
                            CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrSet : name is NULL !!!\n");
                        }
                    }

                    if(bOkOnce && !bErrOnce)
                    {
                        tlv->actRetVal = CTMDW_ACTION_OK;
                    }
                    else if ( bOkOnce && bErrOnce )
                    {
                        tlv->actRetVal = CTMDW_ACTION_OK_PART;
                    }
                    else
                    {
                        tlv->actRetVal = CTMDW_ACTION_ERR_GEN;
                    }

                    if(tlv->actRetVal == CTMDW_ACTION_OK_PART)
                    {
                        ctmdw_buildReturnValueSuccessNames(tlv);
                    }
                }
            }
        }

        /* free pSetParamAttrList buffer */
        VOS_MEM_FREE_BUF_AND_NULL_PTR(pSetParamAttr);
    }
    return ret;
}

static CTMDW_STATUS ctmdw_doParameterAttrGet(PCTMDW_MSG ctmdw_msg)
{
    CTMDW_STATUS ret = CTMDW_STATUS_OK;
    VOS_RET_E ret1 = VOS_RET_SUCCESS;
    PCTMDW_TLV tlv;
    PCTMDW_VAL val;
    int i = 0;
    int j = 0;
    int k = 0;
    UINT32 twValue = 0;
    CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;
    UINT32 paramNum = 0;
    CMC_PHL_GET_PARAM_ATTR_T *pParamAttr = NULL;
    UBOOL8 isParamPath = FALSE;

    if(ctmdw_msg == NULL)
    {
        return ret;
    }

    /*Get T/M/W value first*/
    twValue = g_twCodeValue;
    if(ctmdw_msg->tvlLst != NULL)
    {
        for( i = 0; i < ctmdw_msg->numOfTLV; i++)
        {
            tlv = ctmdw_msg->tvlLst + i;

            if(tlv->type == CTMDW_TYPE_PARAMETERNAMES && tlv->valLst != NULL)
            {
                for( j = 0; j < tlv->numofVal; j++)
                {  
                    val = tlv->valLst + j;
                    /* Please use name to retrieve value */
                    CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrGet name %s\n", val->name);

                    const char  *pp = val->name;
                    char tmpbuf[8];

                    if (CMC_phlGetParamNameNum(pp, FALSE, &paramNum))
                    {
                        vosLog_error("CMC_phlGetParamNameNum run failed!");
                    }

                    pParamAttr = VOS_MALLOC_FLAGS(sizeof(CMC_PHL_GET_PARAM_ATTR_T) * paramNum, ALLOC_ZEROIZE);
                    memset(pParamAttr, 0, sizeof(CMC_PHL_GET_PARAM_ATTR_T) * paramNum);
                    ret = CMC_phlGetParamAttrList(pp, FALSE, &isParamPath, pParamAttr, &paramNum);

                    if (VOS_RET_SUCCESS == ret1 || VOS_RET_NO_MORE_INSTANCES == ret1)
                    {
                        if (FALSE == isParamPath)
                        {
                            /* this is a parameter path */
                            if(twValue)
                            {
                                if(pParamAttr->notification >= CTMDW_NOTIFICATION_READABLE)
                                {
                                    UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "1");
                                }
                                else
                                {
                                    UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "0");
                                }
                            }
                            else
                            {
                                if(pParamAttr->notification >= CTMDW_NOTIFICATION_READABLE)
                                {
                                    UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "%d", pParamAttr->notification -CTMDW_NOTIFICATION_READABLE);
                                }
                                else
                                {
                                    UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "%d", pParamAttr->notification );
                                }
                            }
                            
                            val->value = VOS_MALLOC (util_strlen(tmpbuf) + 1);
                            UTIL_STRNCPY(val->value, tmpbuf, util_strlen(tmpbuf) + 1);
                            CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrGet Successful %s = %s\n", val->name, val->value);
                            actRetVal = CTMDW_ACTION_OK;
                        }
                        else
                        {
                            /* this is an object path */
                            /* traverse the sub-tree below the object node */
                            for (;k < paramNum; k++)
                            {
                                if (ret1 == VOS_RET_SUCCESS)
                                {
                                    if(twValue)
                                    {
                                        if(pParamAttr->notification >= CTMDW_NOTIFICATION_READABLE)
                                        {
                                            UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "1");
                                        }
                                        else
                                        {
                                            UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "0");
                                        }
                                    }
                                    else
                                    {
                                        if(pParamAttr->notification >= CTMDW_NOTIFICATION_READABLE)
                                        {
                                            UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "%d", pParamAttr->notification -CTMDW_NOTIFICATION_READABLE);
                                        }
                                        else
                                        {
                                            UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "%d", pParamAttr->notification );
                                        }
                                    }
                                    
                                    val->value = VOS_MALLOC (util_strlen(tmpbuf) + 1);
                                    UTIL_STRNCPY(val->value, tmpbuf, util_strlen(tmpbuf) +1);
                                    CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrGet Successful %s = %s\n", val->name, val->value);
                                    actRetVal = CTMDW_ACTION_OK;
                                }
                                else if (ret1 == VOS_RET_NO_MORE_INSTANCES)
                                {
                                    ret = VOS_RET_SUCCESS;
                                    actRetVal = CTMDW_ACTION_OK;
                                    break;   /* out of while (acsState.fault == VOS_RET_SUCCESS) */
                                }
                                else
                                {
                                    actRetVal = CTMDW_ACTION_ERR_GEN;
                                    CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrGet can't find %s from the TR069 Object Tree!!!\n", val->name);
                                }

                                pParamAttr ++;
                            }  /* while (acsState.fault == VOS_RET_SUCCESS) */
                        }
                    }
                    else
                    {
                        actRetVal = CTMDW_ACTION_ERR_GEN;
                        CTMDW_DEBUG("ctmdw: ctmdw_doParameterAttrGet can't find %s from the TR069 Object Tree!!!\n", val->name);
                    }
                }

                VOS_MEM_FREE_BUF_AND_NULL_PTR(pParamAttr);
            }

            /* Build rvalue and status */
            tlv->actRetVal = actRetVal;
            if(tlv->actRetVal == CTMDW_ACTION_OK)
            ret = ctmdw_buildReturnValue(tlv);
        }
    }

    return ret;
}

static CTMDW_STATUS ctmdw_doParameterGet(PCTMDW_MSG ctmdw_msg)
{
    CTMDW_STATUS ret = CTMDW_STATUS_OK;
    PCTMDW_TLV tlv;
    PCTMDW_VAL val;
    int i = 0;
    int j = 0;
    VOS_RET_E fault = VOS_RET_INTERNAL_ERROR;
    CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;
    char *paramValue = NULL;
    
    if(ctmdw_msg == NULL)
    {
        return ret;
    }

    if(ctmdw_msg->tvlLst != NULL)
    {
        for( i = 0; i < ctmdw_msg->numOfTLV; i++)
        {
            tlv = ctmdw_msg->tvlLst + i;

            if(tlv->type == CTMDW_TYPE_PARAMETERNAMES && tlv->valLst != NULL)
            {
                for( j = 0; j < tlv->numofVal; j++)
                {  
                    val = tlv->valLst + j;
                    
                    /* Please use name to retrieve value */
                    CTMDW_DEBUG("ctmdw: ctmdw_doParameterGet name %s\n", val->name);
                    const char  *pp = val->name;

                    fault = tr69c_getParamValue(pp, &paramValue);
                    if (VOS_RET_SUCCESS == fault)
                    {
                        val->value = VOS_MALLOC(util_strlen(paramValue) + 1);               
                        UTIL_STRNCPY(val->value, paramValue, util_strlen(paramValue) + 1);                         
                        CTMDW_DEBUG("ctmdw: ctmdw_doParameterGet Successful %s = %s\n", val->name, val->value);
                        actRetVal = CTMDW_ACTION_OK;
                        
                        VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
                    }
                    else
                    {
                        actRetVal = CTMDW_ACTION_ERR_PARAM;
                        actRetVal = CTMDW_ACTION_ERR_PARAM;
                        CTMDW_DEBUG("ctmdw: ctmdw_doParameterGet : name is NULL !!!\n");
                    }

                }
            }

            /* Build rvalue and status */
            tlv->actRetVal = actRetVal;
            if(tlv->actRetVal == CTMDW_ACTION_OK)
            {
                ret = ctmdw_buildReturnValue(tlv);
            }
        }
    }

    return ret;
}


static CTMDW_STATUS ctmdw_doParameterSet(PCTMDW_MSG ctmdw_msg)
{
    CTMDW_STATUS ret = CTMDW_STATUS_OK;
    PCTMDW_TLV tlv;
    PCTMDW_VAL val;
    int i = 0;
    int j = 0;
    UINT32 twValue = 0;
    VOS_RET_E fault=0;
    UBOOL8 bOkOnce = FALSE;
    UBOOL8 bErrOnce = FALSE;
    UBOOL8 bRebootOnce = FALSE;

    if(ctmdw_msg == NULL)
    return ret;

    /*Get T/M/W value first*/
    twValue = g_twCodeValue;

    if(ctmdw_msg->tvlLst != NULL)
    {
        for ( i = 0; i < ctmdw_msg->numOfTLV; i++)
        {
            tlv = ctmdw_msg->tvlLst + i;
            if(tlv->type == CTMDW_TYPE_PARAVALUES && tlv->valLst != NULL)
            {
                int numEntries = 0;
                CMC_PHL_SET_PARAM_VALUE_T *pSetParamValueList = NULL;
                VOS_RET_E status[TR69C_PARAM_SET_VALUE_ARRAY];
                numEntries = tlv->numofVal;

                /* allocate memory for the set parameter value list */
                pSetParamValueList = VOS_MALLOC_FLAGS(numEntries * sizeof(CMC_PHL_SET_PARAM_VALUE_T), ALLOC_ZEROIZE);
                memset(pSetParamValueList, 0, sizeof(CMC_PHL_SET_PARAM_VALUE_T) * numEntries);
                
                if (pSetParamValueList == NULL)
                {
                    vosLog_error("ctmdw_doParameterSet: malloc failed\n");
                    ret = CTMDW_STATUS_ERR_NOMEM;
                }
                else
                {
                    CMC_PHL_SET_PARAM_VALUE_T *pSetParamValue = pSetParamValueList;

                    for( j = tlv->numofVal-1; j >= 0; j--)
                    {
                        val = tlv->valLst + j;
                        val->actRetVal = CTMDW_ACTION_ERR_GEN;
                        /* Please set value */
                        fprintf(stderr,"****Mid**** ctmdw: ctmdw_doParameterSet name %s value %s\n\n", val->name, val->value);

                        const char  *pp = val->name;                   
                        if ((pp != NULL) && (util_strlen(pp) != 0)) 
                        {
                            if (val->value == NULL) /* only valid for tSting params */
                            {
                                /* fake up a null string to avoid NULL ptr problem*/
                                val->value = VOS_STRDUP("");
                            }
                            UTIL_STRNCPY(pSetParamValue->value, val->value, sizeof(pSetParamValue->value));
                            UTIL_STRNCPY(pSetParamValue->paramPath, val->name, sizeof(pSetParamValue->paramPath));
                            fault = CMC_phlSetMdwParamValueList(pSetParamValue, 1, status, twValue);
                        }
                        else
                        {
                            CTMDW_DEBUG("ctmdw: ctmdw_doParameterSet : name is NULL !!!\n");
                        }

                        if ( fault == VOS_RET_SUCCESS)
                        {
                            int k = 0;
                            ctmdw_saveConfigFlag = TRUE;
                            CTMDW_DEBUG("ctmdw: ctmdw_doParameterSet Successful\n");
                            
                            for( k = 0; k < tlv->numofVal; k++)
                            {
                                val = tlv->valLst + k;
                                val->actRetVal = CTMDW_ACTION_OK;
                            } 
                            
                            bOkOnce = TRUE;
                        }
                        else if ( fault == VOS_RET_SUCCESS_REBOOT_REQUIRED)
                        {
                            int k = 0;
                            ctmdw_saveConfigFlag = TRUE;
                            CTMDW_DEBUG("ctmdw: ctmdw_doParameterSet Successful and Need Reboot\n");
                            
                            for( k = 0; k < tlv->numofVal; k++)
                            {
                                val = tlv->valLst + k;
                                val->actRetVal = CTMDW_ACTION_OK;
                            }
                            
                            val->actRetVal = CTMDW_ACTION_OK_REBOOT;
                            bRebootOnce = TRUE;
                            bOkOnce = TRUE;
                        }
                        else
                        {
                            val->actRetVal = CTMDW_ACTION_ERR_GEN;
                            bErrOnce = TRUE;
                            CTMDW_DEBUG("ctmdw: ctmdw_doParameterSet Fail\n");
                        }
                        if(bOkOnce && !bErrOnce)
                        {
                            if(bRebootOnce)
                            {
                                tlv->actRetVal = CTMDW_ACTION_OK_REBOOT;
                            }
                            else
                            {
                                tlv->actRetVal = CTMDW_ACTION_OK;
                            }
                        }
                        else if ( bOkOnce && bErrOnce )
                        {
                            tlv->actRetVal = CTMDW_ACTION_OK_PART;
                        }
                        else
                        {
                            tlv->actRetVal = ((PCTMDW_VAL)tlv->valLst)->actRetVal;
                        }

                        if(tlv->actRetVal == CTMDW_ACTION_OK_PART)
                        {
                            ctmdw_buildReturnValueSuccessNames(tlv);
                        }

                        pSetParamValue++;
                    }
                }
                
                /* free pSetParamValue buffer.
                * pValue buffer will be freed by freeing ctmdw_msg.
                */
                VOS_MEM_FREE_BUF_AND_NULL_PTR(pSetParamValueList);
            } 
        }    
    }

    return ret;
}

static CTMDW_STATUS ctmdw_doFileGet(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS ret = CTMDW_STATUS_ERR_GEN;
   CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;
   char *log = NULL;
   UINT16 logLen = 0;
   UINT32 len = 0;
   
   if(ctmdw_msg == NULL)
      return ret;
   
   // one for FWCONFIG and one for LOGCONFIG
   ctmdw_msg->tvlLst = VOS_MALLOC(sizeof(CTMDW_TLV) * 2);
   if(ctmdw_msg->tvlLst == NULL)
      return ret;
   memset(ctmdw_msg->tvlLst, 0, sizeof(CTMDW_TLV) * 2);
   ctmdw_msg->numOfTLV = 2;
#if 0//zanshi
   cfgBufLen = cmcImg_getConfigFlashSize();

   if(cfgBufLen != 0){
    cfgBuf = VOS_MALLOC_FLAGS(cfgBufLen, 0);
    if (cfgBuf == NULL)
    {
            CTMDW_DEBUG("malloc of %d bytes failed in ctmdw_doFileGet", cfgBufLen);
       }
   }
   
   if ((cfgBuf != NULL) && (cmcMgm_writeConfigToBuf(cfgBuf, &cfgBufLen) == VOS_RET_SUCCESS))
   {

      ctmdw_msg->tvlLst->type = CTMDW_TYPE_PARAVALUES;
      ctmdw_msg->tvlLst->rvalue = VOS_MALLOC(util_strlen(CTMDW_CONFIG_FWCONFIG) + util_strlen(cfgBufLen) + util_strlen(CTMDW_VALUE_DELIM) + 1);
      if(ctmdw_msg->tvlLst->rvalue != NULL)
      {
         ctmdw_msg->tvlLst->type = CTMDW_TYPE_PARAVALUES;
         ctmdw_msg->tvlLst->rvalue[0] = 0;
         UTIL_STRNCAT(ctmdw_msg->tvlLst->rvalue, CTMDW_CONFIG_FWCONFIG);
         UTIL_STRNCAT(ctmdw_msg->tvlLst->rvalue, CTMDW_VALUE_DELIM);
         UTIL_STRNCAT(ctmdw_msg->tvlLst->rvalue, cfgBuf);

      VOS_FREE(cfgBuf);
      cfgBuf = NULL;
      
         ctmdw_msg->tvlLst->actRetVal = actRetVal;
         ret = CTMDW_STATUS_OK;
      }
   }   
#endif
   CMC_sysGetDeviceLogLen(&logLen);

   if (log != NULL && logLen != 0)
   {
      len = util_strlen(CTMDW_LOG_FWLOG) + util_strlen(log)+ util_strlen(CTMDW_VALUE_DELIM);
      ctmdw_msg->tvlLst[1].type = CTMDW_TYPE_PARAVALUES;
      ctmdw_msg->tvlLst[1].rvalue = VOS_MALLOC(len + 1);
      if(ctmdw_msg->tvlLst[1].rvalue != NULL)
      {
         ctmdw_msg->tvlLst[1].type = CTMDW_TYPE_PARAVALUES;
         ctmdw_msg->tvlLst[1].rvalue[0] = 0;
         UTIL_STRNCAT(ctmdw_msg->tvlLst[1].rvalue, CTMDW_LOG_FWLOG, len);
         UTIL_STRNCAT(ctmdw_msg->tvlLst[1].rvalue, CTMDW_VALUE_DELIM, len);
         UTIL_STRNCAT(ctmdw_msg->tvlLst[1].rvalue, log, len);

      VOS_FREE(log);
      log = NULL;
      
         ctmdw_msg->tvlLst[1].actRetVal = actRetVal;
         ret = CTMDW_STATUS_OK;
      }  
   }

   return ret = CTMDW_STATUS_OK;
}

static CTMDW_STATUS ctmdw_doDownload(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS ret = CTMDW_STATUS_ERR_GEN;
   PCTMDW_TLV tlv;
   PCTMDW_VAL val;
   int i, j;
   CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;
   RPCAction *dwnldRPCAction = NULL;

   if(ctmdw_msg == NULL)
      return ret;

   dwnldRPCAction = newRPCAction();
   dwnldRPCAction->rpcMethod = rpcDownload;
   
   if(ctmdw_msg->tvlLst != NULL)
   {
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {
         tlv = ctmdw_msg->tvlLst + i;

         if(tlv->type == CTMDW_TYPE_PARAVALUES && tlv->valLst != NULL)
         {
            for( j = 0; j < tlv->numofVal; j++)
            {  
               val = tlv->valLst + j;

               if (!strcmp(val->name, CTMDW_DOWNLOAD_COMMANDKEY))
                  dwnldRPCAction->ud.downloadReq.commandKey = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_FILETYPE))
                  dwnldRPCAction->ud.downloadReq.efileType = atoi(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_URL))
                  dwnldRPCAction->ud.downloadReq.url = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_USERNAME))
                  dwnldRPCAction->ud.downloadReq.user = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_PASSWORD))
                  dwnldRPCAction->ud.downloadReq.pwd = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_FILESIZE))
                  dwnldRPCAction->ud.downloadReq.fileSize = atoi(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_TARGETFILENAME))
                  dwnldRPCAction->ud.downloadReq.fileName = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_DELAYSECONDS))
                  dwnldRPCAction->ud.downloadReq.delaySec = atoi(val->value);       
            }
         }
         
         tlv->actRetVal = actRetVal;
      }
   }

   g_downloadState = CTMDW_DOWNLOAD_STATE_IDLE;
   sleep((1 + dwnldRPCAction->ud.downloadReq.delaySec));
   downloadStart((void *)dwnldRPCAction);

   return ret = CTMDW_STATUS_OK;
}

static CTMDW_STATUS ctmdw_doUpload(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS ret = CTMDW_STATUS_ERR_GEN;
   PCTMDW_TLV tlv;
   PCTMDW_VAL val;
   int i, j;
   CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;
   RPCAction *dwnldRPCAction = NULL;

   if(ctmdw_msg == NULL)
      return ret;

   dwnldRPCAction = newRPCAction();
   dwnldRPCAction->rpcMethod = rpcDownload;
   
   if(ctmdw_msg->tvlLst != NULL)
   {
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {
         tlv = ctmdw_msg->tvlLst + i;

         if(tlv->type == CTMDW_TYPE_PARAVALUES && tlv->valLst != NULL)
         {
            for( j = 0; j < tlv->numofVal; j++)
            {  
               val = tlv->valLst + j;

               if (!strcmp(val->name, CTMDW_DOWNLOAD_COMMANDKEY))
                  dwnldRPCAction->ud.downloadReq.commandKey = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_FILETYPE))
               {
                  if ( atoi(val->value) == eWebContent )
                     dwnldRPCAction->ud.downloadReq.efileType = eVendorLog;
                  else
                     dwnldRPCAction->ud.downloadReq.efileType = eVendorConfig;
               }
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_URL))
                  dwnldRPCAction->ud.downloadReq.url = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_USERNAME))
                  dwnldRPCAction->ud.downloadReq.user = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_PASSWORD))
                  dwnldRPCAction->ud.downloadReq.pwd = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_FILESIZE))
                  dwnldRPCAction->ud.downloadReq.fileSize = atoi(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_TARGETFILENAME))
                  dwnldRPCAction->ud.downloadReq.fileName = VOS_STRDUP(val->value);
               else if (!strcmp(val->name, CTMDW_DOWNLOAD_DELAYSECONDS))
                  dwnldRPCAction->ud.downloadReq.delaySec = atoi(val->value);       
            }
         }
         
         tlv->actRetVal = actRetVal;
      }
   }

   g_downloadState = CTMDW_DOWNLOAD_STATE_IDLE;
   sleep((1 + dwnldRPCAction->ud.downloadReq.delaySec));
   uploadStart((void *)dwnldRPCAction);

   return ret = CTMDW_STATUS_OK;
}

static CTMDW_STATUS ctmdw_doReboot(void)
{
   CTMDW_DEBUG("ctmdw: ctmdw_doReboot ............!!!\n");

   /* restore to default first if necessary... */
   if(enblResetDefault)
   {
      if(enblResetDefault == 1)
      {
         CTMDW_DEBUG("ctmdw: reset first REMOTE RESTORE DEFAULT............!!!\n");
         /* need more handle in future for remote restore default */
      CMC_sysResetConfig(CMC_SYS_CONFIG_RESET_REMOTE);
      }
      else
      {
         CTMDW_DEBUG("ctmdw: reset first LOCAL RESTORE DEFAULT............!!!\n");
         /* need more handle in future for local restore default */
      CMC_sysResetConfig(CMC_SYS_CONFIG_RESET_REMOTE);
      }
   }

   UTIL_reboot(GetTR69CMsgHandler());
   
   return CTMDW_STATUS_OK;
}

static CTMDW_STATUS ctmdw_doSetDefault(void)
{
   CTMDW_DEBUG("ctmdw: ctmdw_doSetDefault ............!!!\n");
   enblResetDefault = 1;
   return CTMDW_STATUS_OK;
}

CTMDW_STATUS ctmdw_doSetLocalDefault(void)
{
   CTMDW_DEBUG("ctmdw: ctmdw_doSetLocalDefault ............!!!\n");
   enblResetDefault = 2;
   return CTMDW_STATUS_OK;
}

static CTMDW_STATUS ctmdw_doAddObject(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   PCTMDW_TLV tlv;
   PCTMDW_VAL val;
   int i, retvalue;
   CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;

   if(ctmdw_msg == NULL)
      return ret;

   if(ctmdw_msg->tvlLst != NULL)
   {
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {
         tlv = ctmdw_msg->tvlLst + i;

         if(tlv->type == CTMDW_TYPE_OBJECT && tlv->valLst != NULL)
         {
            const char  *pp;
            char  *value = NULL;
            val = tlv->valLst;

            pp = val->name;
            //Please use name to retrieve value
            CTMDW_DEBUG("ctmdw: ctmdw_doAddObject name %s\n", val->name);

         if ((retvalue = ctmdw_doAddObjectFromTR69(val->name, &value)) != -1)
            {
                  tlv->rvalue = VOS_MALLOC(util_strlen(value) + 1);  
                if(tlv->rvalue == NULL)
                {  
                    return ret = CTMDW_STATUS_ERR_NOMEM;             
                }
                else
                {
                    UTIL_STRNCPY(tlv->rvalue, value, util_strlen(value) + 1);     
                    CTMDW_DEBUG("ctmdw: ctmdw_doAddObject name %s instance=%s\n", val->name, tlv->rvalue);
                } 
          VOS_FREE (value);

          ctmdw_saveConfigFlag = TRUE;
                actRetVal = CTMDW_ACTION_OK;
                if(retvalue == 2)
                  actRetVal = CTMDW_ACTION_OK_REBOOT;
            }                
            else
            {
               actRetVal = CTMDW_ACTION_ERR_GEN;
            }           
         }

         //Build rvalue and status
         tlv->actRetVal = actRetVal;
      }
   }
   return ret;
}

static CTMDW_STATUS ctmdw_doDeleteObject(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   PCTMDW_TLV tlv;
   PCTMDW_VAL val;
   int i, j, retvalue;
   CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;

   if(ctmdw_msg == NULL)
      return ret;

   if(ctmdw_msg->tvlLst != NULL)
   {
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {
         tlv = ctmdw_msg->tvlLst + i;

         if(tlv->type == CTMDW_TYPE_OBJECT && tlv->valLst != NULL)
         {
            for( j = 0; j < tlv->numofVal; j++)
            {  
               val = tlv->valLst + j;
               //Please use name to retrieve value
               CTMDW_DEBUG("ctmdw: ctmdw_doDeleteObject name %s\n", val->name);
               if ((retvalue = ctmdw_doDeleteObjectFromTR69(val->name)) != -1)
               {
                  ctmdw_saveConfigFlag = TRUE;
                  actRetVal = CTMDW_ACTION_OK;
                  if(retvalue == 2)
                     actRetVal = CTMDW_ACTION_OK_REBOOT;
               }                
               else
               {
                  actRetVal = CTMDW_ACTION_ERR_GEN;
               }               
            }
         }
         //Build rvalue and status
         tlv->actRetVal = actRetVal;
      }
   }
   return ret;
}

static CTMDW_STATUS ctmdw_doGetParaNames(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   PCTMDW_TLV tlv;
   PCTMDW_VAL val;
   int i, retvalue;
   char* path;
   CTMDW_ACTION_RETVAL actRetVal = CTMDW_ACTION_OK;
   
   if(ctmdw_msg == NULL)
      return ret;
   
   if(ctmdw_msg->tvlLst != NULL)
   {
      
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {
         tlv = ctmdw_msg->tvlLst + i;
         
         if(tlv->type == CTMDW_TYPE_OBJECT && tlv->valLst != NULL)
         {
            
            char tmpbuf[CTMDW_PACKET_MAXLEN+CTMDW_PACKET_MAXLEN+CTMDW_PACKET_MAXLEN];
            memset(tmpbuf, 0 ,CTMDW_PACKET_MAXLEN+CTMDW_PACKET_MAXLEN+CTMDW_PACKET_MAXLEN); 
            val = tlv->valLst;

            
            if(val->name[util_strlen(val->name)-1] != '.')
            {
               //attach '.' to the names
               path = VOS_MALLOC(util_strlen(val->name) + 2);
               if(path != NULL)
               {
                  UTIL_STRNCPY(path, val->name, util_strlen(val->name) + 2);
                  UTIL_STRNCAT(path, ".", util_strlen(val->name) + 2);
                  VOS_FREE(val->name);
                  val->name = path;
                  
               }
            }
            
            //Please use name to retrieve value
            if ((retvalue=ctmdw_doGetParameterNamesFromTR69(val->name, tmpbuf)) != -1)
            {  
               if(util_strlen(tmpbuf) == 0)
               {
                  tlv->rvalue = VOS_MALLOC(util_strlen("NULL") + 1);  
                  if(tlv->rvalue == NULL)
                  {  
                     return ret = CTMDW_STATUS_ERR_NOMEM;             
                  }
                  else
                  {
                     UTIL_STRNCPY(tlv->rvalue, "NULL", util_strlen("NULL") + 1);     
                     CTMDW_DEBUG("ctmdw: ctmdw_doGetParaNames name %s retvalue=%s\n", val->name, tlv->rvalue);
                  }
               }
               else if(util_strlen(tmpbuf)<=CTMDW_PACKET_MAXLEN*3)
               {
                  tlv->rvalue = VOS_MALLOC(util_strlen(tmpbuf));  
                  if(tlv->rvalue == NULL)
                  {  
                     return ret = CTMDW_STATUS_ERR_NOMEM;             
                  }
                  else
                  {
                     UTIL_STRNCPY(tlv->rvalue, tmpbuf+1, util_strlen(tmpbuf) + 1);     
                     CTMDW_DEBUG("ctmdw: ctmdw_doGetParaNames name %s retvalue=%s\n", val->name, tlv->rvalue);
                  }
               }
               else
                  actRetVal = CTMDW_ACTION_ERR_OUTOFRANGE;
            }
            else
            {
               actRetVal = CTMDW_ACTION_ERR_PARAM;
            }
         }
         //Build rvalue and status
         tlv->actRetVal = actRetVal;
      }
   }
   return ret;
}

static CTMDW_STATUS ctmdw_sendRegister(void)
{
   char           buf[CTMDW_PACKET_MAXLEN];
   char           *pBuf = NULL, *pTLVLen = NULL;
   char           bufOffset = 0;
   unsigned short len = 0;
   CMC_SYS_DEVICE_INFO_T deviceInfo;
   
   pBuf = buf;
   
   //Optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_REGISTER;
   bufOffset++;
   pBuf++;

   //NumOfTLV(ModName, ParaValues)
   *((unsigned char*)pBuf) = 2;
   bufOffset++;
   pBuf++;

   /* ModName TLV */
   //type
   *((unsigned char*)pBuf) = CTMDW_TYPE_MODNAME;
   bufOffset++;
   pBuf++;

   //length
   len = util_strlen(CTMDW_REGISTER_MODNAME);
   *((unsigned short*)pBuf) = htons(len);
   pBuf += 2;
   bufOffset += 2;

   //value
   *pBuf = 0;
   UTIL_STRNCAT(pBuf, CTMDW_REGISTER_MODNAME, sizeof(buf));
   pBuf += len;
   bufOffset += len;

   /* Paravalues TLV */
   //type
   *((unsigned char*)pBuf) = CTMDW_TYPE_PARAVALUES;
   bufOffset++;
   pBuf++;

   //length, skip first and fill it later
   pTLVLen = pBuf;
   pBuf += 2;
   bufOffset += 2;

   *pBuf = 0;

   memset(&deviceInfo, 0, sizeof(CMC_SYS_DEVICE_INFO_T));
   if (VOS_RET_SUCCESS != CMC_sysGetDeviceInfo(&deviceInfo))
   {
         CTMDW_DEBUG("could not get device info object in ctmdw_sendRegister!");
   }
   else
   {
    UTIL_STRNCAT(pBuf, CTMDW_REGISTER_HWVER, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_VALUE_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, deviceInfo.hardwareVersion, sizeof(buf));
   
    UTIL_STRNCAT(pBuf, CTMDW_PARAM_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_REGISTER_SWVER, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_VALUE_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, deviceInfo.softwareVersion, sizeof(buf));
  
    UTIL_STRNCAT(pBuf, CTMDW_PARAM_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_REGISTER_OUI, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_VALUE_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, deviceInfo.manufacturerOUI, sizeof(buf));

    UTIL_STRNCAT(pBuf, CTMDW_PARAM_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_REGISTER_VENDOR, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_VALUE_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, deviceInfo.manufacturer, sizeof(buf));

    UTIL_STRNCAT(pBuf, CTMDW_PARAM_DELIM, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_REGISTER_SN, sizeof(buf));
    UTIL_STRNCAT(pBuf, CTMDW_VALUE_DELIM, sizeof(buf));
       UTIL_STRNCAT(pBuf, deviceInfo.serialNumber, sizeof(buf));
   }
   
   //length, skip first and fill it later
   len = util_strlen(pBuf);
   *((unsigned short*)pTLVLen) = htons(len);
   pBuf += len;
   bufOffset += len;
   CTMDW_DEBUG("send register buf =%s, len = %d\n", buf+16, len);
   sendto_ctmdw_server(buf, bufOffset);

   return CTMDW_STATUS_OK;
}

#if 0
static CTMDW_STATUS ctmdw_sendRebootRet(void)
{
   char           buf[CTMDW_PACKET_MAXLEN];
   char           *pBuf = NULL;
   char           bufOffset = 0;
   unsigned short len = 0;

   pBuf = buf;
   
   //Optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_REBOOT_RET;
   bufOffset++;
   pBuf++;

   //NumOfTLV(ModName, ParaValues)
   *((unsigned char*)pBuf) = 1;
   bufOffset++;
   pBuf++;

   /* ModName TLV */
   //type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   bufOffset++;
   pBuf++;

   //length
   len = util_strlen(CTMDW_RETCODE_OK);
   *((unsigned short*)pBuf) = htons(len);
   pBuf += 2;
   bufOffset += 2;

   //value
   *pBuf = 0;
   UTIL_STRNCAT(pBuf, CTMDW_RETCODE_OK);
   pBuf += len;
   bufOffset += len;

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);

   return CTMDW_STATUS_OK;
}
#endif

void ctmdw_sendUpgradeRet(void)
{
   char           buf[CTMDW_PACKET_MAXLEN];
   char           *pBuf = NULL;
   char           bufOffset = 0;
   unsigned short len = 0;
   char           retInfo[64];

   pBuf = buf;
   
   //Optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_DOWNLOAD_RET;
   pBuf++;

   *((unsigned char*)pBuf) = 1;
   pBuf++;

   /* ModName TLV */
   //type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   pBuf++;

   UTIL_STRNCPY(retInfo, CTMDW_RETCODE_OK, sizeof(retInfo));
   if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_FAILED)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR, sizeof(retInfo));
   else if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_CONNECT)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR_SERVER, sizeof(retInfo));
   else if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR_PASSWORD, sizeof(retInfo));
   else if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_FILE)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR_FILE, sizeof(retInfo));

   //length
   len = util_strlen(retInfo);
   *((unsigned short*)pBuf) = htons(len);
   pBuf += 2;

   //value
   *pBuf = 0;
   UTIL_STRNCAT(pBuf, retInfo, sizeof(buf));
   pBuf += len;
   bufOffset += len;

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);
}

void ctmdw_sendUploadRet(void)
{
   char           buf[CTMDW_PACKET_MAXLEN];
   char           *pBuf = NULL;
   char           bufOffset = 0;
   unsigned short len = 0;
   char           retInfo[64];

   pBuf = buf;
   
   //Optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_UPLOAD_RET;
   pBuf++;

   *((unsigned char*)pBuf) = 1;
   pBuf++;

   /* ModName TLV */
   //type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   pBuf++;

   UTIL_STRNCPY(retInfo, CTMDW_RETCODE_OK, sizeof(retInfo));
   if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_FAILED)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR, sizeof(retInfo));
   else if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_CONNECT)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR_SERVER, sizeof(retInfo));
   else if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR_PASSWORD, sizeof(retInfo));
   else if(g_downloadState == CTMDW_DOWNLOAD_STATE_ERR_FILE)
      UTIL_STRNCPY(retInfo, CTMDW_RETCODE_ERR_FILE, sizeof(retInfo));

   //length
   len = util_strlen(retInfo);
   *((unsigned short*)pBuf) = htons(len);
   pBuf += 2;

   //value
   *pBuf = 0;
   UTIL_STRNCAT(pBuf, retInfo, sizeof(buf));
   pBuf += len;

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);
}



static CTMDW_STATUS ctmdw_sendSetParameterResponse(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   char           bReqSuccess = TRUE;
   char           bReqPartSuccess = FALSE;
   char           bReqSuccessReboot = FALSE;
   char           bReqAccessDenied = FALSE;
   int            i;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_PARAVALUES && tlv->actRetVal != CTMDW_ACTION_OK && tlv->actRetVal != CTMDW_ACTION_OK_REBOOT)
         bReqSuccess = FALSE;
      if(tlv->type == CTMDW_TYPE_PARAVALUES && tlv->actRetVal == CTMDW_ACTION_OK_PART)
         bReqPartSuccess = TRUE;
      if(tlv->type == CTMDW_TYPE_PARAVALUES && tlv->actRetVal == CTMDW_ACTION_OK_REBOOT)
         bReqSuccessReboot = TRUE;
      if(tlv->type == CTMDW_TYPE_PARAVALUES && tlv->actRetVal == CTMDW_ACTION_ERR_NOTACCESS)
         bReqAccessDenied = TRUE;
   }
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_PARAMETERSET_RET;
   pBuf++;
   
   
   /* retcode only */
   // Number of Value
   *((unsigned char*)pBuf) = bReqPartSuccess ? 2 : 1;
   pBuf++;
   
   // type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   pBuf++;

   if(bReqPartSuccess)
   {
      // len
      len = util_strlen(CTMDW_RETCODE_OK_PART);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      strncpy(pBuf, CTMDW_RETCODE_OK_PART, len);
      pBuf += len;

      // paraNames success
      *((unsigned char*)pBuf) = CTMDW_TYPE_PARAMETERNAMES;
      pBuf++;

      //len
      len  = util_strlen(ctmdw_msg->tvlLst->rvalue);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;

      //para names   
      strncpy(pBuf, ctmdw_msg->tvlLst->rvalue, len);
      pBuf += len;    
   }
   else if (!bReqSuccess)
   {
      // len
      if(bReqAccessDenied)
      {
         len = util_strlen(CTMDW_RETCODE_ERR_ACCESSDENIED);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;
         
         // err info
         strncpy(pBuf, CTMDW_RETCODE_ERR_ACCESSDENIED, len);
         pBuf += len;
      }
      else
      {
         len = util_strlen(CTMDW_RETCODE_ERR);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;
         
         // err info
         strncpy(pBuf, CTMDW_RETCODE_ERR, len);
         pBuf += len;     
      }
   }
   else
   {
      if (!bReqSuccessReboot)
      {
         // len
         len = util_strlen(CTMDW_RETCODE_OK);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;
         
         // err info
         strncpy(pBuf, CTMDW_RETCODE_OK, len);
         pBuf += len;
      }
      else
      {
         // len
         len = util_strlen(CTMDW_RETCODE_OK_REBOOT);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;
         
         // err info
         strncpy(pBuf, CTMDW_RETCODE_OK_REBOOT, len);
         pBuf += len;
      }
   }

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);
   return ret;
}

static CTMDW_STATUS ctmdw_sendGetParameterResponse(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   char           bReqSuccess = TRUE;
   char           bOutOfBuffer = FALSE;
   int            i;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_PARAMETERNAMES && tlv->actRetVal != CTMDW_ACTION_OK)
         bReqSuccess = FALSE;
      if(tlv->type == CTMDW_TYPE_PARAMETERNAMES && tlv->actRetVal == CTMDW_ACTION_ERR_OUTOFRANGE)
         bOutOfBuffer = TRUE;
   }
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_PARAMETERGET_RET;
   pBuf++;
   
   if (!bReqSuccess)
   {
      /* retcode only */
      // Number of Value
      *((unsigned char*)pBuf) = 1;
      pBuf++;
      
      // type
      *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
      pBuf++;

      // len
      if(bOutOfBuffer)
         len = util_strlen(CTMDW_RETCODE_ERR_DATALONG);
      else
         len = util_strlen(CTMDW_RETCODE_ERR_PARAM);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      if(bOutOfBuffer)
         strncpy(pBuf, CTMDW_RETCODE_ERR_DATALONG, len);
      else         
         strncpy(pBuf, CTMDW_RETCODE_ERR_PARAM, len);
      pBuf += len;
   }
   else
   {
      /* retcode and values */
      // Number of Value
      *((unsigned char*)pBuf) = (unsigned char)(ctmdw_msg->numOfTLV + 1);
      pBuf++;
      
      // type
      *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
      pBuf++;

      // len
      len = util_strlen(CTMDW_RETCODE_OK);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // ok info
      strncpy(pBuf, CTMDW_RETCODE_OK, len);
      pBuf += len;

      // tlv
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {  
         tlv = ctmdw_msg->tvlLst + i;
         if(tlv->type == CTMDW_TYPE_PARAMETERNAMES)
         {
            if ((pBuf - buf + 3) >= CTMDW_PACKET_MAXLEN)
            {
               bOutOfBuffer = TRUE;
               break;
            }
            
            // type
            *((unsigned char*)pBuf) = CTMDW_TYPE_PARAVALUES;
            pBuf++;

            // len
            len = util_strlen(tlv->rvalue);
            *((unsigned short*)pBuf) = htons(len);
            pBuf += 2;

            if ((pBuf - buf + len) > CTMDW_PACKET_MAXLEN)
            {
               bOutOfBuffer = TRUE;
               break;
            }

            // data
            strncpy(pBuf, tlv->rvalue, len);
            pBuf += len;           
         }
      }

      if(bOutOfBuffer == TRUE)
      {
         /* retcode only out of range*/
         // Number of Value
         pBuf = buf + 1;
         *((unsigned char*)pBuf) = 1;
         pBuf++;
         
         // type
         *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
         pBuf++;
   
         // len
         len = util_strlen(CTMDW_RETCODE_ERR_DATALONG);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;
         
         // err info
         strncpy(pBuf, CTMDW_RETCODE_ERR_DATALONG, len);
         pBuf += len;
      }
   }

   bufOffset = pBuf - buf;
   sendto_ctmdw_server(buf, bufOffset);
   return ret;
}

static CTMDW_STATUS ctmdw_sendGetParaNamesRet(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   char           bReqSuccess = TRUE;
   char           bOutOfBuffer = FALSE;
   int            i;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_OBJECT && tlv->actRetVal != CTMDW_ACTION_OK)
         bReqSuccess = FALSE;
      if(tlv->type == CTMDW_TYPE_OBJECT && tlv->actRetVal == CTMDW_ACTION_ERR_OUTOFRANGE)
         bOutOfBuffer = TRUE;
   }
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_GETPARANAMES_RET;
   pBuf++;
   
   if (!bReqSuccess)
   {
      /* retcode only */
      // Number of Value
      *((unsigned char*)pBuf) = 1;
      pBuf++;
      
      // type
      *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
      pBuf++;

      // len
      if(bOutOfBuffer)
         len = util_strlen(CTMDW_RETCODE_ERR_DATALONG);
      else
         len = util_strlen(CTMDW_RETCODE_ERR_PARAM);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      if(bOutOfBuffer)
         strncpy(pBuf, CTMDW_RETCODE_ERR_DATALONG, len);
      else         
         strncpy(pBuf, CTMDW_RETCODE_ERR_PARAM, len);
      pBuf += len;
   }
   else
   {
      /* retcode and values */
      // Number of Value
      *((unsigned char*)pBuf) = (unsigned char)(ctmdw_msg->numOfTLV + 1);
      pBuf++;
      
      // type
      *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
      pBuf++;

      // len
      len = util_strlen(CTMDW_RETCODE_OK);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // ok info
      strncpy(pBuf, CTMDW_RETCODE_OK, len);
      pBuf += len;

      // tlv
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {  
         tlv = ctmdw_msg->tvlLst + i;
         if(tlv->type == CTMDW_TYPE_OBJECT)
         {
            if ((pBuf - buf + 3) >= CTMDW_PACKET_MAXLEN)
            {
               bOutOfBuffer = TRUE;
               break;
            }
            
            // type
            *((unsigned char*)pBuf) = CTMDW_TYPE_PARALIST;
            pBuf++;

            // len
            len = util_strlen(tlv->rvalue);
            *((unsigned short*)pBuf) = htons(len);
            pBuf += 2;

            if ((pBuf - buf + len) > CTMDW_PACKET_MAXLEN)
            {
               bOutOfBuffer = TRUE;
               break;
            }

            // data
            strncpy(pBuf, tlv->rvalue, len);
            pBuf += len;           
         }
      }

      if(bOutOfBuffer == TRUE)
      {
         /* retcode only out of range*/
         // Number of Value
         pBuf = buf + 1;
         *((unsigned char*)pBuf) = 1;
         pBuf++;
         
         // type
         *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
         pBuf++;
   
         // len
         len = util_strlen(CTMDW_RETCODE_ERR_DATALONG);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;
         
         // err info
         strncpy(pBuf, CTMDW_RETCODE_ERR_DATALONG, len);
         pBuf += len;
      }
   }

   bufOffset = pBuf - buf;
   sendto_ctmdw_server(buf, bufOffset);
   return ret;
}


void ctmdw_sendInform(void)
{
   unsigned short len = 0;
   char value[CTMDW_PACKET_MAXLEN];
   char buf[CTMDW_PACKET_MAXLEN];
   memset(buf, 0, CTMDW_PACKET_MAXLEN);
   memset(value, 0, CTMDW_PACKET_MAXLEN);
   if (VOS_RET_SUCCESS != ctmdw_getAllMDWNotifications((void *)value))
   {
        vosLog_error("ctmdw_getAllMDWNotifications run failed!");
   }

   len = 0;
   buf[0] = CTMDW_OPTCODE_PARAINFORM;
   buf[1] = 1;
   len += 2;
   
   buf[2] = CTMDW_TYPE_PARAVALUES;
   len++;
/* Why value+1 or util_strlen(value)-1, it is because ctmdw_getAllNotifications will return:
 & paraname1=value1&paraname2=value2, it has one more '&'
*/
   if(util_strlen(value) == 0)
      return;
   
   *((unsigned short *)(buf + len)) = htonl(util_strlen(value)-1);
   len += 2;

   memcpy(buf+len, value+1, util_strlen(value));
   len += util_strlen(value)-1;

   CTMDW_DEBUG("ctmdw_sendInform: buf=%s len=%d\n" , buf+5, len); 

   sendto_ctmdw_server(buf, len);
}

void ctmdw_sendChangeInform(void)
{
   unsigned short len = 0;
   char value[CTMDW_PACKET_MAXLEN];
   char buf[CTMDW_PACKET_MAXLEN];
   
   memset(buf, 0, CTMDW_PACKET_MAXLEN);
   memset(value, 0, CTMDW_PACKET_MAXLEN);
   
   ctmdw_getAllITMSNotifications((void *)value);
   

   len = 0;
   buf[0] = CTMDW_OPTCODE_PARACHANGEINFORM;
   buf[1] = 1;
   len += 2;
   
   buf[2] = CTMDW_TYPE_PARAVALUES;
   len++;
/* 
 Why value+1 or util_strlen(value)-1, it is because ctmdw_getAllNotifications will return:
 "&paraname1=value1&paraname2=value2", it has one more '&'
*/
   if(util_strlen(value) == 0)
      return;
   
   *((unsigned short *)(buf + len)) = htonl(util_strlen(value)-1);
   len += 2;

   memcpy(buf+len, value+1, util_strlen(value));
   len += util_strlen(value)-1;

   CTMDW_DEBUG("ctmdw_sendChangeInform: buf=%s len=%d\n" , buf+5, len); 

   sendto_ctmdw_server(buf, len);
}

/* Hard coding this stuff, since I'm in us. */
void ctmdw_sendCTAccountChangeInform(void)
{

   unsigned short len = 0;
   char buf[CTMDW_PACKET_MAXLEN];
   //int bufsz = 0;
   int paraLen = util_strlen("CTEvent=ACCOUNTCHANGE");
   
   memset(buf, 0, CTMDW_PACKET_MAXLEN);   
   len = 0;
   buf[0] = CTMDW_OPTCODE_PARACHANGEINFORM;
   buf[1] = 1;
   len += 2;
   
   buf[2] = CTMDW_TYPE_PARAVALUES;
   len++;
   
   *((unsigned short *)(buf + len)) = htonl(paraLen);
   len += 2;

   strncpy(buf+len, "CTEvent=ACCOUNTCHANGE", paraLen);
   len += paraLen;

   CTMDW_DEBUG("ctmdw_sendCTAccountChangeInform: buf=%s len=%d\n" , buf+5, len); 

   sendto_ctmdw_server(buf, len);
}

void ctmdw_sendCTBindInform(void)
{
#if 0
   unsigned short len = 0;
   char buf[CTMDW_PACKET_MAXLEN];
   int bufsz = 0;
   int paraLen = 0;
   TRxObjNode  *n, *m;
   char strEvent[]  = "CTEvent=BIND";
   //char strUserId[] = "InternetGatewayDevice.ManagementServer.URL";
   char strUserId[] = "InternetGatewayDevice.X_CT-COM_UserInfo.UserId";
   char strInternetPvc[] = "InternetGatewayDevice.ManagementServer.InternetPvc";
   char strInternetPvcValue[CTMDW_PACKET_MAXLEN];
   char strUsername[] = ".Username";
   char* strWanPath = NULL;
   char* strWanPathNext = NULL;
   char strWanFullPath[CTMDW_PACKET_MAXLEN];
   
   memset(buf, 0, CTMDW_PACKET_MAXLEN);   
   len = 0;
   buf[0] = CTMDW_OPTCODE_PARACHANGEINFORM;
   buf[1] = 1;
   len += 2;
   
   buf[2] = CTMDW_TYPE_PARAVALUES;
   len++;

   //skip len
   len += 2;

   paraLen = util_strlen(strEvent);
   strncpy(buf+len, strEvent, paraLen);
   len += paraLen;

   //"&InternetGatewayDevice.X_CT-COM_UserInfo.UserId=xxxxxxx"
   n = findGWParameter(strUserId);
   if(n && n->getTRxParam != NULL)
   {
      char *tmpval = NULL; 
      if ( n->getTRxParam(&tmpval) == TRX_OK )
      {
         strncpy(buf + len, "&" , 1);
         len++;

         strncpy(buf + len, strUserId, util_strlen(strUserId));
         len += util_strlen(strUserId);

         strncpy(buf + len, "=", 1);
         len++;

         strncpy(buf + len, tmpval, util_strlen(tmpval));
         len += util_strlen(tmpval);         
         VOS_FREE(tmpval);
      }
      else
      {
         CTMDW_DEBUG("ctmdw: ctmdw_sendCTBindInform can't get %s value\n", strUserId);
         return;
      }
   }
   else 
   {
      CTMDW_DEBUG("ctmdw: ctmdw_sendCTBindInform can't find %s\n", strUserId);
      return;
   }

   //"&WAN{i}.PPP{j}.Username=XXX&WAN{k}.PPP{l}.Username=YYY"
   n = NULL;
   n = findGWParameter(strInternetPvc);
   if(n && n->getTRxParam != NULL)
   {
      char *tmpval = NULL;
      char *tmpval2 = NULL;
      if ( n->getTRxParam(&tmpval)==TRX_OK)
      {
         if(strcmp(tmpval, "NULL") != 0)
         {
            //WAN{i}.PPP{j} or WAN{i}.PPP{j}$WAN{k}.PPP{l}
            strWanPathNext = tmpval;
            do
            {
               strWanPath = strWanPathNext;
               strWanPathNext = strstr(strWanPathNext, "$");
               if(strWanPathNext != NULL)
               {
                  *strWanPathNext = 0;
                  strWanPathNext++;
               }

               strWanFullPath[0] = 0;
               if(mappingCTNameToTR69Name(strWanPath, strWanFullPath) == CTMDW_STATUS_OK)
               {
                  UTIL_STRNCAT(strWanFullPath, strUsername);
                  m = findGWParameter(strWanFullPath);
                  if(m && m->getTRxParam != NULL)
                  {
                     tmpval2 = NULL; 
                     if ( m->getTRxParam(&tmpval2) == TRX_OK )
                     {
                        strncpy(buf + len, "&", 1);
                        len++;

                        //WAN{i}.PPP{j}
                        paraLen = util_strlen(strWanPath);
                        strncpy(buf + len, strWanPath, paraLen);
                        len += paraLen;
                        
                        //.Username
                        paraLen = util_strlen(strUsername);
                        strncpy(buf + len, strUsername, paraLen);
                        len += paraLen;
  
                        strncpy(buf + len, "=", 1);
                        len++;

                        paraLen = util_strlen(tmpval2);
                        strncpy(buf + len, tmpval2, paraLen);
                        len += paraLen;         
                        VOS_FREE(tmpval2);
                     }
                     else
                     {
                        CTMDW_DEBUG("ctmdw: ctmdw_sendCTBindInform can't get %s value\n", strWanFullPath);
                     }
                  }
                  else 
                  {
                     CTMDW_DEBUG("ctmdw: ctmdw_sendCTBindInform can't find %s\n", strWanFullPath);
                  }

               }
            }while(strWanPathNext != NULL);
         }
         VOS_FREE(tmpval);
      }
      else
      {
         CTMDW_DEBUG("ctmdw: ctmdw_sendCTBindInform can't get %s value\n", strInternetPvc);
      }
   }
   else 
   {
      CTMDW_DEBUG("ctmdw: ctmdw_sendCTBindInform can't find %s\n", strInternetPvc);
   }

   *((unsigned short *)(buf + 3)) = htonl(len - 5);

   CTMDW_DEBUG("ctmdw_sendCTAccountChangeInform: buf=%s len=%d\n" , buf+5, len);
   sendto_ctmdw_server(buf, len);
#else
   return;
#endif
}

static CTMDW_STATUS ctmdw_sendSetAttributeResponse(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   char           bReqSuccess = TRUE;
   char           bReqPartSuccess = FALSE;
   int            i;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_PARAATTRIBUTES && tlv->actRetVal != CTMDW_ACTION_OK)
         bReqSuccess = FALSE;
      if(tlv->type == CTMDW_TYPE_PARAATTRIBUTES && tlv->actRetVal == CTMDW_ACTION_OK_PART)
         bReqPartSuccess = TRUE;
   }
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_PARAATTRIBUTESET_RET;
   pBuf++;
   
   
   /* retcode only */
   // Number of Value
   *((unsigned char*)pBuf) = bReqPartSuccess ? 2 : 1;
   pBuf++;
   
   // type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   pBuf++;
   
   if(bReqPartSuccess)
   {
      // len
      len = util_strlen(CTMDW_RETCODE_OK_PART);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      strncpy(pBuf, CTMDW_RETCODE_OK_PART, len);
      pBuf += len;

      // paraNames success
      *((unsigned char*)pBuf) = CTMDW_TYPE_PARAMETERNAMES;
      pBuf++;

      //len
      len  = util_strlen(ctmdw_msg->tvlLst->rvalue);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;

      //para names   
      strncpy(pBuf, ctmdw_msg->tvlLst->rvalue, len);
      pBuf += len;    
   }
   else if (!bReqSuccess)
   {
      // len
      len = util_strlen(CTMDW_RETCODE_ERR);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      strncpy(pBuf, CTMDW_RETCODE_ERR, len);
      pBuf += len;
   
   }
   else
   {
      // len
      len = util_strlen(CTMDW_RETCODE_OK);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      strncpy(pBuf, CTMDW_RETCODE_OK, len);
      pBuf += len;  
   }

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);
   return ret;
}

static CTMDW_STATUS ctmdw_sendGetAttributeResponse(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   char           bReqSuccess = TRUE;
   char           bOutOfBuffer = FALSE;
   int            i;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_PARAMETERNAMES && tlv->actRetVal != CTMDW_ACTION_OK)
         bReqSuccess = FALSE;
      if(tlv->type == CTMDW_TYPE_PARAMETERNAMES && tlv->actRetVal == CTMDW_ACTION_ERR_OUTOFRANGE)
         bOutOfBuffer = TRUE;
   }
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_PARAATTRIBUTEGET_RET;
   pBuf++;
   
   if (!bReqSuccess)
   {
      /* retcode only */
      // Number of Value
      *((unsigned char*)pBuf) = 1;
      pBuf++;
      
      // type
      *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
      pBuf++;

      // len
      if(bOutOfBuffer)
         len = util_strlen(CTMDW_RETCODE_ERR_DATALONG);
      else
         len = util_strlen(CTMDW_RETCODE_ERR_PARAM);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      if(bOutOfBuffer)
         strncpy(pBuf, CTMDW_RETCODE_ERR_DATALONG, len);
      else         
         strncpy(pBuf, CTMDW_RETCODE_ERR_PARAM, len);
      pBuf += len;
   }
   else
   {
      /* retcode and values */
      // Number of Value
      *((unsigned char*)pBuf) = (unsigned char)(ctmdw_msg->numOfTLV);
      pBuf++;
      
      // type
      *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
      pBuf++;

      // len
      len = util_strlen(CTMDW_RETCODE_OK);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // ok info
      strncpy(pBuf, CTMDW_RETCODE_OK, len);
      pBuf += len;

      // tlv
      for( i = 0; i < ctmdw_msg->numOfTLV; i++)
      {  
         tlv = ctmdw_msg->tvlLst + i;
         if(tlv->type == CTMDW_TYPE_PARAMETERNAMES)
         {
            if ((pBuf - buf + 3) >= CTMDW_PACKET_MAXLEN)
            {
               bOutOfBuffer = TRUE;
               break;
            }
            
            // type
            *((unsigned char*)pBuf) = CTMDW_TYPE_PARAATTRIBUTES;
            pBuf++;

            // len
            len = util_strlen(tlv->rvalue);
            *((unsigned short*)pBuf) = htons(len);
            pBuf += 2;

            if ((pBuf - buf + len) > CTMDW_PACKET_MAXLEN)
            {
               bOutOfBuffer = TRUE;
               break;
            }

            // data
            strncpy(pBuf, tlv->rvalue, len);
            pBuf += len;           
         }
      }

      if(bOutOfBuffer == TRUE)
      {
         /* retcode only out of range*/
         // Number of Value
         pBuf = buf + 1;
         *((unsigned char*)pBuf) = 1;
         pBuf++;
         
         // type
         *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
         pBuf++;
   
         // len
         len = util_strlen(CTMDW_RETCODE_ERR_DATALONG);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;
         
         // err info
         strncpy(pBuf, CTMDW_RETCODE_ERR_DATALONG, len);
         pBuf += len;
      }
   }

   bufOffset = pBuf - buf;
   sendto_ctmdw_server(buf, bufOffset);
   return ret;
}

static CTMDW_STATUS ctmdw_sendFileName(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   int            i;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_FILEGET_RET;
   pBuf++;

   /* retcode only */
   // Number of Value
   *((unsigned char*)pBuf) = ctmdw_msg->numOfTLV;
   pBuf++;
     
   // tlv
   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_PARAVALUES)
      {
         // type
         *((unsigned char*)pBuf) = CTMDW_TYPE_PARAVALUES;
         pBuf++;

         // len
         len = util_strlen(tlv->rvalue);
         *((unsigned short*)pBuf) = htons(len);
         pBuf += 2;

         // data
         strncpy(pBuf, tlv->rvalue, len);
         pBuf += len;           
      }
   }  

   bufOffset = pBuf - buf;
   sendto_ctmdw_server(buf, bufOffset);

   return ret;
}

void ctmdw_sendSetDefaultRet(void)
{
   char           buf[CTMDW_PACKET_MAXLEN];
   char           *pBuf = NULL;
   char           bufOffset = 0;
   unsigned short len = 0;

   pBuf = buf;
   
   //Optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_SETDEFAULT_RET;
   pBuf++;

   *((unsigned char*)pBuf) = 1;
   pBuf++;

   /* ModName TLV */
   //type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   pBuf++;

   //length
   len = util_strlen(CTMDW_RETCODE_OK);
   *((unsigned short*)pBuf) = htons(len);
   pBuf += 2;

   //value
   *pBuf = 0;
   UTIL_STRNCAT(pBuf, CTMDW_RETCODE_OK, sizeof(buf));
   pBuf += len;

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);
}

static CTMDW_STATUS ctmdw_sendAddObjectRet(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   int            i;
   char           bReqSuccess = TRUE;
   char           bReboot = FALSE;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_OBJECT && (tlv->actRetVal != CTMDW_ACTION_OK && tlv->actRetVal != CTMDW_ACTION_OK_REBOOT))
         bReqSuccess = FALSE;

      if(tlv->type == CTMDW_TYPE_OBJECT && tlv->actRetVal == CTMDW_ACTION_OK_REBOOT)
         bReboot = TRUE;
   }
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_ADDOBJECT_RET;
   pBuf++;

   /* retcode only */
   // Number of Value
   *((unsigned char*)pBuf) = bReqSuccess ? 2 : 1;
   pBuf++;
     
   // type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   pBuf++;

   if (!bReqSuccess)
   {
      // len
      len = util_strlen(CTMDW_RETCODE_ERR);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      strncpy(pBuf, CTMDW_RETCODE_ERR, len);
      pBuf += len;     
   }
   else
   {
      // len
      if(bReboot)
         len = util_strlen(CTMDW_RETCODE_OK_REBOOT);
      else
         len = util_strlen(CTMDW_RETCODE_OK);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // OK info
      if(bReboot)
         strncpy(pBuf, CTMDW_RETCODE_OK_REBOOT, len);
      else         
         strncpy(pBuf, CTMDW_RETCODE_OK, len);
      pBuf += len; 

      // instance
      *((unsigned char*)pBuf) = CTMDW_TYPE_INSTANCE;
      pBuf++;

      //len
      len  = util_strlen(ctmdw_msg->tvlLst->rvalue);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;

      //para names   
      strncpy(pBuf, ctmdw_msg->tvlLst->rvalue, len);
      pBuf += len; 
   }
   
   bufOffset = pBuf - buf;
   sendto_ctmdw_server(buf, bufOffset);

   return ret;
}

static CTMDW_STATUS ctmdw_sendDeleteObjectRet(PCTMDW_MSG ctmdw_msg)
{
   CTMDW_STATUS   ret = CTMDW_STATUS_OK;
   char           buf[CTMDW_PACKET_MAXLEN], *pBuf = NULL;
   int            bufOffset = 0;
   unsigned short len = 0;
   PCTMDW_TLV     tlv;
   int            i;
   char           bReqSuccess = TRUE;
   char           bReboot = FALSE;

   if(ctmdw_msg == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;

   for( i = 0; i < ctmdw_msg->numOfTLV; i++)
   {  
      tlv = ctmdw_msg->tvlLst + i;
      if(tlv->type == CTMDW_TYPE_OBJECT && tlv->actRetVal != CTMDW_ACTION_OK && tlv->actRetVal != CTMDW_ACTION_OK_REBOOT)
         bReqSuccess = FALSE;

      if(tlv->type == CTMDW_TYPE_OBJECT && tlv->actRetVal == CTMDW_ACTION_OK_REBOOT)
         bReboot = TRUE;
   }
     
   pBuf = buf;

   //optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_DELETEOBJECT_RET;
   pBuf++;

   /* retcode only */
   // Number of Value
   *((unsigned char*)pBuf) = 1;
   pBuf++;
     
   // type
   *((unsigned char*)pBuf) = CTMDW_TYPE_RETCODE;
   pBuf++;

   if (!bReqSuccess)
   {
      // len
      len = util_strlen(CTMDW_RETCODE_ERR);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // err info
      strncpy(pBuf, CTMDW_RETCODE_ERR, len);
      pBuf += len;     
   }
   else
   {
      // len
      if(bReboot)
         len = util_strlen(CTMDW_RETCODE_OK_REBOOT);
      else
         len = util_strlen(CTMDW_RETCODE_OK);
      *((unsigned short*)pBuf) = htons(len);
      pBuf += 2;
      
      // OK info
      if(bReboot)
         strncpy(pBuf, CTMDW_RETCODE_OK_REBOOT, len);
      else
         strncpy(pBuf, CTMDW_RETCODE_OK, len);
      pBuf += len; 
   }
   
   bufOffset = pBuf - buf;
   sendto_ctmdw_server(buf, bufOffset);

   return ret;
}

void ctmdw_sendOperation(void)
{
   char           buf[CTMDW_PACKET_MAXLEN];
   char           *pBuf = NULL;
   char           bufOffset = 0;
   unsigned short len = 0;
   char           value[16];

   if(g_diagState == CTMDW_DIAGNOSTIC_NONE)
      return;

   if(g_diagState == CTMDW_DIAGNOSTIC_PING)
      UTIL_STRNCPY(value, "1", sizeof(value));
   else if(g_diagState == CTMDW_DIAGNOSTIC_ATM)
      UTIL_STRNCPY(value, "2", sizeof(value));
   else
      UTIL_STRNCPY(value, "3", sizeof(value));

   pBuf = buf;
   
   //Optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_OPERATIONDONE;
   pBuf++;

   //NumOfTLV(ModName, ParaValues)
   *((unsigned char*)pBuf) = 1;
   pBuf++;

   /* ModName TLV */
   //type
   *((unsigned char*)pBuf) = CTMDW_TYPE_OPERATION;
   pBuf++;

   //length
   len = util_strlen(value);
   *((unsigned short*)pBuf) = htons(len);
   pBuf += 2;

   //value
   *pBuf = 0;
   UTIL_STRNCAT(pBuf, value, sizeof(buf));
   pBuf += len;

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);
}

#if 0   
static CTMDW_STATUS ctmdw_checkReboot(void)
{
  char* buf;
   uint32 bufLen;
   if(BcmCfm_scratchPadGet(CTMDW_REBOOT_TOKEN_ID, (void *)&buf, &bufLen) == BcmCfm_Ok)
   {
      if (!strcmp(buf, CTMDW_REBOOT_SENDRET))
      {
         ctmdw_sendRebootRet();
         BcmCfm_scratchPadSet(CTMDW_REBOOT_TOKEN_ID, CTMDW_REBOOT_NOTSENDRET, strlen(CTMDW_REBOOT_NOTSENDRET) + 1);
      }
      BcmCfm_free(buf);
   }
}
#endif

int ctmdw_calcPkt(char* buf, int bufLen)
{
   int            ret = -1;
   char*          pPkt = NULL;
   int            pktOffset = 0;
   int            optCode = -1;
   int            numOfTLV = -1;
   int            i;
   int            type, length;

   if ( buf == NULL || bufLen <= 0)
      return ret;

   CTMDW_DEBUG("ctmdw: calcCtMDWPkt ...ok!\n");

   pPkt = buf;
   pktOffset = 0;

   //optCode
   optCode = *((unsigned char *)pPkt);
   if(validateOptCode(optCode) != CTMDW_STATUS_OK)
      return ret;
   
   pktOffset++;
   pPkt = buf + pktOffset;

   
   //numOfTLV
   numOfTLV = *((unsigned char *)pPkt);
   pktOffset++;
   pPkt = buf + pktOffset;

   if(pktOffset > bufLen)
      return ret;
   
   //TLV
   for(i = 0; i < numOfTLV; i++)
   {   
      //TLV type;
      type = *((unsigned char *)pPkt);
      if(validateType(type) != CTMDW_STATUS_OK)
         return ret;
      pktOffset++;
      pPkt = buf + pktOffset;

      //TLV len
      length = ntohs(*((unsigned short *)pPkt));
      pktOffset += 2;
      pPkt = buf + pktOffset;

      //TLV value
      pktOffset += length;
      pPkt = buf + pktOffset;

      //TLV check boundary
      if(pktOffset > bufLen)
         return ret;
   }

   if(pktOffset > bufLen)
      return ret;

   return ret = pktOffset;
}


void ctmdw_rebootcb(void* handle)
{
   CTMDW_DEBUG("ctmdw: ctmdw_rebootcb....\n");
   ctmdw_doReboot();
}

int ctmdw_getCTMDWEnable(void)
{
    if(enblCTMiddleware == CTMDW_MODE_0 || enblCTMiddleware == CTMDW_MODE_2)
        return 1;
    else
        return 0;
}

void ctmdw_sendMode2Inform(void)
{

   unsigned short len = 0;
   char value[CTMDW_PACKET_MAXLEN];
   char buf[CTMDW_PACKET_MAXLEN];
   int i=0;
   VOS_RET_E fault=0;
   char *paramValue = NULL;
   static char* Mode2InformParam[] = {
   "InternetGatewayDevice.DeviceInfo.X_CT-COM_MiddlewareMgt.MiddlewareURL",
   "InternetGatewayDevice.ManagementServer.CTMgtIPAddress",
   "InternetGatewayDevice.ManagementServer.MgtDNS",
   "InternetGatewayDevice.ManagementServer.InternetPvc",
   "InternetGatewayDevice.ManagementServer.CTUserIPAddress.", 
   }; 

   memset(buf, 0, CTMDW_PACKET_MAXLEN);
   memset(value, 0, CTMDW_PACKET_MAXLEN);   
   
   for(i=0; i<5;i++)
   {
        /* this is a parameter path */
    fault = tr69c_getParamValue(Mode2InformParam[i], &paramValue);
    if (fault == VOS_RET_SUCCESS)
    {
        char tmpbuf[512];
        char tmpctname[512];

        memset(tmpbuf, 0, 512);
        memset(tmpctname, 0, 512);
    
        if(CTMDW_STATUS_OK != mappingTR69NameToCTName(Mode2InformParam[i], tmpctname, sizeof(tmpctname)))
        {
            UTIL_STRNCPY(tmpctname, Mode2InformParam[i], sizeof(tmpctname));
        }

        UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "&%s=%s", tmpctname, paramValue);
        UTIL_STRNCAT(value, tmpbuf, sizeof(value));
        
        VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
    }
   }
  
   len = 0;
   buf[0] = CTMDW_OPTCODE_PARAINFORM;
   buf[1] = 1;
   len += 2;
   
   buf[2] = CTMDW_TYPE_PARAVALUES;
   len++;
/* Why value+1 or strlen(value)-1, it is because ctmdw_getAllNotifications will return:
 & paraname1=value1&paraname2=value2, it has one more '&'
*/
   if(util_strlen(value) == 0)
      return;
   
   *((unsigned short *)(buf + len)) = htonl(util_strlen(value)-1);
   len += 2;

   memcpy(buf+len, value+1, util_strlen(value));
   len += util_strlen(value)-1;

   CTMDW_DEBUG("ctmdw_sendInform: buf=%s len=%d\n" , buf+5, len); 

   sendto_ctmdw_server(buf, len);
}

void ctmdw_sendMWExit(void)
{
   char           buf[CTMDW_PACKET_MAXLEN];
   char           *pBuf = NULL;
   char           bufOffset = 0;

   pBuf = buf;
   
   //Optcode
   *((unsigned char*)pBuf) = CTMDW_OPTCODE_MWEXIT;
   pBuf++;

   *((unsigned char*)pBuf) = 0;
   pBuf++;   

   // cale offset
   bufOffset = pBuf - buf;
   
   sendto_ctmdw_server(buf, bufOffset);
}

void getPidName(SINT32 pid, char *s, UINT32 slen)
{
   char filename[256];
   char process_name[32];
   int rc, i, process_pid = 0;
   char state;
   FILE *fp;

   //memset(pidInfo, 0, sizeof(PidInfo));

   snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);

   if ((fp = fopen(filename, "r")) == NULL)
   {
      vosLog_error("getPidName could not open %s", filename);
      return;
   }

   /* get the name */
   rc = fscanf(fp, "%d (%s %c", &process_pid, process_name, &state);
   i = util_strlen(process_name);
   if (process_name[i-1] == ')') 
   {
      process_name[i-1] = '\0';
   }
   vosLog_debug("fscanf of stat returned %s[%d]", process_name, process_pid);
   UTIL_STRNCPY(s, process_name, slen);

   fclose(fp);

}

int findPidByName(char *s)
{
   DIR *dir;
   struct dirent *dent;
   int i=0, pid = 0;
   VOS_RET_E ret;
   char name[32] = "\0";

   //memset(pidInfoArray, 0, PID_INFO_ARRAY_LENGTH * sizeof(PidInfo));

   dir = opendir("/proc");
   while ((dent = readdir(dir)) != NULL)
   {
      vosLog_debug("readdir: %s type=%d", dent->d_name, dent->d_type);

      /*
       * each process has its own directory under /proc, the name of the
       * directory is the pid number.  On the modem, the d_type is not
       * properly reported, so we must rely on the number conversion to
       * tell us if the entry is a pid directory or just a file.
       */
      /*      if (dent->d_type == DT_DIR) */
      {
         ret = util_strtol(dent->d_name, NULL, 10, &pid);
         if (ret == VOS_RET_SUCCESS)
         {
            getPidName(pid, name, sizeof(name));

        if(strcmp(name, "ctadmin") == 0){
        vosLog_debug("index=%d pid=%d cmdline=%s", i, pid, name);
          closedir(dir);
        return pid;
        }

        //UTIL_STRNCPY(name, "\0");
        name[0] = '\0';
            
            i++;  /* now that we've filled the array, we can increment index */
         }
#if 0
         if (i >= 50)
         {
            vosLog_error("too many pids to report on, increase PID_INFO_ARRAY_LENGTH(%d)", 50);
            break;
         }
#endif
      }
   }

   closedir(dir);
   return pid;
}

void ctmdw_stopCTMDWClient(void* handle)
{
    char cmd[60];
    int pid = 0;//bcmGetPid("/usr/local/ct/ctadmin s=2");

    pid = findPidByName("ctadmin");
#if 0
    FILE *pidfile;
    char line[6];

    pidfile = fopen("/var/ct/ctmdwpid", "r");
    if (pidfile != NULL) {
    fgets(line, 6, pidfile);
    sscanf(line,"%d",&pid);
    fclose(pidfile);
    } 
    else
    {
       CTMDW_DEBUG("ctmdw: ctmdw_stopCTMDWClient: can't find ct middleware pid\n");     
    return;
    }
#endif
    if(g_ctmdwFd != -1)
    {
        stopListener(g_ctmdwFd);   
        close(g_ctmdwFd);
        g_ctmdwFd = -1;
    }
    enblCTMiddleware = CTMDW_MODE_1;    

    if ( pid > 0 ) {
        UTIL_SNPRINTF(cmd, sizeof(cmd), "kill %d", pid);
        prctl_runCommandInShellWithTimeout(cmd);    
    }
#if 0
    pidfile = fopen("/var/ct/ctmdwpid", "w");
    if (pidfile != NULL) {
                fprintf(pidfile, "%d\n", 0);
                fclose(pidfile);
    }
    else {
        printf("\nctmdw_stopCTMDWClient:ct middleware pid file open error\n");
    }
#endif

    prctl_runCommandInShellWithTimeout("echo 0 > /var/ct/status");
    prctl_runCommandInShellWithTimeout("echo 1 > /var/ct/mode");
    CTMDW_DEBUG("ctmdw: ctmdw_stopCTMDWClient: ct mdw socket is closed\n");     
}

void ctmdw_startCTMDWClient(void* handle)
{
    struct stat statbuf;
#if 0
    FILE *pidfile;
    int  ctmdw_pid = 0;
    char line[6];
#endif  
    if(stat("/usr/local/ct/ctadmin", &statbuf))
        prctl_runCommandInShellWithTimeout("cp /bin/ctadmin /usr/local/ct -f");
    
    if (!stat("/usr/local/ct/ctadmin", &statbuf))
    {
         enblCTMiddleware = CTMDW_MODE_2;
         prctl_runCommandInShellWithTimeout("echo 0 > /var/ct/status");
          prctl_runCommandInShellWithTimeout("echo 2 > /var/ct/mode"); 
         prctl_runCommandInShellWithTimeout("chmod ugo+x /usr/local/ct/ctadmin");
#if 0
      pidfile = fopen("/var/ct/ctmdwpid", "r");
     if (pidfile != NULL) {
            fgets(line, 6, pidfile);
        sscanf(line,"%d",&ctmdw_pid);
        fclose(pidfile);
        }   

    if(ctmdw_pid> 0)
    {
        printf("\nct middleware already run\n");
        return;
    }
    
    
    ctmdw_pid = fork();

        if (ctmdw_pid == -1) {
            printf("\nUnable to launch ct middleware process\n");
        return;
        }
    else if (ctmdw_pid == 0) {
        int i;
            char *argv[4];
        printf("\nThis is child process\n");
             /* close all of the child's other fd's */
            for (i=3; i <= 50; i++)
            {
                 close(i);
            }

            argv[0] = "sh";
            argv[1] = "-c";
        argv[2] = "/usr/local/ct/ctadmin s=2";
            argv[3] = 0;
            execv("/bin/sh", argv);
            vosLog_error("Should not have reached here!");
            exit(127);
        }
        else {
        /* Parent process */
        /* Save pid to file so that it can be shared with other processes. */
        printf("\nThis is parent process\n");
        pidfile = fopen("/var/ct/ctmdwpid", "w");
            if (pidfile != NULL) {
                    fprintf(pidfile, "%d\n", ctmdw_pid);
                    fclose(pidfile);
            }
        else {
            printf("\nct middleware pid file open error\n");
        }
        }
#endif
    }

    prctl_runCommandInShellWithTimeout("/usr/local/ct/ctadmin s=2");
        
      /* init CT middleware client*/
    initCTMdwClient();

#if 0
    ctmdw_sendMode2Inform();
#endif  
    CTMDW_DEBUG("ctmdw: ctmdw_startCTMDWClient\n");     
}

static CTMDW_STATUS ctmdw_client_handler(char *buf, int bufLen)
{
   PCTMDW_MSG ctmdw_msg = NULL;
   CTMDW_STATUS ret = CTMDW_STATUS_OK;
   int TWCodeValue = 0;

   if(buf == NULL)
      return ret = CTMDW_STATUS_ERR_PARAM;
   CTMDW_DEBUG("ctmdw: ctmdw_client_handler: param...ok\n");

   ctmdw_msg = alloc_ctmdwmsg();
   if(ctmdw_msg == NULL)
   {
      ret = CTMDW_STATUS_ERR_NOMEM;
      return ret;
   }
   else
      memset(ctmdw_msg, 0, sizeof(CTMDW_MSG));

   CTMDW_DEBUG("ctmdw: ctmdw_client_handler: alloc msg... ok! %p\n", ctmdw_msg); 

   //Generic Parsing
   ret = parseCTMDWMsgGeneric(buf, bufLen, ctmdw_msg);
   if(ret != CTMDW_STATUS_OK)
      goto clthandler_out;
   
   //Advance Parsing
   ret = parseCTMDWMsgAdvance(ctmdw_msg);
   if(ret != CTMDW_STATUS_OK)
      goto clthandler_out;

   CTMDW_DEBUG("ctmdw: ctmdw_client_handler state: %d, opcode: %d\n", g_ctmdwState, ctmdw_msg->opcode);
   if(g_ctmdwState == CTMDW_STATE_NOTINIT && ctmdw_msg->opcode != CTMDW_OPTCODE_REGISTER_OK)
      goto clthandler_out;

    switch(ctmdw_msg->opcode)
    {   
        case CTMDW_OPTCODE_REGISTER_OK:
            if(g_ctmdwState == CTMDW_STATE_NOTINIT && 
            !strncmp(ctmdw_msg->tvlLst->value, CTMDW_RETCODE_OK, util_strlen(CTMDW_RETCODE_OK)))
            {
                g_ctmdwState = CTMDW_STATE_REGISTERED;
#if 0
                ctmdw_checkReboot();
#endif
                CTMDW_DEBUG("ctmdw: ctmdw_client_handler state changed: %d\n", g_ctmdwState);
            }
            break;
        case CTMDW_OPTCODE_PARAMETERSET:
            TWCodeValue = g_twCodeValue;
            ctmdw_doParameterSet(ctmdw_msg);
            ctmdw_sendSetParameterResponse(ctmdw_msg);
         //cmcMgm_saveConfigToFlash();
            break;
        case CTMDW_OPTCODE_PARAINFORM_RET:
            break;
        case CTMDW_OPTCODE_PARAMETERGET:
            ctmdw_doParameterGet(ctmdw_msg);
            ctmdw_sendGetParameterResponse(ctmdw_msg);
            break;
        case CTMDW_OPTCODE_PARAATTRIBUTESET:
            ctmdw_doParameterAttrSet(ctmdw_msg);
            ctmdw_sendSetAttributeResponse(ctmdw_msg);
            break;
        case CTMDW_OPTCODE_PARAATTRIBUTEGET:
            ctmdw_doParameterAttrGet(ctmdw_msg);
            ctmdw_sendGetAttributeResponse(ctmdw_msg);
            break;
        case CTMDW_OPTCODE_PARACHANGEINFORM_RET:
            break;
        case CTMDW_OPTCODE_DOWNLOAD:
            ctmdw_doDownload(ctmdw_msg);
            break;
        case CTMDW_OPTCODE_UPLOAD:
            ctmdw_doUpload(ctmdw_msg);
            break;
        case CTMDW_OPTCODE_REBOOT:
            ctmdw_doReboot();
            break;
        case CTMDW_OPTCODE_FILEGET:
            ctmdw_doFileGet(ctmdw_msg);
            ctmdw_sendFileName(ctmdw_msg);
            break;
        case CTMDW_OPTCODE_SETDEFAULT:
            ctmdw_doSetDefault();
            ctmdw_sendSetDefaultRet();
            break;
        case CTMDW_OPTCODE_ADDOBJECT:
            ctmdw_doAddObject(ctmdw_msg);
            ctmdw_sendAddObjectRet(ctmdw_msg);
         //cmcMgm_saveConfigToFlash();
            break;
        case CTMDW_OPTCODE_DELETEOBJECT:
            ctmdw_doDeleteObject(ctmdw_msg);
            ctmdw_sendDeleteObjectRet(ctmdw_msg);
         //cmcMgm_saveConfigToFlash();
            break;
        case CTMDW_OPTCODE_GETPARANAMES:
            ctmdw_doGetParaNames(ctmdw_msg);
            ctmdw_sendGetParaNamesRet(ctmdw_msg);
            break;
        case CTMDW_OPTCODE_MWEXIT_RET:
            utilTmr_cancel(tmrHandle, ctmdw_stopCTMDWClient, 0);
            if(enblCTMiddleware == CTMDW_MODE_2)
                ctmdw_stopCTMDWClient(NULL);
        default:
            break;
    }
     
    ctmdw_saveConfigurations();
   
clthandler_out:
   //Free resource
   free_ctmdwmsg(ctmdw_msg);
   
   return ret;
}

void init_ctmdwDefNotification(void)
{
   ctmdw_doParameterAttrSetSimple("InternetGatewayDevice.ManagementServer.URL", "1", atoi(CTMDW_TW_MDW));
   ctmdw_doParameterAttrSetSimple("InternetGatewayDevice.DeviceInfo.X_CT-COM_MiddlewareMgt.MiddlewareURL", "1", atoi(CTMDW_TW_MDW));
   ctmdw_doParameterAttrSetSimple("InternetGatewayDevice.ManagementServer.CTMgtIPAddress", "1", atoi(CTMDW_TW_MDW));
   ctmdw_doParameterAttrSetSimple("InternetGatewayDevice.ManagementServer.MgtDNS", "1", atoi(CTMDW_TW_MDW));
   ctmdw_doParameterAttrSetSimple("InternetGatewayDevice.ManagementServer.InternetPvc", "1", atoi(CTMDW_TW_MDW));
   ctmdw_doParameterAttrSetSimple("InternetGatewayDevice.ManagementServer.CTUserIPAddress.", "1", atoi(CTMDW_TW_MDW));      
   ctmdw_saveConfigurations();
}

static void ctMdwListener(void *handle)
{
   int   fd = (int)handle;
   int   cnt;
   char  buf[CTMDW_PACKET_MAXLEN];
   int   pktLen = 0;
   char  *pkt;

   CTMDW_DEBUG("ctmdw: ctMdwListener: listener has been actived.\n");   
   cnt = recvfrom_ctmdw_server(buf, sizeof(buf));
   if(cnt <= 0)
   {
      if(cnt < 0)
      close(fd);
      return; 
   }
   CTMDW_DEBUG("ctmdw: ctMdwListener: recvd bytes %d.\n", cnt);
   
   pkt = buf;
   do {
      pkt = pkt + pktLen;
      pktLen = ctmdw_calcPkt(pkt, cnt);
      CTMDW_DEBUG("ctmdw: ctMdwListener: recvd pkt %d.\n", pktLen);

      if(pktLen <= 0)
         break;
      
      ctmdw_client_handler(pkt, pktLen);
      cnt -= pktLen;
   }while(cnt > 0);
}

void initCTMdwClient(void)
{
   g_ctmdwFd = init_ctmdw_socket();
#ifdef CTMDW_STREAM
   if(g_ctmdwFd != -1 && connect_ctmdw_server(CTMDW_UNIX_SOCK) == 0)
#else
   if(g_ctmdwFd != -1)
#endif
   {
      init_ctmdwDefNotification();
      ctmdw_sendRegister();
      setListener(g_ctmdwFd, ctMdwListener, (void *)g_ctmdwFd);
      
      prctl_runCommandInShellWithTimeout("echo 1 > /var/ct/status");

      CTMDW_DEBUG("ctmdw: initCTCClient: ct mdw socket is initialized %d\n", g_ctmdwFd);   
   }
   else
   {
      CTMDW_DEBUG("ctmdw: initCTCClient: ct mdw socket fail to initialize\n");   
   }
}


void unitCTMdwClient(void)
{

}
void initCTClient(void)
{
}

void unitCTClient(void)
{
}



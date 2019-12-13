#ifndef CT_MIDDLEWARE_H

#if 1
    #define CTMDW_DEBUG  printf
#else
    #define CTMDW_DEBUG(format, args...)
#endif
#define CTMDW_STREAM

/* unix domain name */
#define CTMDW_UNIX_SOCK "/var/ct/tmp/interface2sock"

/* reboot timeout*/
#define CTMDW_REBOOT_TIMEOUT  (10*1000)

/* max packet length */
#define CTMDW_PACKET_MAXLEN   2048

/* max param length */
#define CTMDW_PARAM_MAXLEN    1024

/* param delim */
#define CTMDW_PARAM_DELIM     "&"

/* value delim */
#define CTMDW_VALUE_DELIM     "="

/* value ret code */
#define CTMDW_RETCODE_OK            "200"
#define CTMDW_RETCODE_OK_PART       "202"
#define CTMDW_RETCODE_OK_REBOOT     "203"
#define CTMDW_RETCODE_ERR           "400"
#define CTMDW_RETCODE_ERR_PASSWORD  "402"
#define CTMDW_RETCODE_ERR_SERVER    "405"
#define CTMDW_RETCODE_ERR_FILE      "407"
#define CTMDW_RETCODE_ERR_PARAM     "412"
#define CTMDW_RETCODE_ERR_DATALONG  "420"
#define CTMDW_RETCODE_ERR_ACCESSDENIED "425"


/* value T/W */
#define CTMDW_TW_MDW          "2"
#define CTMDW_TW_WEBGUI       "1"
#define CTMDW_TW_TR69         "0"

/*value CTMDW mode*/
#define CTMDW_MODE_0	0  //CT Middleware Eabled, TR069 Disabled, BCM Tr069 is disable, Midware Tr069 is enable
#define CTMDW_MODE_1	1  //CT Middleware Disabled, TR069 Eabled
#define CTMDW_MODE_2	2  //CT Middleware Eabled, TR069 Eabled

/* value ATTRIBUTES */
#define CTMDW_ATTRIBUTES_ITMS_NOINFORM_READABLE           "0"
#define CTMDW_ATTRIBUTES_ITMS_REBOOTINFORM_READABLE       "1"
#define CTMDW_ATTRIBUTES_ITMS_INFORM_READABLE             "2"
#define CTMDW_ATTRIBUTES_ITMS_NOINFORM_WRITABLE           "4"
#define CTMDW_ATTRIBUTES_ITMS_REBOOTINFORM_WRITABLE       "5"
#define CTMDW_ATTRIBUTES_ITMS_INFORM_WRITABLE             "6"

#define CTMDW_ATTRIBUTES_MDW_NOINFORM            "0"
#define CTMDW_ATTRIBUTES_MDW_INFORM              "1"



/* value download */
#define CTMDW_DOWNLOAD_COMMANDKEY      "CommandKey"
#define CTMDW_DOWNLOAD_FILETYPE        "FileType"
#define CTMDW_DOWNLOAD_URL             "URL"
#define CTMDW_DOWNLOAD_USERNAME        "Username"
#define CTMDW_DOWNLOAD_PASSWORD        "Password"
#define CTMDW_DOWNLOAD_FILESIZE        "FileSize"
#define CTMDW_DOWNLOAD_TARGETFILENAME  "TargetFileName"
#define CTMDW_DOWNLOAD_DELAYSECONDS    "DelaySeconds"
#define CTMDW_DOWNLOAD_SUCCESSURL      "SuccessURL"
#define CTMDW_DOWNLOAD_FAILUREURL      "FailureURL"

#define CTMDW_DOWNLOAD_FILETYPE_FIRMWARE "1 Firmware Upgrade Image"
#define CTMDW_DOWNLOAD_FILETYPE_CONFIG   "3 Vendor Configuration File"

#define CTMDW_REBOOT_TOKEN_ID          "CTMDWREBOOT"
#define CTMDW_REBOOT_SENDRET           "1"
#define CTMDW_REBOOT_NOTSENDRET        "0"


/* opt code */
#define CTMDW_OPTCODE_REGISTER               61
#define CTMDW_OPTCODE_REGISTER_OK            62
#define CTMDW_OPTCODE_PARAMETERSET           63
#define CTMDW_OPTCODE_PARAMETERSET_RET       64
#define CTMDW_OPTCODE_PARAINFORM             65
#define CTMDW_OPTCODE_PARAINFORM_RET         66
#define CTMDW_OPTCODE_PARAMETERGET           67
#define CTMDW_OPTCODE_PARAMETERGET_RET       68
#define CTMDW_OPTCODE_PARAATTRIBUTESET       69
#define CTMDW_OPTCODE_PARAATTRIBUTESET_RET   70
#define CTMDW_OPTCODE_PARAATTRIBUTEGET       71
#define CTMDW_OPTCODE_PARAATTRIBUTEGET_RET   72
#define CTMDW_OPTCODE_PARACHANGEINFORM       73
#define CTMDW_OPTCODE_PARACHANGEINFORM_RET   74
#define CTMDW_OPTCODE_DOWNLOAD               75
#define CTMDW_OPTCODE_DOWNLOAD_RET           76
#define CTMDW_OPTCODE_REBOOT                 77
#define CTMDW_OPTCODE_OPERATIONDONE          78
#define CTMDW_OPTCODE_UPLOAD                 79
#define CTMDW_OPTCODE_UPLOAD_RET             80
#define CTMDW_OPTCODE_ADDOBJECT              81
#define CTMDW_OPTCODE_ADDOBJECT_RET          82
#define CTMDW_OPTCODE_DELETEOBJECT           83
#define CTMDW_OPTCODE_DELETEOBJECT_RET       84
#define CTMDW_OPTCODE_GETPARANAMES           85
#define CTMDW_OPTCODE_GETPARANAMES_RET       86
#define CTMDW_OPTCODE_SETDEFAULT             87
#define CTMDW_OPTCODE_SETDEFAULT_RET         88
#define CTMDW_OPTCODE_MWEXIT             89
#define CTMDW_OPTCODE_MWEXIT_RET             90
//reserve
#define CTMDW_OPTCODE_FILEGET                201
#define CTMDW_OPTCODE_FILEGET_RET            202
#define CTMDW_OPTCODE_REBOOT_RET             203


/*type code*/
#define CTMDW_TYPE_MODNAME                   0
#define CTMDW_TYPE_PARAMETERNAMES            4
#define CTMDW_TYPE_PARAVALUES                5
#define CTMDW_TYPE_RETCODE                   7
#define CTMDW_TYPE_PARAATTRIBUTES            8
#define CTMDW_TYPE_TW                        9
#define CTMDW_TYPE_OBJECT                    12
#define CTMDW_TYPE_INSTANCE                  13
#define CTMDW_TYPE_PARALIST                  15
#define CTMDW_TYPE_OPERATION                 16
#define CTMDW_TYPE_OPTION                    17

/*Predefine Mode Name */
#define CTMDW_REGISTER_MODNAME          "firmware"
#define CTMDW_REGISTER_HWVER            "HWVER"
#define CTMDW_REGISTER_SWVER            "SWVER"
#define CTMDW_REGISTER_OUI              "OUI"
#define CTMDW_REGISTER_VENDOR           "VENDOR"
#define CTMDW_REGISTER_SN               "SN"

/*Predefine config Name*/
#define CTMDW_CONFIG_FWCONFIG           "FWConfig"
#define CTMDW_LOG_FWLOG                 "FWLog"

/* operation state */
typedef enum
{
    CTMDW_DIAGNOSTIC_NONE = 0,
    CTMDW_DIAGNOSTIC_PING,
    CTMDW_DIAGNOSTIC_ATM,
    CTMDW_DIAGNOSTIC_DSL,
} CTMDW_DIAGNOSTIC_STATE;

/* download state */
typedef enum
{
    CTMDW_DOWNLOAD_STATE_IDLE = -1,
    CTMDW_DOWNLOAD_STATE_OK = 0,
    CTMDW_DOWNLOAD_STATE_ERR_FAILED,
    CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT,
    CTMDW_DOWNLOAD_STATE_ERR_CONNECT,
    CTMDW_DOWNLOAD_STATE_ERR_FILE,
} CTMDW_DOWNLOAD_STATE;

/* ctmdw client state */
typedef enum
{
    CTMDW_STATE_NOTINIT = -1,
    CTMDW_STATE_REGISTERED = 0
} CTMDW_STATE;

/*mapping table*/
typedef struct __MAPCTTOTR69TABLE
{
    char ctName[64];
    char tr69Name[64];
} MAPCTTOTR69TABLE, *PMAPCTTOTR69TABLE;

typedef enum
{
    CTMDW_STATUS_OK = 0,
    CTMDW_STATUS_ERR_GEN,
    CTMDW_STATUS_ERR_NOMEM,
    CTMDW_STATUS_ERR_FMT,
    CTMDW_STATUS_ERR_PARAM,
} CTMDW_STATUS;

/*ret value*/
typedef enum
{
    CTMDW_ACTION_OK,
    CTMDW_ACTION_OK_PART,
    CTMDW_ACTION_OK_REBOOT,
    CTMDW_ACTION_ERR_GEN,
    CTMDW_ACTION_ERR_FMT,
    CTMDW_ACTION_ERR_NOMEM,
    CTMDW_ACTION_ERR_PARAM,
    CTMDW_ACTION_ERR_OUTOFRANGE,
    CTMDW_ACTION_ERR_NOTACCESS,
} CTMDW_ACTION_RETVAL;

typedef struct __CTMDW_VAL
{
    char                    *name;     //tr69 data module fomatted name
    char                    *value;    //value for set and returned value for get
    CTMDW_ACTION_RETVAL     actRetVal; //status of value instance
} CTMDW_VAL, *PCTMDW_VAL;

typedef struct __CTMDW_TLV
{
    int                     type;      //tlv type
    int                     length;    //tlv length
    char                    *value;    //pointer to value string of incoming TLV
    int                     numofVal;  //number of value instance
    PCTMDW_VAL              valLst;    //array of value instance
    char                    *rvalue;   //pointer to return value string
    CTMDW_ACTION_RETVAL     actRetVal; //status of TLV instance
} CTMDW_TLV, *PCTMDW_TLV;

typedef struct CTMDW_MSG
{
    int                     opcode;    //optcode
    int                     numOfTLV;  //number of TLV instance
    PCTMDW_TLV              tvlLst;    //array of TVL instance
} CTMDW_MSG, *PCTMDW_MSG;

typedef struct
{
    char *paraName;
    char pvalue[256];
} CTMDW_INFORM_VALUE, *PCTMDW_INFORM_VALUE;

extern void initCTClient(void);
extern void unitCTClient(void);
extern void init_ctmdwDefNotification(void);

extern int enblCTMiddleware;
//extern CTMDW_STATUS ctmdw_doSetDefault(void);
extern void ctmdw_sendSetDefaultRet(void);
extern void ctmdw_rebootcb(void *handle);
void initCTMdwClient(void);
void ctmdw_sendInform(void);
void ctmdw_sendChangeInform(void);
void ctmdw_sendCTAccountChangeInform(void);
void ctmdw_sendCTBindInform(void);
void ctmdw_sendOperation(void);
CTMDW_STATUS mappingTR69NameToCTName(char *paraNameMapped, char *pPName, UINT32 nameLen);
extern int ctmdw_getCTMDWEnable(void);
extern void ctmdw_startCTMDWClient(void *handle);
extern void stopCTMDWTimer(void *handle);
extern void ctmdw_sendMode2Inform(void);
void ctmdw_stopCTMDWClient(void *handle);
void ctmdw_sendMWExit(void);
#endif

#ifndef __TR69C_API_H__
#define __TR69C_API_H__


#define UTIL_CLI_TR69_RESULT_FILE      "/tmp/logTr69"


typedef struct
{
    char name[BUFLEN_64];              /**< name of configuration file */
    char version[BUFLEN_16];           /**< version of configuration file */
    char date[BUFLEN_64];              /**< date when config is updated */
    char description[BUFLEN_256];      /**< description of config file */
} vendorConfigUpdateMsgBody;

typedef enum
{
    TR69C_DOWNLOAD_FILE_FORMAT_INVALID = 0, /**< invalid or unrecognized format */
    TR69C_DOWNLOAD_FILE_FORMAT_BROADCOM = 1, /**< broadcom image (with our header) */
    TR69C_DOWNLOAD_FILE_FORMAT_FLASH = 2,   /**< raw flash image */
    TR69C_DOWNLOAD_FILE_FORMAT_XML_CFG = 3, /**< CMS XML config file */
    TR69C_DOWNLOAD_FILE_XML_FORMAT_INVALID = 4  /**< invalid XML config file */
} TR69C_DOWNLOAD_FILE_FORMAT_T;

typedef struct
{
    char diagnosticsState[BUFLEN_32];  /**< Ping state: requested, none, completed... */
    char interface[BUFLEN_32];   /**< interface on which ICMP request is sent */
    char host[BUFLEN_32]; /**< Host -- either IP address form or hostName to send ICMP request to */
    UINT32 numberOfRepetitions; /**< Number of ICMP requests to send */
    UINT32    timeout;	/**< Timeout in seconds */
    UINT32    dataBlockSize;	/**< DataBlockSize  */
    UINT32    DSCP;	/**< DSCP */
    UINT32    successCount;	/**< SuccessCount */
    UINT32    failureCount;	/**< FailureCount */
    UINT32    averageResponseTime;	/**< AverageResponseTime */
    UINT32    minimumResponseTime;	/**< MinimumResponseTime */
    UINT32    maximumResponseTime;	/**< MaximumResponseTime */
    VosEntityId requesterId;
} TR69C_PING_STATUS_MSG_T;

#endif

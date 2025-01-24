/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
************************************************************************/

// #include "cms.h"
// #include "cms_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "cms_mem.h"



void *tmrHandle = NULL;

#define cmsLog_error(fmt,args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args )
#define cmsLog_notice(fmt,args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args )
#define cmsLog_debug(fmt,args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args )


#ifndef NULL
#define NULL  0
#endif


typedef signed char	      int8_t;
typedef unsigned char	    u_int8_t;
typedef short		     int16_t;
typedef unsigned short	   u_int16_t;
typedef int		         int32_t;
typedef unsigned int	   u_int32_t;
typedef unsigned long uint64_t;

typedef uint64_t   UINT64;

/** Signed 64 bit integer.
 */
typedef int64_t    SINT64;

/** Unsigned 32 bit integer. */
typedef uint32_t   UINT32;

/** Signed 32 bit integer. */
typedef int32_t    SINT32;

/** Unsigned 16 bit integer. */
typedef uint16_t   UINT16;

/** Signed 16 bit integer. */
typedef int16_t    SINT16;

/** Unsigned 8 bit integer. */
typedef uint8_t    UINT8;

/** Signed 8 bit integer. */
typedef int8_t     SINT8;


typedef enum
{
   CMSRET_SUCCESS              = 0,     /**<Success. */
   CMSRET_METHOD_NOT_SUPPORTED = 9000,  /**<Method not supported. */
   CMSRET_REQUEST_DENIED       = 9001,  /**< Request denied (no reason specified). */
   CMSRET_INTERNAL_ERROR       = 9002,  /**< Internal error. */
   CMSRET_INVALID_ARGUMENTS    = 9003,  /**< Invalid arguments. */
   CMSRET_RESOURCE_EXCEEDED    = 9004,  /**< Resource exceeded.
                                        *  (when used in association with
                                        *  setParameterValues, this MUST not be
                                        *  used to indicate parameters in error)
                                        */
   CMSRET_INVALID_PARAM_NAME   = 9005,  /**< Invalid parameter name.
                                        *  (associated with set/getParameterValues,
                                        *  getParameterNames,set/getParameterAtrributes)
                                        */
   CMSRET_INVALID_PARAM_TYPE   = 9006,  /**< Invalid parameter type.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_INVALID_PARAM_VALUE  = 9007,  /**< Invalid parameter value.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_SET_NON_WRITABLE_PARAM = 9008,/**< Attempt to set a non-writable parameter.
                                        *  (associated with setParameterValues)
                                        */
   CMSRET_NOTIFICATION_REQ_REJECTED = 9009, /**< Notification request rejected.
                                            *  (associated with setParameterAttributes)
                                            */
   CMSRET_DOWNLOAD_FAILURE     = 9010,  /**< Download failure.
                                         *  (associated with download or transferComplete)
                                         */
   CMSRET_UPLOAD_FAILURE       = 9011,  /**< Upload failure.
                                        *  (associated with upload or transferComplete)
                                        */
   CMSRET_FILE_TRANSFER_AUTH_FAILURE = 9012,  /**< File transfer server authentication
                                              *  failure.
                                              *  (associated with upload, download
                                              *  or transferComplete)
                                              */
   CMSRET_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,/**< Unsupported protocol for file
                                                    *  transfer.
                                                    *  (associated with upload or
                                                    *  download)
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_JOIN_MULTICAST = 9014,/**< File transfer failure,
                                                    *  unable to join multicast
                                                    *  group.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_CONTACT_FILE_SERVER = 9015,/**< File transfer failure,
                                                    *  unable to contact file server.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_ACCESS_FILE = 9016,/**< File transfer failure,
                                                    *  unable to access file.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_COMPLETE = 9017,/**< File transfer failure,
                                                    *  unable to complete download.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CORRUPTED = 9018,/**< File transfer failure,
                                                    *  file corrupted.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_AUTHENTICATION_ERROR = 9019,/**< File transfer failure,
                                                    *  file authentication error.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_TIMEOUT = 9020,/**< File transfer failure,
                                                    *  download timeout.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CANCELLATION_NOT_ALLOW = 9021,/**< File transfer failure,
                                                    *  cancellation not permitted.
                                                    */
   CMSRET_INVALID_UUID_FORMAT = 9022,/**< Invalid UUID Format
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_UNKNOWN_EE = 9023,/**< Unknown Execution Environment
                                                    * (associated with ChangeDUState)
                                                    */

   CMSRET_EE_DISABLED = 9024,/**< Execution Environment disabled
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_DU_EE_MISMATCH = 9025,/**< Execution Environment and Deployment Unit mismatch
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_DUPLICATE = 9026,/**< Duplicate Deployment Unit
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_SW_MODULE_SYSTEM_RESOURCE_EXCEEDED = 9027,/**< System resources exceeded
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_UNKNOWN = 9028,/**< Unknown Deployment Unit
                                                    * (associated with ChangeDUState:update/uninstall)
                                                    */
   CMSRET_DU_STATE_INVALID = 9029,/**< Invalid Deployment Unit State
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_DOWNGRADE_NOT_ALLOWED = 9030,/**< Invalid Deployment Unit Update, downgrade not permitted
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_VERSION_NOT_SPECIFIED = 9031,/**< Invalid Deployment Unit Update, version not specified
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_VERSION_EXISTED= 9032,/**< Invalid Deployment Unit Update, version already exists
                                                    * (associated with ChangeDUState:update)
                                                    */
   
   CMSRET_SUCCESS_REBOOT_REQUIRED = 9800, /**< Config successful, but requires reboot to take effect. */
   CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED = 9801,  /**<Success, but some unrecognized data was ignored. */
   CMSRET_SUCCESS_OBJECT_UNCHANGED = 9802,  /**<Success, furthermore object has not changed, returned by STL handler functions. */
   CMSRET_SUCCESS_APPLY_NOT_COMPLETE = 9803, /**< Config validated/commited, but requires more action to take effect. */
   CMSRET_NO_MORE_INSTANCES = 9804,     /**<getnext operation cannot find any more instances to return. */
   CMSRET_MDM_TREE_ERROR = 9805,         /**<Error during MDM tree traversal */
   CMSRET_WOULD_DEADLOCK = 9806, /**< Caller is requesting a lock while holding the same lock or a different one. */
   CMSRET_LOCK_REQUIRED = 9807,  /**< The MDM lock is required for this operation. */
   CMSRET_OP_INTR = 9808,      /**<Operation was interrupted, most likely by a Linux signal. */
   CMSRET_TIMED_OUT = 9809,     /**<Operation timed out. */
   CMSRET_DISCONNECTED = 9810,  /**< Communications link is disconnected. */
   CMSRET_MSG_BOUNCED = 9811,   /**< Msg was sent to a process not running, and the
                                 *   bounceIfNotRunning flag was set on the header.  */
   CMSRET_OP_ABORTED_BY_USER = 9812,  /**< Operation was aborted/discontinued by the user */
   CMSRET_FAIL_REBOOT_REQUIRED = 9813,  /**<Config failed, and now system is in a bad state requiring reboot. */
   CMSRET_ACCESS_DENIED = 9814,  /**< Data model access denied (no reason specified). */
   CMSRET_OPERATION_NOT_PERMITTED= 9815,  /**< Operation not permitted (errno EPERM) */
   CMSRET_RECURSION_ERROR = 9817,     /**< too many levels of recursion */
   CMSRET_OPEN_FILE_ERROR = 9818,     /**< open file error */
   CMSRET_EAGAIN_ERROR = 9820,        /**< socket write EAGAIN error */
   CMSRET_SOCKET_ERROR = 9821,        /**< socket error */
   CMSRET_KEY_GENERATION_ERROR = 9830,     /** certificate key generation error */
   CMSRET_INVALID_CERT_REQ = 9831,     /** requested certificate does not match with issued certificate */
   CMSRET_INVALID_CERT_SUBJECT = 9832,     /** certificate has invalid subject information */
   CMSRET_OBJECT_ALREADY_EXISTS = 9839,    /** The object already exists */
   CMSRET_OBJECT_NOT_FOUND = 9840,     /** failed to find object */
   CMSRET_OBJECT_SEQUENCE_ERROR = 9841, /**< Sequence number check failed. */

   CMSRET_INVALID_FILENAME = 9850,  /**< filename was not given for download */
   CMSRET_INVALID_IMAGE = 9851,     /**< bad image was given for download */
   CMSRET_INVALID_CONFIG_FILE = 9852,  /**< invalid config file was detected */
   CMSRET_CONFIG_PSI = 9853,         /**< old PSI/3.x config file was detected */
   CMSRET_IMAGE_FLASH_FAILED = 9854, /**< could not write the image to flash */
   CMSRET_RESOURCE_NOT_CONFIGURED = 9855, /**< requested resource is not configured/found */
   CMSRET_EE_UNPACK_ERROR = 9856,   /**< EE download unpack failure (associated with ChangeEEState) */
   CMSRET_EE_DUPLICATE = 9857,   /**< Duplicate Execution Environment (associated with ChangeEEState:install) */
   CMSRET_EE_UPDATE_DOWNGRADE_NOT_ALLOWED = 9858,/**< Invalid Execution Environment Update, downgrade not permitted
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_EE_UPDATE_VERSION_NOT_SPECIFIED = 9859,/**< Invalid Execution Environment Update, version not specified
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_EE_UPDATE_VERSION_EXISTED = 9860,/**< Invalid Execution Environment Update, version already exists
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_PMD_CALIBRATION_FILE_SUCCESS = 9861, /**<Success. PMD Calibration file was uploaded.*/
   CMSRET_PMD_TEMP2APD_FILE_SUCCESS = 9862, /**<Success. PMD Temp2Apd file was uploaded.*/
   CMSRET_MANIFEST_PARSE_ERROR = 9863,   /**< Manifest parse error. */
   CMSRET_USERNAME_IN_USE = 9864,   /**< Username in use. */
   CMSRET_ADD_USER_ERROR = 9865,    /**< Add user error. */
   CMSRET_DELETE_USER_ERROR = 9866, /**< Delete user error. */
   CMSRET_USER_NOT_FOUND = 9867,    /**<User not found. */
   CMSRET_SET_BUSGATE_POLICY_ERROR = 9868,   /**<Set busgate policy error. */
   CMSRET_OPERATION_IN_PROCESS = 9869,    /**<Operation is still in process. */
   CMSRET_CONVERSION_ERROR = 9870, /**< Error during data conversion. */
   CMSRET_PARENTEE_EE_VERSION_MISMATCH = 9871, /**<EE version is not supported by platform version (BEEP Version)*/
   CMSRET_INVALID_URL_FORMAT = 9872,/**< Invalid URL Format
                                      * (associated with ChangeDUState)
                                      */
   CMSRET_INVALID_PACKAGE_FILE = 9873, /**<Invalid openplat package file*/
   CMSRET_EE_START_ERROR = 9874, /**<Fail to start EE*/
   CMSRET_EU_START_ERROR = 9875, /**<Fail to start EU*/
   CMSRET_EE_MANIFEST = 9876, /**<Found EE manifest*/
   CMSRET_MANIFEST_NO_PRIVILEGE_INFO = 9877, /**<Manifest without privilege information*/
   CMSRET_UNINSTALL_IN_PROCESS = 9878,    /**<Uninstall operation is still in process. */
   CMSRET_UPGRADE_IN_PROCESS = 9879,    /**<Upgrade operation is still in process. */
   CMSRET_PMD_JSON_FILE_SUCCESS = 9880, /**<Success. PMD JSON file was uploaded.*/
   CMSRET_UNKNOWN_ERROR = 9899,  /**< Can't figure out the real cause.  Use sparingly. */
//================================================================================
//__HG_BGN__ Added by fangc, 2022-01-26
   CMSRET_TELNET_PERMISSION_CHANGE = 9902,/** <yangtai add 20180306 for telnet children process exit>**/
//__HG_END__ Added by fangc, 2022-01-26
//================================================================================
   CMSRET_EXCEED_MAX_LIMIT = 9920,/**/
} CmsRet;


typedef struct
{
   UINT32 sec;   /**< Number of seconds since some arbitrary point. */
   UINT32 nsec;  /**< Number of nanoseconds since some arbitrary point. */
} CmsTimestamp;


typedef void (*CmsEventHandler)(void*);

/** Internal event timer structure
 */
typedef struct cms_timer_event
{
   struct cms_timer_event *next;      /**< pointer to the next timer. */
   CmsTimestamp            expireTms; /**< Timestamp (in the future) of when this
                                       *   timer event will expire. */
   CmsEventHandler         func;      /**< handler func to call when event expires. */
   void *                  ctxData;   /**< context data to pass to func */
   char name[CMS_EVENT_TIMER_NAME_LENGTH]; /**< name of this timer */
   CmsTimestamp            pausedTms; /**< Timestamp of when this timer was paused */
} CmsTimerEvent;


/** Internal timer handle. */
typedef struct
{
   CmsTimerEvent *events;     /**< Singly linked list of events */
   UINT32         numEvents;  /**< Number of events in this handle. */
} CmsTimerHandle;


CmsRet cmsTmr_init(void **tmrHandle)
{

   (*tmrHandle) = cmsMem_alloc(sizeof(CmsTimerHandle), ALLOC_ZEROIZE);
   if ((*tmrHandle) == NULL)
   {
      cmsLog_error("could not malloc mem for tmrHandle");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   return CMSRET_SUCCESS;
}


void cmsTmr_cleanup(void **handle)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;

   while ((tmrEvent = tmrHandle->events) != NULL)
   {
      tmrHandle->events = tmrEvent->next;
      CMSMEM_FREE_BUF_AND_NULL_PTR(tmrEvent);
   }

   CMSMEM_FREE_BUF_AND_NULL_PTR((*handle));

   return;
}

/** This macro will evaluate TRUE if a is earlier than b */
#define IS_EARLIER_THAN(a, b) (((a)->sec < (b)->sec) || \
                               (((a)->sec == (b)->sec) && ((a)->nsec < (b)->nsec)))


void _cmsTmr_insert(CmsTimerHandle *tmrHandle, CmsTimerEvent *newEvent)
{
   CmsTimerEvent *currEvent, *prevEvent;
   /* 
    * Now we just need to insert it in the correct place in the timer handle.
    * We just insert the events in absolute order, i.e. smallest expire timer
    * at the head of the queue, largest at the end of the queue.  If the
    * modem is up long enough where timestamp rollover is an issue (139 years!)
    * cmsTmr_executeExpiredEvents and cmsTmr_getTimeToNextEvent will have to
    * be careful about where they pick the next timer to expire.
    */
   if (tmrHandle->numEvents == 0)
   {
      tmrHandle->events = newEvent;
   }
   else 
   {
      currEvent = tmrHandle->events;

      if (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms)) ||
         !((currEvent->pausedTms.sec == 0) && (currEvent->pausedTms.nsec == 0)))
      {
         /* queue at the head */
         newEvent->next = currEvent;
         tmrHandle->events = newEvent;
      }
      else
      {
         UBOOL8 done = FALSE;

         while (!done)
         {
            prevEvent = currEvent;
            currEvent = currEvent->next;

            if ((currEvent == NULL) ||
                (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms))) ||
                !((currEvent->pausedTms.sec == 0) && (currEvent->pausedTms.nsec == 0)))
            {
               newEvent->next = prevEvent->next;
               prevEvent->next = newEvent;
               done = TRUE;
            }
         }
      }
   }

   tmrHandle->numEvents++;

   return;
}


CmsRet cmsTmr_set(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *newEvent;

   /*
    * First verify there is not a duplicate event.
    * (The original code first deleted any existing timer,
    * which is a "side-effect", bad style, but maybe tr69c requires
    * that functionality?)
    */
   if (cmsTmr_isEventPresent(handle, func, ctxData))
   {
      cmsLog_error("There is already an event func 0x%p ctxData 0x%p",
                   func, ctxData);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* make sure name is not too long */
   if ((name != NULL) && (strlen(name) >= CMS_EVENT_TIMER_NAME_LENGTH))
   {
      cmsLog_error("name of timer event is too long, max %d", CMS_EVENT_TIMER_NAME_LENGTH);
      return CMSRET_INVALID_ARGUMENTS;
   }


   /*
    * Allocate a structure for the timer event.
    */
   newEvent = cmsMem_alloc(sizeof(CmsTimerEvent), ALLOC_ZEROIZE);
   if (newEvent == NULL)
   {
      cmsLog_error("malloc of new timer event failed");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* fill in fields of new event timer structure. */
   newEvent->func = func;
   newEvent->ctxData = ctxData;

   cmsTms_get(&(newEvent->expireTms));
   cmsTms_addMilliSeconds(&(newEvent->expireTms), ms);

   if (name != NULL)
   {
      sprintf(newEvent->name, "%s", name);
   }

   /* Insert the new event timer in the timer handle */
   _cmsTmr_insert(tmrHandle, newEvent);

   cmsLog_debug("added event %s, expires in %ums (at %u.%03u), func=0x%x data=%p count=%d",
                newEvent->name,
                ms,
                newEvent->expireTms.sec,
                newEvent->expireTms.nsec/NSECS_IN_MSEC,
                func,
                ctxData,
                tmrHandle->numEvents);

   return CMSRET_SUCCESS;
}  


CmsTimerEvent * _cmsTmr_remove(CmsTimerHandle *tmrHandle, CmsEventHandler func, void *ctxData)
{
   CmsTimerEvent *currEvent, *prevEvent;

   if ((currEvent = tmrHandle->events) == NULL)
   {
      return NULL;
   }

   if (currEvent->func == func && currEvent->ctxData == ctxData)
   {
      /* remove from head of the queue */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
   }
   else
   {
      UBOOL8 done = FALSE;

      while ((currEvent != NULL) && (!done))
      {
         prevEvent = currEvent;
         currEvent = currEvent->next;

         if (currEvent != NULL && currEvent->func == func && currEvent->ctxData == ctxData)
         {
            prevEvent->next = currEvent->next;
            currEvent->next = NULL;
            done = TRUE;
         }
      }
   }

   if (currEvent != NULL)
   {
      tmrHandle->numEvents--;
   }

   return currEvent;
}


void cmsTmr_cancel(void *handle, CmsEventHandler func, void *ctxData)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;

   if ((currEvent = _cmsTmr_remove(tmrHandle, func, ctxData)) == NULL)
   {
      cmsLog_debug("no events to cancel (func=0x%x data=%p)", func, ctxData);
      return;
   }

   cmsLog_debug("canceled event %s, count=%d", currEvent->name, tmrHandle->numEvents);

   CMSMEM_FREE_BUF_AND_NULL_PTR(currEvent);

   return;
}


CmsRet cmsTmr_pause(void *handle, CmsEventHandler func, void *ctxData)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *newEvent;

   /* First take the event out of the queue */
   if ((newEvent = _cmsTmr_remove(tmrHandle, func, ctxData)) == NULL)
   {
      cmsLog_debug("no events to pause (func=0x%x data=%p)", func, ctxData);
      return CMSRET_RESOURCE_NOT_CONFIGURED;
   }

   /* Pause it if it was not already paused */
   if ((newEvent->pausedTms.sec == 0) && (newEvent->pausedTms.nsec == 0))
   {
      cmsTms_get(&(newEvent->pausedTms));
   }

   /* Add it to the end of the queue because it is paused */
   if (tmrHandle->numEvents == 0)
   {
      tmrHandle->events = newEvent;
   }
   else 
   {
      currEvent = tmrHandle->events;

      while (currEvent->next != NULL)
      {
         currEvent = currEvent->next;
      }
      currEvent->next = newEvent;
   }
   tmrHandle->numEvents++;

   cmsLog_debug("paused event %s, count=%d", newEvent->name, tmrHandle->numEvents);

   return CMSRET_SUCCESS;
}


CmsRet cmsTmr_resume(void *handle, CmsEventHandler func, void *ctxData)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   UINT32 ms;

   /* First take the event out of the queue */
   if ((currEvent = _cmsTmr_remove(tmrHandle, func, ctxData)) == NULL)
   {
      cmsLog_debug("no events to pause (func=0x%x data=%p)", func, ctxData);
      return CMSRET_RESOURCE_NOT_CONFIGURED;
   }

   /* Calculate the new expiry delay */
   if ((currEvent->pausedTms.sec != 0) || (currEvent->pausedTms.nsec != 0))
   {
      if (IS_EARLIER_THAN(&(currEvent->expireTms), &(currEvent->pausedTms)))
      {
         cmsTms_get(&(currEvent->expireTms));
      }
      else
      {
         ms = cmsTms_deltaInMilliSeconds(&currEvent->expireTms, &currEvent->pausedTms);
         cmsTms_get(&(currEvent->expireTms));
         cmsTms_addMilliSeconds(&(currEvent->expireTms), ms);
      }
      currEvent->pausedTms.sec = currEvent->pausedTms.nsec = 0;
   }

   /* Resume it */
   _cmsTmr_insert(tmrHandle, currEvent);
   cmsLog_debug("resumed event %s, count=%d", currEvent->name, tmrHandle->numEvents);

   return CMSRET_SUCCESS;
}


CmsRet cmsTmr_getTimeToNextEvent(const void *handle, UINT32 *ms)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;

   cmsTms_get(&nowTms);
   currEvent = tmrHandle->events;

   if (currEvent == NULL)
   {
      *ms = MAX_UINT32;
      return CMSRET_NO_MORE_INSTANCES;
   }

   /* this is the same code as in dumpEvents, integrate? */
   if (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms))
   {
      /*
       * the next event is past due (nowTms is later than currEvent),
       * so time to next event is 0.
       */
      *ms = 0;
   }
   else
   {
      /*
       * nowTms is earlier than currEvent, so currEvent is still in
       * the future.  
       */
      (*ms) = cmsTms_deltaInMilliSeconds(&(currEvent->expireTms), &nowTms);
   }

   return CMSRET_SUCCESS;
}


UINT32 cmsTmr_getNumberOfEvents(const void *handle)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;

   return (tmrHandle->numEvents);
}


void cmsTmr_executeExpiredEvents(void *handle)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;

   cmsTms_get(&nowTms);
   currEvent = tmrHandle->events;

   while ((currEvent != NULL) && (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms)) &&
          (currEvent->pausedTms.sec == 0) && (currEvent->pausedTms.nsec == 0))
   {
      /*
       * first remove the currEvent from the tmrHandle because
       * when we execute the callback function, it might call the
       * cmsTmr API again.
       */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
      tmrHandle->numEvents--;

      cmsLog_debug("executing timer event %s func 0x%x data 0x%x",
                   currEvent->name, currEvent->func, currEvent->ctxData);

      /* call the function */
      (*currEvent->func)(currEvent->ctxData);

      /* free the event struct */
      cmsMem_free(currEvent);

      currEvent = tmrHandle->events;
   }

   return;
}

CmsRet cmsTmr_getTimeRemaining(const void *handle, CmsEventHandler func, void *ctxData, UINT32 *ms)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *)handle;
   CmsTimerEvent *tmrEvent;
   CmsTimestamp   nowTms;
   UINT32 msRem = 0;
   CmsRet retVal = CMSRET_OBJECT_NOT_FOUND;

   tmrEvent = tmrHandle->events;
   while (tmrEvent != NULL)
   {
      if ((tmrEvent->func == func) && (tmrEvent->ctxData == ctxData))
      {
          retVal = CMSRET_SUCCESS;
          cmsTms_get(&nowTms);
          if ( IS_EARLIER_THAN(&nowTms, &tmrEvent->expireTms) )
          {
              msRem = cmsTms_deltaInMilliSeconds(&tmrEvent->expireTms, &nowTms);
          }
          else
          {
              msRem = 0;
          }
          break;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   *ms = msRem;

   return retVal;
}

UBOOL8 cmsTmr_isEventPresent(const void *handle, CmsEventHandler func, void *ctxData)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && tmrEvent->ctxData == ctxData)
      {
         found = TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   return found;
}

void cmsTmr_dumpEvents(const void *handle __attribute__((unused)))
{
#ifdef CMS_LOG3
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;
   UINT32 expires;

   cmsLog_debug("dumping %d events", tmrHandle->numEvents);
   cmsTms_get(&nowTms);

   currEvent = tmrHandle->events;

   while (currEvent != NULL)
   {

      /* this is the same code as in getTimeToNextEvent, integrate? */
      if (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms))
      {
         /*
          * the currentevent is past due (nowTms is later than currEvent),
          * so expiry time is 0.
          */
         expires = 0;
      }
      else
      {
         /*
          * nowTms is earlier than currEvent, so currEvent is still in
          * the future.  
          */
         expires = cmsTms_deltaInMilliSeconds(&(currEvent->expireTms), &nowTms);
      }


      cmsLog_debug("event %s expires in %ums (at %u.%03u) func=0x%x data=%p",
                   currEvent->name,
                   expires,
                   currEvent->expireTms.sec,
                   currEvent->expireTms.nsec/NSECS_IN_MSEC,
                   currEvent->func,
                   currEvent->ctxData);

      currEvent = currEvent->next;
   }
#endif
   return;
}


CmsRet cmsTmr_replaceIfSooner(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   CmsTimestamp nowTms;
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && tmrEvent->ctxData == ctxData)
      {
         found = TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }
   if (found)
   {
      /* find out the expire time of this event.  If it's sooner then the one in the 
       * timer list, then replace the one in list with this one.
       */
      cmsTms_get(&nowTms);
      cmsTms_addMilliSeconds(&nowTms, ms);
      if (IS_EARLIER_THAN(&nowTms, &(tmrEvent->expireTms)))
      {
         cmsTmr_cancel((void*)tmrHandle, func, (void*)NULL);
      }
      else
      {
         return CMSRET_SUCCESS;
      }
   } /* found */
   return(cmsTmr_set(handle, func, ctxData, ms, name));
}







int main(int argc, char const *argv[])
{
   CmsRet ret;
  	if ((ret = cmsTmr_init(&tmrHandle)) != CMSRET_SUCCESS)
	{
		cmsLog_error("cmsTmr_init failed, ret=%d", ret);
		exit(1);
	}

   return 0;
}

/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

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

#include "cms.h"
#include "cms_util.h"
#include "genutil_hexbinary.h"

static CmsRet hexret_to_cmsret(SINT32 hexRet)
{
   CmsRet ret;

   switch (hexRet)
   {
      case HEXRET_SUCCESS:
         ret = CMSRET_SUCCESS;
         break;

      case HEXRET_INTERNAL_ERROR:
         ret = CMSRET_INTERNAL_ERROR;
         break;

      case HEXRET_RESOURCE_EXCEEDED:
         ret = CMSRET_RESOURCE_EXCEEDED;
         break;

      case HEXRET_INVALID_ARGUMENTS:
         ret = CMSRET_INVALID_ARGUMENTS;
         break;

      case HEXRET_CONVERSION_ERROR:
         ret = CMSRET_CONVERSION_ERROR;
         break;

      default:
         ret = CMSRET_UNKNOWN_ERROR;
         break;
   }

   return ret;
}


CmsRet cmsUtl_binaryBufToHexStringStrict(const UINT8 *binaryBuf, UINT32 binaryBufLen, char **hexStr)
{
   /* reject if there is no input data to convert */
   if (binaryBuf == NULL || binaryBufLen == 0)
   {
      return CMSRET_INVALID_ARGUMENTS;
   }
   return (cmsUtl_binaryBufToHexString(binaryBuf, binaryBufLen, hexStr));
}

CmsRet cmsUtl_binaryBufToHexString(const UINT8 *binaryBuf, UINT32 binaryBufLen, char **hexStr)
{
   SINT32 hexRet;

   if (hexStr == NULL)
   {
      cmsLog_error("hexStr buffer is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }

   *hexStr = cmsMem_alloc((binaryBufLen*2)+1, ALLOC_ZEROIZE);
   if (*hexStr == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* genUtl_binaryBufToHexString does not allow 0 length binary buf, so
    * catch that case here and return with an empty string buf to
    * maintain original behavior. */
   if (binaryBufLen == 0)
   {
      return CMSRET_SUCCESS;
   }

   hexRet = genUtl_binaryBufToHexString(binaryBuf, binaryBufLen, *hexStr);
   if (hexRet != HEXRET_SUCCESS)
   {
      cmsMem_free(*hexStr);
      *hexStr = NULL;
      return hexret_to_cmsret(hexRet);
   }

   return CMSRET_SUCCESS;
}


CmsRet cmsUtl_hexStringToBinaryBufStrict(const char *hexStr, UINT8 **binaryBuf, UINT32 *binaryBufLen)
{
   /* reject if there is no input data */
   if (hexStr == NULL || hexStr[0] == '\0')
   {
      return CMSRET_INVALID_ARGUMENTS;
   }
   return (cmsUtl_hexStringToBinaryBuf(hexStr, binaryBuf, binaryBufLen));
}

CmsRet cmsUtl_hexStringToBinaryBuf(const char *hexStr, UINT8 **binaryBuf, UINT32 *binaryBufLen)
{
   UINT32 hexStrLen;
   SINT32 hexRet;

   if (hexStr == NULL || binaryBuf == NULL || binaryBufLen == NULL)
   {
      return CMSRET_INVALID_ARGUMENTS;
   }

   hexStrLen = strlen(hexStr);
   if (hexStrLen % 2 != 0)
   {
      cmsLog_error("hexStr must be an even number of characters");
      return CMSRET_INVALID_ARGUMENTS;
   }

   *binaryBuf = cmsMem_alloc(hexStrLen/2, ALLOC_ZEROIZE);
   if (*binaryBuf == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* genUtl_hexStringToBinaryBuf does not allow 0 length hexString, so
    * catch that case here and return with a "zero length buf" to
    * maintain original behavior. Caller still needs to free the buf. */
   if (hexStrLen == 0)
   {
      *binaryBufLen = 0;
      return CMSRET_SUCCESS;
   }

   hexRet = genUtl_hexStringToBinaryBuf(hexStr, *binaryBuf);
   if (hexRet != HEXRET_SUCCESS)
   {
      cmsMem_free(*binaryBuf);
      *binaryBuf = NULL;
      return hexret_to_cmsret(hexRet);
   }

   /* if we get here, we were successful, set length */
   *binaryBufLen = hexStrLen / 2;

   return CMSRET_SUCCESS;
}

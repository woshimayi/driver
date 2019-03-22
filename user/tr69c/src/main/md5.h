/*----------------------------------------------------------------------*
<:copyright-broadcom 
 
 Copyright (c) 2005 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
 *----------------------------------------------------------------------*/
#ifndef MD5_H
#define MD5_H

#define MD5_DIGEST_LEN     16

typedef struct MD5Context {
  u_int32_t buf[4];
  u_int32_t bits[2];
  u_char in[64];
} MD5Context;

void MD5Init(MD5Context *ctx);
void MD5Update(MD5Context *ctx, u_char const *buf, u_int len);
void MD5Final(u_char digest[16], MD5Context *ctx);
void tr69_md5it(unsigned char *out, const unsigned char *in);

#endif

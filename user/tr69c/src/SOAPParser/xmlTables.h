#ifndef XMLTABLES_H
#define XMLTABLES_H
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
 *----------------------------------------------------------------------*
 * File Name  : xmlTables.h
 *
 * Description: SOAP xmlTables and data structures 
 * $Revision: 1.7 $
 * $Id: xmlTables.h,v 1.7 2005/12/02 21:53:30 dmounday Exp $
 *----------------------------------------------------------------------*/
extern NameSpace    nameSpaces[];
extern XmlNodeDesc  envelopeDesc[];

/* MACROS for referencing the above namespace */
/* strings from xml node description tables   */
/* must match initializations in xmlTables */
/* MACROS for sending namespace prefix */
#define nsSOAP       nameSpaces[0].sndPrefix
#define nsSOAP_ENC   nameSpaces[1].sndPrefix
#define nsXSD        nameSpaces[2].sndPrefix
#define nsXSI        nameSpaces[3].sndPrefix
#define nsCWMP       nameSpaces[4].sndPrefix

#endif 

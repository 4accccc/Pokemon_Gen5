/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - demos - netconnect
  File:     sitedefs.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITROWIFI_SITEDEFS_H_
#define NITROWIFI_SITEDEFS_H_

#ifdef __cplusplus
extern "C" {
#endif


#define SITEDEFS_DEFAULTCLASS               "WiFi.LAN.1.AP.1"
#define SITEDEFS_DEFAULTCLASS_FOR_TEST      "WiFi.LAN.2.AP.1"
void    SiteDefs_Init(void);

#ifdef __cplusplus
}
#endif

#endif // NITROWIFI_SITEDEFS_H_

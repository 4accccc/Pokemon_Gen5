/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - include
  File:     nitroWiFi.h

  Copyright 2003,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-12-25#$
  $Rev: 194 $
  $Author: adachi_hiroaki $
 *---------------------------------------------------------------------------*/

#ifndef NITROWIFI_H_
#define NITROWIFI_H_

#ifdef SDK_ARM9

#include <nitroWiFi/wcm.h>

#ifndef SDK_WIFI_INET

#include <nitroWiFi/socket.h>
#include <nitroWiFi/ssl.h>

#ifdef SDK_WIFI_COMPAT
#include <nitroWiFi/so2soc.h>
#endif

#else // SDK_WIFI_INET

#include <ninet.h>

#ifdef SDK_WIFI_COMPAT
#include <nitroWiFi/soc2so.h>
#endif

#endif // SDK_WIFI_INET

#else //SDK_ARM7
#endif

#endif //NITROWIFI_H_

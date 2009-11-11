/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_nonport.h

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
#ifndef __NHTTP_NONPORT_H__
#define __NHTTP_NONPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(NHTTP_FOR_NTR)
#include "NHTTP_os_NTR.h"
#include "NHTTP_stdlib_NTR.h"

#ifdef NHTTP_USE_DWC
#ifndef TWLDWC_NOGS
#define TWLDWC_NOGS
#endif
#include <base/dwc_core.h>
#endif

typedef struct
{
    u16         authType;					// Proxy 認証形式
    u16         port;						// Proxy port
    u8          hostName      [ 0x64 ];		// Proxy hostname
    u8          authId        [ 0x20 ];		// proxy basic認証用 ID
    u8          authPass      [ 0x20 ];		// proxy basic認証用 パスワード
} NHTTPDWCProxySetting;

#ifdef DWC_CONNECTINET_H_
SDK_COMPILER_ASSERT(sizeof(NHTTPDWCProxySetting) == sizeof(DWCProxySetting));
#else
typedef enum
{
    DWC_PROXY_AUTHTYPE_NONE  = 0x00,	// 認証なし
    DWC_PROXY_AUTHTYPE_BASIC 			// BASIC 認証
} DWCProxyAuthType;
typedef NHTTPDWCProxySetting DWCProxySetting;
BOOL DWC_GetProxySetting( DWCProxySetting *proxy );
#endif

#else
#include "NHTTP_os_RVL.h"
#include "NHTTP_stdlib_RVL.h"
#endif

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_NONPORT_H__

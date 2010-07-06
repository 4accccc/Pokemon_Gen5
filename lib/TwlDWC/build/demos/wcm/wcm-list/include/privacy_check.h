/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WCM - demos - wcm-list-2
  File:     privacy_check.h

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
#ifndef WIFI_SECURITY_H_
#define WIFI_SECURITY_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#ifdef SDK_TWL
#include <twl.h>
#include <twlWiFi.h>
#else
#include <nitro.h>
#include <nitroWiFi.h>
#endif

#define ELEMENTID_RSN 48              /* RSN ID */
#define ELEMENTID_VENDOR_SPECIFIC 221 /* Vendor Specific ID (WPA IE はこの ID を仕様)*/

#ifndef SDK_TWL
/*=== DS互換モードでは定義されていない WPA 関連の定義 ===*/
#define WCM_PRIVACYMODE_WPA_TKIP    4
#define WCM_PRIVACYMODE_WPA2_TKIP   5
#define WCM_PRIVACYMODE_WPA_AES     6
#define WCM_PRIVACYMODE_WPA2_AES    7
#endif
/*---------------------------------------------------------------------------*
    構造体/列挙体 定義
 *---------------------------------------------------------------------------*/

typedef struct WPACipherType
{
    u8 oui[3];  /* WPAの場合は 00：50：F2、WPA2の場合は 00:0F:AC */
    u8 type;
}
WPACipherType;

typedef enum WPAKeyType
{
    WPA_KEYTYPE_GPOUP_CIPHER = 0,
    WPA_KEYTYPE_WEP_40,
    WPA_KEYTYPE_TKIP,
    WPA_KEYTYPE_RESERVED,
    WPA_KEYTYPE_AES,
    WPA_KEYTYPE_WEP104,
    WPA_KEYTYPE_MAX
}
WPAKeyType;

typedef enum WPAAuthType
{
    WPA_AUTHTYPE_RESERVED = 0,
    WPA_AUTHTYPE_IEEE802_1X,
    WPA_AUTHTYPE_PSK,
    WPA_AUTHTYPE_MAX
}
WPAAuthType;

/*---------------------------------------------------------------------------*
    関数 定義
 *---------------------------------------------------------------------------*/

u8 GetPrivacyMode(const WCMBssDesc *bssDesc);
u8 GetWPA2PrivacyMode(const WCMBssDesc *bssDesc);
u8 GetWPAPrivacyMode(const WCMBssDesc *bssDesc);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* WIFI_SECURITY_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

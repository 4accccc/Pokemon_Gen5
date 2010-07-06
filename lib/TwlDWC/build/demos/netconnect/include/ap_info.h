/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - demos - netconnect
  File:     ap_info.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 1071 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef SHARED_AP_INFO_H_
#define SHARED_AP_INFO_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <nitroWiFi/wcm.h>

// AP èÓïÒç\ë¢ëÃ å^íËã`
typedef struct WcmControlApInfo
{
    u8          bssId[WCM_BSSID_SIZE];
    WCMWepDesc  wepDesc;
    u8          essId[WCM_ESSID_SIZE];
    u32         auth_option;

} WcmControlApInfo;

// å≈íË AP èÓïÒÇÃéQè∆
extern const WcmControlApInfo   constApInfo;

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* SHARED_AP_INFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

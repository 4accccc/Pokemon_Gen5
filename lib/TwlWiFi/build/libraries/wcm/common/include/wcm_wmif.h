/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     wcm_wmif.h

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

#ifndef NITROWIFI_WCM_WMIF_H_
#define NITROWIFI_WCM_WMIF_H_

#ifdef  __cplusplus
extern "C" {
#endif
/*===========================================================================*/

#include "wcm_cpsif.h"

/*---------------------------------------------------------------------------*
  定数定義
 *---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*
  構造体定義
 *---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*
  関数プロトタイプ定義
 *---------------------------------------------------------------------------*/

s32     WCMi_WmifStartupAsync(void);
s32     WCMi_WmifCleanupAsync(void);
s32     WCMi_WmifBeginSearchAsync(const void* bssid, const void* essid, u32 option);
s32     WCMi_WmifConnectAsync(void);
s32     WCMi_WmifDisconnectAsync(void);
s32     WCMi_WmifTerminateAsync(void);
s32     WCMi_WmifSetDCFData(const u8* dstAddr, const u16* buf, u16 len, WCMSendInd callback);
void    WCMi_WmifSendNullPacket(void);
u16 WCMi_WmifGetAllowedChannel(void);

/*===========================================================================*/
#ifdef  __cplusplus
}       /* extern "C" */
#endif  /* __cplusplus */

#endif  /* NITROWIFI_WCM_WMIF_H_ */


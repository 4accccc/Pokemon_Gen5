/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     wcm_nwmif.h

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

#ifndef NITROWIFI_WCM_NWMIF_H_
#define NITROWIFI_WCM_NWMIF_H_

#ifdef  __cplusplus
extern "C" {
#endif
/*===========================================================================*/
 
 /*---------------------------------------------------------------------------*
  定数定義
 *---------------------------------------------------------------------------*/
#ifdef SDK_NWM_USE_UAPSD
#define WCM_NWM_POWER_SAVE_ON    NWM_POWERMODE_UAPSD  //(擬似)uAPSDによる省電力機能を ON
#else
#define WCM_NWM_POWER_SAVE_ON    NWM_POWERMODE_STANDARD  // 802.11 標準の省電力機能を ON
#endif
#define WCM_NWM_POWER_SAVE_OFF   NWM_POWERMODE_ACTIVE  //消費電力を削減しない


/*---------------------------------------------------------------------------*
  構造体定義
 *---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*
  関数プロトタイプ定義
 *---------------------------------------------------------------------------*/

s32     WCMi_NwmifStartupAsync(void);
s32     WCMi_NwmifCleanupAsync(void);
s32     WCMi_NwmifBeginSearchAsync(const void* bssid, const void* essid, u32 option);
s32     WCMi_NwmifConnectAsync(void);
s32     WCMi_NwmifDisconnectAsync(void);
s32     WCMi_NwmifTerminateAsync(void);
s32     WCMi_NwmifSetDCFData(const u8* dstAddr, const u16* buf, u16 len, WCMSendInd callback);
void    WCMi_NwmifSendNullPacket(void);
u16 WCMi_NwmifGetAllowedChannel(void);

 /*===========================================================================*/
#ifdef  __cplusplus
}       /* extern "C" */
#endif  /* __cplusplus */

#endif  /* NITROWIFI_WCM_NWMIF_H_ */

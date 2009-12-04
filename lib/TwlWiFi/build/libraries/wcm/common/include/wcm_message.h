/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     wcm_message.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-10#$
  $Rev: 28 $
  $Author: $
 *---------------------------------------------------------------------------*/

#ifndef WCM_MESSAGE_H_
#define WCM_MESSAGE_H_

#ifdef  __cplusplus
extern "C" {
#endif
/*===========================================================================*/

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/

// 警告文用テキスト雛形
extern const char WCMi_WText_AlreadyInit[];
extern const char WCMi_WText_IllegalParam[];
extern const char WCMi_WText_NotInit[];
extern const char WCMi_WText_IllegalPhase[];
extern const char WCMi_WText_InvalidWmState[];
extern const char WCMi_WText_UnsuitableArm7[];
extern const char WCMi_WText_WirelessForbidden[];
extern const char WCMi_WText_InIrqMode[];
extern const char WCMi_WText_NoIndexData[];

// 報告文用テキスト雛形
extern const char WCMi_RText_WmSyncError[];
extern const char WCMi_RText_WmAsyncError[];
extern const char WCMi_RText_WmDisconnected[];
extern const char WCMi_RText_InvalidAid[];
extern const char WCMi_RText_SupportRateset[];

/*===========================================================================*/
#ifdef  __cplusplus
}       /* extern "C" */
#endif  /* __cplusplus */

#endif  /* WCM_MESSAGE_H_ */


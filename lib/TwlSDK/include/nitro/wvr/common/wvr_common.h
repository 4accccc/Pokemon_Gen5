/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - include
  File:     wvr_common.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_WVR_COMMON_WVR_COMMON_H_
#define NITRO_WVR_COMMON_WVR_COMMON_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// API 処理結果
typedef enum WVRResult
{
    WVR_RESULT_SUCCESS = 0,            // 成功
    WVR_RESULT_OPERATING,              // 非同期処理の開始に成功
    WVR_RESULT_DISABLE,                // コンポーネントがWVRライブラリに対応していない
    WVR_RESULT_INVALID_PARAM,          // 無効なパラメータ指定
    WVR_RESULT_FIFO_ERROR,             // PXIでのARM7への要求送信に失敗
    WVR_RESULT_ILLEGAL_STATUS,         // 要求を実行できない状態
    WVR_RESULT_VRAM_LOCKED,            // VRAMがロックされていて使用できない。
    WVR_RESULT_FATAL_ERROR,            // 想定外の致命的なエラー

    WVR_RESULT_MAX
}
WVRResult;

// PXI コマンド
#define     WVR_PXI_COMMAND_STARTUP     0x00010000
#define     WVR_PXI_COMMAND_TERMINATE   0x00020000


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_WVR_COMMON_WVR_COMMON_H_ */

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/

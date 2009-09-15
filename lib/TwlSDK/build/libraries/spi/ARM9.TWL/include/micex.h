/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - mic
  File:     micex.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_LIBRARIES_SPI_ARM9_MICEX_H_
#define TWL_LIBRARIES_SPI_ARM9_MICEX_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include    <nitro/spi/ARM9/mic.h>

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
/* 非同期関数排他処理用ロック定義 */
typedef enum MICLock
{
    MIC_LOCK_OFF = 0,                  // ロック開錠状態
    MIC_LOCK_ON,                       // ロック施錠状態
    MIC_LOCK_MAX
}
MICLock;

/* ワーク用構造体 */
typedef struct MICWork
{
    MICLock lock;                      // 排他ロック
    MICCallback callback;              // 非同期関数コールバック退避用
    void   *callbackArg;               // コールバック関数の引数保存用
    MICResult commonResult;            // 非同期関数の処理結果退避用
    MICCallback full;                  // サンプリング完了コールバック退避用
    void   *fullArg;                   // 完了コールバック関数の引数保存用
    void   *dst_buf;                   // 単体サンプリング結果格納エリア退避用

}
MICWork;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
MICWork*    MICi_GetSysWork(void);

MICResult   MICEXi_StartLimitedSampling(const MICAutoParam* param);
MICResult   MICEXi_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg);
MICResult   MICEXi_StopLimitedSampling(void);
MICResult   MICEXi_StopLimitedSamplingAsync(MICCallback callback, void* arg);
MICResult   MICEXi_AdjustLimitedSampling(u32 rate);
MICResult   MICEXi_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg);


/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_LIBRARIES_SPI_ARM9_MICEX_H_ */

/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - include
  File:     api.h

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

#ifndef NITRO_SPI_ARM9_MIC_H_
#define NITRO_SPI_ARM9_MIC_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/spi/common/type.h>
#include    <nitro/pxi.h>


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// 処理結果定義
typedef enum MICResult
{
    MIC_RESULT_SUCCESS = 0,            // 成功
    MIC_RESULT_BUSY,                   // 排他制御中
    MIC_RESULT_ILLEGAL_PARAMETER,      // 不正なパラメータ
    MIC_RESULT_SEND_ERROR,             // PXIによる送信に失敗
    MIC_RESULT_INVALID_COMMAND,        // 不明なコマンド
    MIC_RESULT_ILLEGAL_STATUS,         // 処理実行不可能な状態
    MIC_RESULT_FATAL_ERROR,            // 上記以外のエラー
    MIC_RESULT_MAX
}
MICResult;

// サンプリング種別定義
typedef enum MICSamplingType
{
    MIC_SAMPLING_TYPE_8BIT = 0,        //  8ビットサンプリング
    MIC_SAMPLING_TYPE_12BIT,           // 12ビットサンプリング
    MIC_SAMPLING_TYPE_SIGNED_8BIT,     // 符号付き 8ビットサンプリング
    MIC_SAMPLING_TYPE_SIGNED_12BIT,    // 符号付き12ビットサンプリング
    MIC_SAMPLING_TYPE_12BIT_FILTER_OFF,
    MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF,
    MIC_SAMPLING_TYPE_MAX
}
MICSamplingType;

// 代表的なサンプリング周期をARM7のクロック数で定義
typedef enum MICSamplingRate
{
    MIC_SAMPLING_RATE_8K = (HW_CPU_CLOCK_ARM7 / 8000),  // 約  8.0   kHz
    MIC_SAMPLING_RATE_11K = (HW_CPU_CLOCK_ARM7 / 11025),        // 約 11.025 kHz
    MIC_SAMPLING_RATE_16K = (HW_CPU_CLOCK_ARM7 / 16000),        // 約 16.0   kHz
    MIC_SAMPLING_RATE_22K = (HW_CPU_CLOCK_ARM7 / 22050),        // 約 22.05  kHz
    MIC_SAMPLING_RATE_32K = (HW_CPU_CLOCK_ARM7 / 32000),        // 約 32.0   kHz
    MIC_SAMPLING_RATE_LIMIT = 1024
}
MICSamplingRate;

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// コールバック関数型定義
typedef void (*MICCallback) (MICResult result, void *arg);

// オートサンプリング用設定定義
typedef struct MICAutoParam
{
    MICSamplingType type;              // サンプリング種別
    void   *buffer;                    // 結果格納バッファへのポインタ
    u32     size;                      // バッファサイズ
    u32     rate;                      // サンプリング周期( ARM7のクロック数 )
    BOOL    loop_enable;               // バッファフル時のループ可否
    MICCallback full_callback;         // バッファフル時のコールバック
    void   *full_arg;                  // 上記コールバックに指定する引数

}
MICAutoParam;


/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MIC_Init

  Description:  マイクライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MIC_Init(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSamplingAsync

  Description:  マイクを一回非同期にサンプリングする。

  Arguments:    type      - サンプリング種別を指定。
                buf       - サンプリングデータを格納するバッファを指定。
                callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSamplingAsync(MICSamplingType type, void *buf, MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSamplingAsync

  Description:  マイクの自動サンプリングを非同期に開始する。

  Arguments:    param     - 自動サンプリング設定を構造体へのポインタで指定。
                callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSamplingAsync(const MICAutoParam *param, MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSamplingAsync

  Description:  マイクの自動サンプリングを非同期に停止する。

  Arguments:    callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSamplingAsync(MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSamplingAsync

  Description:  マイクの自動サンプリングにおけるサンプリングレートを非同期に
                調整する。

  Arguments:    rate         - サンプリングレートを指定。
                callback     - 非同期処理が完了した際に呼び出す関数を指定。
                arg          - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult    - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSamplingAsync(u32 rate, MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_GetLastSamplingAddress

  Description:  マイクの最新サンプリング結果格納アドレスを取得する。

  Arguments:    None.

  Returns:      void* - サンプリング結果格納アドレスを返す。
                        まだサンプリングしていない場合はNULLを返す。
 *---------------------------------------------------------------------------*/
void   *MIC_GetLastSamplingAddress(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSampling

  Description:  マイクを一回サンプリングする。

  Arguments:    type      - サンプリング種別を指定。
                buf       - サンプリングデータを格納するバッファを指定。

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSampling(MICSamplingType type, void *buf);

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSampling

  Description:  マイクの自動サンプリングを開始する。

  Arguments:    param     - 自動サンプリング設定を構造体へのポインタで指定。

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSampling(const MICAutoParam *param);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSampling

  Description:  マイクの自動サンプリングを停止する。
                自動サンプリング開始時にループ指定しなかった場合はバッファが
                埋まった時点で自動的にサンプリングは停止される。

  Arguments:    None.

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSampling(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSampling

  Description:  マイクの自動サンプリングにおけるサンプリングレートを調整する。

  Arguments:    rate      - サンプリングレートを指定。

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSampling(u32 rate);

#ifdef  SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         MIC_StartLimitedSamplingAsync

  Description:  サンプリングレート限定のマイクの自動サンプリングを非同期に開始
                する。ハードウェアによるサンプリングを行うため正確な周波数での
                サンプリングを低い CPU 負荷で行うことができるが、サンプリング
                レートはハードウェアのサポートするレートに限定される。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
                なお、NITRO 上で呼び出した場合には既存の CPU による自動サンプ
                リング開始関数で代替される。

  Arguments:    param   -   自動サンプリング設定を構造体へのポインタで指定。
                callback -  非同期処理が完了した際に呼び出す関数を指定。
                arg     -   コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult   MIC_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopLimitedSamplingAsync

  Description:  サンプリングレート限定のマイクの自動サンプリングを非同期に停止
                する。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
                なお、NITRO 上で呼び出した場合には既存の CPU による自動サンプ
                リング停止関数で代替される。

  Arguments:    callback -  非同期処理が完了した際に呼び出す関数を指定。
                arg     -   コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult   MIC_StopLimitedSamplingAsync(MICCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustLimitedSamplingAsync

  Description:  サンプリングレート限定のマイクの自動サンプリングにおけるサンプ
                リングレートを非同期に調整する。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
                なお、NITRO 上で呼び出した場合には既存の CPU による自動サンプ
                リングにおけるサンプリングレート調整関数で代替される。

  Arguments:    rate    -   サンプリング間隔を ARM7 の CPU クロック単位で指定。
                callback -  非同期処理が完了した際に呼び出す関数を指定。
                arg     -   コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult   MIC_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StartLimitedSampling

  Description:  サンプリングレート限定のマイクの自動サンプリングを開始する。
                ハードウェアによるサンプリングを行うため正確な周波数でのサンプ
                リングを低い CPU 負荷で行うことができるが、サンプリングレート
                はハードウェアのサポートするレートに限定される。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
                なお、NITRO 上で呼び出した場合には既存の CPU による自動サンプ
                リング開始関数で代替される。

  Arguments:    param   -   自動サンプリング設定を構造体へのポインタで指定。

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult   MIC_StartLimitedSampling(const MICAutoParam* param);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopLimitedSampling

  Description:  サンプリングレート限定のマイクの自動サンプリングを停止する。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
                なお、NITRO 上で呼び出した場合には既存の CPU による自動サンプ
                リング停止関数で代替される。

  Arguments:    None.

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult   MIC_StopLimitedSampling(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustLimitedSampling

  Description:  サンプリングレート限定のマイクの自動サンプリングにおけるサンプ
                リングレートを調整する。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
                なお、NITRO 上で呼び出した場合には既存の CPU による自動サンプ
                リングにおけるサンプリングレート調整関数で代替される。

  Arguments:    rate    -   サンプリング間隔を ARM7 の CPU クロック単位で指定。

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult   MIC_AdjustLimitedSampling(u32 rate);

#endif

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_RTC_ARM9_API_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

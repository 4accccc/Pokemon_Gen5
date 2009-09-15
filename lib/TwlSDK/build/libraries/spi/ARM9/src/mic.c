/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - libraries
  File:     mic.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro/spi.h>
#include    <nitro/os/common/systemWork.h>

#ifdef  SDK_TWL
#include    <twl/os/common/codecmode.h>
#include    "micex.h"
#endif

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWL
// 非同期関数排他処理用ロック定義
typedef enum MICLock
{
    MIC_LOCK_OFF = 0,                  // ロック開錠状態
    MIC_LOCK_ON,                       // ロック施錠状態
    MIC_LOCK_MAX
}
MICLock;

// ワーク用構造体
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
#endif

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u16 micInitialized;             // 初期化確認フラグ
static MICWork micWork;                // ワーク変数をまとめた構造体


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void MicCommonCallback(PXIFifoTag tag, u32 data, BOOL err);
static BOOL MicDoSampling(u16 type);
static BOOL MicStartAutoSampling(void *buf, u32 size, u32 span, u8 flags);
static BOOL MicStopAutoSampling(void);
static BOOL MicAdjustAutoSampling(u32 span);
static void MicGetResultCallback(MICResult result, void *arg);
static void MicWaitBusy(void);


/*---------------------------------------------------------------------------*
  Name:         MIC_Init

  Description:  マイクライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MIC_Init(void)
{
    // 未初期化確認
    if (micInitialized)
    {
        return;
    }
    micInitialized = 1;

    // ワーク用変数初期化
    micWork.lock = MIC_LOCK_OFF;
    micWork.callback = NULL;

    // ARM7のMICライブラリが開始されるまで待つ
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_MIC, PXI_PROC_ARM7))
    {
    }

    // shared領域の最新サンプリング結果格納アドレスをクリア
    OS_GetSystemWork()->mic_last_address = 0;

    // PXIコールバック関数を設定
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_MIC, MicCommonCallback);
}

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSamplingAsync

  Description:  マイクを一回非同期にサンプリングする。

  Arguments:    type      - サンプリング種別を指定。
                buf       - サンプリングデータを格納するバッファを指定。
                callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSamplingAsync(MICSamplingType type, void *buf, MICCallback callback, void *arg)
{
    OSIntrMode enabled;
    u16     wtype;

    SDK_NULL_ASSERT(buf);
    SDK_NULL_ASSERT(callback);

    // パラメータ確認
    if (type >= MIC_SAMPLING_TYPE_MAX)
    {
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }
    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_8BIT;
        break;
    case MIC_SAMPLING_TYPE_12BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_12BIT;
        break;
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_S8BIT;
        break;
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_S12BIT;
        break;
    default:
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }

    // ロック確認
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // サンプリング実行コマンドを送信
    micWork.callback = callback;
    micWork.callbackArg = arg;
    micWork.dst_buf = buf;
    if (MicDoSampling(wtype))
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSampling

  Description:  マイクを一回サンプリングする。

  Arguments:    type      - サンプリング種別を指定。
                buf       - サンプリングデータを格納するバッファを指定。

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSampling(MICSamplingType type, void *buf)
{
    micWork.commonResult = MIC_DoSamplingAsync(type, buf, MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSamplingAsync

  Description:  マイクの自動サンプリングを非同期に開始する。

  Arguments:    param        - 自動サンプリング設定を構造体へのポインタで指定。
                callback     - 非同期処理が完了した際に呼び出す関数を指定。
                arg          - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult    - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSamplingAsync(const MICAutoParam *param, MICCallback callback, void *arg)
{
    OSIntrMode enabled;
    u8      flags;

    SDK_NULL_ASSERT(callback);
    SDK_NULL_ASSERT(param->buffer);

    // パラメータ確認
    {
        // バッファ32バイトアライン
        if ((u32)(param->buffer) & 0x01f)
        {
#ifdef  SDK_DEBUG
            OS_TWarning("Parameter param->buffer must be 32-byte aligned.\n");
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // バッファサイズアライン
        if (param->size & 0x01f)
        {
#ifdef  SDK_DEBUG
            OS_TWarning("Parameter param->size must be a multiple of 32-byte.\n");
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // バッファサイズ
        if (param->size <= 0)
        {
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // サンプリング周期
        if (param->rate < MIC_SAMPLING_RATE_LIMIT)
        {
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // AD変換ビット幅
        switch (param->type)
        {
        case MIC_SAMPLING_TYPE_8BIT:
            flags = SPI_MIC_SAMPLING_TYPE_8BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT:
            flags = SPI_MIC_SAMPLING_TYPE_12BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_8BIT:
            flags = SPI_MIC_SAMPLING_TYPE_S8BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT:
            flags = SPI_MIC_SAMPLING_TYPE_S12BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
            flags = (SPI_MIC_SAMPLING_TYPE_12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
            flags = (SPI_MIC_SAMPLING_TYPE_S12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        default:
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // ループ可否
        if (param->loop_enable)
        {
            flags = (u8)(flags | SPI_MIC_SAMPLING_TYPE_LOOP_ON);
        }
        else
        {
            flags = (u8)(flags | SPI_MIC_SAMPLING_TYPE_LOOP_OFF);
        }
        // 拡張用補正フラグは現状 OFF 固定
        flags = (u8)(flags | SPI_MIC_SAMPLING_TYPE_CORRECT_OFF);
    }

    // ロック確認
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // 自動サンプリング開始コマンドを送信
    micWork.callback = callback;
    micWork.callbackArg = arg;
    micWork.full = param->full_callback;
    micWork.fullArg = param->full_arg;
    if (MicStartAutoSampling(param->buffer, param->size, param->rate, flags))
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSampling

  Description:  マイクの自動サンプリングを開始する。

  Arguments:    param     - 自動サンプリング設定を構造体へのポインタで指定。

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSampling(const MICAutoParam *param)
{
    micWork.commonResult = MIC_StartAutoSamplingAsync(param, MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSamplingAsync

  Description:  マイクの自動サンプリングを非同期に停止する。

  Arguments:    callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSamplingAsync(MICCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    // ロック確認
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // 自動サンプリング停止コマンドを送信
    micWork.callback = callback;
    micWork.callbackArg = arg;
    if (MicStopAutoSampling())
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSampling

  Description:  マイクの自動サンプリングを停止する。
                自動サンプリング開始時にループ指定しなかった場合はバッファが
                埋まった時点で自動的にサンプリングは停止される。

  Arguments:    None.

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSampling(void)
{
    micWork.commonResult = MIC_StopAutoSamplingAsync(MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSamplingAsync

  Description:  マイクの自動サンプリングにおけるサンプリングレートを非同期に
                調整する。

  Arguments:    rate         - サンプリングレートを指定。
                callback     - 非同期処理が完了した際に呼び出す関数を指定。
                arg          - コールバック関数呼び出し時の引数を指定。

  Returns:      MICResult    - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSamplingAsync(u32 rate, MICCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    // パラメータ確認
    if (rate < MIC_SAMPLING_RATE_LIMIT)
    {
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }

    // ロック確認
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // 自動サンプリング調整コマンドを送信
    micWork.callback = callback;
    micWork.callbackArg = arg;
    if (MicAdjustAutoSampling(rate))
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSampling

  Description:  マイクの自動サンプリングにおけるサンプリングレートを調整する。

  Arguments:    rate         - サンプリングレートを指定。

  Returns:      MICResult    - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSampling(u32 rate)
{
    micWork.commonResult = MIC_AdjustAutoSamplingAsync(rate, MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_GetLastSamplingAddress

  Description:  マイクの最新サンプリング結果格納アドレスを取得する。

  Arguments:    None.

  Returns:      void* - サンプリング結果格納アドレスを返す。
                        まだサンプリングしていない場合はNULLを返す。
 *---------------------------------------------------------------------------*/
void   *MIC_GetLastSamplingAddress(void)
{
    return (void *)(OS_GetSystemWork()->mic_last_address);
}

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
MICResult
MIC_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StartLimitedSamplingAsync(param, callback, arg) :
            MIC_StartAutoSamplingAsync(param, callback, arg));
}

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
MICResult
MIC_StopLimitedSamplingAsync(MICCallback callback, void* arg)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StopLimitedSamplingAsync(callback, arg) :
            MIC_StopAutoSamplingAsync(callback, arg));
}

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
MICResult
MIC_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_AdjustLimitedSamplingAsync(rate, callback, arg) :
            MIC_AdjustAutoSamplingAsync(rate, callback, arg));
}

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
MICResult
MIC_StartLimitedSampling(const MICAutoParam* param)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StartLimitedSampling(param) :
            MIC_StartAutoSampling(param));
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StopLimitedSampling

  Description:  サンプリングレート限定のマイクの自動サンプリングを停止する。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
                なお、NITRO 上で呼び出した場合には既存の CPU による自動サンプ
                リング停止関数で代替される。

  Arguments:    None.

  Returns:      MICResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult
MIC_StopLimitedSampling(void)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StopLimitedSampling() :
            MIC_StopAutoSampling());
}

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
MICResult
MIC_AdjustLimitedSampling(u32 rate)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_AdjustLimitedSampling(rate) :
            MIC_AdjustAutoSampling(rate));
}
#endif

/*---------------------------------------------------------------------------*
  Name:         MicCommonCallback

  Description:  非同期MIC関数用の共通コールバック関数。

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MicCommonCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    u16     command;
    u16     pxiresult;
    MICResult result;
    MICCallback cb;

    // PXI通信エラーを確認
    if (err)
    {
        if (micWork.lock != MIC_LOCK_OFF)
        {
            micWork.lock = MIC_LOCK_OFF;
        }
        if (micWork.callback)
        {
            cb = micWork.callback;
            micWork.callback = NULL;
            cb(MIC_RESULT_FATAL_ERROR, micWork.callbackArg);
        }
    }

    // 受信データを解析
    command = (u16)((data & SPI_PXI_RESULT_COMMAND_MASK) >> SPI_PXI_RESULT_COMMAND_SHIFT);
    pxiresult = (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);

    // 処理結果を確認
    switch (pxiresult)
    {
    case SPI_PXI_RESULT_SUCCESS:
        result = MIC_RESULT_SUCCESS;
        break;
    case SPI_PXI_RESULT_INVALID_COMMAND:
        result = MIC_RESULT_INVALID_COMMAND;
        break;
    case SPI_PXI_RESULT_INVALID_PARAMETER:
        result = MIC_RESULT_ILLEGAL_PARAMETER;
        break;
    case SPI_PXI_RESULT_ILLEGAL_STATUS:
        result = MIC_RESULT_ILLEGAL_STATUS;
        break;
    case SPI_PXI_RESULT_EXCLUSIVE:
        result = MIC_RESULT_BUSY;
        break;
    default:
        result = MIC_RESULT_FATAL_ERROR;
    }

    // 処理対象コマンドを確認
    if (command == SPI_PXI_COMMAND_MIC_BUFFER_FULL)
    {
        // バッファフル通知をコールバック
        if (micWork.full)
        {
            micWork.full(result, micWork.fullArg);
        }
    }
    else
    {
        if (command == SPI_PXI_COMMAND_MIC_SAMPLING)
        {
            // サンプリング結果をバッファに格納
            if (micWork.dst_buf)
            {
                *(u16 *)(micWork.dst_buf) = OS_GetSystemWork()->mic_sampling_data;
            }
        }
        // 排他ロック開錠
        if (micWork.lock != MIC_LOCK_OFF)
        {
            micWork.lock = MIC_LOCK_OFF;
        }
        // 処理結果をコールバック
        if (micWork.callback)
        {
            cb = micWork.callback;
            micWork.callback = NULL;
            cb(result, micWork.callbackArg);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MicDoSampling

  Description:  マイクを一回サンプリングするコマンドをARM7に送信する。

  Arguments:    type - サンプリングタイプ
                ( 0: 8ビット , 1: 12ビット , 2: signed 8ビット , 3: signed 12ビット )

  Returns:      BOOL - 命令をPXI経由で正常に送信できた場合TRUE、
                       失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL MicDoSampling(u16 type)
{
    // PXIパケット[ 0 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_SAMPLING << 8) | (u32)type, 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicStartAutoSampling

  Description:  マイクの自動サンプリング開始コマンドをARM7に送信する。

  Arguments:    buf   - サンプリングしたデータを格納するバッファのアドレス。
                size  - バッファのサイズ。バイト単位で指定。
                span  - サンプリング間隔(ARM7のCPUクロックで指定)。
                        タイマーの性質上、16ビット×1or64or256or1024 の数字
                        しか正確には設定できない。端数ビットは切り捨てられる。
                flags - AD変換のビット幅、ループ可否、補正可否を指定。

  Returns:      BOOL - 命令をPXI経由で正常に送信できた場合TRUE、
                       失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL MicStartAutoSampling(void *buf, u32 size, u32 span, u8 flags)
{
    // PXIパケット[ 0 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_AUTO_ON << 8) | (u32)flags, 0))
    {
        return FALSE;
    }

    // PXIパケット[ 1 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (1 << SPI_PXI_INDEX_SHIFT) | ((u32)buf >> 16), 0))
    {
        return FALSE;
    }

    // PXIパケット[ 2 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               (2 << SPI_PXI_INDEX_SHIFT) | ((u32)buf & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // PXIパケット[ 3 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (3 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    // PXIパケット[ 4 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               (4 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // PXIパケット[ 5 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (5 << SPI_PXI_INDEX_SHIFT) | (span >> 16), 0))
    {
        return FALSE;
    }

    // PXIパケット[ 6 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_END_BIT |
                               (6 << SPI_PXI_INDEX_SHIFT) | (span & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicStopAutoSampling

  Description:  マイクの自動サンプリング停止コマンドをARM7に送信する。

  Arguments:    None.

  Returns:      BOOL - 命令をPXI経由で正常に送信できた場合TRUE、
                       失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL MicStopAutoSampling(void)
{
    // PXIパケット[ 0 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_AUTO_OFF << 8), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicAdjustAutoSampling

  Description:  マイクの自動サンプリングレート変更コマンドをARM7に送信する。

  Arguments:    span - サンプリング間隔を ARM7 の CPU クロック単位で指定。

  Returns:      BOOL - 命令をPXI経由で正常に送信できた場合TRUE、
                       失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL MicAdjustAutoSampling(u32 span)
{
    // PXIパケット[ 0 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_AUTO_ADJUST << 8), 0))
    {
        return FALSE;
    }

    // PXIパケット[ 1 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (1 << SPI_PXI_INDEX_SHIFT) | (span >> 16), 0))
    {
        return FALSE;
    }

    // PXIパケット[ 2 ]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_END_BIT |
                               (2 << SPI_PXI_INDEX_SHIFT) | (span & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicGetResultCallback

  Description:  非同期処理の完了時に呼び出され、内部変数の処理結果を更新する。

  Arguments:    result - 非同期関数の処理結果。
                arg    - 使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MicGetResultCallback(MICResult result, void *arg)
{
#pragma unused( arg )

    micWork.commonResult = result;
}

#include    <nitro/code32.h>
/*---------------------------------------------------------------------------*
  Name:         MicWaitBusy

  Description:  MICの非同期処理がロックされている間待つ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
MicWaitBusy( void )
{
    ldr     r12,    =micWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #MIC_LOCK_ON
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>

#ifdef  SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         MICi_GetSysWork

  Description:  MIC ライブラリ制御用構造体を取得する。

  Arguments:    None.

  Returns:      MICWork*    -   構造体へのポインタを返す。
 *---------------------------------------------------------------------------*/
MICWork*
MICi_GetSysWork(void)
{
    return &micWork;
}
#endif

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

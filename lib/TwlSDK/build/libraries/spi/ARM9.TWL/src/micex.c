/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - mic
  File:     micex.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro/os.h>
#include    <nitro/spi.h>
#include    "micex.h"

#include    <twl/spi/common/mic_common.h>

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void     MicexSyncCallback(MICResult result, void* arg);
static BOOL     MicexStartLimitedSampling(void* buf, u32 size, u32 rate, u8 flags);
static BOOL     MicexStopLimitedSampling(void);
static BOOL     MicexAdjustLimitedSampling(u32 rate);

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StartLimitedSampling
  Description:  サンプリングレート限定のマイク自動サンプリングを開始する。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
  Arguments:    param   -   自動サンプリング設定を構造体へのポインタで指定。
  Returns:      MICResult - 同期的な処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StartLimitedSampling(const MICAutoParam* param)
{
    MICResult       result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 割り込みハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return MIC_RESULT_ILLEGAL_STATUS;
    }

    /* 応答取得用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   MICEXi_StartLimitedSamplingAsync(param, MicexSyncCallback, (void*)(&msgQ));
    if (result == MIC_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StartLimitedSamplingAsync
  Description:  サンプリングレート限定のマイク自動サンプリングを開始する。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
  Arguments:    param   -   自動サンプリング設定を構造体へのポインタで指定。
                callback -  非同期処理完了時に呼び出すコールバック関数を指定。
                arg     -   コールバック関数に渡すパラメータを指定。
  Returns:      MICResult - 非同期処理開始処理の結果を返す。
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg)
{
    OSIntrMode  e;
    MICWork*    w   =   MICi_GetSysWork();
    u8          flags;

    SDK_NULL_ASSERT(callback);
    SDK_NULL_ASSERT(param->buffer);

    /* パラメータ確認 */
    {
        /* バッファサイズ */
        if (param->size <= 0)
        {
#ifdef  SDK_DEBUG
            OS_TWarning("%s: Illegal desination buffer size. (%d)\n", __FUNCTION__, param->size);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* バッファ 32 バイトアライン */
        if (((u32)(param->buffer) % HW_CACHE_LINE_SIZE != 0) || ((param->size % HW_CACHE_LINE_SIZE) != 0))
        {
#ifdef  SDK_DEBUG
            OS_TWarning("%s: Destination buffer (%p - %p) is not aligned on %d bytes boundary.\n",
                    __FUNCTION__, param->buffer, (void*)((u32)param->buffer + param->size), HW_CACHE_LINE_SIZE);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* AD 変換ビット幅 */
        switch (param->type)
        {
        case MIC_SAMPLING_TYPE_8BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_8BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_12BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_8BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_S8BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_S12BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
            flags   =   (SPI_MIC_SAMPLING_TYPE_12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
            flags   =   (SPI_MIC_SAMPLING_TYPE_S12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        default:
#ifdef  SDK_DEBUG
            OS_TWarning("%s: Illegal sampling type. (%d)\n", __FUNCTION__, param->type);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* サンプリング周期 */
        switch (param->rate)
        {
        case MIC_SAMPLING_RATE_32730:   case MIC_SAMPLING_RATE_16360:
        case MIC_SAMPLING_RATE_10910:   case MIC_SAMPLING_RATE_8180:
        case MIC_SAMPLING_RATE_47610:   case MIC_SAMPLING_RATE_23810:
        case MIC_SAMPLING_RATE_15870:   case MIC_SAMPLING_RATE_11900:
            break;
        default:
#ifdef  SDK_TWL
            OS_TWarning("%s: Illegal sampling rate. (%d)\n", __FUNCTION__, param->rate);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* ループ可否 */
        if (param->loop_enable)
        {
            flags   =   (u8)((flags & ~SPI_MIC_SAMPLING_TYPE_LOOP_MASK) | SPI_MIC_SAMPLING_TYPE_LOOP_ON);
        }
        else
        {
            flags   =   (u8)((flags & ~SPI_MIC_SAMPLING_TYPE_LOOP_MASK) | SPI_MIC_SAMPLING_TYPE_LOOP_OFF);
        }
        /* 拡張用補正フラグ */
        flags   =   (u8)((flags & ~SPI_MIC_SAMPLING_TYPE_CORRECT_MASK) | SPI_MIC_SAMPLING_TYPE_CORRECT_OFF);
    }

    /* 排他ロック確認 */
    e   =   OS_DisableInterrupts();
    if (w->lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return MIC_RESULT_BUSY;
    }
    w->lock =   MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(e);

    /* 制限付き自動サンプリング開始コマンドを送信 */
    w->callback =   callback;
    w->callbackArg  =   arg;
    w->full =   param->full_callback;
    w->fullArg  =   param->full_arg;
    if (MicexStartLimitedSampling(param->buffer, param->size, param->rate, flags))
    {
        return MIC_RESULT_SUCCESS;
    }
    w->lock =   MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StopLimitedSampling
  Description:  サンプリングレート限定のマイク自動サンプリングを停止する。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
  Arguments:    None.
  Returns:      MICResult - 同期的な処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StopLimitedSampling(void)
{
    MICResult       result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 割り込みハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return MIC_RESULT_ILLEGAL_STATUS;
    }

    /* 応答取得用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   MICEXi_StopLimitedSamplingAsync(MicexSyncCallback, (void*)(&msgQ));
    if (result == MIC_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StopLimitedSamplingAsync
  Description:  サンプリングレート限定のマイク自動サンプリングを停止する。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
  Arguments:    callback -  非同期処理完了時に呼び出すコールバック関数を指定。
                arg     -   コールバック関数に渡すパラメータを指定。
  Returns:      MICResult - 非同期処理開始処理の結果を返す。
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StopLimitedSamplingAsync(MICCallback callback, void* arg)
{
    OSIntrMode  e;
    MICWork*    w   =   MICi_GetSysWork();

    SDK_NULL_ASSERT(callback);

    /* 排他ロック確認 */
    e   =   OS_DisableInterrupts();
    if (w->lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return MIC_RESULT_BUSY;
    }
    w->lock =   MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(e);

    /* 制限付き自動サンプリング停止コマンドを送信 */
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (MicexStopLimitedSampling())
    {
        return MIC_RESULT_SUCCESS;
    }
    w->lock =   MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_AdjustLimitedSampling
  Description:  サンプリングレート限定のマイク自動サンプリングにおける
                サンプリング間隔を調整する。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
  Arguments:    rate    -   サンプリング間隔を ARM7 の CPU クロック単位で指定。
  Returns:      MICResult - 同期的な処理結果を返す。
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_AdjustLimitedSampling(u32 rate)
{
    MICResult       result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 割り込みハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return MIC_RESULT_ILLEGAL_STATUS;
    }

    /* 応答取得用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   MICEXi_AdjustLimitedSamplingAsync(rate, MicexSyncCallback, (void*)(&msgQ));
    if (result == MIC_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_AdjustLimitedSamplingAsync
  Description:  サンプリングレート限定のマイク自動サンプリングにおける
                サンプリング間隔を調整する。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
  Arguments:    rate    -   サンプリング間隔を ARM7 の CPU クロック単位で指定。
                callback -  非同期処理完了時に呼び出すコールバック関数を指定。
                arg     -   コールバック関数に渡すパラメータを指定。
  Returns:      MICResult - 非同期処理開始処理の結果を返す。
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg)
{
    OSIntrMode  e;
    MICWork*    w   =   MICi_GetSysWork();

    SDK_NULL_ASSERT(callback);

    /* パラメータ確認 */
    switch (rate)
    {
    case MIC_SAMPLING_RATE_32730:   case MIC_SAMPLING_RATE_16360:
    case MIC_SAMPLING_RATE_10910:   case MIC_SAMPLING_RATE_8180:
    case MIC_SAMPLING_RATE_47610:   case MIC_SAMPLING_RATE_23810:
    case MIC_SAMPLING_RATE_15870:   case MIC_SAMPLING_RATE_11900:
        break;
    default:
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Illegal sampling rate. (%d)\n", __FUNCTION__, rate);
#endif
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }

    /* 排他ロック確認 */
    e   =   OS_DisableInterrupts();
    if (w->lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return MIC_RESULT_BUSY;
    }
    w->lock =   MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(e);

    /* 制限付き自動サンプリング調整コマンドを送信 */
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (MicexAdjustLimitedSampling(rate))
    {
        return MIC_RESULT_SUCCESS;
    }
    w->lock =   MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MicexSyncCallback
  Description:  同期関数共用のコールバック関数。休止しているスレッドに非同期
                処理結果を送ることで動作可能状態に復帰させる。
  Arguments:    result  -   マイク操作の非同期処理結果が渡される。
                arg     -   同期関数呼び出しスレッドが受信待機しているメッセージ
                            キューへのポインタが渡される。
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
MicexSyncCallback(MICResult result, void* arg)
{
    SDK_NULL_ASSERT(arg);

    (void)OS_SendMessage((OSMessageQueue*)arg, (OSMessage)result, OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         MicexStartLimitedSampling
  Description:  制限付き自動サンプリング開始コマンドを ARM7 に発行する。
  Arguments:    buf     -   サンプリングしたデータを格納するバッファのアドレス。
                size    -   バッファのサイズをバイト単位で指定。
                rate    -   サンプリング間隔を ARM7 の CPU クロック単位で指定。
                flags   -   AD 変換のビット幅、ループ可否、補正可否を指定。
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合 TRUE、
                            失敗した場合 FALSE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
MicexStartLimitedSampling(void* buf, u32 size, u32 rate, u8 flags)
{
    /* PXIパケット[0] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_START_BIT | (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_LTDAUTO_ON << 8) | (u32)flags, 0))
    {
        return FALSE;
    }

    /* PXIパケット[1] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (1 << SPI_PXI_INDEX_SHIFT) | ((u32)buf >> 16), 0))
    {
        return FALSE;
    }

    /* PXIパケット[2] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (2 << SPI_PXI_INDEX_SHIFT) | ((u32)buf & 0x0000ffff), 0))
    {
        return FALSE;
    }

    /* PXIパケット[3] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (3 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    /* PXIパケット[4] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (4 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    /* PXIパケット[5] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (5 << SPI_PXI_INDEX_SHIFT) | (rate >> 16), 0))
    {
        return FALSE;
    }

    /* PXIパケット[6] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_END_BIT | (6 << SPI_PXI_INDEX_SHIFT) | (rate & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicexStopLimitedSampling
  Description:  制限付き自動サンプリング停止コマンドを ARM7 に発行する。
  Arguments:    None.
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合 TRUE、
                            失敗した場合 FALSE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
MicexStopLimitedSampling(void)
{
    /* PXIパケット[0] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_START_BIT | SPI_PXI_END_BIT | (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_LTDAUTO_OFF << 8), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicexAdjustLimitedSampling
  Description:  制限付き自動サンプリングレート変更コマンドを ARM7 に発行する。
  Arguments:    rate    -   サンプリング間隔を ARM7 の CPU クロック単位で指定。
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合 TRUE、
                            失敗した場合 FALSE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
MicexAdjustLimitedSampling(u32 rate)
{
    /* PXIパケット[0] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_START_BIT | (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_LTDAUTO_ADJUST << 8), 0))
    {
        return FALSE;
    }

    /* PXIパケット[1] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (1 << SPI_PXI_INDEX_SHIFT) | (rate >> 16), 0))
    {
        return FALSE;
    }

    /* PXIパケット[2] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_END_BIT | (2 << SPI_PXI_INDEX_SHIFT) | (rate & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}

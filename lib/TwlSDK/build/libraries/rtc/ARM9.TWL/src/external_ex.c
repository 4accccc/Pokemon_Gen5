/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     external_ex.c

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

#include    <nitro/os.h>
#include    <nitro/rtc.h>
#include    <twl/rtc/common/type_ex.h>
#include    <twl/rtc/ARM9/api_ex.h>
#include    "private.h"

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void     RtcexGetResultCallback(RTCResult result, void* arg);
static void     RtcexWaitBusy(u32* lock);

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetCounterAsync

  Description:  RTCからアップカウンタの値を非同期で読み出す。

  Arguments:    count     - アップカウンタの値を格納するバッファを指定。
                callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      RTCResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetCounterAsync(u32* count, RTCCallback callback, void* arg)
{
    OSIntrMode  enabled;
    RTCWork*    w   =   RTCi_GetSysWork();

    SDK_NULL_ASSERT(count);
    SDK_NULL_ASSERT(callback);

    /* ロック確認 */
    enabled =   OS_DisableInterrupts();
    if (w->lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    w->lock =   RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    /* アップカウンタ読み出しコマンドを送信 */
    w->sequence =   RTC_SEQ_GET_COUNTER;
    w->index    =   0;
    w->buffer[0]    =   (void*)count;
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (RTCEXi_ReadRawCounterAsync() == TRUE)
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        w->lock =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetCounter

  Description:  RTCからアップカウンタの値を読み出す。

  Arguments:    count     - アップカウンタの値を格納するバッファを指定。

  Returns:      RTCResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetCounter(u32* count)
{
    RTCWork*    w   =   RTCi_GetSysWork();

    w->commonResult =   RTCEXi_GetCounterAsync(count, RtcexGetResultCallback, NULL);
    if (w->commonResult == RTC_RESULT_SUCCESS)
    {
        RtcexWaitBusy(&(w->lock));
    }
    return w->commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetFoutAsync

  Description:  RTCからFOUTの設定値を非同期で読み出す。

  Arguments:    fout      - FOUTの設定値を格納するバッファを指定。
                callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      RTCResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetFoutAsync(u16* fout, RTCCallback callback, void* arg)
{
    OSIntrMode  enabled;
    RTCWork*    w   =   RTCi_GetSysWork();

    SDK_NULL_ASSERT(fout);
    SDK_NULL_ASSERT(callback);

    /* ロック確認 */
    enabled =   OS_DisableInterrupts();
    if (w->lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    w->lock =   RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    /* FOUT設定値読み出しコマンドを送信 */
    w->sequence =   RTC_SEQ_GET_FOUT;
    w->index    =   0;
    w->buffer[0]    =   (void*)fout;
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (RTCEXi_ReadRawFoutAsync() == TRUE)
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        w->lock =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetFout

  Description:  RTCからFOUTの設定値を非同期で読み出す。

  Arguments:    fout      - FOUTの設定値を格納するバッファを指定。

  Returns:      RTCResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetFout(u16* fout)
{
    RTCWork*    w   =   RTCi_GetSysWork();

    w->commonResult =   RTCEXi_GetFoutAsync(fout, RtcexGetResultCallback, NULL);
    if (w->commonResult == RTC_RESULT_SUCCESS)
    {
        RtcexWaitBusy(&(w->lock));
    }
    return w->commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_SetFoutAsync

  Description:  RTCにFOUT設定値を非同期で書き込む。

  Arguments:    fout      - FOUT設定値が格納されているバッファを指定。
                callback  - 非同期処理が完了した際に呼び出す関数を指定。
                arg       - コールバック関数呼び出し時の引数を指定。

  Returns:      RTCResult - 非同期デバイス操作開始の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_SetFoutAsync(const u16* fout, RTCCallback callback, void* arg)
{
    OSIntrMode  enabled;
    RTCWork*    w   =   RTCi_GetSysWork();

    SDK_NULL_ASSERT(fout);
    SDK_NULL_ASSERT(callback);

    /* ロック確認 */
    enabled =   OS_DisableInterrupts();
    if (w->lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    w->lock =   RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    /* 設定するデータを編集 */
    ((RTCRawDataEx*)(OS_GetSystemWork()->real_time_clock))->a.fout.fout =   *fout;

    /* FOUT設定値書き込みコマンドを送信 */
    w->sequence =   RTC_SEQ_SET_FOUT;
    w->index    =   0;
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (RTCEXi_WriteRawFoutAsync() == TRUE)
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        w->lock =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_SetFout

  Description:  RTCにFOUT設定値を書き込む。

  Arguments:    fout      - FOUT設定値が格納されているバッファを指定。

  Returns:      RTCResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_SetFout(const u16* fout)
{
    RTCWork*    w   =   RTCi_GetSysWork();

    w->commonResult =   RTCEXi_SetFoutAsync(fout, RtcexGetResultCallback, NULL);
    if (w->commonResult == RTC_RESULT_SUCCESS)
    {
        RtcexWaitBusy(&(w->lock));
    }
    return w->commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_CommonCallback

  Description:  非同期RTC関数用の追加コマンドに対する共通コールバック関数。

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      RTCResult   -   非同期デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_CommonCallback(void)
{
    RTCWork*    w   =   RTCi_GetSysWork();
    RTCResult   result  =   RTC_RESULT_SUCCESS;

    /* 内部状態に応じて各種事後処理 */
    switch(w->sequence)
    {
    /* アップカウンタの値取得シーケンス */
    case RTC_SEQ_GET_COUNTER:
        {
            u32*    pDst    =   (u32*)(w->buffer[0]);
            RTCRawCounter*  pSrc    =
                &(((RTCRawDataEx*)(OS_GetSystemWork()->real_time_clock))->a.counter);
            /* エンディアン変更 */
            *pDst   =   (u32)(pSrc->bytes[0] | (pSrc->bytes[1] << 8) | (pSrc->bytes[2] << 16));
        }
        break;
    /* FOUT設定値取得シーケンス */
    case RTC_SEQ_GET_FOUT:
        {
            u16*    pDst    =   (u16*)(w->buffer[0]);
            RTCRawFout* pSrc    =
                &(((RTCRawDataEx*)(OS_GetSystemWork()->real_time_clock))->a.fout);
            *pDst   =   (u16)(pSrc->fout);
        }
        break;
    /* FOUT設定値変更シーケンス */
    case RTC_SEQ_SET_FOUT:
        /* 特に処理なし */
        break;
    default:
        /* その他不明なシーケンス */
        w->index    =   0;
        result  =   RTC_RESULT_INVALID_COMMAND;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         RtcexGetResultCallback

  Description:  非同期処理の完了時に呼び出され、内部変数の処理結果を更新する。

  Arguments:    result - 非同期関数の処理結果。
                arg    - 使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RtcexGetResultCallback(RTCResult result, void* arg)
{
#pragma unused(arg)

    RTCi_GetSysWork()->commonResult =   result;
}

#include    <nitro/code32.h>
/*---------------------------------------------------------------------------*
  Name:         RtcWaitBusy

  Description:  RTCの非同期処理がロックされている間待つ。

  Arguments:    r0  -   ライブラリ共用ワーク内のロック制御メンバへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
RtcexWaitBusy(u32* lock)
{
loop:
    ldr     r1, [r0]
    cmp     r1, #RTC_LOCK_ON
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>

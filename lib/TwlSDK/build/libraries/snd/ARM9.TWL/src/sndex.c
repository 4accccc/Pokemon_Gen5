/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - snd
  File:     sndex.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-13#$
  $Rev: 10889 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/

#include    <twl/os.h>
#include    <twl/snd/ARM9/sndex.h>
#include    <nitro/pxi.h>
#include    <twl/os/common/codecmode.h>


/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/
#ifdef  SDK_DEBUG
#define     SNDEXi_Warning          OS_TWarning
#else
#define     SNDEXi_Warning(...)     ((void)0)
#endif

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

/* 非同期処理用ワーク構造体 */
typedef struct SNDEXWork
{
    SNDEXCallback   callback;
    void*           cbArg;
    void*           dest;
    u8              command;
    u8              padding[3];

} SNDEXWork;

/* 内部状態 */
typedef enum SNDEXState
{
    SNDEX_STATE_BEFORE_INIT       =   0,
    SNDEX_STATE_INITIALIZING      =   1,
    SNDEX_STATE_INITIALIZED       =   2,
    SNDEX_STATE_LOCKED            =   3,
    SNDEX_STATE_PLAY_SHUTTER      =   4,        // シャッター音再生中（ Pre,PostProcessForShutterSound 以外の SNDEX API を排他する）
    SNDEX_STATE_POST_PROC_SHUTTER =   5,        // シャッター音再生の後処理中（全ての SNDEX API を排他する）
    SNDEX_STATE_MAX

} SNDEXState;

/* 音声出力デバイス設定 */
typedef enum SNDEXDevice
{
    SNDEX_DEVICE_AUTO       =   0,
    SNDEX_DEVICE_SPEAKER    =   1,
    SNDEX_DEVICE_HEADPHONE  =   2,
    SNDEX_DEVICE_BOTH       =   3,
    SNDEX_DEVICE_MAX

} SNDEXDevice;

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/
static volatile SNDEXState   sndexState  =   SNDEX_STATE_BEFORE_INIT;
static SNDEXWork    sndexWork;
static SNDEXVolumeSwitchCallbackInfo SNDEXi_VolumeSwitchCallbackInfo = {NULL, NULL, NULL, 0};

PMSleepCallbackInfo sleepCallbackInfo;
PMExitCallbackInfo  exitCallbackInfo;
static volatile BOOL         isIirFilterSetting = FALSE;     // すでに SetIirFilterAsync の実行中なら TRUE
static volatile BOOL         isLockSpi = FALSE;              // SNDEX_SetIirFilter[Async] の中で、SPI の排他制御中かどうか

static volatile BOOL         isPlayShutter = FALSE;          // シャッター音再生処理中ならば TRUE
static volatile BOOL         isStoreVolume = FALSE;          // SNDEXi_SetIgnoreHWVolume によって保存されている音量値が存在するかどうか
static          u8           storeVolume   = 0;              // SNDEXi_SetIgnoreHWVolume によって一時的に変更される前の音量値

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
static void         CommonCallback  (PXIFifoTag tag, u32 data, BOOL err);
static void         SyncCallback    (SNDEXResult result, void* arg);
static BOOL         SendCommand     (u8 command, u8 param);
static BOOL         SendCommandEx   (u8 command, u16 param);
static SNDEXResult  CheckState       (void);
static void         ReplyCallback   (SNDEXResult result);
static void         SetSndexWork    (SNDEXCallback cb, void* cbarg, void* dst, u8 command);

static void         SndexSleepAndExitCallback    (void *arg);

static void         WaitIirFilterSetting   (void);
static void         WaitResetSoundCallback (SNDEXResult result, void* arg);
static void         ResetTempVolume        (void);

SNDEXResult SNDEXi_GetDeviceAsync       (SNDEXDevice* device, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetDevice            (SNDEXDevice* device);
SNDEXResult SNDEXi_SetDeviceAsync       (SNDEXDevice device, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetDevice            (SNDEXDevice device);

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_Init

  Description:  サウンド拡張機能ライブラリを初期化する。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void
SNDEXi_Init (void)
{
    OSIntrMode  e   =   OS_DisableInterrupts();

    /* 初期化済み確認 */
    if (sndexState != SNDEX_STATE_BEFORE_INIT)
    {
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("%s: Already initialized.\n", __FUNCTION__);
        return;
    }
    sndexState  =   SNDEX_STATE_INITIALIZING;
    (void)OS_RestoreInterrupts(e);

    /* 変数初期化 */
    SetSndexWork(NULL, NULL, NULL, 0);

    /* ARM7 の SNDEX ライブラリが開始されるまで待機 */
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_SNDEX, PXI_PROC_ARM7))
    {
        SVC_WaitByLoop(10);
    }
    /* PXI コールバック関数を設定 */
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_SNDEX, CommonCallback);
    
    /* ハードウェアリセット、シャットダウン時の 後処理コールバック関数を設定*/
    PM_SetExitCallbackInfo(&exitCallbackInfo, SndexSleepAndExitCallback, NULL);
    PMi_InsertPostExitCallbackEx(&exitCallbackInfo, PM_CALLBACK_PRIORITY_SNDEX);
    
    /* スリープ前のコールバック登録 */
    PM_SetSleepCallbackInfo(&sleepCallbackInfo, SndexSleepAndExitCallback, NULL);
    PMi_InsertPreSleepCallbackEx(&sleepCallbackInfo, PM_CALLBACK_PRIORITY_SNDEX);

    /* 初期化完了 */
    sndexState  =   SNDEX_STATE_INITIALIZED;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetMuteAsync

  Description:  音声出力の消音設定状態を非同期に取得する。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    mute        -   消音設定状態を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetMuteAsync (SNDEXMute* mute, SNDEXCallback callback, void* arg)
{
    /* パラメータ制限なし */
    
    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, (void*)mute, SNDEX_PXI_COMMAND_GET_SMIX_MUTE);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetMute

  Description:  音声出力の消音設定状態を取得する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    mute    -   消音設定状態を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetMute (SNDEXMute* mute)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_GetMuteAsync(mute, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetI2SFrequencyAsync

  Description:  I2S 周波数情報を非同期に取得する。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    freq        -   周波数情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetI2SFrequencyAsync (SNDEXFrequency* freq, SNDEXCallback callback, void* arg)
{
    /* パラメータ制限なし */

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, (void*)freq, SNDEX_PXI_COMMAND_GET_SMIX_FREQ);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetI2SFrequency

  Description:  I2S 周波数情報を取得する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    freq    -   周波数情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetI2SFrequency(SNDEXFrequency* freq)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_GetI2SFrequencyAsync(freq, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDSPMixRateAsync

  Description:  CPU と DSP の音声出力合成比情報を取得する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    rate        -   合成比情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetDSPMixRateAsync (u8* rate, SNDEXCallback callback, void* arg)
{
    /* パラメータ制限なし */

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, (void*)rate, SNDEX_PXI_COMMAND_GET_SMIX_DSP);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDSPMixRate

  Description:  CPU と DSP の音声出力合成比情報を取得する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    rate    -   合成比情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetDSPMixRate (u8* rate)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_GetDSPMixRateAsync(rate, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolumeAsync

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 7 。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    volume      -   音量情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。
                eightlv     -   音量を 8段階で取得する。
                keep        -   変更予約されている音量を取得する。予約が無い場合は現在の値を取得する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetVolumeAsync (u8* volume, SNDEXCallback callback, void* arg, BOOL eightlv, BOOL keep)
{
    /* パラメータ制限なし */

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, (void*)volume, SNDEX_PXI_COMMAND_GET_VOLUME);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command,
        (u8)((keep << SNDEX_PXI_PARAMETER_SHIFT_VOL_KEEP) | (eightlv << SNDEX_PXI_PARAMETER_SHIFT_VOL_8LV))))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolume

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 7 。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    volume  -   音量情報を取得するバッファを指定する。
                eightlv -   音量を 8段階で取得する。
                keep    -   変更予約されている音量を取得する。予約が無い場合は現在の値を取得する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetVolume (u8* volume, BOOL eightlv, BOOL keep)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_GetVolumeAsync(volume, SyncCallback, (void*)(&msgQ), eightlv, keep);
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolumeExAsync

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 31 。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    volume      -   音量情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetVolumeExAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolumeAsync(volume, callback, arg, FALSE, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolumeEx

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 31 。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    volume  -   音量情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetVolumeEx (u8* volume)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolume(volume, FALSE, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetCurrentVolumeEx[Async]

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 31 。
                同期関数の場合、割込みハンドラ内からの呼び出しは禁止。
                非同期関数の場合、結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    volume   -   音量情報を取得するバッファを指定する。
               [callback -   非同期処理完了時に結果を通知するコールバック関数を指定する。]
               [arg      -   コールバック関数に渡すパラメータを指定する。]

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetCurrentVolumeExAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolumeAsync(volume, callback, arg, FALSE, FALSE);
}

SNDEXResult
SNDEXi_GetCurrentVolumeEx (u8* volume)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolume(volume, FALSE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDeviceAsync

  Description:  音声出力デバイス設定情報を取得する。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    device      -   出力デバイス情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetDeviceAsync (SNDEXDevice* device, SNDEXCallback callback, void* arg)
{
    /* パラメータ制限なし */

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, (void*)device, SNDEX_PXI_COMMAND_GET_SND_DEVICE);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDevice

  Description:  音声出力デバイス設定情報を取得する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    device  -   出力デバイス情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetDevice (SNDEXDevice* device)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_GetDeviceAsync(device, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetMuteAsync

  Description:  音声出力の消音設定状態を変更する。

  Arguments:    mute        -   消音設定状態を指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetMuteAsync (SNDEXMute mute, SNDEXCallback callback, void* arg)
{
    /* パラメータ制限確認 */
    if (mute >= SNDEX_MUTE_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (mute: %d)\n", __FUNCTION__, mute);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SMIX_MUTE);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, (u8)mute))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetMute

  Description:  音声出力の消音設定状態を変更する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    mute    -   消音設定状態を指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetMute (SNDEXMute mute)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_SetMuteAsync(mute, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetI2SFrequencyAsync

  Description:  I2S 周波数を変更する。

  Arguments:    freq        -   周波数を指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetI2SFrequencyAsync (SNDEXFrequency freq, SNDEXCallback callback, void* arg)
{
    /* パラメータ制限確認 */
    if (freq >= SNDEX_FREQUENCY_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (freq: %d)\n", __FUNCTION__, freq);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* CODEC 動作モードを確認 */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SMIX_FREQ);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, (u8)freq))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetI2SFrequency

  Description:  I2S 周波数を変更する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    freq    -   周波数を指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetI2SFrequency (SNDEXFrequency freq)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_SetI2SFrequencyAsync(freq, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDSPMixRateAsync

  Description:  CPU と DSP の音声出力合成比を変更する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。

  Arguments:    rate        -   合成比を 0 から 8 の数値で指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetDSPMixRateAsync (u8 rate, SNDEXCallback callback, void* arg)
{
    /* CODEC 動作モードを確認 */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        SNDEXi_Warning("%s: Illegal state\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* パラメータ制限確認 */
    if (rate > SNDEX_DSP_MIX_RATE_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (rate: %u)\n", __FUNCTION__, rate);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SMIX_DSP);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, (u8)rate))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDSPMixRate

  Description:  CPU と DSP の音声出力合成比を変更する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    rate    -   合成比を 0 から 8 の数値で指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetDSPMixRate (u8 rate)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_SetDSPMixRateAsync(rate, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeAsync

  Description:  音声出力音量を変更する。音量は最小: 0, 最大: 7 。

  Arguments:    volume      -   音量を 0 から 7 の数値で指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。
                eightlv     -   音量を 8段階で設定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetVolumeAsync (u8 volume, SNDEXCallback callback, void* arg, BOOL eightlv)
{
    /* パラメータ制限確認 */
    if (volume > (eightlv ? SNDEX_VOLUME_MAX : SNDEX_VOLUME_MAX_EX))
    {
        SNDEXi_Warning("%s: Invalid parameter (volume: %u)\n", __FUNCTION__, volume);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_VOLUME);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 
        (u8)((eightlv << SNDEX_PXI_PARAMETER_SHIFT_VOL_8LV) | (SNDEX_PXI_PARAMETER_MASK_VOL_VALUE & volume))))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolume

  Description:  音声出力音量を変更する。音量は最小: 0, 最大: 7 。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    volume  -   音量を 0 から 7 の数値で指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetVolume (u8 volume, BOOL eightlv)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_SetVolumeAsync(volume, SyncCallback, (void*)(&msgQ), eightlv);
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeExAsync

  Description:  音声出力音量を変更する。音量は最小: 0, 最大: 31 。

  Arguments:    volume      -   音量を 0 から 31 の数値で指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetVolumeExAsync (u8 volume, SNDEXCallback callback, void* arg)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_SetVolumeAsync(volume, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeEx

  Description:  音声出力音量を変更する。音量は最小: 0, 最大: 31 。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    volume  -   音量を 0 から 31 の数値で指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetVolumeEx (u8 volume)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_SetVolume(volume, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDeviceAsync

  Description:  音声出力デバイス設定を変更する。

  Arguments:    device      -   出力デバイス設定を指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetDeviceAsync (SNDEXDevice device, SNDEXCallback callback, void* arg)
{
    /* パラメータ制限確認 */
    if (device >= SNDEX_DEVICE_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (device: %d)\n", __FUNCTION__, device);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* CODEC 動作モードを確認 */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SND_DEVICE);

    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, (u8)device))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDevice

  Description:  音声出力デバイス設定を変更する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    device  -   出力デバイス設定を指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetDevice (SNDEXDevice device)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_SetDeviceAsync(device, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIirFilterAsync

  Description:  IIRフィルター(Biquad)のパラメータ設定を非同期で行います。

  !! CAUTION !!: ARM7へ１度のPXI送信ではパラメータを全て渡すことができないので
                 複数回連続送信している。ただ連続で送信するだけでは、前に送った
                 パラメータの処理が終わってないことがあるため、PXI送信の前に
                 処理の終了を待つことにしている。

  Arguments:    target : 
                pParam :

  Returns:      None
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetIirFilterAsync(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam, SNDEXCallback callback, void* arg)
{
    // 既に SetIirFilterAsync の処理中ならばエラーを返す
    if (isIirFilterSetting)
    {
        return SNDEX_RESULT_EXCLUSIVE;
    }
    isIirFilterSetting = TRUE;
    
    /* CODEC 動作モードを確認 */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        SNDEXi_Warning("%s: Illegal state\n", __FUNCTION__);
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            isIirFilterSetting = FALSE;
            return result;
        }
    }
    
  /* ターゲット */
    /* 非同期処理登録 */
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_TARGET);
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommandEx(sndexWork.command, (u16)target))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    // CheckState() による Warning 抑制のために空ループを回してコールバックを待つ
    OS_SpinWait(67 * 1000); //約 1ms
    
    // 前の PXI送信による処理が終わるまで待つ
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //約 1ms
    }
    
  /* フィルタパラメータ */
    // n0
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N0);
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->n0))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //約 1ms

    // 前の PXI送信による処理が終わるまで待つ
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //約 1ms
    }

    // n1
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N1);
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->n1))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //約 1ms
    
    // 前の PXI送信による処理が終わるまで待つ
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //約 1ms
    }
    
    // n2
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N2);
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->n2))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //約 1ms
    
    // 前の PXI送信による処理が終わるまで待つ
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //約 1ms
    }
    
    // d1
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_D1);
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->d1))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //約 1ms
    
    // 前の PXI送信による処理が終わるまで待つ
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //約 1ms
    }
    
    // d2
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_D2);
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->d2))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //約 1ms
    
    // 前の PXI送信による処理が終わるまで待つ
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //約 1ms
    }
    
  /* ターゲット、フィルタパラメータをセット */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER);
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommandEx(sndexWork.command, 0))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetIirFilter

  Description:  IIRフィルター(Biquad)のパラメータ設定を行います。

  Arguments:    target : 
                pParam :

  Returns:      None
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetIirFilter(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);
    
    result = SNDEXi_SetIirFilterAsync(target, pParam, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    
    // SetIirFilter による SPI_Lock が終わったとみなしてフラグをおろす
    isLockSpi = FALSE;
    
    isIirFilterSetting = FALSE;
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphoneAsync

  Description:  ヘッドフォンの接続の有無を取得する。

  Arguments:    hp          -   ヘッドフォンの接続状況を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_IsConnectedHeadphoneAsync(SNDEXHeadphone *hp, SNDEXCallback callback, void* arg)
{
    /* 状態確認 */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* 非同期処理登録 */
    SetSndexWork(callback, arg, (void*)hp, SNDEX_PXI_COMMAND_HP_CONNECT);
    
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphone

  Description:  ヘッドフォンの接続の有無を取得する。。

  Arguments:    hp          -   ヘッドフォンの接続状況を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_IsConnectedHeadphone(SNDEXHeadphone *hp)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   SNDEXi_IsConnectedHeadphoneAsync(hp, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeSwitchCallback

  Description:  音量調整ボタン押下時に呼ばれるコールバック関数を設定する。

  Arguments:    callback    -   音量調整ボタン押下時に呼ばれるコールバック関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      なし
 *---------------------------------------------------------------------------*/
void SNDEXi_SetVolumeSwitchCallback(SNDEXCallback callback, void* arg)
{
    SNDEXi_VolumeSwitchCallbackInfo.callback = callback;
    SNDEXi_VolumeSwitchCallbackInfo.cbArg = arg;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetIgnoreHWVolume

  Description:  時計のアラーム音など、そのときの本体音量の値にとらわれず
                指定した音量で音を鳴らす必要がある場合、ユーザの操作によらず
                音量変更を行わねばならない。
                本関数では、音量変更を行うだけでなく、変更前の音量を保存する。

  Arguments:    volume      -   変更したい音量値。

  Returns:      SNDEXResult -   処理結果を返す。内部で呼んでいる SNDEX API の結果。
 *---------------------------------------------------------------------------*/
SNDEXResult SNDEXi_SetIgnoreHWVolume(u8 volume, BOOL eightlv)
{
    SNDEXResult result;
    
    // 前回 SetIgnoreHWVolume を呼んでから ResetIgnoreHWVolume が実行されていない場合は
    // 音量値を保存しない
    if (!isStoreVolume)
    {
        result = SNDEXi_GetVolumeEx(&storeVolume);
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }
    
    // 指定した音量へ変更
    result = eightlv ? SNDEX_SetVolume(volume) : SNDEXi_SetVolumeEx(volume);
    if (result != SNDEX_RESULT_SUCCESS)
    {
        return result;
    }
    
    isStoreVolume = TRUE;
    return result;      // SNDEX_RESULT_SUCCESS
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_ResetIgnoreHWVolume

  Description:  SNDEX_SetIgnoreHWVolume で変更した音量を、変更前の音量へ戻す。

  Arguments:    なし
  
  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult SNDEXi_ResetIgnoreHWVolume(void)
{
    SNDEXResult result;
    if ((result = SNDEXi_SetVolumeEx(storeVolume)) != SNDEX_RESULT_SUCCESS)
    {
        return result;
    }
    
    isStoreVolume = FALSE;
    return result;          // SNDEX_RESULT_SUCCESS
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_PreProcessForShutterSound

  Description:  シャッター音再生のために必要な前処理をまとめた関数。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    なし

  Returns:      成功すれば SNDEX_RESULT_SUCCESS を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_PreProcessForShutterSound  (void)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];
    
    /* CODEC 動作モードを確認 */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* 例外ハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* 既に SNDEXi_PreProcessForShutterSound() が呼ばれている状態なら CheckState はスルー */
    if (sndexState != SNDEX_STATE_PLAY_SHUTTER)
    {
        /* 状態確認 */
        result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }
    else if (sndexState == SNDEX_STATE_POST_PROC_SHUTTER)
    {
        return SNDEX_RESULT_EXCLUSIVE;
    }
    
    /* 同期用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);
    
    
    /* 処理登録 */
    SetSndexWork(SyncCallback, (void*)(&msgQ), NULL, SNDEX_PXI_COMMAND_PRE_PROC_SHUTTER);
    
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }

    /* リプライ待ち */
    (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_PostProcessCallbackForShutterSound

  Description:  シャッター音再生のための後処理を実行する。
                割り込みの中で実行されるため、非同期で実行する。

  Arguments:    callback    -   後処理終了時に呼ばれるコールバック関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      成功すれば SNDEX_RESULT_SUCCESS を返す。
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_PostProcessForShutterSound (SNDEXCallback callback, void* arg)
{
    /* CODEC 動作モードを確認 */
    // この関数が呼ばれるときは、SNDEXi_PreProcessCallbackForShutterSound の実行後が前提
    if (OSi_IsCodecTwlMode() == FALSE || sndexState != SNDEX_STATE_PLAY_SHUTTER)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* ステートを 後処理中に遷移（優先度最大） */
    sndexState = SNDEX_STATE_POST_PROC_SHUTTER;
    
    /* 非同期処理登録 */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_POST_PROC_SHUTTER);
    
    /* ARM7 へコマンド発行 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CommonCallback

  Description:  サウンド拡張機能に関する PXI FIFO 受信コールバック関数。

  Arguments:    tag     -   PXI メッセージ種別。
                data    -   ARM7 からのメッセージ内容。
                err     -   PXI 転送エラーフラグ。

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
CommonCallback (PXIFifoTag tag, u32 data, BOOL err)
{
    u8      command =   (u8)((data & SNDEX_PXI_COMMAND_MASK) >> SNDEX_PXI_COMMAND_SHIFT);
    u8      result  =   (u8)((data & SNDEX_PXI_RESULT_MASK) >> SNDEX_PXI_RESULT_SHIFT);
    u8      param   =   (u8)((data & SNDEX_PXI_PARAMETER_MASK) >> SNDEX_PXI_PARAMETER_SHIFT);
    
    /* ARM7 から受信したコマンドが本体ボリュームスイッチ押下通知ならロックを試みる */
    if (command == SNDEX_PXI_COMMAND_PRESS_VOLSWITCH)
    {
        SNDEXResult result = CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return;
        }
        // sndexWork 構造体をセットする
        SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_PRESS_VOLSWITCH);
    }
    
    /* 状態確認 */
    if ((tag != PXI_FIFO_TAG_SNDEX) || (err == TRUE)
            || (sndexState != SNDEX_STATE_LOCKED && sndexState != SNDEX_STATE_PLAY_SHUTTER && sndexState != SNDEX_STATE_POST_PROC_SHUTTER)
            || (sndexWork.command != command))
    {
        /* ロジック上は通り得ないが、念のため警告を出力 */
        OS_TWarning("SNDEX: Library state is inconsistent.\n");
        ReplyCallback(SNDEX_RESULT_FATAL_ERROR);
        return;
    }

    /* 処理結果解析 */
    switch (result)
    {
    case SNDEX_PXI_RESULT_SUCCESS:
        /* コマンドに応じて完了処理 */
        switch (command)
        {
        case SNDEX_PXI_COMMAND_GET_SMIX_MUTE:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXMute*)sndexWork.dest)   = (SNDEXMute)param;
            }
            break;
        case SNDEX_PXI_COMMAND_GET_SMIX_FREQ:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXFrequency*)sndexWork.dest)  =   (SNDEXFrequency)param;
            }
            break;
        case SNDEX_PXI_COMMAND_GET_SMIX_DSP:
        case SNDEX_PXI_COMMAND_GET_VOLUME:
            if (sndexWork.dest != NULL)
            {
                *((u8*)sndexWork.dest)  =   param;
            }
            break;
        case SNDEX_PXI_COMMAND_GET_SND_DEVICE:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXDevice*)sndexWork.dest) =   (SNDEXDevice)param;
            }
            break;
        case SNDEX_PXI_COMMAND_PRESS_VOLSWITCH:
            /* ARM7からの音量調整ボタン押下通知 */
            if (SNDEXi_VolumeSwitchCallbackInfo.callback != NULL)
            {
                (SNDEXi_VolumeSwitchCallbackInfo.callback)((SNDEXResult)result, SNDEXi_VolumeSwitchCallbackInfo.cbArg);
            }
            break;
        case SNDEX_PXI_COMMAND_HP_CONNECT:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXHeadphone*)sndexWork.dest) =   (SNDEXHeadphone)param;
            }
            break;
        }
        ReplyCallback(SNDEX_RESULT_SUCCESS);
        break;
    case SNDEX_PXI_RESULT_INVALID_PARAM:
        ReplyCallback(SNDEX_RESULT_INVALID_PARAM);
        break;
    case SNDEX_PXI_RESULT_EXCLUSIVE:
        ReplyCallback(SNDEX_RESULT_EXCLUSIVE);
        break;
    case SNDEX_PXI_RESULT_ILLEGAL_STATE:
        ReplyCallback(SNDEX_RESULT_ILLEGAL_STATE);
        break;
    case SNDEX_PXI_RESULT_DEVICE_ERROR:
        if (command == SNDEX_PXI_COMMAND_GET_VOLUME)
        {
            if (sndexWork.dest != NULL)
            {
                *((u8*)sndexWork.dest)  =   SNDEX_VOLUME_MIN;
            }
        }
        ReplyCallback(SNDEX_RESULT_DEVICE_ERROR);
        break;
    case SNDEX_PXI_RESULT_INVALID_COMMAND:
    default:
        ReplyCallback(SNDEX_RESULT_FATAL_ERROR);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SyncCallback

  Description:  同期関数制御用の共通コールバック関数。

  Arguments:    result  -   非同期処理の処理結果。
                arg     -   同期関数が受信を待ってブロックしているメッセージ
                            キューの制御構造体へのポインタ。

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
SyncCallback (SNDEXResult result, void* arg)
{
    SDK_NULL_ASSERT(arg);

    if (FALSE == OS_SendMessage((OSMessageQueue*)arg, (OSMessage)result, OS_MESSAGE_NOBLOCK))
    {
        /* ロジック上は通り得ないが、念のため警告を出力 */
        OS_TWarning("SNDEX: Temporary message queue is full.\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         SendCommand

  Description:  PXI 経由で ARM7 にサウンド拡張機能操作コマンドを発行する。

  Arguments:    command -   コマンド種別。
                param   -   コマンドに付随するパラメータ。

  Returns:      BOOL    -   PXI 送信に成功した場合に TRUE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
SendCommand (u8 command, u8 param)
{
    OSIntrMode  e   =   OS_DisableInterrupts();
    u32     packet  =   (u32)(((command << SNDEX_PXI_COMMAND_SHIFT) & SNDEX_PXI_COMMAND_MASK)
                        | ((param << SNDEX_PXI_PARAMETER_SHIFT) & SNDEX_PXI_PARAMETER_MASK));

    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_SNDEX, packet, 0))
    {
        /* 送信に失敗した場合は、排他制御を解除 */
        sndexState  =   SNDEX_STATE_INITIALIZED;
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Failed to send PXI command.\n");
        return FALSE;
    }
    
    (void)OS_RestoreInterrupts(e);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SendCommand

  Description:  PXI 経由で ARM7 にサウンド拡張機能操作コマンドを発行する。

  Arguments:    command -   コマンド種別。
                param   -   コマンドに付随するパラメータ(IirFilter のパラメータのために 16ビットにしている）

  Returns:      BOOL    -   PXI 送信に成功した場合に TRUE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
SendCommandEx (u8 command, u16 param)
{
    OSIntrMode  e   =   OS_DisableInterrupts();
    u32     packet  =   (u32)(((command << SNDEX_PXI_COMMAND_SHIFT) & SNDEX_PXI_COMMAND_MASK)
                        | ((param << SNDEX_PXI_PARAMETER_SHIFT) & SNDEX_PXI_PARAMETER_MASK_IIR));

    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_SNDEX, packet, 0))
    {
        /* 送信に失敗した場合は、排他制御を解除 */
        sndexState  =  SNDEX_STATE_INITIALIZED;
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Failed to send PXI command.\n");
        return FALSE;
    }
    if( command == SNDEX_PXI_COMMAND_SET_IIRFILTER )
    {
        isLockSpi = TRUE;
    }
    
    (void)OS_RestoreInterrupts(e);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CheckState

  Description:  内部状態が非同期処理開始できる状態であるかを調べ、
                できるならば SNDEX ライブラリをロックする。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SNDEXResult
CheckState (void)
{
    OSIntrMode  e   =   OS_DisableInterrupts();

    /* 内部状態確認 */
    switch (sndexState)
    {
    case SNDEX_STATE_BEFORE_INIT:
    case SNDEX_STATE_INITIALIZING:
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Library is not initialized yet.\n");
        return SNDEX_RESULT_BEFORE_INIT;
    case SNDEX_STATE_LOCKED:
    case SNDEX_STATE_PLAY_SHUTTER:
    case SNDEX_STATE_POST_PROC_SHUTTER:
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Another request is in progress.\n");
        return SNDEX_RESULT_EXCLUSIVE;
    }

    /* 排他制御中状態に移行 */
    sndexState  =   SNDEX_STATE_LOCKED;
    (void)OS_RestoreInterrupts(e);
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ReplyCallback

  Description:  非同期処理の完了を通知する。内部状態の更新、排他状態の解除も
                合わせて行う。

  Arguments:    result  -   コールバック関数に通知する非同期処理結果。

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
ReplyCallback (SNDEXResult result)
{
    OSIntrMode      e           =   OS_DisableInterrupts();
    SNDEXCallback   callback    =   sndexWork.callback;
    void*           cbArg       =   sndexWork.cbArg;
    u8              command     =   sndexWork.command;
    
    // SetIirFilterAsync による SPI_Lock が終わったとみなしてフラグをおろす
    if (sndexWork.command == SNDEX_PXI_COMMAND_SET_IIRFILTER)
    {
        isLockSpi = FALSE;
        isIirFilterSetting = FALSE;
    }
    
    /* 状態リセット */
    SetSndexWork(NULL, NULL, NULL, 0);
    // シャッター音再生前処理を実行すると、後処理の実行までロックを解除しない
    if (command == SNDEX_PXI_COMMAND_PRE_PROC_SHUTTER)
    {
        // シャッター音再生後処理を行う SNDEXi_PostProcessForShutterSound() 以外は
        // ロック状態と見なして排他される。
        sndexState          =   SNDEX_STATE_PLAY_SHUTTER;
    }
    else
    {
        sndexState          =   SNDEX_STATE_INITIALIZED;
    }
    (void)OS_RestoreInterrupts(e);

    /* 登録されているコールバック関数呼び出し */
    if (callback != NULL)
    {
        callback(result, cbArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetSndexWork

  Description:  sndexWork にパラメータをセット

  Arguments:    cb : コールバック関数へのポインタ
                cbarg : cb へ渡す引数
                dst : コマンドの処理結果を格納する変数へのポインタ
                command : ARM7 へ送信するコマンド

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
SetSndexWork    (SNDEXCallback cb, void* cbarg, void* dst, u8 command)
{
    sndexWork.callback = cb;
    sndexWork.cbArg = cbarg;
    sndexWork.dest = dst;
    sndexWork.command = command;
}

/*---------------------------------------------------------------------------*
  Name:         SndexSleepAndExitCallback

  Description:  ハードウェアリセット・シャットダウン、またはスリープ時に
                呼ばれるコールバック関数

  Arguments:    arg

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
SndexSleepAndExitCallback (void *arg)
{
#pragma unused( arg )
    // IIR フィルタ処理実行中は、SNDEX 内で他の SNDEX API が排他されてしまうため
    // 終了を必ず先に待つこと
    WaitIirFilterSetting();
    
    ResetTempVolume();
}

/*---------------------------------------------------------------------------*
  Name:         WaitIirFilterSetting

  Description:  SNDEX_SetIirFilter[Async] が終了するまで待つ

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
WaitIirFilterSetting (void)
{
    // SNDEX_SetIirFilter[Async]内で行っている SPI_Lock が終了するまで待ち続ける
    while (isLockSpi)
    {
        OS_SpinWait(67 * 1000); //約 1ms
        PXIi_HandlerRecvFifoNotEmpty();
    }
}

static void 
WaitResetSoundCallback(SNDEXResult result, void* arg)
{
    static u32 i = 0;    // リトライは5回までとする
#pragma unused(arg)
    if (result != SNDEX_RESULT_SUCCESS && i < 5)
    {
        (void)SNDEXi_SetVolumeExAsync(storeVolume, WaitResetSoundCallback, NULL);
        i++;
        return;
    }
    isStoreVolume = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ResetTempVolume

  Description:  SNDEXi_SetIgnoreHWVolume で一時保存された音量へ戻す

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
ResetTempVolume (void)
{
    if (isStoreVolume)
    {
        static i = 0;    // リトライは5回までとする
        // SNDEXi_SetIgnoreHWVolume で一時保存されていた音量に戻す
        // 非同期関数の実行が成功するまで繰り返す
        // 音量変更の成否についてのリトライはコールバック内で行う
        while( SNDEX_RESULT_SUCCESS != SNDEXi_SetVolumeExAsync(storeVolume, WaitResetSoundCallback, NULL) && i < 5)
        {
            i++;
        }
        while (isStoreVolume)
        {
            OS_SpinWait(67 * 1000); //約 1ms
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}


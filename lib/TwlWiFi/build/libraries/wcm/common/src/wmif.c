/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     wmif.c

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

#include "wcm_private.h"
#include "wcm_message.h"

/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/
static void         WcmWmReset(void);
static void         WcmEditScanExParam(const void* bssid, const void* essid, u32 option);

static void         WcmWmcbIndication(void* arg);
static void         WcmWmcbCommon(void* arg);
static void         WcmWmcbScanEx(void* arg);
static void         WcmWmcbEndScan(void* arg);
static void         WcmWmcbConnect(void* arg);
static void         WcmWmcbDisconnect(void* arg);
static void         WcmWmcbStartDCF(void* arg);
static void         WcmWmcbEndDCF(void* arg);
static void         WcmWmcbReset(void* arg);
static void         WcmWmcbSetDCFData(void* arg);
static void         WcmWmcbSetDCFDataAsync(void* arg);
static void         WcmKACallback(void* arg);

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifStartupAsync

  Description:  無線機能の起動シーケンスを開始

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_WmifStartupAsync(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    WMErrCode   wmResult;

    // WM ライブラリ初期化
    wmResult = WM_Init(wcmw->wm.work, (u16) wcmw->config.dmano);
    switch (wmResult)
    {
    case WM_ERRCODE_SUCCESS:
        break;

    case WM_ERRCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // WM ライブラリが既に初期化されている

    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_UnsuitableArm7);
        return WCM_RESULT_WMDISABLE;            // ARM7 側で WM ライブラリが動作していない

    case WM_ERRCODE_INVALID_PARAM:
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }

    // 不定期な ARM7 からの通知を受け取るベクトルを設定
    wmResult = WM_SetIndCallback(WcmWmcbIndication);
    if (wmResult != WM_ERRCODE_SUCCESS)
    {
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }

    // CPS に渡す MAC アドレスを設定
    {
        u8 macAddress[WM_SIZE_MACADDR];
        OS_GetMacAddress(macAddress);
        WCMi_CpsifSetMacAddress(macAddress);
    }

    // 無線ハードウェア使用許可要求を発行
    wmResult = WM_Enable(WcmWmcbCommon);
    switch (wmResult)
    {
    case WM_ERRCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_WAIT_TO_IDLE);    // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_STARTUP;
        break;

    case WM_ERRCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "WM_Enable", wmResult);
        WCMi_SetPhase(WCM_PHASE_IRREGULAR);
        return WCM_RESULT_FAILURE;              // ARM7 への要求発行に失敗

    case WM_ERRCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifCleanupAsync

  Description:  無線機能の停止シーケンスを開始

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_WmifCleanupAsync(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    WMErrCode   wmResult;

    // 無線ハードウェアのシャットダウン要求を発行
    wmResult = WM_PowerOff(WcmWmcbCommon);
    switch (wmResult)
    {
    case WM_ERRCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_IDLE_TO_WAIT);    // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_CLEANUP;
        break;

    case WM_ERRCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "WM_PowerOff", wmResult);
        return WCM_RESULT_REJECT;               // ARM7 への要求発行に失敗( リトライ可能 )

    case WM_ERRCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifBeginSearchAsync

  Description:  AP 自動探索開始シーケンスを開始、もしくは探索条件変更

  Arguments:    bssid        - 
                essid        - 
                option       - 

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_WmifBeginSearchAsync(const void* bssid, const void* essid, u32 option)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    WMErrCode   wmResult;

    switch (wcmw->phase)
    {
    case WCM_PHASE_IDLE:
        break;

    case WCM_PHASE_IDLE_TO_SEARCH:
        WcmEditScanExParam(bssid, essid, option);
        return WCM_RESULT_PROGRESS;             // 既に同じリクエストの非同期処理中

    case WCM_PHASE_SEARCH:
        WcmEditScanExParam(bssid, essid, option);
        return WCM_RESULT_SUCCESS;              // 状態遷移目標フェーズに移行済み

    default:
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        return WCM_RESULT_FAILURE;              // 要求を受け付けられないフェーズ
    }

    // スキャン開始要求を発行
    WcmEditScanExParam(bssid, essid, option);
    DC_InvalidateRange(wcmw->wm.scanExParam.scanBuf, wcmw->wm.scanExParam.scanBufSize);
    wcmw->wm.scanCount++;
    wmResult = WM_StartScanEx(WcmWmcbScanEx, &(wcmw->wm.scanExParam));
    switch (wmResult)
    {
    case WM_ERRCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_IDLE_TO_SEARCH);  // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_BEGIN_SEARCH;
        break;

    case WM_ERRCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "WM_StartScanEx", wmResult);
        return WCM_RESULT_REJECT;               // ARM7 への要求発行に失敗( リトライ可能 )

    case WM_ERRCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifConnectAsync

  Description:  AP との無線接続シーケンスを開始する。
                内部的には IDLE から DCF へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_CONNECT となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCMi_WmifConnectAsync(void)
{
    s32         wcmResult   = WCM_RESULT_SUCCESS;
    WCMWork*    wcmw        = WCMi_GetSystemWork();

    SDK_POINTER_ASSERT(wcmw);

    if (wcmw->phase == WCM_PHASE_IDLE)
    {
        // サポートするデータ転送レートを NITRO 用に改変
        wcmw->bssDesc.rateSet.support = (u16) (WCM_ADDITIONAL_RATESET | wcmw->bssDesc.rateSet.basic);
    }

    // AP との無線接続シーケンスを開始
    {
        WMErrCode wmResult;
        // 無線ファームウェアのタイムアウト設定要求を発行
        wmResult = WM_SetLifeTime(WcmWmcbCommon, 0xffff, WCM_CAM_LIFETIME, 0xffff, 0xffff);
        switch (wmResult)
        {
        case WM_ERRCODE_OPERATING:
            WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF); // 非同期的なシーケンスの開始に成功
            wcmw->notifyId = WCM_NOTIFY_CONNECT;
            break;

        case WM_ERRCODE_FIFO_ERROR:
            WCMi_Printf(WCMi_RText_WmSyncError, "WM_SetLifeTime", wmResult);
            return WCM_RESULT_REJECT;           // ARM7 への要求発行に失敗( リトライ可能 )

        case WM_ERRCODE_ILLEGAL_STATE:
            WCMi_Warning(WCMi_WText_InvalidWmState);

        /* Don't break here */
        default:
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
            return WCM_RESULT_FATAL_ERROR;      // 想定範囲外のエラー
        }
    }
    // 正常終了
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifDisconnectAsync

  Description:  

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_WmifDisconnectAsync(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    WMErrCode   wmResult;

    // DCF 通信モード終了要求を発行
    wmResult = WM_EndDCF(WcmWmcbEndDCF);
    switch (wmResult)
    {
    case WM_ERRCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE); // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_DISCONNECT;
        break;

    case WM_ERRCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "WM_EndDCF", wmResult);
        return WCM_RESULT_REJECT;           // ARM7 への要求発行に失敗( リトライ可能 )

    case WM_ERRCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;      // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifTerminateAsync

  Description:  

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_WmifTerminateAsync(void)
{
    WMStatus*   ws;
    u16         wmState;
    WCMWork*    wcmw = WCMi_GetSystemWork();
    WMErrCode   wmResult;

    SDK_POINTER_ASSERT(wcmw);

    // WM ライブラリの内部状態を調査
    ws = (WMStatus*)WMi_GetStatusAddress();
    DC_InvalidateRange(ws, 2);
    wmState = ws->state;

    // WM 内部状態により停止状態に遷移するよう連鎖を開始
    switch (wmState)
    {
    case WM_STATE_READY:
        // WM ライブラリ終了
        wmResult = WM_Finish();
        if (wmResult == WM_ERRCODE_SUCCESS)
        {
            WCMi_SetPhase(WCM_PHASE_WAIT);
            wcmw->notifyId = WCM_NOTIFY_COMMON;
            return WCM_RESULT_SUCCESS;      // 同期的に WAIT フェーズへの移行が完了
        }
        break;

    case WM_STATE_STOP:
        // 無線ハードウェアの使用禁止要求を発行
        wmResult = WM_Disable(WcmWmcbCommon);
        break;

    case WM_STATE_IDLE:
        // 無線ハードウェアのシャットダウン要求を発行
        wmResult = WM_PowerOff(WcmWmcbCommon);
        break;

    default:
        // リセット重複呼び出し管理フラグをセット
        wcmw->resetting = WCM_RESETTING_ON;

        // 無線接続状態のリセット要求を発行
        wmResult = WM_Reset(WcmWmcbReset);
    }

    // 要求発行に対する同期的な処理結果を確認
    switch (wmResult)
    {
    case WM_ERRCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_TERMINATING); // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_TERMINATE;
        break;

    case WM_ERRCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "WM_***", wmResult);
        return WCM_RESULT_REJECT;           // ARM7 への要求発行に失敗( リトライ可能 )

    case WM_ERRCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;      // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifSetDCFData

  Description:  送信要求発行

  Arguments:    dstAddr      - 
                buf          - 
                len          - 

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_WmifSetDCFData(const u8* dstAddr, const u16* buf, u16 len, WCMSendInd callback)
{
    WMErrCode               wmResult;
    volatile WCMCpsifWork*  wcmiw = WCMi_GetCpsifWork();

    SDK_POINTER_ASSERT(wcmiw);

    if (!callback)
    {
        wmResult = WM_SetDCFData(WcmWmcbSetDCFData, (const u8*)dstAddr, buf, len);
    }
    else
    {
        wcmiw->sendCallback = callback;
        wmResult = WM_SetDCFData(WcmWmcbSetDCFDataAsync, (const u8*)dstAddr, buf, len);
    }
    
    switch (wmResult)
    {
    case WM_ERRCODE_OPERATING:
        break;

    case WM_ERRCODE_ILLEGAL_STATE:
    case WM_ERRCODE_INVALID_PARAM:
    case WM_ERRCODE_FIFO_ERROR:
    default:
        // 送信要求に失敗した場合
        WCMi_Printf(WCMi_RText_WmSyncError, "WM_SetDCFData", wmResult);
        return WCM_CPSIF_RESULT_SEND_FAILURE;   // DCF 送信に失敗
    }

    if (!callback)
    {
        // 送信完了待ちブロック
        OS_SleepThread((OSThreadQueue*)&(wcmiw->sendQ));
        /*
         * ブロック状態から復帰した時点で、WCMライブラリの状態は不明。
         * WCM ライブラリが終了されていても、送信結果は保持されているので
         * WCM ライブラリの状態を確認する必要はない。
         */
        switch (wcmiw->sendResult)
        {
        case WM_ERRCODE_SUCCESS:
            break;

        case WM_ERRCODE_FAILED:
        default:
            // 送信に失敗した場合
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_SetDCFData", wcmiw->sendResult);
            return WCM_CPSIF_RESULT_SEND_FAILURE;       // DCF 送信に失敗
        }
    }
    
    return len;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifSendNullPacket

  Description:  キープアライブ NULL パケット を送信する。
                他の DCF パケットを送信中の場合は何も行わない。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_WmifSendNullPacket(void)
{
    volatile WCMWork*   w = WCMi_GetSystemWork();
    volatile WCMCpsifWork* wcmiw = WCMi_GetCpsifWork();

    // WCM ライブラリの内部状態を確認
    if (w == NULL)
    {
        return;
    }

    if ((w->phase != WCM_PHASE_DCF) || (w->resetting == WCM_RESETTING_ON))
    {
        return;
    }

    // 排他ロック
    if (FALSE == WCMi_CpsifTryLockMutexInIRQ((OSMutex*)&(wcmiw->sendMutex)))
    {
        return;
    }

    // 送信要求発行
    if (WM_ERRCODE_OPERATING != WM_SetDCFData(WcmKACallback, (const u8*)w->bssDesc.bssid, (const u16*)(w->sendBuf),
        0))
    {
        WCMi_CpsifUnlockMutexInIRQ((OSMutex*)&(wcmiw->sendMutex));
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_WmifGetAllowedChannel

  Description:  NITREO 無線の使用許可チャンネルリストを取得する。

  Arguments:    None.

  Returns:      u16    - 1～13chのうち、使用を許可されたチャンネル群のビットフィールドを返します。
                         最下位ビットから順に 1 チャンネル, 2 チャンネル……に対応します。
 *---------------------------------------------------------------------------*/
u16 WCMi_WmifGetAllowedChannel(void)
{
    u16 tmp =WM_GetAllowedChannel();

    u16 low  = 0x0001;
    u16 high = 0x8000;

    if(tmp == 0)
    {
        return 0;
    }

    while(!(high & tmp))
    {
        high = (u16)((high>>1) | 0x8000);
    }
    high <<= 1;

    while(!(low & tmp))
    {
        low = (u16)((low<<1) | 0x0001);
    }
    low >>= 1;

    return (u16)~(high | low);

}

/*---------------------------------------------------------------------------*
  Name:         WcmCpsifKACallback

  Description:  DCF フレーム送信要求に対する無線ドライバからの非同期的な応答を
                受け取るコールバック関数。キープアライブ NULL パケット送信時
                専用。通常のパケット送信とは排他的に送信制御を行うことで、
                一つしか登録できないコールバック関数を切り替えて使うことが可能。

  Arguments:    arg -   無線ドライバからの応答パラメータが格納されているバッファ
                        のアドレスが渡される。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmKACallback(void* arg)
{
    volatile WCMCpsifWork* wcmiw = WCMi_GetCpsifWork();
#if WCM_DEBUG
    if (((WMCallback*)arg)->errcode == WM_ERRCODE_SUCCESS)
    {
        WCMi_Printf("Send NULL packet for KeepAlive.\n");
    }

#else
#pragma unused(arg)
#endif

    // 処理の成否に関わらず排他ロックを解除する
    WCMi_CpsifUnlockMutexInIRQ((OSMutex*)&(wcmiw->sendMutex));
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmReset

  Description:  WM_Reset 関数により WM ライブラリのリセット処理を開始する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmReset(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    WMErrCode   wmResult;

    if (wcmw->resetting == WCM_RESETTING_OFF)
    {
        // リセット重複呼び出し管理フラグをセット
        wcmw->resetting = WCM_RESETTING_ON;

        // 無線接続状態のリセット要求を発行
        wmResult = WM_Reset(WcmWmcbReset);
        if (wmResult != WM_ERRCODE_OPERATING)
        {
            /* リセットに失敗した場合は復旧不可能 */
            WCMi_Printf(WCMi_RText_WmSyncError, "WM_Reset", wmResult);
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbIndication

  Description:  WM ライブラリからの不定期通知を受け取るハンドラ。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbIndication(void* arg)
{
    WMIndCallback*  cb = (WMIndCallback*)arg;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    /* WCM 内部処理に関係しない通知は無視する */
    if (cb->errcode == WM_ERRCODE_FIFO_ERROR)
    {
        if ((cb->state == WM_STATECODE_FIFO_ERROR) && (cb->reason == WM_APIID_AUTO_DISCONNECT))
        {
            /*
             * 通信の整合性が保てなくなる致命的なハードウェアエラーが ARM7 側で検知され、
             * 自動的に切断しようとしたがリクエストキューが詰まっていて切断要求を予約できなかった場合
             */
            switch (wcmw->phase)
            {
            case WCM_PHASE_IDLE_TO_DCF:
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);
                break;

            case WCM_PHASE_DCF:
            case WCM_PHASE_IRREGULAR:
                WcmWmReset();
                break;

            case WCM_PHASE_DCF_TO_IDLE:
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);
                break;

                /* 想定外のフェーズでの通知は無視する */
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbCommon

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_Enable , WM_Disable , WM_PowerOn , WM_PowerOff ,
                WM_SetLifeTime , WM_SetBeaconIndication , WM_SetWEPKeyEx
                に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbCommon(void* arg)
{
    WMCallback*     cb = (WMCallback*)arg;
    WMErrCode       wmResult = WM_ERRCODE_MAX;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        // 次のステップとして行うべき処理を決定
        switch (cb->apiid)
        {
        case WM_APIID_ENABLE:
            // 無線ハードウェアへの電力供給要求を発行
            wmResult = WM_PowerOn(WcmWmcbCommon);
            break;

        case WM_APIID_DISABLE:
            // WM ライブラリ終了
            wmResult = WM_Finish();
            switch (wmResult)
            {
            case WM_ERRCODE_SUCCESS:
                WCMi_SetPhase(WCM_PHASE_WAIT);        // 非同期シーケンス正常終了
                WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__);
                break;

            case WM_ERRCODE_WM_DISABLE:
                WCMi_Warning(WCMi_WText_InvalidWmState);

            /* Don't break here */
            default:
                WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
            }

            return; // 次のステップはないのでここで終了

        case WM_APIID_POWER_ON:
            WCMi_SetPhase(WCM_PHASE_IDLE);    // 非同期シーケンス正常終了
            WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__);
            return; // 次のステップはないのでここで終了

        case WM_APIID_POWER_OFF:
            // 無線ハードウェアの使用禁止要求を発行
            wmResult = WM_Disable(WcmWmcbCommon);
            break;

        case WM_APIID_SET_LIFETIME:
            // ビーコン送受信通知 OFF 要求を発行
            wmResult = WM_SetBeaconIndication(WcmWmcbCommon, 0);
            break;

        case WM_APIID_SET_BEACON_IND:
            // WEP 暗号化設定要求を発行
            wmResult = WM_SetWEPKeyEx(WcmWmcbCommon, (u16) (wcmw->privacyDesc.mode), (u16) (wcmw->privacyDesc.keyId),
                                      (u8*)(wcmw->privacyDesc.key));
            break;

        case WM_APIID_SET_WEPKEY_EX:
            // AP への無線接続要求を発行
            wmResult = WM_StartConnectEx(WcmWmcbConnect, (WMBssDesc*)&(wcmw->bssDesc), NULL,
                                         (BOOL)((wcmw->option & WCM_OPTION_MASK_POWER) == WCM_OPTION_POWER_ACTIVE ? FALSE : TRUE),
                                         (u16) ((wcmw->option & WCM_OPTION_MASK_AUTH) == WCM_OPTION_AUTH_SHAREDKEY ? WM_AUTHMODE_SHARED_KEY :
                                                     WM_AUTHMODE_OPEN_SYSTEM));
            break;
        }

        // 同期的な処理結果を確認
        switch (wmResult)
        {
        case WM_ERRCODE_OPERATING:
            break;

        case WM_ERRCODE_FIFO_ERROR:
#if WCM_DEBUG
            switch (cb->apiid)
            {
            case WM_APIID_ENABLE:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_PowerOn", wmResult);
                break;

            case WM_APIID_POWER_OFF:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_Disable", wmResult);
                break;

            case WM_APIID_SET_LIFETIME:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_SetBeaconIndication", wmResult);
                break;

            case WM_APIID_SET_BEACON_IND:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_SetWEPKeyEx", wmResult);
                break;

            case WM_APIID_SET_WEPKEY_EX:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_StartConnectEx", wmResult);
                break;
            }
#endif
            WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
            WCMi_Notify(WCM_RESULT_FAILURE, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
            break;

        case WM_ERRCODE_ILLEGAL_STATE:
            WCMi_Warning(WCMi_WText_InvalidWmState);

        /* Don't break here */
        default:
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
        }
        break;

    case WM_ERRCODE_FAILED:
#if WCM_DEBUG
        switch (cb->apiid)
        {
        case WM_APIID_ENABLE:
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_Enable", cb->errcode, cb->wlCmdID, cb->wlResult);
            break;

        case WM_APIID_DISABLE:
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_Disable", cb->errcode, cb->wlCmdID, cb->wlResult);
            break;

        case WM_APIID_POWER_ON:
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_PowerOn", cb->errcode, cb->wlCmdID, cb->wlResult);
            break;

        case WM_APIID_POWER_OFF:
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_PowerOff", cb->errcode, cb->wlCmdID, cb->wlResult);
            break;

        case WM_APIID_SET_LIFETIME:
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_SetLifeTime", cb->errcode, cb->wlCmdID, cb->wlResult);
            break;

        case WM_APIID_SET_BEACON_IND:
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_SetBeaconIndication", cb->errcode, cb->wlCmdID, cb->wlResult);
            break;

        case WM_APIID_SET_WEPKEY_EX:
            WCMi_Printf(WCMi_RText_WmAsyncError, "WM_SetWEPKeyEx", cb->errcode, cb->wlCmdID, cb->wlResult);
            break;
        }
#endif
        WCMi_SetPhase(WCM_PHASE_IRREGULAR);       // ARM7 でなんらかの理由でエラー
        WCMi_Notify(WCM_RESULT_FAILURE, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
        break;

    case WM_ERRCODE_ILLEGAL_STATE:
    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbScanEx

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_StartScanEx に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbScanEx(void* arg)
{
    WMStartScanExCallback*  cb = (WMStartScanExCallback*)arg;
    WMErrCode   wmResult = WM_ERRCODE_MAX;
    WCMWork*    wcmw = WCMi_GetSystemWork();

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        // 自動探索開始中の場合は、正常に自動探索モードに入ったことを通知
        if (wcmw->phase == WCM_PHASE_IDLE_TO_SEARCH)
        {
            WCMi_SetPhase(WCM_PHASE_SEARCH);

            /* この通知内にて自動探索停止を要求される場合も想定される */
            WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__);
        }

        // 次のステップとして行うべき処理を決定
        switch (wcmw->phase)
        {
        case WCM_PHASE_SEARCH:
            wcmw->notifyId = WCM_NOTIFY_FOUND_AP;
            if (cb->state == WM_STATECODE_PARENT_FOUND)
            {
                // AP 情報を発見した場合、内部のリストを編集しつつアプリケーションに通知
                s32 i;

                DC_InvalidateRange(wcmw->wm.scanExParam.scanBuf, wcmw->wm.scanExParam.scanBufSize);
                for (i = 0; i < cb->bssDescCount; i++)
                {
                    WCMi_EntryApList((WCMBssDesc*)cb->bssDesc[i], cb->linkLevel[i]);

                    /*
                     * IW とI/O を合わせる為に下記のようなパラメータ構成にしてあるが、
                     * cb と i をパラメータに持ったほうが妥当と思われる。
                     */
                    WCMi_NotifyEx(WCM_NOTIFY_FOUND_AP, WCM_RESULT_SUCCESS, cb->bssDesc[i], (void*)cb, __LINE__);
                }
            }

            // 指定されたチャンネルを一巡したかを判定
            if ((wcmw->option & WCM_OPTION_MASK_ROUNDSCAN) == WCM_OPTION_ROUNDSCAN_NOTIFY)
            {
                u32 channels = MATH_CountPopulation(wcmw->option & WCM_OPTION_FILTER_CHANNEL);

                if (channels > 0)
                {
                    if ((wcmw->wm.scanCount % channels) == 0)
                    {
                        // 各チャンネルのスキャンが一巡したことを通知
                        WCMi_NotifyEx(WCM_NOTIFY_SEARCH_AROUND, WCM_RESULT_SUCCESS, (void*)(wcmw->wm.scanCount), 0, __LINE__);
                    }
                }
            }

            // 次のチャンネルのスキャン開始要求を発行
            wcmw->wm.scanExParam.channelList = (u16) ((0x0001 << WCMi_GetNextScanChannel((u16) (32 - MATH_CountLeadingZeros(cb->channelList)))) >> 1);
            DC_StoreRange(wcmw->wm.scanExParam.scanBuf, wcmw->wm.scanExParam.scanBufSize);
            wcmw->wm.scanCount++;
            wmResult = WM_StartScanEx(WcmWmcbScanEx, &(wcmw->wm.scanExParam));
            break;

        case WCM_PHASE_SEARCH_TO_IDLE:
            // スキャン停止要求を発行
            wmResult = WM_EndScan(WcmWmcbEndScan);
            break;

        case WCM_PHASE_TERMINATING:
            // 強制終了シーケンス中の場合はここでリセット
            WcmWmReset();
            return;
        }

        // 同期的な処理結果を確認
        switch (wmResult)
        {
        case WM_ERRCODE_OPERATING:
            break;

        case WM_ERRCODE_FIFO_ERROR:
#if WCM_DEBUG
            switch (wcmw->phase)
            {
            case WCM_PHASE_SEARCH:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_StartScanEx", wmResult);
                break;

            case WCM_PHASE_SEARCH_TO_IDLE:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_EndScan", wmResult);
                break;
            }
#endif
            WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
            WCMi_Notify(WCM_RESULT_FAILURE, 0, 0, __LINE__);
            break;

        case WM_ERRCODE_ILLEGAL_STATE:
            WCMi_Warning(WCMi_WText_InvalidWmState);

        /* Don't break here */
        default:
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
        }
        break;

    case WM_ERRCODE_FAILED:
        // スキャン要求に失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "WM_StartScanEx", cb->errcode, cb->wlCmdID, cb->wlResult);
        WcmWmReset();
        break;

    case WM_ERRCODE_ILLEGAL_STATE:
    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbEndScan

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_EndScan に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbEndScan(void* arg)
{
    WMCallback*     cb = (WMCallback*)arg;

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        WCMi_SetPhase(WCM_PHASE_IDLE);        // 非同期シーケンス正常終了
        WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__);
        break;

    case WM_ERRCODE_FAILED:
        // スキャン停止要求に失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "WM_EndScan", cb->errcode, cb->wlCmdID, cb->wlResult);
        WcmWmReset();
        break;

    case WM_ERRCODE_ILLEGAL_STATE:
    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbConnect

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_StartConnectEx に対する結果応答、及び無線接続後の被切断通知
                を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbConnect(void* arg)
{
    WMStartConnectCallback*     cb = (WMStartConnectCallback*)arg;
    WMErrCode   wmResult = WM_ERRCODE_MAX;
    WCMWork*    wcmw = WCMi_GetSystemWork();

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        switch (cb->state)
        {
        case WM_STATECODE_DISCONNECTED:                     // 被切断通知
        case WM_STATECODE_BEACON_LOST:                      // 切断された場合と同等の処理を行う
            switch (wcmw->phase)
            {
            case WCM_PHASE_DCF_TO_IDLE:
                // Auth-ID をクリア
                wcmw->wm.authId = 0;

            /* Don't break here */
            case WCM_PHASE_IDLE_TO_DCF:
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);
                break;

            case WCM_PHASE_DCF:
                // Auth-ID をクリア
                wcmw->wm.authId = 0;

                // 通知種別を設定
                wcmw->notifyId = WCM_NOTIFY_DISCONNECT;

            /* Don't break here */
            case WCM_PHASE_IRREGULAR:
                WcmWmReset();
                break;

                /* 想定外のフェーズでの切断通知は無視する */
            }
            break;

#if SDK_VERSION_MAJOR > 3 || (SDK_VERSION_MAJOR == 3 && SDK_VERSION_MINOR > 0) || \
        (SDK_VERSION_MAJOR == 3 && SDK_VERSION_MINOR == 0 && SDK_VERSION_RELSTEP >= 20100)

        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:         // 自分からの切断では切断関数内で処理しているので、処理なし
            break;
#endif

        case WM_STATECODE_CONNECT_START:                    // 接続の途中経過なので、処理なし
            break;

        case WM_STATECODE_CONNECTED:                        // 接続完了通知
            if (wcmw->phase == WCM_PHASE_IRREGULAR)
            {
                // 切断された後に接続完了通知が来た場合は、失敗とみなしリセット
                WCMi_Printf(WCMi_RText_WmDisconnected, "WM_StartConnectEx");
                WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF);         // フェーズを上書き
                WcmWmReset();
            }
            else
            {
                // AID は 1 ～ 2007 までの範囲で与えられるはず
                if ((WCM_AID_MIN <= cb->aid) && (cb->aid <= WCM_AID_MAX))
                {
                    // Auth-ID を退避
                    wcmw->wm.authId = cb->aid;

                    // DCM 通信モード開始要求を発行
                    wmResult = WM_StartDCF(WcmWmcbStartDCF, (WMDcfRecvBuf *) (wcmw->recvBuf), WCM_DCF_RECV_BUF_SIZE);
                    switch (wmResult)
                    {
                    case WM_ERRCODE_OPERATING:
                        break;

                    case WM_ERRCODE_FIFO_ERROR:
                        WCMi_Printf(WCMi_RText_WmSyncError, "WM_StartDCF", wmResult);
                        WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                        WCMi_Notify(WCM_RESULT_FAILURE, &(wcmw->bssDesc), 0, __LINE__);
                        break;

                    case WM_ERRCODE_ILLEGAL_STATE:
                        WCMi_Warning(WCMi_WText_InvalidWmState);

                    /* Don't break here */
                    default:
                        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
                    }
                }
                else
                {
                    // 想定外の AID が与えられた場合は、失敗とみなしリセット
                    WCMi_Printf(WCMi_RText_InvalidAid, "WM_StartConnectEx");
                    WcmWmReset();
                }
            }
            break;

        default:    // 想定外のステートコード
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), (void*)(cb->state), __LINE__);
        }
        break;

    case WM_ERRCODE_FAILED:
        wcmw->wm.wlStatusOnConnectFail = cb->wlStatus;

    case WM_ERRCODE_NO_ENTRY:
    case WM_ERRCODE_INVALID_PARAM:
    case WM_ERRCODE_OVER_MAX_ENTRY:
        // 無線接続に失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "WM_StartConnectEx", cb->errcode, cb->wlCmdID, cb->wlResult);
        WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF);     // IRREGULAR になっている可能性があるのでフェーズを上書き
        WcmWmReset();
        break;

    case WM_ERRCODE_ILLEGAL_STATE:
    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbDisconnect

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_Disconnect に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbDisconnect(void* arg)
{
    WMCallback*     cb = (WMCallback*)arg;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        if (wcmw->phase == WCM_PHASE_IRREGULAR)
        {
            // 切断された後に明示的な切断要求の完了通知が来た場合、念のためリセット
            WCMi_Printf(WCMi_RText_WmDisconnected, "WM_Disconnect");
            WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
            WcmWmReset();
        }
        else
        {
            // Auth-ID をクリア
            wcmw->wm.authId = 0;

            WCMi_SetPhase(WCM_PHASE_IDLE);    // 非同期シーケンス正常終了
            WCMi_Notify(WCM_RESULT_SUCCESS, &(wcmw->bssDesc), 0, __LINE__);
        }
        break;

    case WM_ERRCODE_FAILED:
    case WM_ERRCODE_ILLEGAL_STATE:          // クリティカルなタイミングで通信が切れた場合に返ってきうる
        // 無線接続の切断要求が失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "WM_Disconnect", cb->errcode, cb->wlCmdID, cb->wlResult);
        WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
        WcmWmReset();
        break;

    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbStartDCF

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_StartDCF に対する結果応答、及び DCF モードでのデータ受信通知
                を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbStartDCF(void* arg)
{
    WMStartDCFCallback*     cb = (WMStartDCFCallback*)arg;
    WCMWork*                wcmw = WCMi_GetSystemWork();

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        switch (cb->state)
        {
        case WM_STATECODE_DCF_START:
            if (wcmw->phase == WCM_PHASE_IRREGULAR)
            {
                // 切断された後に DCF 開始完了通知が来た場合は、失敗とみなしリセット
                WCMi_Printf(WCMi_RText_WmDisconnected, "WM_StartDCF");
                WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF); // フェーズを上書き
                WcmWmReset();
            }
            else
            {
                WCMi_SetPhase(WCM_PHASE_DCF);         // 非同期シーケンス正常終了
                WCMi_Notify(WCM_RESULT_SUCCESS, &(wcmw->bssDesc), 0, __LINE__);
            }
            break;

        case WM_STATECODE_DCF_IND:
            // 電波強度を退避
            WCMi_ShelterRssi((u8) (cb->recvBuf->rate_rssi >> 8));

            // CPS とのインターフェースに DCF 受信を通知
            DC_InvalidateRange(cb->recvBuf, WCM_DCF_RECV_BUF_SIZE);
            WCMi_CpsifRecvCallback(cb->recvBuf->srcAdrs, cb->recvBuf->destAdrs, (u8 *)cb->recvBuf->data, (s32)cb->recvBuf->length);
            break;

        default:    // 想定外のステートコード
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), (void*)(cb->state), __LINE__);
        }
        break;

    /* StartDCF には WM_ERRCODE_FAILED は返ってこないので省略 */
    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbEndDCF

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_EndDCF に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbEndDCF(void* arg)
{
    WMCallback*     cb = (WMCallback*)arg;
    WMErrCode       wmResult = WM_ERRCODE_MAX;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        if (wcmw->phase == WCM_PHASE_IRREGULAR)
        {
            // 切断された後に DCF 終了完了通知が来た場合は、失敗とみなしリセット
            WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
            WcmWmReset();
        }
        else
        {
            // AP との無線接続切断要求を発行
            wmResult = WM_Disconnect(WcmWmcbDisconnect, 0);
            switch (wmResult)
            {
            case WM_ERRCODE_OPERATING:
                break;

            case WM_ERRCODE_FIFO_ERROR:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_Disconnect", wmResult);
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                WCMi_Notify(WCM_RESULT_FAILURE, &(wcmw->bssDesc), 0, __LINE__);
                break;

            case WM_ERRCODE_ILLEGAL_STATE:          // クリティカルなタイミングで通信が切れた場合
                // 切断する寸前に AP 側から切断された場合は、失敗とみなしリセット
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_Disconnect", wmResult);
                WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
                WcmWmReset();
                break;

            default:
                WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
            }
        }
        break;

    case WM_ERRCODE_FAILED:
    case WM_ERRCODE_ILLEGAL_STATE:                  // クリティカルなタイミングで通信が切れた場合
        // DCF 通信モード終了に失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "WM_EndDCF", cb->errcode, cb->wlCmdID, cb->wlResult);
        WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);         // IRREGULAR になっている可能性があるのでフェーズを上書き
        WcmWmReset();
        break;

    case WM_ERRCODE_WM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);         // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbReset

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_Reset に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbReset(void* arg)
{
    WMCallback*     cb = (WMCallback*)arg;
    WMErrCode       wmResult = WM_ERRCODE_MAX;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    switch (cb->errcode)
    {
    case WM_ERRCODE_SUCCESS:
        // リセット重複呼び出し管理フラグをクリア
        wcmw->resetting = WCM_RESETTING_OFF;

        // Auth-ID をクリア
        wcmw->wm.authId = 0;

        switch (wcmw->phase)
        {
        case WCM_PHASE_IDLE_TO_SEARCH:  // AP 自動探索開始中
        case WCM_PHASE_SEARCH:          // AP 自動探索中
            // スキャン処理に失敗してリセットすることになった旨を通知
            WCMi_SetPhase(WCM_PHASE_IDLE);
            WCMi_Notify(WCM_RESULT_FAILURE, 0, 0, __LINE__);
            break;

        case WCM_PHASE_SEARCH_TO_IDLE:  // AP 自動探索停止中
            // 要求した AP 自動探索停止処理が完了したことを通知
            WCMi_SetPhase(WCM_PHASE_IDLE);
            WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__);
            break;

        case WCM_PHASE_IDLE_TO_DCF:     // 接続中
            // 接続処理に失敗してリセットすることになった旨を通知
            {
                u16 wlStatus = wcmw->wm.wlStatusOnConnectFail;

                wcmw->wm.wlStatusOnConnectFail = 0x0000;
#ifdef WCM_CAMOUFLAGE_RATESET
                /* レートセットの不整合により接続に失敗した時の対策 */
                if (wlStatus == WCM_CONNECT_STATUSCODE_ILLEGAL_RATESET)
                {
                    if ((wcmw->bssDesc.rateSet.support & WCM_CAMOUFLAGE_RATESET) != WCM_CAMOUFLAGE_RATESET)
                    {
                        WCMi_Printf(WCMi_RText_SupportRateset, wcmw->bssDesc.ssid);
                        wcmw->bssDesc.rateSet.support |= WCM_CAMOUFLAGE_RATESET;

                        // 自動的に接続をリトライ
                        wmResult = WM_StartConnectEx(WcmWmcbConnect, (WMBssDesc*)&(wcmw->bssDesc), NULL,
                                                     (BOOL)((wcmw->option & WCM_OPTION_MASK_POWER) == WCM_OPTION_POWER_ACTIVE ? FALSE : TRUE),
                                                     (u16) ((wcmw->option & WCM_OPTION_MASK_AUTH) == WCM_OPTION_AUTH_SHAREDKEY ?
                                                                     WM_AUTHMODE_SHARED_KEY : WM_AUTHMODE_OPEN_SYSTEM
                                                                     ));

                        // 同期的な処理結果を確認
                        switch (wmResult)
                        {
                        case WM_ERRCODE_OPERATING:
                            break;

                        case WM_ERRCODE_FIFO_ERROR:
                            WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                            WCMi_Notify(WCM_RESULT_FAILURE, &(wcmw->bssDesc), (void*)wlStatus, __LINE__);
                            break;

                        case WM_ERRCODE_ILLEGAL_STATE:
                            WCMi_Warning(WCMi_WText_InvalidWmState);

                        /* Don't break here */
                        default:
                            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                            WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), (void*)wlStatus, __LINE__);
                        }

                        return;
                    }
                }
#endif
                WCMi_SetPhase(WCM_PHASE_IDLE);
                WCMi_Notify(WCM_RESULT_FAILURE, &(wcmw->bssDesc), (void*)wlStatus, __LINE__);
            }
            break;

        case WCM_PHASE_DCF:         // DCF 通信中
        case WCM_PHASE_IRREGULAR:
            // AP から切断されたことを通知
            WCMi_SetPhase(WCM_PHASE_IDLE);
            WCMi_Notify(WCM_RESULT_SUCCESS, &(wcmw->bssDesc), (void*)1, __LINE__);

            break;

        case WCM_PHASE_DCF_TO_IDLE: // 切断中
            // 要求した切断処理が完了したことを通知
            WCMi_SetPhase(WCM_PHASE_IDLE);
            WCMi_Notify(WCM_RESULT_SUCCESS, &(wcmw->bssDesc), 0, __LINE__);
            break;

        case WCM_PHASE_TERMINATING: // 強制終了中
            // リセットの完了に続いて、PHASE_WAIT に戻すように連鎖的に処理を継続
            wmResult = WM_PowerOff(WcmWmcbCommon);
            switch (wmResult)
            {
            case WM_ERRCODE_OPERATING:
                break;

            case WM_ERRCODE_FIFO_ERROR:
                WCMi_Printf(WCMi_RText_WmSyncError, "WM_Reset", wmResult);
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                WCMi_Notify(WCM_RESULT_FAILURE, 0, 0, __LINE__);
                break;

            case WM_ERRCODE_ILLEGAL_STATE:
                WCMi_Warning(WCMi_WText_InvalidWmState);

            /* Don't break here */
            default:
                WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
            }
            break;

        default:
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, (void*)(wcmw->phase), __LINE__);
        }
        break;

    default:
        /* リセットに失敗した場合は復旧不可能 */
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);         // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmEditScanExParam

  Description:  WM ライブラリにスキャンを指示する際のスキャン設定構造体を編集する。

  Arguments:    bssid   -   探索時の BSSID フィルター。WCM_BSSID_ANY は全て 0xff
                            である BSSID なので、フィルターしない設定となる。
                essid   -   探索時の ESSID フィルター。WCM_ESSID_ANY を指定すると
                            長さ 0 の ESSID とみなされ、フィルターしない設定となる。
                option  -   オプション変更用変数。変更を行うと WCM ライブラリ内で
                            共通使用される option が変わって、元には戻らない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmEditScanExParam(const void* bssid, const void* essid, u32 option)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    (void)WCM_UpdateOption(option);
    wcmw->wm.scanExParam.scanBuf = (WMBssDesc*)wcmw->recvBuf;
    wcmw->wm.scanExParam.scanBufSize = WM_SIZE_SCAN_EX_BUF;
    wcmw->wm.scanExParam.channelList = (u16) ((0x0001 << WCMi_GetNextScanChannel(0)) >> 1);
    wcmw->wm.scanExParam.maxChannelTime = (u16) ((wcmw->maxScanTime != 0) ? wcmw->maxScanTime : WM_GetDispersionScanPeriod());
    wcmw->wm.scanExParam.scanType = (u16) (((wcmw->option & WCM_OPTION_MASK_SCANTYPE) == WCM_OPTION_SCANTYPE_ACTIVE) ? WM_SCANTYPE_ACTIVE : WM_SCANTYPE_PASSIVE);
    if (bssid == NULL)
    {
        MI_CpuCopy8(WCM_Bssid_Any, wcmw->wm.scanExParam.bssid, WCM_BSSID_SIZE);
    }
    else
    {
        MI_CpuCopy8(bssid, wcmw->wm.scanExParam.bssid, WCM_BSSID_SIZE);
    }

    //MI_CpuCopy8( bssid , wcmw->wm.scanExParam.bssid , WCM_BSSID_SIZE );
    if ((essid == NULL) || (essid == (void*)WCM_Essid_Any))
    {
        MI_CpuCopy8(WCM_Essid_Any, wcmw->wm.scanExParam.ssid, WCM_ESSID_SIZE);
        wcmw->wm.scanExParam.ssidLength = 0;
    }
    else
    {
        s32 len;

        MI_CpuCopy8(essid, wcmw->wm.scanExParam.ssid, WCM_ESSID_SIZE);
        for (len = 0; len < WCM_ESSID_SIZE; len++)
        {
            if (((u8*)essid)[len] == '\0')
            {
                break;
            }
        }

        wcmw->wm.scanExParam.ssidLength = (u16) len;
    }

    wcmw->wm.scanCount = 0;
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbSetDCFData

  Description:  DCF フレーム送信要求に対する無線ドライバからの非同期的な応答を
                受け取るコールバック関数。

  Arguments:    arg -   無線ドライバからの応答パラメータが格納されているバッファ
                        のアドレスが渡される。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbSetDCFData(void* arg)
{
    WMCallback*             cb = (WMCallback*)arg;
    volatile WCMCpsifWork*  wcmiw = WCMi_GetCpsifWork();

    switch (cb->apiid)
    {
    case WM_APIID_SET_DCF_DATA:
        wcmiw->sendResult = (u16) (cb->errcode);
        if (cb->errcode == WM_ERRCODE_SUCCESS)
        {
            WCMi_ResetKeepAliveAlarm();
        }

        OS_WakeupThread((OSThreadQueue*)&(wcmiw->sendQ));
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmWmcbSetDCFDataAsync

  Description:  DCF フレーム送信要求に対する無線ドライバからの非同期的な応答を
                受け取るコールバック関数。

  Arguments:    arg -   無線ドライバからの応答パラメータが格納されているバッファ
                        のアドレスが渡される。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmWmcbSetDCFDataAsync(void* arg)
{
    WMCallback*             cb = (WMCallback*)arg;
    volatile WCMCpsifWork*  wcmiw = WCMi_GetCpsifWork();
    s32                     wcmResult = WCM_RESULT_SUCCESS;
    WCMSendInd              sendCallback = NULL;

    switch (cb->apiid)
    {
    case WM_APIID_SET_DCF_DATA:
        wcmiw->sendResult = (WMErrCode) (cb->errcode);
        if (cb->errcode == WM_ERRCODE_SUCCESS)
        {
            WCMi_ResetKeepAliveAlarm();
        }

        /*
         * コールバックが呼び出された時点で、WCMライブラリの状態は不明。
         * WCM ライブラリが終了されていても、送信結果は保持されているので
         * WCM ライブラリの状態を確認する必要はない。
         */
        switch (wcmiw->sendResult)
        {
        case WM_ERRCODE_SUCCESS:
            // 正常終了
            wcmResult = WCM_RESULT_SUCCESS;
            break;

        case WM_ERRCODE_FAILED:
        default:
            // 送信に失敗した場合
            wcmResult = WCM_CPSIF_RESULT_SEND_FAILURE;
            WCMi_Printf(WCMi_RText_WmAsyncError, "WCM_SendDCFData", wcmiw->sendResult);
        }

        sendCallback = wcmiw->sendCallback;
        wcmiw->sendCallback = NULL;
        WCMi_CpsifUnlockMutexInIRQ((OSMutex*)&(wcmiw->sendMutex));

        SDK_POINTER_ASSERT(sendCallback);
        
        // 送信完了コールバックを通知
        sendCallback(wcmResult);
        break;
    }
}

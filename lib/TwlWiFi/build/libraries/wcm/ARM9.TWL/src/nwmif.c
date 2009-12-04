/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     nwmif.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-24#$
  $Rev: 1187 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include "wcm_private.h"
#include "wcm_message.h"
#include <twl/nwm.h>

/* DWC に通知すべき接続失敗理由 */
#define WCM_NWMIF_STATECODE_AUTH_ERROR     13
#define WCM_NWMIF_STATECODE_KEY_ERROR      15
#define WCM_NWMIF_STATECODE_CAPACITY_ERROR 17

#include <twl/ltdmain_begin.h>
/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/
static void         WcmNwmReset(void);
static void         WcmEditScanExParam(const void* bssid, const void* essid, u32 option);

static void         WcmNwmcbCommon(void* arg);
static void         WcmNwmcbScanEx(void* arg);
static void         WcmNwmcbConnect(void* arg);
static void         WcmNwmcbDisconnect(void* arg);
static void         WcmNwmcbSetRecvFrmBuf(void* arg);
static void         WcmNwmcbUnsetRecvFrmBuf(void* arg);
static void         WcmNwmcbReset(void* arg);
static void         WcmNwmcbSetDCFData(void* arg);
static void         WcmNwmcbSetDCFDataAsync(void* arg);
static void         WcmKACallback(void* arg);
static void         WcmNwmcbSetQos(void* arg);

static OSMessage mesgBuffer[10];
static OSMessageQueue mesgQueue;

/*---------------------------------------------------------------------------*
  Name:         WCM_GetWPAPSK

  Description:  WPAパスフレーズと接続対象の SSID から WPA-PSK(事前認証鍵)を生成
                ※ PSK の計算は接続設定時にのみ行い、接続には計算済みの PSK を使用する

  Arguments:    passphrase   - WPA パスフレーズ、8 文字以上 63 文字以下の ASCII 文字列
                ssid         - SSID
                ssidlen      - SSID の長さ(32未満の必要あり)
                psk          - 生成された PSK の格納先。32Byte

  Returns:      BOOL         - 成功の可否。
                               パスフレーズ長が異なる場合も失敗となる
 *---------------------------------------------------------------------------*/

BOOL    WCM_GetWPAPSK(const u8 passphrase[WCM_WPA_PASSPHRASE_LENGTH_MAX], const u8 *ssid, u8 ssidlen, u8 psk[WCM_WPA_PSK_SIZE])
{
    int passLength = STD_GetStringLength ((char*)passphrase);
    u8  zero[WCM_WPA_PSK_SIZE];

    /* パスフレーズの文字列長確認 */
    if (passLength < 8 || passLength > 63)
    {
        return FALSE;
    }

    MI_CpuClear8(psk, WCM_WPA_PSK_SIZE);
    MI_CpuClear8(zero, WCM_WPA_PSK_SIZE);

    NWM_Passphrase2PSK(passphrase, ssid, ssidlen, psk);

    if( MI_CpuComp8(psk, zero, WCM_WPA_PSK_SIZE) == 0 ) /* psk の正当性自体は接続してみないと判別不能 */
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifStartupAsync

  Description:  無線機能の起動シーケンスを開始

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_NwmifStartupAsync(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    NWMRetCode  nwmResult;
    
    SDK_POINTER_ASSERT(wcmw);

    // NWM ライブラリ初期化
    nwmResult = NWM_Init(wcmw->nwm.work, sizeof(wcmw->nwm.work), (u8)wcmw->config.dmano);
    switch (nwmResult)
    {
    case NWM_RETCODE_SUCCESS:
        break;

    case NWM_RETCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // NWM ライブラリが既に初期化されている

    case NWM_RETCODE_NWM_DISABLE:
        WCMi_Warning(WCMi_WText_UnsuitableArm7);
        return WCM_RESULT_WMDISABLE;            // ARM7 側で NWM ライブラリが動作していない

    case NWM_RETCODE_INVALID_PARAM:
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }

    wcmw->nwm.pseudoStatusOnConnectFail = 0x0000; // 疑似 802.11 STATE コードを初期化しておく

    // 無線ハードウェア使用許可要求を発行
    nwmResult = NWM_LoadDevice(WcmNwmcbCommon);
    switch (nwmResult)
    {
    case NWM_RETCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_WAIT_TO_IDLE);    // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_STARTUP;
        break;

    case NWM_RETCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "NWM_LoadDevice", nwmResult);
        WCMi_SetPhase(WCM_PHASE_IRREGULAR);
        return WCM_RESULT_FAILURE;              // ARM7 への要求発行に失敗

    case NWM_RETCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifCleanupAsync

  Description:  無線機能の停止シーケンスを開始

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_NwmifCleanupAsync(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    NWMRetCode  nwmResult;

    SDK_POINTER_ASSERT(wcmw);

    // 無線ハードウェアのシャットダウン要求を発行
    nwmResult = NWM_Close(WcmNwmcbCommon);
    switch (nwmResult)
    {
    case NWM_RETCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_IDLE_TO_WAIT);    // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_CLEANUP;
        break;

    case NWM_RETCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "NWM_Close", nwmResult);
        return WCM_RESULT_REJECT;               // ARM7 への要求発行に失敗( リトライ可能 )

    case NWM_RETCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifBeginSearchAsync

  Description:  AP 自動探索開始シーケンスを開始、もしくは探索条件変更

  Arguments:    bssid        - 
                essid        - 
                option       - 

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_NwmifBeginSearchAsync(const void* bssid, const void* essid, u32 option)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    NWMRetCode  nwmResult;

    SDK_POINTER_ASSERT(wcmw);

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
    DC_InvalidateRange(wcmw->nwm.scanExParam.scanBuf, wcmw->nwm.scanExParam.scanBufSize);
    wcmw->nwm.scanCount++;
    nwmResult = NWM_StartScanEx(WcmNwmcbScanEx, &(wcmw->nwm.scanExParam));
    switch (nwmResult)
    {
    case NWM_RETCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_IDLE_TO_SEARCH);  // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_BEGIN_SEARCH;
        break;

    case NWM_RETCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "NWM_StartScanEx", nwmResult);
        return WCM_RESULT_REJECT;               // ARM7 への要求発行に失敗( リトライ可能 )

    case NWM_RETCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;          // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifConnectAsync

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
s32 WCMi_NwmifConnectAsync(void)
{
    s32         wcmResult   = WCM_RESULT_SUCCESS;
    WCMWork*    wcmw        = WCMi_GetSystemWork();

    SDK_POINTER_ASSERT(wcmw);

    // begins the AP wireless connection sequence
    {
        NWMRetCode  nwmResult;

        switch(wcmw->privacyDesc.mode)
        {
            case WCM_PRIVACYMODE_NONE:
            case WCM_PRIVACYMODE_WEP40:
            case WCM_PRIVACYMODE_WEP104:
            case WCM_PRIVACYMODE_WEP128:
            /* WEP での接続の場合 */
            {
                u16 authMode = NWM_AUTHMODE_OPEN;
                
                if ((wcmw->option & WCM_OPTION_MASK_AUTH) == WCM_OPTION_AUTH_SHAREDKEY)
                {
                    authMode = NWM_AUTHMODE_SHARED;
                }

                nwmResult = NWM_SetWEPKey(WcmNwmcbCommon, 
                                                wcmw->privacyDesc.mode,
                                                wcmw->privacyDesc.keyId,
                                                wcmw->privacyDesc.key,
                                                authMode);
            }
            break;

            case WCM_PRIVACYMODE_WPA_TKIP:
            case WCM_PRIVACYMODE_WPA2_TKIP:
            case WCM_PRIVACYMODE_WPA_AES:
            case WCM_PRIVACYMODE_WPA2_AES:
            /* WPA での接続の場合 */
            {
                NWMWpaParam wpaParam;
                MI_CpuCopy8(wcmw->privacyDesc.key, wpaParam.psk, NWM_WPA_PSK_LENGTH);
                wpaParam.auth = wcmw->privacyDesc.mode;

                nwmResult = NWM_SetWPAPSK(WcmNwmcbCommon, &wpaParam);
            }
                break;

            default:
                /* 未知の暗号化方式は無視 */
                break;
        }

        switch (nwmResult) // miz
        {
        case NWM_RETCODE_OPERATING: // miz
            WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF); // asynchronous sequence start successful
            wcmw->notifyId = WCM_NOTIFY_CONNECT;
            break;

        case NWM_RETCODE_FIFO_ERROR: // miz
            WCMi_Printf(WCMi_RText_WmSyncError, "NWM_SetWEPKey or NWM_SetWPAPSK", nwmResult); // miz
            return WCM_RESULT_REJECT;           // request to ARM7 failed (retry possible)

        case NWM_RETCODE_ILLEGAL_STATE: // miz
            WCMi_Warning(WCMi_WText_InvalidWmState); // miz
        /* Don't break here*/
        default:
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
            return WCM_RESULT_FATAL_ERROR;      // error outside of anticipated range
        }
    }

    // normal end
    return WCM_RESULT_ACCEPT;
}


/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifDisconnectAsync

  Description:  

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_NwmifDisconnectAsync(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    NWMRetCode  nwmResult;

    SDK_POINTER_ASSERT(wcmw);

    // DCF 通信モード終了要求を発行
    nwmResult = NWM_UnsetReceivingFrameBuffer(WcmNwmcbUnsetRecvFrmBuf, NWM_PORT_IPV4_ARP);
    switch (nwmResult)
    {
    case NWM_RETCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
        wcmw->notifyId = WCM_NOTIFY_DISCONNECT;
        break;

    case NWM_RETCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "NWM_UnsetReceivingFrameBuffer", nwmResult);
        return WCM_RESULT_REJECT;           // ARM7 への要求発行に失敗( リトライ可能 )

    case NWM_RETCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);
    
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;
    }
    
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifTerminateAsync

  Description:  

  Arguments:    None.

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_NwmifTerminateAsync(void)
{
    u16         nwmState;
    WCMWork*    wcmw = WCMi_GetSystemWork();
    NWMRetCode  nwmResult;

    SDK_POINTER_ASSERT(wcmw);

    // NWM ライブラリの内部状態を調査
    nwmState = NWM_GetState();
    
    // WM 内部状態により停止状態に遷移するよう連鎖を開始
    switch (nwmState)
    {
    case NWM_STATE_INITIALIZED:
        // WM ライブラリ終了
        nwmResult = NWM_End();
        if (nwmResult == NWM_RETCODE_SUCCESS)
        {
            WCMi_SetPhase(WCM_PHASE_WAIT);
            wcmw->notifyId = WCM_NOTIFY_COMMON;
            return WCM_RESULT_SUCCESS;      // 同期的に WAIT フェーズへの移行が完了
        }
        break;

    case NWM_STATE_LOADED:
        // 無線ハードウェアの使用禁止要求を発行
        nwmResult = NWM_UnloadDevice(WcmNwmcbCommon);
        break;

    case NWM_STATE_DISCONNECTED:
        // 無線ハードウェアのシャットダウン要求を発行
        nwmResult = NWM_Close(WcmNwmcbCommon);
        break;

    case NWM_STATE_INFRA_CONNECTED:
        // NWM にはリセット関数が用意されていないため、通信中の Terminate では Disconnect を呼んでおく
        nwmResult = NWM_Disconnect(WcmNwmcbDisconnect);
        break;

    default:
        // リセット重複呼び出し管理フラグをセット
        wcmw->resetting = WCM_RESETTING_ON;

        // 無線接続状態のリセット要求を発行 NWM_にはリセット機能がないため状態を直接変更
        nwmResult = NWM_RETCODE_NWM_DISABLE;
    }

    // 要求発行に対する同期的な処理結果を確認
    switch (nwmResult)
    {
    case NWM_RETCODE_OPERATING:
        WCMi_SetPhase(WCM_PHASE_TERMINATING); // 非同期的なシーケンスの開始に成功
        wcmw->notifyId = WCM_NOTIFY_TERMINATE;
        break;

    case NWM_RETCODE_FIFO_ERROR:
        WCMi_Printf(WCMi_RText_WmSyncError, "NWM_***", nwmResult);
        return WCM_RESULT_REJECT;           // ARM7 への要求発行に失敗( リトライ可能 )

    case NWM_RETCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        return WCM_RESULT_FATAL_ERROR;      // 想定範囲外のエラー
    }
    return WCM_RESULT_ACCEPT;
}


/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifSetDCFData

  Description:  送信要求発行

  Arguments:    dstAddr      - 
                buf          - 
                len          - 

  Returns:      s32          - 
 *---------------------------------------------------------------------------*/
s32 WCMi_NwmifSetDCFData(const u8* dstAddr, const u16* buf, u16 len, WCMSendInd callback)
{
    NWMRetCode              nwmResult;
    volatile WCMCpsifWork*  wcmiw = WCMi_GetCpsifWork();

    SDK_POINTER_ASSERT(wcmiw);

    /*
     * The WCM library state is unclear upon returning from the block state.
     * As you can't send unless connected to AP, you need to confirm the WCM
     * library state here.
     */

    if (!callback)
    {
        nwmResult = NWM_SendFrame(WcmNwmcbSetDCFData, (const u8*)dstAddr, buf, len);
    }
    else
    {
        wcmiw->sendCallback = callback;
        nwmResult = NWM_SendFrame(WcmNwmcbSetDCFDataAsync, (const u8*)dstAddr, buf, len);
    }
    
    switch (nwmResult)
    {
    case NWM_RETCODE_OPERATING:
        break;

    case NWM_RETCODE_ILLEGAL_STATE:
    case NWM_RETCODE_INVALID_PARAM:
    case NWM_RETCODE_FIFO_ERROR:
    default:
        // 送信要求に失敗した場合
        WCMi_Printf(WCMi_RText_WmSyncError, "NWM_SendFrame", nwmResult);
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
        case NWM_RETCODE_SUCCESS:
            break;

        case NWM_RETCODE_FAILED:
        default:
            // 送信に失敗した場合
            WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_SendFrame", wcmiw->sendResult);
            return WCM_CPSIF_RESULT_SEND_FAILURE;       // DCF 送信に失敗
        }
    }
    // 正常終了
    return len;
}


/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifSendNullPacket

  Description:  キープアライブ NULL パケット を送信する。
                他の DCF パケットを送信中の場合は何も行わない。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_NwmifSendNullPacket(void)
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
    if (NWM_RETCODE_OPERATING != NWM_SendFrame(WcmKACallback, (const u8*)w->bssDesc.bssid, (const u16*)(w->sendBuf),
        0))
    {
        WCMi_CpsifUnlockMutexInIRQ((OSMutex*)&(wcmiw->sendMutex));
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_NwmifGetAllowedChannel

  Description:  TWL 無線の使用許可チャンネルリストを取得する。

  Arguments:    None.

  Returns:      u16    - 1～13chのうち、使用を許可されたチャンネル群のビットフィールドを返します。
                         最下位ビットから順に 1 チャンネル, 2 チャンネル……に対応します。
 *---------------------------------------------------------------------------*/
u16 WCMi_NwmifGetAllowedChannel(void)
{
    /* NWM では最下位ビットが 0ch を意味するので、一回シフトして通知する */
    return (u16)(NWM_GetAllowedChannel()>>1);
}

/*---------------------------------------------------------------------------*
  Name:         WcmKACallback

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
    if (((NWMCallback*)arg)->retcode == NWM_RETCODE_SUCCESS)
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
  Name:         WcmNwmReset

  Description:  NWM にはリセット機能がないため、WM_Reset 関数相当の処理を行い
                WM ライブラリのリセット処理を開始する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmReset(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();
    NWMRetCode  nwmResult = NWM_RETCODE_MAX; // miz

    if (wcmw->resetting == WCM_RESETTING_OFF)
    {
        // sets reset duplicate call management flags
        wcmw->resetting = WCM_RESETTING_ON;

        // miz : NWM にリセット機能がないので以下からコールバック処理開始--------------------------

        // リセット重複呼び出し管理フラグをクリア
        wcmw->resetting = WCM_RESETTING_OFF;

        // Auth-ID をクリア
        wcmw->nwm.authId = 0;

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
                u16 wlStatus = wcmw->nwm.pseudoStatusOnConnectFail;
                wcmw->nwm.pseudoStatusOnConnectFail = 0x0000;
                
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
            nwmResult = NWM_Close(WcmNwmcbCommon); // miz
            switch (nwmResult) // miz
            {
            case NWM_RETCODE_OPERATING: // miz
                break;

            case NWM_RETCODE_FIFO_ERROR: // miz
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_Close", nwmResult);
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                WCMi_Notify(WCM_RESULT_FAILURE, 0, 0, __LINE__);
                break;

            case NWM_RETCODE_ILLEGAL_STATE: // miz
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
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbCommon

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_Enable , WM_Disable , WM_PowerOn , WM_PowerOff ,
                WM_SetLifeTime , WM_SetBeaconIndication , WM_SetWEPKeyEx
                に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbCommon(void* arg)
{
    NWMCallback*    cb = (NWMCallback*)arg;
    NWMRetCode      nwmResult = NWM_RETCODE_MAX;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    switch (cb->retcode)
    {
    case NWM_RETCODE_SUCCESS:
        // 次のステップとして行うべき処理を決定
        switch (cb->apiid)
        {
        case NWM_APIID_LOAD_DEVICE:
            // 無線ハードウェアへの電力供給要求を発行
            nwmResult = NWM_Open(WcmNwmcbCommon);
            break;

        case NWM_APIID_UNLOAD_DEVICE:
            // NWM ライブラリ終了
            nwmResult = NWM_End();
            switch (nwmResult)
            {
            case NWM_RETCODE_SUCCESS:
                WCMi_SetPhase(WCM_PHASE_WAIT);        // 非同期シーケンス正常終了
                WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__);
                break;

            case NWM_RETCODE_NWM_DISABLE:
                WCMi_Warning(WCMi_WText_InvalidWmState);

            /* Don't break here */
            default:
                WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
            }

            return; // 次のステップはないのでここで終了

        case NWM_APIID_OPEN:
             {
                u8 macAddress[NWM_SIZE_MACADDR];
                OS_GetMacAddress(macAddress);
                WCMi_CpsifSetMacAddress(macAddress);
            }

            WCMi_SetPhase(WCM_PHASE_IDLE);    // 非同期シーケンス正常終了
            WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__);
            return; // 次のステップはないのでここで終了

        case NWM_APIID_CLOSE:
            // 無線ハードウェアの使用禁止要求を発行
            nwmResult = NWM_UnloadDevice(WcmNwmcbCommon);
            break;

        case NWM_APIID_SET_WEPKEY:
        case NWM_APIID_SET_WPA_PSK:
            nwmResult = NWM_SetPowerSaveMode(WcmNwmcbCommon, 
                                             ((wcmw->option & WCM_OPTION_MASK_POWER) == WCM_OPTION_POWER_ACTIVE ? WCM_NWM_POWER_SAVE_OFF : WCM_NWM_POWER_SAVE_ON));
            break;

        case NWM_APIID_SET_PS_MODE:

            // DCF 通信モード開始要求を発行
            nwmResult = NWM_SetReceivingFrameBuffer(WcmNwmcbSetRecvFrmBuf, wcmw->recvBuf, sizeof(wcmw->recvBuf), NWM_PORT_IPV4_ARP);
            break;

        }

        // 同期的な処理結果を確認
        switch (nwmResult)
        {
        case NWM_RETCODE_OPERATING:
            break;

        case NWM_RETCODE_FIFO_ERROR:
#if WCM_DEBUG
            switch (cb->apiid)
            {
            case NWM_APIID_LOAD_DEVICE:
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_Open", nwmResult);
                break;

            case NWM_APIID_CLOSE:
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_UnloadDevice", nwmResult);
                break;

            case NWM_APIID_SET_WEPKEY:
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_Connect", nwmResult);
                break;

            case NWM_APIID_SET_RECEIVING_BUF:
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_SetReceivingFrameBuffer", nwmResult);
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                WCMi_Notify(WCM_RESULT_FAILURE, &(wcmw->bssDesc), 0, __LINE__);
                break;
            }
#endif
            WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
            WCMi_Notify(WCM_RESULT_FAILURE, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
            break;

        case NWM_RETCODE_ILLEGAL_STATE:
            WCMi_Warning(WCMi_WText_InvalidWmState);

        /* Don't break here */
        default:
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
        }
        break;

    case NWM_RETCODE_FAILED:
#if WCM_DEBUG
        switch (cb->apiid)
        {
        case NWM_APIID_LOAD_DEVICE:
            WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_LoadDevice", cb->retcode); //, cb->wlCmdID, cb->wlResult);
            break;

        case NWM_APIID_UNLOAD_DEVICE:
            WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_UnloadDevice", cb->retcode); //, cb->wlCmdID, cb->wlResult);
            break;

        case NWM_APIID_OPEN:
            WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_Open", cb->retcode); //, cb->wlCmdID, cb->wlResult);
            break;

        case NWM_APIID_CLOSE:
            WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_Close", cb->retcode); //, cb->wlCmdID, cb->wlResult);
            break;

        case NWM_APIID_SET_WEPKEY:
            WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_SetWEPKey", cb->retcode); //, cb->wlCmdID, cb->wlResult);
            break;

        }
#endif
        WCMi_SetPhase(WCM_PHASE_IRREGULAR);       // ARM7 でなんらかの理由でエラー
        WCMi_Notify(WCM_RESULT_FAILURE, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
        break;

    case NWM_RETCODE_SDIO_ERROR:    // 無線モジュールとの通信に失敗 (主に LoadDevice の失敗などで発生)
        WCMi_Printf("SDIO Error: APIID=%02X, ERROR_CODE=%02X\n", cb->apiid, cb->retcode);
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
        break;
        
    case NWM_RETCODE_ILLEGAL_STATE:
    case NWM_RETCODE_NWM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */

    case NWM_RETCODE_FATAL_ERROR:  //無線モジュール絡みのソフトウェアでの対応が不可能なエラー
    default:
        WCMi_Printf("%s is received fatal error asyncronously: APIID=%02X, ERROR_CODE=%02X\n", __func__, cb->apiid, cb->retcode);
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, (wcmw->notifyId == WCM_NOTIFY_CONNECT ? &(wcmw->bssDesc) : 0), 0, __LINE__);
    }

}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbScanEx

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_StartScanEx に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbScanEx(void* arg)
{
    NWMStartScanCallback*  cb = (NWMStartScanCallback*)arg;
    NWMRetCode   nwmResult = NWM_RETCODE_MAX;
    WCMWork*    wcmw = WCMi_GetSystemWork();

    switch (cb->retcode)
    {
    case NWM_RETCODE_SUCCESS:
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
            if (cb->bssDescCount != 0) // miz : NWM に state はないが、見つかった AP 数は分かる
            {
                // AP 情報を発見した場合、内部のリストを編集しつつアプリケーションに通知
                s32 i;

                DC_InvalidateRange(wcmw->nwm.scanExParam.scanBuf, wcmw->nwm.scanExParam.scanBufSize);
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
                    if ((wcmw->nwm.scanCount % channels) == 0)
                    {
                        // 各チャンネルのスキャンが一巡したことを通知
                        WCMi_NotifyEx(WCM_NOTIFY_SEARCH_AROUND, WCM_RESULT_SUCCESS, (void*)(wcmw->nwm.scanCount), 0, __LINE__);
                    }
                }
            }

            // 次のチャンネルのスキャン開始要求を発行
            wcmw->nwm.scanExParam.channelList = (u16) ((0x0001 << WCMi_GetNextScanChannel((u16) (32 - MATH_CountLeadingZeros(cb->channelList)))) >> 1);
            DC_StoreRange(wcmw->nwm.scanExParam.scanBuf, wcmw->nwm.scanExParam.scanBufSize);
            wcmw->nwm.scanCount++;
            nwmResult = NWM_StartScanEx(WcmNwmcbScanEx, &(wcmw->nwm.scanExParam));
            break;

        case WCM_PHASE_SEARCH_TO_IDLE:
            // スキャン停止要求を発行
            nwmResult = NWM_RETCODE_OPERATING; // miz : 下記 switch で正常終了させるためのダミー
            WCMi_SetPhase(WCM_PHASE_IDLE); // miz : SCAN フェーズから IDLE フェーズへ移行
            WCMi_Notify(WCM_RESULT_SUCCESS, 0, 0, __LINE__); // miz : これで、スキャン停止のコールバックが呼ばれる。このコールバックを受けて接続処理に遷移するべき。
            break;

        case WCM_PHASE_TERMINATING:
            // 強制終了シーケンス中の場合はここでリセット
            WcmNwmReset();
            return;
        }

        // 同期的な処理結果を確認
        switch (nwmResult)
        {
        case NWM_RETCODE_OPERATING:
            break;

        case NWM_RETCODE_FIFO_ERROR:
#if WCM_DEBUG
            switch (wcmw->phase)
            {
            case WCM_PHASE_SEARCH:
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_StartScanEx", nwmResult);
                break;

            case WCM_PHASE_SEARCH_TO_IDLE:// miz : NWM_EndScan がないのでスルー
                break;
            }
#endif
            WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
            WCMi_Notify(WCM_RESULT_FAILURE, 0, 0, __LINE__);
            break;

        case NWM_RETCODE_ILLEGAL_STATE:
            WCMi_Warning(WCMi_WText_InvalidWmState);

        /* Don't break here */
        default:
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
        }
        break;

    case NWM_RETCODE_FAILED:
        // スキャン要求に失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_StartScanEx", cb->retcode); //, cb->wlCmdID, cb->wlResult);
        WcmNwmReset();
        break;

    case NWM_RETCODE_ILLEGAL_STATE:
    case NWM_RETCODE_NWM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, 0, 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbConnect

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_StartConnectEx に対する結果応答、及び無線接続後の被切断通知
                を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbConnect(void* arg)
{
    NWMConnectCallback*     cb = (NWMConnectCallback*)arg;
    NWMRetCode  nwmResult = NWM_RETCODE_MAX;
    WCMWork*    wcmw = WCMi_GetSystemWork();

    u16         nwmState =NWM_GetState();

    switch (cb->retcode)
    {
    case NWM_RETCODE_SUCCESS:
        switch (nwmState)
        {
        case NWM_STATE_INFRA_CONNECTED:                        // 接続完了通知
            if (wcmw->phase == WCM_PHASE_IRREGULAR)
            {
                // 切断された後に接続完了通知が来た場合は、失敗とみなしリセット
                WCMi_Printf(WCMi_RText_WmDisconnected, "NWM_Connect");
                WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF);         // フェーズを上書き
                WcmNwmReset();
            }
            else
            {
                    // Auth-ID を退避
                    wcmw->nwm.authId = cb->aid;
                WCMi_SetPhase(WCM_PHASE_DCF);         // 非同期シーケンス正常終了
                WCMi_Notify(WCM_RESULT_SUCCESS, &(wcmw->bssDesc), 0, __LINE__);
                }
            break;

        default:    // 想定外のステートコード
            WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
            WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), /*(void*)(cb->state)*/0, __LINE__);
        }
        break;

    case NWM_RETCODE_FAILED:
        switch (cb->reason)
        {
        case NWM_REASON_AUTH_FAILED: // miz : Auth失敗した場合も同様
        case NWM_REASON_WEP_KEY_ERROR:
        case NWM_REASON_WPA_KEY_ERROR:
        case NWM_REASON_NO_NETWORK_AVAIL: // sato adds
        case NWM_REASON_BSS_DISCONNECTED: // miz : terminated notifications
        case NWM_REASON_LOST_LINK: // miz : performs process similar to terminated one
        case NWM_REASON_ASSOC_FAILED: // miz : Assoc失敗した場合も同様
        case NWM_REASON_NO_RESOURCES_AVAIL: // sato adds
        case NWM_REASON_CSERV_DISCONNECT:   // sato adds
        case NWM_REASON_INVAILD_PROFILE:    // sato adds

        case NWM_REASON_TKIP_MIC_ERROR:
        
        case NWM_REASON_NO_QOS_RESOURCES_AVAIL: // QoS 関連のエラー、現状では出ないはず

            switch (wcmw->phase)
            {
                case WCM_PHASE_DCF_TO_IDLE:
                case WCM_PHASE_DCF:
                    // 通知種別を設定
                    wcmw->notifyId = WCM_NOTIFY_DISCONNECT;

                case WCM_PHASE_IDLE_TO_DCF: // NWM が接続に失敗した場合
                    // Auth-ID をクリア
                    wcmw->nwm.authId = 0;

                    // 接続失敗の理由が特定可能な REASON を格納
                    switch (cb->reason)
                    {
                        case NWM_REASON_AUTH_FAILED: // miz : Auth失敗した場合も同様
                            wcmw->nwm.pseudoStatusOnConnectFail = WCM_NWMIF_STATECODE_AUTH_ERROR;
                            break;
                        case NWM_REASON_WEP_KEY_ERROR:
                        case NWM_REASON_WPA_KEY_ERROR:
                            wcmw->nwm.pseudoStatusOnConnectFail = WCM_NWMIF_STATECODE_KEY_ERROR;
                            break;
                        case NWM_REASON_NO_RESOURCES_AVAIL:
                            wcmw->nwm.pseudoStatusOnConnectFail = WCM_NWMIF_STATECODE_CAPACITY_ERROR;
                            break;
                        default:
                            break;
                    }

                /* Don't break here */
                case WCM_PHASE_IRREGULAR:
                    WcmNwmReset();
                    break;
                /* 想定外のフェーズでの切断通知は無視する */
            }
            break;

        case NWM_REASON_DISCONNECT_CMD:
            if(wcmw->phase == WCM_PHASE_IDLE_TO_DCF) //接続を試みている場合のみ再接続するようにする
            {
                wcmw->nwm.authId = 0;
                WcmNwmReset();
            }
            break;

        default:
             WCMi_Printf("Receive Unknown Reason Code: %d (phase:%d)\n", cb->reason, wcmw->phase);
             WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
             WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), (void*)(cb->reason), __LINE__);
        }
        break;

    case NWM_RETCODE_INVALID_PARAM:
//    case NWM_RETCODE_OVER_MAX_ENTRY:
        // 無線接続に失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_Connect", cb->retcode); //, cb->wlCmdID, cb->wlResult);
        WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF);     // IRREGULAR になっている可能性があるのでフェーズを上書き
        WcmNwmReset();
        break;

    case NWM_RETCODE_ILLEGAL_STATE:
    case NWM_RETCODE_NWM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_Printf("Receive Unknown Result Code: %d (phase:%d)\n", cb->reason, wcmw->phase);
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);     // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbDisconnect

  Description:  WM ライブラリからの通知を受け取るハンドラ。
                WM_Disconnect に対する結果応答を受け取る。

  Arguments:    arg     -   WM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbDisconnect(void* arg)
{
    NWMCallback*    cb = (NWMCallback*)arg;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    switch (cb->retcode)
    {
    case NWM_RETCODE_SUCCESS:
        if (wcmw->phase == WCM_PHASE_IRREGULAR)
        {
            // 切断された後に明示的な切断要求の完了通知が来た場合、念のためリセット
            WCMi_Printf(WCMi_RText_WmDisconnected, "NWM_Disconnect");
            WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
            WcmNwmReset();
        }
        else if ( wcmw->phase == WCM_PHASE_TERMINATING )
        {
            // 強制終了シーケンス中の場合はここでリセット
            WcmNwmReset();
        }
        else
        {
            // Auth-ID をクリア
            wcmw->nwm.authId = 0;

            WCMi_SetPhase(WCM_PHASE_IDLE);    // 非同期シーケンス正常終了
            WCMi_Notify(WCM_RESULT_SUCCESS, &(wcmw->bssDesc), 0, __LINE__);
        }
        break;

    case NWM_RETCODE_FAILED:
    case NWM_RETCODE_ILLEGAL_STATE:          // クリティカルなタイミングで通信が切れた場合に返ってきうる
        // 無線接続の切断要求が失敗した場合はリセット
        WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_Disconnect", cb->retcode); //,  cb->wlCmdID, cb->wlResult);
        WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
        WcmNwmReset();
        break;

    case NWM_RETCODE_NWM_DISABLE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbSetRecvFrmBuf

  Description:  NWM ライブラリからの通知を受け取るハンドラ。
                NWM_SetReceivingFrameBuffer に対する結果応答、及びデータ受信通知
                を受け取る。

  Arguments:    arg     -   NWM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbSetRecvFrmBuf(void* arg) // miz
{
    NWMReceivingFrameCallback*  cb = (NWMReceivingFrameCallback*)arg; // miz
    WCMWork*                    wcmw = WCMi_GetSystemWork();
    static u8 myNwType = NWM_NWTYPE_INFRA;

   switch(cb->retcode)
    {
    case NWM_RETCODE_SUCCESS:  // NWM_SetReceivingFrameBuffer に対する結果応答
        if (wcmw->phase == WCM_PHASE_IRREGULAR)
        {
        // 切断された後に DCF 開始完了通知が来た場合は、失敗とみなしリセット
            WCMi_Printf(WCMi_RText_WmDisconnected, "NWM_SetReceivingFrameBuffer");
            WCMi_SetPhase(WCM_PHASE_IDLE_TO_DCF); // フェーズを上書き
            WcmNwmReset(); // miz
        }
        else
        {
            NWMRetCode  nwmResult = NWM_RETCODE_MAX;

            // issues an AP wireless connection request
            if( wcmw->bssDesc.capaInfo & NWM_CAPABILITY_ADHOC_MASK )
            {
                myNwType = NWM_NWTYPE_ADHOC;
            }
            else
            {
                myNwType = NWM_NWTYPE_INFRA;
            }
            
            nwmResult = NWM_Connect(WcmNwmcbConnect, myNwType, (NWMBssDesc*)&(wcmw->bssDesc));

            switch (nwmResult)
            {
            case NWM_RETCODE_OPERATING:
                break;

            case NWM_RETCODE_FIFO_ERROR:
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_Connect", nwmResult);
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                WCMi_Notify(WCM_RESULT_FAILURE, &(wcmw->bssDesc), 0, __LINE__);
                break;

            case NWM_RETCODE_ILLEGAL_STATE:
                WCMi_Warning(WCMi_WText_InvalidWmState);

            /* Don't break here */
            default:
                WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
            }
        }
        break;

    case NWM_RETCODE_RECV_IND: //データ受信
        // CPS とのインターフェースに DCF 受信を通知
        DC_InvalidateRange(cb->recvBuf, WCM_DCF_RECV_BUF_SIZE);

        /* NWM が返す RSSI 値は s16 で -4 ～ 110 の範囲で変動するが、            *
          * WCM では RSSI 値を u8 として扱うため、マイナスは 0 に置き換えキャスト */
        {
            s16 tmpRssi = cb->rssi;
            if (tmpRssi < 0 )
            {
                tmpRssi = 0;
            }
            if (tmpRssi > 256)
            {
                tmpRssi = 255;
            }
            WCMi_ShelterRssi((u8) tmpRssi);
        }

        WCMi_CpsifRecvCallback(cb->recvBuf->sa, cb->recvBuf->da, cb->recvBuf->frame, (s32)(cb->length - 14)); 
        break;
    default:
        /* [TODO] エラー処理 */
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbUnsetRecvFrmBuf

  Description:  NWM ライブラリからの通知を受け取るハンドラ。
                NWM_UnsetRecvFrmBuf に対する結果応答を受け取る。

  Arguments:    arg     -   NWM ライブラリから渡される通知パラメータへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbUnsetRecvFrmBuf(void* arg)
{
    NWMCallback*    cb = (NWMCallback*)arg;
    NWMRetCode  	nwmResult;
    WCMWork*        wcmw = WCMi_GetSystemWork();

    switch (cb->retcode)
    {
    case NWM_RETCODE_SUCCESS:
        if (wcmw->phase == WCM_PHASE_IRREGULAR)
        {
            // 切断された後に DCF 終了完了通知が来た場合は、失敗とみなしリセット
            WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
            WcmNwmReset();
        }
        else
        {
            nwmResult = NWM_Disconnect(WcmNwmcbDisconnect);
            switch (nwmResult)
            {
            case NWM_RETCODE_OPERATING:
                WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE); // 非同期的なシーケンスの開始に成功
                wcmw->notifyId = WCM_NOTIFY_DISCONNECT;
                break;

            case NWM_RETCODE_FIFO_ERROR:
                WCMi_Printf(WCMi_RText_WmSyncError, "NWM_Disconnect", nwmResult);
                WCMi_SetPhase(WCM_PHASE_IRREGULAR);   // ARM7 への要求発行に失敗( シーケンス途上でリトライ不能 )
                WCMi_Notify(WCM_RESULT_FAILURE, &(wcmw->bssDesc), 0, __LINE__);
                break;

            case NWM_RETCODE_ILLEGAL_STATE:         // クリティカルなタイミングで通信が切れた場合
            // 切断する寸前に AP 側から切断された場合は、失敗とみなしリセット
                WCMi_Warning(WCMi_WText_InvalidWmState);
                WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
                WcmNwmReset();
                break;

            /* Don't break here */
             default:
                WCMi_SetPhase(WCM_PHASE_FATAL_ERROR); // 想定範囲外のエラー
                WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
            }
        }
        break;

    case NWM_RETCODE_FAILED:
    case NWM_RETCODE_ILLEGAL_STATE:
        WCMi_Warning(WCMi_WText_InvalidWmState);

    /* Don't break here */
    default:
        WCMi_SetPhase(WCM_PHASE_FATAL_ERROR);
        WCMi_Notify(WCM_RESULT_FATAL_ERROR, &(wcmw->bssDesc), 0, __LINE__);
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
    wcmw->nwm.scanExParam.scanBuf = (NWMBssDesc*)wcmw->recvBuf;
    wcmw->nwm.scanExParam.scanBufSize = sizeof(wcmw->recvBuf);
    wcmw->nwm.scanExParam.channelList = (u16) ((0x0001 << WCMi_GetNextScanChannel(0)) >> 1);
    wcmw->nwm.scanExParam.scanType = (u16) (((wcmw->option & WCM_OPTION_MASK_SCANTYPE) == WCM_OPTION_SCANTYPE_ACTIVE) ? NWM_SCANTYPE_ACTIVE : NWM_SCANTYPE_PASSIVE);
    wcmw->nwm.scanExParam.channelDwellTime = (u16) ((wcmw->maxScanTime != 0) ? wcmw->maxScanTime : NWM_GetDispersionScanPeriod(wcmw->nwm.scanExParam.scanType));
    
    if (bssid == NULL)
    {
        MI_CpuCopy8(WCM_Bssid_Any, wcmw->nwm.scanExParam.bssid, WCM_BSSID_SIZE);
    }
    else
    {
        MI_CpuCopy8(bssid, wcmw->nwm.scanExParam.bssid, WCM_BSSID_SIZE);
    }

    //MI_CpuCopy8( bssid , wcmw->nwm.scanExParam.bssid , WCM_BSSID_SIZE );
    if ((essid == NULL) || (essid == (void*)WCM_Essid_Any))
    {
        MI_CpuCopy8(WCM_Essid_Any, wcmw->nwm.scanExParam.ssid, WCM_ESSID_SIZE);
        wcmw->nwm.scanExParam.ssidLength = 0;
        wcmw->nwm.scanExParam.ssidMatchLength = 0;
    }
    else
    {
        s32 len;

        MI_CpuCopy8(essid, wcmw->nwm.scanExParam.ssid, WCM_ESSID_SIZE);
        for (len = 0; len < WCM_ESSID_SIZE; len++)
        {
            if (((u8*)essid)[len] == '\0')
            {
                break;
            }
        }

        wcmw->nwm.scanExParam.ssidLength = (u16) len;
        wcmw->nwm.scanExParam.ssidMatchLength = (u16) len;
    }

    wcmw->nwm.scanCount = 0;
}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbSetDCFData

  Description:  DCF フレーム送信要求に対する無線ドライバからの非同期的な応答を
                受け取るコールバック関数。

  Arguments:    arg -   無線ドライバからの応答パラメータが格納されているバッファ
                        のアドレスが渡される。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbSetDCFData(void* arg)
{
    NWMSendFrameCallback*   cb      = (NWMSendFrameCallback*)arg;
    volatile WCMCpsifWork*  wcmiw   = WCMi_GetCpsifWork();

    switch (cb->apiid)
    {
    case NWM_APIID_SEND_FRAME:
        wcmiw->sendResult = (u16) (cb->retcode);
        if (cb->retcode == NWM_RETCODE_SUCCESS)
        {
            WCMi_ResetKeepAliveAlarm();
        }

        OS_WakeupThread((OSThreadQueue*)&(wcmiw->sendQ));
        break;
    default:
        WCMi_Printf("WcmNwmcbSetDCFData: Invalid callback.\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmNwmcbSetDCFDataAsync

  Description:  DCF フレーム送信要求に対する無線ドライバからの非同期的な応答を
                受け取るコールバック関数。

  Arguments:    arg -   無線ドライバからの応答パラメータが格納されているバッファ
                        のアドレスが渡される。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmNwmcbSetDCFDataAsync(void* arg)
{
    NWMSendFrameCallback*   cb      = (NWMSendFrameCallback*)arg;
    volatile WCMCpsifWork*  wcmiw   = WCMi_GetCpsifWork();
    s32                     wcmResult = WCM_RESULT_SUCCESS;
    WCMSendInd              sendCallback = NULL;

    switch (cb->apiid)
    {
    case NWM_APIID_SEND_FRAME:
        wcmiw->sendResult = (u16) (cb->retcode);
        if (cb->retcode == NWM_RETCODE_SUCCESS)
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
        case NWM_RETCODE_SUCCESS:
            // 正常終了
            wcmResult = WCM_RESULT_SUCCESS;
            break;

        case NWM_RETCODE_FAILED:
        default:
            // 送信に失敗した場合
            wcmResult = WCM_CPSIF_RESULT_SEND_FAILURE;
            WCMi_Printf(WCMi_RText_WmAsyncError, "NWM_SendFrame", wcmiw->sendResult);
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

// miz : 070612
static void WcmNwmcbSetQos(void* arg)
{
    NWMCallback*    cb = (NWMCallback*)arg;
    NWMRetCode      nwmResult = NWM_RETCODE_MAX;

    OSMessage message = (OSMessage)0x12345;

    switch (cb->retcode)
    {
    case NWM_RETCODE_SUCCESS:
        OS_TPrintf("[WCMN] set_qos success\n");
//		(void)OS_SendMessage(&mesgQueue, message, OS_MESSAGE_BLOCK);
        break;
    default:
        OS_Panic("[WCMN] set_qos error\n");
        break;
    }
}


#include <twl/ltdmain_end.h>


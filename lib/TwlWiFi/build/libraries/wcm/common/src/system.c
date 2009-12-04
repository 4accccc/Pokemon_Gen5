/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     system.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 1094 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include "wcm_private.h"
#include "wcm_message.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

// 全て 0xff の ESSID
const u8    WCM_Essid_Any[WCM_ESSID_SIZE] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// 全て 0xff の BSSID
const u8    WCM_Bssid_Any[WCM_BSSID_SIZE] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

u32                 WCM_DebugFlags = 0;

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static WCMWork*     wcmw = NULL;
static u8           WCMi_OverridedDeviceId = WCM_DEVICEID_DEFAULT;

/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/
static void         WcmConfigure(WCMConfig* config, WCMNotify notify);
static void         WcmInitOption(void);
static void         WcmKeepAliveAlarm(void* arg);

u8                  WCMi_GetDefaultDeviceId(void);
void                WCMi_OverrideDefaultDeviceId(u8 deviceId);

/* コンパイル時アサーション */
#ifndef SDK_ASSERT_ON_COMPILE
#define SDK_ASSERT_ON_COMPILE(expr) extern assert_on_compile ## #__LINE__ (char a[(expr) ? 1 : -1])
#endif

/*---------------------------------------------------------------------------*
  Name:         WCM_Init
  Name:         WCM_InitEx

  Description:  WCM ライブラリを初期化する。同期関数。

  Arguments:    buf     -   WCM ライブラリが内部的に使用するワークバッファへの
                            ポインタを指定する。初期化完了後は、WCM_Finish 関数
                            によって WCM ライブラリを終了するまで別の用途に用い
                            てはいけない。
                len     -   ワークバッファのサイズを指定する。WCM_WORK_SIZE より
                            小さいサイズを指定するとこの関数は失敗する。
                deviceId-   使用する無線デバイスのIDを指定する。

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_NOT_ENOUGH_MEM
 *---------------------------------------------------------------------------*/
s32 WCM_Init(void* buf, s32 len)
{
    return WCM_InitEx(buf, len, WCM_DEVICEID_WM);
}

s32 WCM_InitEx(void* buf, s32 len, u8 deviceId)
{
    OSIntrMode  e = OS_DisableInterrupts();
    s32         expectLen = 0;
    
    // WCM_WORK_SIZE 定数の妥当性をコンパイル時に確認
    SDK_ASSERT_ON_COMPILE(sizeof(WCMWork) <= WCM_WORK_SIZE);
    SDK_ASSERT_ON_COMPILE(sizeof(WCMBssDesc) <= WCM_APLIST_SIZE);
    SDK_ASSERT_ON_COMPILE(sizeof(WCMBssDesc) == sizeof(WMBssDesc));
#ifdef SDK_TWL
    SDK_ASSERT_ON_COMPILE(sizeof(WCMWork) <= WCM_WORK_SIZE_NWM);
    SDK_ASSERT_ON_COMPILE(sizeof(WCMBssDesc) >= sizeof(NWMBssDesc));
#else
    SDK_ASSERT_ON_COMPILE(sizeof(WCMWork) <= WCM_WORK_SIZE_WM);
#endif

    // 未初期化を確認
    if (wcmw != NULL)
    {
        WCMi_Warning(WCMi_WText_AlreadyInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;          // 既に初期化済み
    }

    // パラメータを確認
    if (buf == NULL)
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "buf");
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;          // 指定バッファが NULL
    }

    if ((u32) buf & 0x01f)
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "buf");
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;          // 指定バッファが 32 バイトアラインでない
    }

    // バッファの内容をクリア。無線デバイス専用領域の初期化も兼ねている
    MI_CpuClear8(buf, (u32)len);

    // 無線デバイスの ID が指定されていない場合は動作モードから
    // 使用すべき無線デバイスを決定
    if (deviceId == WCM_DEVICEID_DEFAULT)
    {
        if (WCMi_OverridedDeviceId != WCM_DEVICEID_DEFAULT)
        {
            deviceId = WCMi_OverridedDeviceId;
        }
        else
        {
            deviceId = WCMi_GetDefaultDeviceId();
        }
    }
    
    // 使用する無線デバイスに応じて必要なワーク領域のサイズを求める
    if (!WCM_GetWorkSize(&expectLen, deviceId))
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "deviceId");
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;          // デバイスIDが無効
    }

    // ワーク領域の大きさが十分かを確認
    if (len < expectLen)
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "len");
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_NOT_ENOUGH_MEM;   // 指定バッファのサイズが足りない
    }

    // 各ワーク変数を初期化
    wcmw = (WCMWork*)buf;
    wcmw->phase = WCM_PHASE_WAIT;           // 内部フェーズを初期化
    wcmw->notifyId = WCM_NOTIFY_COMMON;     // 非同期 API 通知 ID を初期化
    wcmw->maxScanTime = 0;                  // 固定スキャン時間を初期化
    wcmw->apListLock = WCM_APLIST_UNLOCK;   // AP 情報保持リストのロック状態を初期化
    wcmw->resetting = WCM_RESETTING_OFF;    // リセット重複呼び出し管理フラグを初期化
    wcmw->deviceId = deviceId;              // 使用する無線デバイスを記憶
    
    WcmInitOption();                        // オプション設定を初期化
    WCMi_InitCpsif();                       // CPS インターフェースを初期化

    // 構造体メンバのアラインメントを確認
    SDK_ASSERT( ((u32)&wcmw->sendBuf & 0x1f) == 0 );
    SDK_ASSERT( ((u32)&wcmw->recvBuf & 0x1f) == 0 );
    SDK_ASSERT( ((u32)&wcmw->bssDesc & 0x1f) == 0 );
    
    SDK_ASSERT( ((u32)&wcmw->wm.work & 0x1f) == 0 );
#ifdef SDK_TWL
    SDK_ASSERT( ((u32)&wcmw->nwm.work & 0x1f) == 0 );
#endif

    // アラームを初期化
    if (!OS_IsTickAvailable())
    {
        OS_InitTick();
    }

    if (!OS_IsAlarmAvailable())
    {
        OS_InitAlarm();
    }

    OS_CreateAlarm(&(wcmw->keepAliveAlarm));

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return WCM_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_Finish

  Description:  WCMライブラリを終了する。同期関数。WCM_Init関数によって指定した
                ワーク用バッファは解放される。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 WCM_Finish(void)
{
    OSIntrMode  e = OS_DisableInterrupts();

    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;  // 初期化されていない
    }

    // 内部フェーズ確認
    if (wcmw->phase != WCM_PHASE_WAIT)
    {
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;  // WAIT フェーズでない
    }

    // ワークバッファを解放
    wcmw = NULL;

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return WCM_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_GetWorkSize

  Description:  指定した無線デバイスを使用するために必要なワークサイズを取得
                する。

  Arguments:    workSize     - 取得したワークサイズを格納するポインタを指定
                deviceId     - 使用する無線デバイスの ID

  Returns:      BOOL         - 成功の可否。
                               NITRO 動作時に新無線のワークサイズを取得しよう
                               とした場合も失敗となる。
 *---------------------------------------------------------------------------*/
BOOL WCM_GetWorkSize(s32* workSize, u8 deviceId)
{
    if (deviceId == WCM_DEVICEID_DEFAULT)
    {
        deviceId = WCMi_GetDefaultDeviceId();
    }
    switch(deviceId)
    {
    case WCM_DEVICEID_WM:
        *workSize = WCM_WORK_SIZE_WM;
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (!OS_IsRunOnTwl())
        {
            return FALSE;
        }
        *workSize = WCM_WORK_SIZE_NWM;
        break;
#endif
    default:
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_StartupAsync

  Description:  無線機能の起動シーケンスを開始する。
                内部的には WAIT から IDLE へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には notify で指定したコールバック関数により非同期的な
                処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_STARTUP となる。

  Arguments:    config  -   WCM の動作設定となる構造体へのポインタを指定。
                notify  -   非同期的な処理結果を通知するコールバック関数を指定。
                            このコールバック関数は以後の非同期的な関数の結果
                            通知に共通して使用される。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_WMDISABLE ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_StartupAsync(WCMConfig* config, WCMNotify notify)
{
    OSIntrMode  e           = OS_DisableInterrupts();
    s32         wcmResult   = WCM_RESULT_SUCCESS;
    
    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;                  // 初期化されていない
    }

    // 内部フェーズ確認
    switch (wcmw->phase)
    {
    case WCM_PHASE_WAIT:
        WcmConfigure(config, notify);
        break;

    case WCM_PHASE_WAIT_TO_IDLE:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_PROGRESS;                 // 既に同じリクエストの非同期処理中

    case WCM_PHASE_IDLE:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_SUCCESS;                  // 状態遷移目標フェーズに移行済み

    default:
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;                  // 要求を受け付けられないフェーズ
    }

    // WM_Init を呼び出す必要が無いか念のため確認しておく 
#ifdef WM_PRECALC_ALLOWEDCHANNEL /* (!SDK_TEG && SDK_VERSION_WL >= 15600) */
    // 無線通信を禁止されている本体でないことを確認
    if (0 == WM_GetAllowedChannel())
    {
        WCMi_Warning(WCMi_WText_WirelessForbidden);
        return WCM_RESULT_WMDISABLE;            // 無線通信が許可されていない
    }
#else
#error
#endif

    switch(wcmw->deviceId)
    {
    case WCM_DEVICEID_WM:
        wcmResult = WCMi_WmifStartupAsync();
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (OS_IsRunOnTwl())
        {
            wcmResult = WCMi_NwmifStartupAsync();
            break;
        }
        // no break;
#endif
    default:
        wcmResult = WCM_RESULT_FATAL_ERROR;
    }

    (void)OS_RestoreInterrupts(e);
    return wcmResult;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_CleanupAsync

  Description:  無線機能の停止シーケンスを開始する。
                内部的には IDLE から WAIT へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_CLEANUP となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_CleanupAsync(void)
{
    OSIntrMode  e           = OS_DisableInterrupts();
    s32         wcmResult   = WCM_RESULT_SUCCESS;

    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;                  // 初期化されていない
    }

    // 内部フェーズ確認
    switch (wcmw->phase)
    {
    case WCM_PHASE_IDLE:
        break;

    case WCM_PHASE_IDLE_TO_WAIT:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_PROGRESS;                 // 既に同じリクエストの非同期処理中

    case WCM_PHASE_WAIT:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_SUCCESS;                  // 状態遷移目標フェーズに移行済み

    default:
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;                  // 要求を受け付けられないフェーズ
    }

    switch(wcmw->deviceId)
    {
    case WCM_DEVICEID_WM:
        wcmResult = WCMi_WmifCleanupAsync();
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (OS_IsRunOnTwl())
        {
            wcmResult = WCMi_NwmifCleanupAsync();
            break;
        }
        // no break;
#endif
    default:
        wcmResult = WCM_RESULT_FATAL_ERROR;
    }

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return wcmResult;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_SearchAsync

  Description:  AP 自動探索開始シーケンス、もしくは停止シーケンスを開始する。
                パラメータによって WCM_BeginSearchAsync 関数を呼び出すか、
                WCM_EndSearchAsync 関数を呼び出すか振り分けるだけの関数。

  Arguments:    bssid   -   探索する AP の BSSID を指定する。NULL を指定した
                            場合には、探索の停止を指示したものとみなされる。
                essid   -   探索する AP の ESSID を指定する。NULL を指定した
                            場合には、探索の停止を指示したものとみなされる。
                option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。0 を指定した場合は
                            変更は行われない。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_SearchAsync(const void* bssid, const void* essid, u32 option)
{
    // パラメータを確認
    if ((bssid == NULL) || (essid == NULL))
    {
        return WCM_EndSearchAsync();
    }

    return WCM_BeginSearchAsync(bssid, essid, option);
}

/*---------------------------------------------------------------------------*
  Name:         WCM_BeginSearchAsync

  Description:  AP 自動探索開始シーケンスを開始する。既に自動探索状態であった
                場合に、探索条件の変更のみ行うことも可能。
                内部的には IDLE から SEARCH へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_BEGIN_SEARCH となる。

  Arguments:    bssid   -   探索する AP の BSSID を指定する。NULL もしくは
                            WCM_BSSID_ANY を指定した場合は任意の BSSID を持つ
                            AP を探索する。
                essid   -   探索する AP の ESSID を指定する。NULL もしくは
                            WCM_ESSID_ANY を指定した場合は任意の ESSID を持つ
                            AP を探索する。
                option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。0 を指定した場合は
                            変更は行わない。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_BeginSearchAsync(const void* bssid, const void* essid, u32 option)
{
    OSIntrMode  e           = OS_DisableInterrupts();
    s32         wcmResult   = WCM_RESULT_SUCCESS;

    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;                  // 初期化されていない
    }

    // AP 自動探索開始シーケンスを開始、もしくは探索条件変更
    switch(wcmw->deviceId)
    {
    case WCM_DEVICEID_WM:
        wcmResult = WCMi_WmifBeginSearchAsync(bssid, essid, option);
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (OS_IsRunOnTwl())
        {
            wcmResult = WCMi_NwmifBeginSearchAsync(bssid, essid, option);
            break;
        }
        // no break;
#endif
    default:
        wcmResult = WCM_RESULT_FATAL_ERROR;
    }

    (void)OS_RestoreInterrupts(e);
    return wcmResult;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_EndSearchAsync

  Description:  AP 自動探索停止シーケンスを開始する。
                内部的には SEARCH から IDLE へフェーズの移行処理が行われる。
                非同期関数であり、動機的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_END_SEARCH となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_EndSearchAsync(void)
{
    OSIntrMode  e           = OS_DisableInterrupts();

    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;  // 初期化されていない
    }

    // 内部フェーズ確認
    switch (wcmw->phase)
    {
    case WCM_PHASE_SEARCH:
        WCMi_SetPhase(WCM_PHASE_SEARCH_TO_IDLE);
        wcmw->notifyId = WCM_NOTIFY_END_SEARCH;
        break;

    case WCM_PHASE_SEARCH_TO_IDLE:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_PROGRESS; // 既に同じリクエストの非同期処理中

    case WCM_PHASE_IDLE:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_SUCCESS;  // 状態遷移目標フェーズに移行済み

    default:
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;  // 要求を受け付けられないフェーズ
    }

    /* スキャン停止要求の発行は StartScanEx のコールバック内で行うので、ここでは行わない */

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return WCM_RESULT_ACCEPT;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_ConnectAsync

  Description:  AP との無線接続シーケンスを開始する。
                内部的には IDLE から DCF へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_CONNECT となる。

  Arguments:    bssDesc -   接続する AP の無線ネットワーク情報を指定する。
                            WCM_SearchAsync によって得られた情報がそのまま指定
                            されることを想定。
                wepDesc -   WCMWepDesc 型の WEP キーによる暗号化設定情報を指定
                            する。NULL の場合は、WEP 暗号化なしという設定になる。
                option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。0 を指定した場合は
                            変更は行われない。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_ConnectAsync(void* bssDesc, void* wepDesc, u32 option)
{
    OSIntrMode      e           = OS_DisableInterrupts();
    WCMWork*        wcmw        = WCMi_GetSystemWork();
    s32             wcmResult   = WCM_RESULT_SUCCESS;
    /*
        WCMWepDesc は基本的に WCMPrivacyDesc にキャストしても安全。
        ただし、mode メンバの値によって共用体のメンバが切り替わる事に注意。
     */
    WCMPrivacyDesc* privacyDesc = (WCMPrivacyDesc*)wepDesc;
    
    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;              // 初期化されていない
    }

    // 内部フェーズ確認
    switch (wcmw->phase)
    {
    case WCM_PHASE_IDLE:
        // パラメータ確認
        if (bssDesc == NULL)
        {
            WCMi_Warning(WCMi_WText_IllegalParam, "bssDesc");
            (void)OS_RestoreInterrupts(e);
            return WCM_RESULT_FAILURE;          // bssDesc 引数が NULL
        }
        if (((WMBssDesc*)bssDesc)->gameInfoLength > 0)
        {
            WCMi_Warning(WCMi_WText_IllegalParam, "bssDesc");
            (void)OS_RestoreInterrupts(e);
            return WCM_RESULT_FAILURE;          // bssDesc 引数が AP 情報ではなく DS 親機
        }
        
        if (privacyDesc != NULL)
        {
            // wepDesc の中身の一貫性をチェック
            if ((privacyDesc->mode <= WCM_PRIVACYMODE_WEP128 && privacyDesc->keyId >= 4)
                || wcmw->privacyDesc.keyLen > 64)
            {
                WCMi_Warning(WCMi_WText_IllegalParam, "wepDesc");
                (void)OS_RestoreInterrupts(e);
                return WCM_RESULT_FAILURE;      // wepDesc 引数が想定外の内容
            }

            // wepDesc の中身を WCMWork 上に安全にコピー
            wcmw->privacyDesc.mode = privacyDesc->mode;
            if (privacyDesc->mode == WCM_PRIVACYMODE_NONE)
            {
                MI_CpuClear8(wcmw->privacyDesc.key, WM_SIZE_WEPKEY);
            }
            else if (privacyDesc->mode <= WCM_PRIVACYMODE_WEP128)
            {
                wcmw->privacyDesc.keyId = privacyDesc->keyId;
                MI_CpuCopy8(privacyDesc->key, wcmw->privacyDesc.key, WM_SIZE_WEPKEY);
            }
            else /* WPA */
            {
                wcmw->privacyDesc.keyLen = privacyDesc->keyLen;
                MI_CpuCopy8(privacyDesc->key, wcmw->privacyDesc.key, wcmw->privacyDesc.keyLen);
            }
        }
        else
        {
            MI_CpuClear8(&(wcmw->privacyDesc), sizeof(WCMPrivacyDesc));
        }
        MI_CpuCopy8(bssDesc, &(wcmw->bssDesc), sizeof(WMBssDesc));
        
        (void)WCM_UpdateOption(option);
        break;

    case WCM_PHASE_IDLE_TO_DCF:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_PROGRESS;             // 既に同じリクエストの非同期処理中

    case WCM_PHASE_DCF:
        (void)OS_RestoreInterrupts(e);          // 状態遷移目標フェーズに移行済み
        return WCM_RESULT_SUCCESS;

    default:
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;              // 要求を受け付けられないフェーズ
    }

    // AP との無線接続シーケンス
    switch(wcmw->deviceId)
    {
    case WCM_DEVICEID_WM:
        wcmResult = WCMi_WmifConnectAsync();
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (OS_IsRunOnTwl())
        {
            wcmResult = WCMi_NwmifConnectAsync();
            break;
        }
        // no break;
#endif
    default:
        wcmResult = WCM_RESULT_FATAL_ERROR;
    }

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return wcmResult;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_DisconnectAsync

  Description:  AP との無線接続を切断するシーケンスを開始する。
                内部的には DCF から IDLE へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_DISCONNECT となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_DisconnectAsync(void)
{
    OSIntrMode  e           = OS_DisableInterrupts();
    s32         wcmResult   = WCM_RESULT_SUCCESS;

    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;              // 初期化されていない
    }

    // 内部フェーズ確認
    switch (wcmw->phase)
    {
    case WCM_PHASE_DCF:
        break;

    case WCM_PHASE_DCF_TO_IDLE:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_PROGRESS;             // 既に同じリクエストの非同期処理中

    case WCM_PHASE_IDLE:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_SUCCESS;              // 状態遷移目標フェーズに移行済み

    default:
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;              // 要求を受け付けられないフェーズ
    }

    // AP との無線接続の切断シーケンスを開始
    if (wcmw->resetting == WCM_RESETTING_ON)
    {
        /* 切断通知を受けてリセット中なので、このリセットをもって切断要求が実行されたことにする */
        WCMi_SetPhase(WCM_PHASE_DCF_TO_IDLE);
        wcmw->notifyId = WCM_NOTIFY_DISCONNECT;
    }
    else
    {
        switch(wcmw->deviceId)
        {
        case WCM_DEVICEID_WM:
            wcmResult = WCMi_WmifDisconnectAsync();
            break;
#ifdef SDK_TWL
        case WCM_DEVICEID_NWM:
            if (OS_IsRunOnTwl())
            {
                wcmResult = WCMi_NwmifDisconnectAsync();
                break;
            }
            // no break;
#endif
        default:
            wcmResult = WCM_RESULT_FATAL_ERROR;
        }
    }

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return wcmResult;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_TerminateAsync

  Description:  無線機能の強制終了シーケンスを開始する。
                別の非同期処理が実行されていない限りはどのフェーズからでも実行
                可能であり、内部的には WAIT へとフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_SartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_TERMINATE となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32 WCM_TerminateAsync(void)
{
    OSIntrMode  e           = OS_DisableInterrupts();
    s32         wcmResult   = WCM_RESULT_SUCCESS;

    // 初期化済みを確認
    if (wcmw == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;              // 初期化されていない
    }

    // 内部フェーズ確認
    switch (wcmw->phase)
    {
    case WCM_PHASE_IDLE:
    case WCM_PHASE_DCF:
    case WCM_PHASE_IRREGULAR:
        break;

    case WCM_PHASE_TERMINATING:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_PROGRESS;             // 既に同じリクエストの非同期処理中

    case WCM_PHASE_WAIT:
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_SUCCESS;              // 状態遷移目標フェーズに移行済み

    case WCM_PHASE_SEARCH:
        WCMi_SetPhase(WCM_PHASE_TERMINATING);
        wcmw->notifyId = WCM_NOTIFY_TERMINATE;
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_ACCEPT;               // 非同期処理要求を受諾

    /* リセット要求の発行は StartScanEx のコールバック内で行うので、ここでは行わない */
    case WCM_PHASE_WAIT_TO_IDLE:
    case WCM_PHASE_IDLE_TO_WAIT:
    case WCM_PHASE_IDLE_TO_SEARCH:
    case WCM_PHASE_SEARCH_TO_IDLE:
    case WCM_PHASE_IDLE_TO_DCF:
    case WCM_PHASE_DCF_TO_IDLE:
    case WCM_PHASE_FATAL_ERROR:
    default:
        WCMi_Warning(WCMi_WText_IllegalPhase, wcmw->phase);
        (void)OS_RestoreInterrupts(e);
        return WCM_RESULT_FAILURE;              // 要求を受け付けられないフェーズ
    }

    // 無線機能の強制終了シーケンスを開始する
    if (wcmw->resetting == WCM_RESETTING_ON)
    {
        /* DCF 通信中に切断されてリセット中なので、このリセットをもって強制終了要求が実行されたことにする */
        WCMi_SetPhase(WCM_PHASE_TERMINATING);
        wcmw->notifyId = WCM_NOTIFY_TERMINATE;
    }
    else
    {
        switch(wcmw->deviceId)
        {
        case WCM_DEVICEID_WM:
            wcmResult = WCMi_WmifTerminateAsync();
            break;
#ifdef SDK_TWL
        case WCM_DEVICEID_NWM:
            if (OS_IsRunOnTwl())
            {
                wcmResult = WCMi_NwmifTerminateAsync();
                break;
            }
            // no break;
#endif
        default:
            wcmResult = WCM_RESULT_FATAL_ERROR;
        }
    }

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return wcmResult;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_GetPhase

  Description:  WCM ライブラリの内部状態 ( フェーズ ) を取得する。同期関数。

  Arguments:    None.

  Returns:      s32     -   現在の WCM ライブラリのフェーズを返す。
 *---------------------------------------------------------------------------*/
s32 WCM_GetPhase(void)
{
    OSIntrMode  e = OS_DisableInterrupts();
    s32         phase = WCM_PHASE_NULL;

    // 初期化済みを確認
    if (wcmw != NULL)
    {
        phase = (s32) (wcmw->phase);
    }

    (void)OS_RestoreInterrupts(e);
    return phase;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_UpdateOption

  Description:  WCM ライブラリのオプション設定を更新する。

  Arguments:    option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。
                            0 を指定した場合は何も更新を行わない動作になる。

  Returns:      u32     -   変更を行う前のオプション変数を返す。
 *---------------------------------------------------------------------------*/
u32 WCM_UpdateOption(u32 option)
{
    OSIntrMode  e = OS_DisableInterrupts();
    u32         filter = 0;
    u32         old_option = wcmw->option;

    // 初期化確認
    if (wcmw == NULL)
    {
        (void)OS_RestoreInterrupts(e);
        return 0;
    }

    // 更新すべきオプションカテゴリを抽出し、一旦クリアすべきビットを編集する
    if (option & WCM_OPTION_TEST_CHANNEL)
    {
        filter |= WCM_OPTION_FILTER_CHANNEL;
        if (!(option & WCM_OPTION_FILTER_CHANNEL))
        {
            option |= WCM_OPTION_CHANNEL_RDC;
        }
    }

    if (option & WCM_OPTION_TEST_POWER)
    {
        filter |= WCM_OPTION_FILTER_POWER;
    }

    if (option & WCM_OPTION_TEST_AUTH)
    {
        filter |= WCM_OPTION_FILTER_AUTH;
    }

    if (option & WCM_OPTION_TEST_SCANTYPE)
    {
        filter |= WCM_OPTION_FILTER_SCANTYPE;
    }

    if (option & WCM_OPTION_TEST_ROUNDSCAN)
    {
        filter |= WCM_OPTION_FILTER_ROUNDSCAN;
    }

    // オプション変数の各ビットを更新
    wcmw->option = (u32) ((old_option &~filter) | option);

    (void)OS_RestoreInterrupts(e);
    return old_option;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_GetAllowedChannel

  Description:  現在使用している無線デバイスの使用許可チャンネルリストを取得する。

  Arguments:    None.

  Returns:      u16    - 1～13chのうち、使用を許可されたチャンネル群のビットフィールドを返します。
                         最下位ビットから順に 1 チャンネル, 2 チャンネル……に対応します。
 *---------------------------------------------------------------------------*/
u16 WCM_GetAllowedChannel(void)
{
    WCMWork*    wcmw = WCMi_GetSystemWork();

    /* 無線使用可能状態でないと使用許可チャンネルを取得できない NWM に合わせて *
     * フェイズが WCM_PHASE_IDLE～WCM_PHASE_DCF_TO_IDLE の時のみ正しい値を返す */
    if( wcmw->phase >= WCM_PHASE_IDLE && wcmw->phase <= WCM_PHASE_DCF_TO_IDLE)
    {
        switch(wcmw->deviceId)
        {
            case WCM_DEVICEID_WM:
                return WCMi_WmifGetAllowedChannel();
                break;
#ifdef SDK_TWL
            case WCM_DEVICEID_NWM:
                if (OS_IsRunOnTwl())
                {
                    return WCMi_NwmifGetAllowedChannel();
                }
                break;
#endif /* SDK_TWL */
        }
    }
    /* 未知のデバイスが選択されている場合、無線デバイスが使用可能な状態でない場合は 0 を返す。*/
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_SetChannelScanTime

  Description:  AP の自動探索時に１つのチャンネルをスキャンする時間を設定する。

  Arguments:    msec    -   １チャンネルをスキャンする時間を ms 単位で指定。
                            10 ～ 1000 までの間で設定可能だが、この範囲外の値を
                            指定すると自動探索はデフォルトの設定時間で探索を行う
                            ようになる。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCM_SetChannelScanTime(u16 msec)
{
    // 初期化確認
    if (wcmw != NULL)
    {
        if ((msec >= 10) && (msec <= 1000))
        {
            wcmw->maxScanTime = msec;
        }
        else
        {
            wcmw->maxScanTime = 0;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_GetSystemWork

  Description:  WCM ライブラリが内部で使用しているワークバッファへのポインタを
                取得するための内部関数。

  Arguments:    None.

  Returns:      WCMWork*    -   ワークバッファへのポインタを返す。初期化前など
                                バッファが存在しない場合は NULL になる。
 *---------------------------------------------------------------------------*/
WCMWork* WCMi_GetSystemWork(void)
{
    return wcmw;
}


/*---------------------------------------------------------------------------*
  Name:         WcmConfigure

  Description:  WCM の内部動作設定を編集する。

  Arguments:    config  -   WCM の動作設定構造体へのポインタを指定。
                notify  -   非同期的な処理結果を通知するコールバック関数を指定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmConfigure(WCMConfig* config, WCMNotify notify)
{
    if (config == NULL)
    {
        // 設定が示されない場合、デフォルト値を使用する
        wcmw->config.dmano = 3;
        wcmw->config.pbdbuffer = NULL;
        wcmw->config.nbdbuffer = 0;
        wcmw->config.nbdmode = 0;
    }
    else
    {
        // DMA 番号を退避
        if (config->dmano &~(0x03))
        {
            // DMA 番号は 0 ～ 3 でないといけない
            WCMi_Warning(WCMi_WText_IllegalParam, "config->dmano");
        }

        wcmw->config.dmano = (config->dmano & 0x03);

        // AP 情報保持リスト領域の設定を退避
        if ((((4 - ((u32) (config->pbdbuffer) & 0x03)) % 4) + sizeof(WCMApListHeader)) > config->nbdbuffer)
        {
            // リスト管理用ヘッダ領域すら確保できないサイズなので、バッファなしの場合と同等
            wcmw->config.pbdbuffer = NULL;
            wcmw->config.nbdbuffer = 0;
        }
        else
        {
            /*
             * 与えられたバッファは 4 バイトアラインされていない可能性もあるため、
             * 4 バイトアラインされた位置にずらし、サイズをその分目減りさせる。
             */
            wcmw->config.pbdbuffer = (void*)WCM_ROUNDUP4((u32) (config->pbdbuffer));
            wcmw->config.nbdbuffer = config->nbdbuffer - (s32) ((4 - ((u32) (config->pbdbuffer) & 0x03)) % 4);
            MI_CpuClear8(wcmw->config.pbdbuffer, (u32) (wcmw->config.nbdbuffer));
        }

        wcmw->config.nbdmode = config->nbdmode;
    }

    // 通知ベクトル退避
    wcmw->notify = notify;
}

/*---------------------------------------------------------------------------*
  Name:         WcmInitOption

  Description:  WCM ライブラリのオプション設定値を初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmInitOption(void)
{
    /*
     * IW と I/O を合わせる為に下記のようなデフォルト設定になっているが、
     * CHANNEL は ALL 、SCANTYPE は ACTIVE であったほうが妥当と思われる。
     */
    wcmw->option = WCM_OPTION_CHANNEL_RDC |
        WCM_OPTION_POWER_SAVE |
        WCM_OPTION_AUTH_OPENSYSTEM |
        WCM_OPTION_SCANTYPE_PASSIVE |
        WCM_OPTION_ROUNDSCAN_IGNORE;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_GetNextScanChannel

  Description:  オプションに設定されている探索時のチャンネルリストから、次に
                スキャンすべきチャンネルを決定する。

  Arguments:    channel -   今回スキャンしたチャンネルを渡す。

  Returns:      u16     -   次回スキャンすべきチャンネルを返す。
 *---------------------------------------------------------------------------*/
u16 WCMi_GetNextScanChannel(u16 channel)
{
    s32 i;

    for (i = 0; i < 13; i++)
    {
        if (wcmw->option & (0x0001 << (((channel + i) % 13) + 1)))
        {
            /* TWL 無線は使用される国毎に使用可能なチャンネルが異なるので、利用できるチャンネルのみを返す。 */
            if (WCM_GetAllowedChannel() & (0x0001 << (((channel + i) % 13))))
            {
                break;
            }
        }
    }

    return(u16) (((channel + i) % 13) + 1);
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_Notify

  Description:  非同期的な処理の処理結果をコールバックする。
                不定期に発生する通知もこの関数を通ってコールバックされる。
                通知種別については WCM 内部共通変数から自動的に設定され、かつ
                重複した通知を避けるために、通知する度にクリアされる。

  Arguments:    result  -   処理結果を指定する。
                para0   -   通知関数に渡すパラメータ[ 0 ]
                para1   -   通知関数に渡すパラメータ[ 1 ]
                para2   -   通知関数に渡すパラメータ[ 2 ]

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_Notify(s16 result, void* para0, void* para1, s32 para2)
{
    s16 notifyId = wcmw->notifyId;

    wcmw->notifyId = WCM_NOTIFY_COMMON;
    WCMi_NotifyEx(notifyId, result, para0, para1, para2);
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_NotifyEx

  Description:  アプリケーションへのコールバックによる通知を行う。

  Arguments:    result  -   処理結果を指定する。
                para0   -   通知関数に渡すパラメータ[ 0 ]
                para1   -   通知関数に渡すパラメータ[ 1 ]
                para2   -   通知関数に渡すパラメータ[ 2 ]

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_NotifyEx(s16 notify, s16 result, void* para0, void* para1, s32 para2)
{
    if (wcmw->notify)
    {
        WCMNotice   notice;

        notice.notify = notify;
        notice.result = result;
        notice.parameter[0].p = para0;
        notice.parameter[1].p = para1;
        notice.parameter[2].n = para2;
        wcmw->notify(&notice);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_SetPhase

  Description:  WCM ライブラリの内部状態 ( フェーズ ) を変更する。
                FATAL_ERROR 状態から別のフェーズに変更はできない。
                また、DCF 通信状態へのフェーズ変更及び DCF 通信状態からの
                フェーズ変更を監視し、キープアライブパケット送信用アラームの
                動作を制御する。

  Arguments:    phase   -   変更するフェーズを指定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_SetPhase(u8 phase)
{
    OSIntrMode  e = OS_DisableInterrupts();

    // DCF 通信状態から別の状態に移行する際にアラームを停止
    if ((wcmw->phase == WCM_PHASE_DCF) && (phase != WCM_PHASE_DCF))
    {
        // Keep Alive 用アラーム停止
        OS_CancelAlarm(&(wcmw->keepAliveAlarm));
    }

    // FATAL ERROR 状態でない場合は指定状態に変更
    if (wcmw->phase != WCM_PHASE_FATAL_ERROR)
    {
        wcmw->phase = phase;
    }

    // DCF 通信状態に移行する際にアラームを開始
    if (phase == WCM_PHASE_DCF)
    {
        // Keep Alive Alarm の二重呼びを防止
        OS_CancelAlarm(&(wcmw->keepAliveAlarm));
        // Keep Alive 用アラーム開始
        OS_SetAlarm(&(wcmw->keepAliveAlarm), OS_SecondsToTicks(WCM_KEEP_ALIVE_SPAN), WcmKeepAliveAlarm, NULL);
    }
    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_ResetKeepAliveAlarm

  Description:  キープアライブ NULL パケット送信用アラームをリセットし、規定
                時間のアラームを設定し直す。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_ResetKeepAliveAlarm(void)
{
    OSIntrMode  e = OS_DisableInterrupts();

    // 改めてアラームをセット
    OS_CancelAlarm(&(wcmw->keepAliveAlarm));
    if (wcmw->phase == WCM_PHASE_DCF)
    {
        OS_SetAlarm(&(wcmw->keepAliveAlarm), OS_SecondsToTicks(WCM_KEEP_ALIVE_SPAN), WcmKeepAliveAlarm, NULL);
    }
    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         WcmKeepAliveAlarm

  Description:  キープアライブ NULL パケット送信用アラームのアラームハンドラ。

  Arguments:    arg     -   未使用。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmKeepAliveAlarm(void* arg)
{
#pragma unused(arg)

    //WCMi_CpsifSendNullPacket();
    WCMi_ResetKeepAliveAlarm();
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_GetDefaultDeviceId

  Description:  

  Arguments:    None.

  Returns:      u8      - 
 *---------------------------------------------------------------------------*/
u8 WCMi_GetDefaultDeviceId(void)
{
#ifdef SDK_NITRO
    return WCM_DEVICEID_WM;
#else  /* !SDK_NITRO */
#ifdef SDK_WIFI_NWM
    if (OS_IsRunOnTwl())
    {
        return WCM_DEVICEID_NWM;
    }
    else
#endif /* SDK_WIFI_NWM */
    {
        return WCM_DEVICEID_WM;
    }
#endif /* !SDK_NITRO */
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_OverrideDefaultDeviceId

  Description:  

  Arguments:    deviceId     - 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_OverrideDefaultDeviceId(u8 deviceId)
{
    if (WCM_DEVICEID_DEFAULT <= deviceId && deviceId <= WCM_DEVICEID_NWM)
    {
        WCMi_OverridedDeviceId = deviceId;
    }
    else
    {
        OS_Panic("Invalide device Id.");
    }
}


#ifndef SDK_FINALROM
/*---------------------------------------------------------------------------*
  Name:         WCMi_DumpMemory

  Description:  

  Arguments:    pDump        - 
                len          - 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_DumpMemory(const u8* pDump, u32 len)
{
    int     i;
    for (i = 0; i < len; i++)
    {
        if (i % 16)
        {
            OS_TPrintf(" ");
        }

        OS_TPrintf("%02X", pDump[i]);
        if ((i % 16) == 15)
        {
            OS_TPrintf("\n");
        }
    }
    OS_TPrintf("\n");
}
#endif
/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/

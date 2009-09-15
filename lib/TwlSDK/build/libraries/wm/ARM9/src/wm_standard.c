/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_standard.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 9342 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL WmCheckParentParameter(const WMParentParam *param);


/*---------------------------------------------------------------------------*
  Name:         WM_Enable

  Description:  無線ハードウェアを使用可能な状態にする。
                内部状態はREADY状態からSTOP状態に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_Enable(WMCallbackFunc callback)
{
    return WMi_EnableEx(callback, 0);
}

/*---------------------------------------------------------------------------*
  Name:         WM_EnableForListening

  Description:  無線ハードウェアを使用可能な状態にする。
                内部状態はREADY状態からSTOP状態に遷移される。
                電波を送出するオペレーションはできない。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                blink       -   LED を点滅させるか

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_EnableForListening(WMCallbackFunc callback, BOOL blink)
{
    u32 miscFlags = WM_MISC_FLAG_LISTEN_ONLY;

    if (!blink)
    {
        miscFlags |= WM_MISC_FLAG_NO_BLINK;
    }

    return WMi_EnableEx(callback, miscFlags);
}

/*---------------------------------------------------------------------------*
  Name:         WMi_EnableEx

  Description:  無線ハードウェアを使用可能な状態にする。
                内部状態はREADY状態からSTOP状態に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                miscFlags   -   初期化時のフラグ

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_EnableEx(WMCallbackFunc callback, u32 miscFlags)
{
    WMErrCode result;

    // READYステート以外では実行不可
    result = WMi_CheckState(WM_STATE_READY);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_ENABLE, callback);
    
    // 通信中のスリープを禁止するためのコールバックを登録
    WMi_RegisterSleepCallback();

    // ARM7にFIFOで通知
    {
        WMArm9Buf *p = WMi_GetSystemWork();

        result = WMi_SendCommand(WM_APIID_ENABLE, 4,
                                 (u32)(p->WM7), (u32)(p->status), (u32)(p->fifo7to9), miscFlags);
        WM_CHECK_RESULT(result);
    }

    // 正常終了
    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Disable

  Description:  無線ハードウェアを使用禁止状態にする。
                内部状態はSTOP状態からREADY状態に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_Disable(WMCallbackFunc callback)
{
    WMErrCode result;

    // STOPステート以外では実行不可
    result = WMi_CheckState(WM_STATE_STOP);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_DISABLE, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_DISABLE, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_PowerOn

  Description:  無線ハードウェアを起動する。
                内部状態はSTOP状態からIDLE状態に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_PowerOn(WMCallbackFunc callback)
{
    WMErrCode result;

    // STOPステート以外では実行不可
    result = WMi_CheckState(WM_STATE_STOP);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_POWER_ON, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_POWER_ON, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_PowerOff

  Description:  無線ハードウェアをシャットダウンする。
                内部状態はIDLE状態からSTOP状態に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_PowerOff(WMCallbackFunc callback)
{
    WMErrCode result;

    // IDLEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_POWER_OFF, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_POWER_OFF, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Initialize

  Description:  WM初期化処理を行う。

  Arguments:    wmSysBuf    -   呼び出し元によって確保されたバッファへのポインタ。
                                バッファのサイズはWM_SYSTEM_BUF_SIZEだけ必要。
                callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                dmaNo       -   WMが使用するDMA番号。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_Initialize(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo)
{
    return WMi_InitializeEx(wmSysBuf, callback, dmaNo, 0);
}

/*---------------------------------------------------------------------------*
  Name:         WM_InitializeForListening

  Description:  WM初期化処理を行う。
                電波を送出するオペレーションはできない。

  Arguments:    wmSysBuf    -   呼び出し元によって確保されたバッファへのポインタ。
                                バッファのサイズはWM_SYSTEM_BUF_SIZEだけ必要。
                callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                dmaNo       -   WMが使用するDMA番号。
                blink       -   LED を点滅させるかどうか

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_InitializeForListening(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo, BOOL blink)
{
    u32 miscFlags = WM_MISC_FLAG_LISTEN_ONLY;

    if (!blink)
    {
        miscFlags |= WM_MISC_FLAG_NO_BLINK;
    }

    return WMi_InitializeEx(wmSysBuf, callback, dmaNo, miscFlags);
}

/*---------------------------------------------------------------------------*
  Name:         WMi_InitializeEx

  Description:  WM初期化処理を行う。

  Arguments:    wmSysBuf    -   呼び出し元によって確保されたバッファへのポインタ。
                                バッファのサイズはWM_SYSTEM_BUF_SIZEだけ必要。
                callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                dmaNo       -   WMが使用するDMA番号。
                miscFlags   -   初期化時のフラグ

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_InitializeEx(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo, u32 miscFlags)
{
    WMErrCode result;

    // 初期化を実行
    result = WM_Init(wmSysBuf, dmaNo);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_INITIALIZE, callback);

    // 通信中のスリープを禁止するためのコールバックを登録
    WMi_RegisterSleepCallback();

    // ARM7にFIFOで通知
    {
        WMArm9Buf *p = WMi_GetSystemWork();

        result = WMi_SendCommand(WM_APIID_INITIALIZE, 4,
                                 (u32)(p->WM7), (u32)(p->status), (u32)(p->fifo7to9), miscFlags);
        WM_CHECK_RESULT(result);
    }

    // 正常終了
    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Reset

  Description:  無線ライブラリをリセットし、初期化直後の状態に戻す。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_Reset(WMCallbackFunc callback)
{
    WMErrCode result;

    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_RESET, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_RESET, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_End

  Description:  無線ライブラリを終了する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_End(WMCallbackFunc callback)
{
    WMErrCode result;

    // IDLEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_END, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_END, 0);
    WM_CHECK_RESULT(result);

    // ARM9側WMライブラリの終了処理はコールバック内で行う

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetParentParameter

  Description:  親機情報を設定する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                pparaBuf    -   親機情報を示す構造体へのポインタ。
                                pparaBuf及びpparaBuf->userGameInfoの実体は強制的に
                                キャッシュストアされる点に注意。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetParentParameter(WMCallbackFunc callback, const WMParentParam *pparaBuf)
{
    WMErrCode result;

    // IDLEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);
    
    #ifdef SDK_TWL // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // パラメータチェック
    if (pparaBuf == NULL)
    {
        WM_WARNING("Parameter \"pparaBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)pparaBuf & 0x01f)
    {
        // アラインチェックは警告のみでエラーにはしない
        WM_WARNING("Parameter \"pparaBuf\" is not 32-byte aligned.\n");
    }
    if (pparaBuf->userGameInfoLength > 0)
    {
        if (pparaBuf->userGameInfo == NULL)
        {
            WM_WARNING("Parameter \"pparaBuf->userGameInfo\" must not be NULL.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
        if ((u32)(pparaBuf->userGameInfo) & 0x01f)
        {
            // アラインチェックは警告のみでエラーにはしない
            WM_WARNING("Parameter \"pparaBuf->userGameInfo\" is not 32-byte aligned.\n");
        }
    }

    // 転送データ最大長をチェック
    if ((pparaBuf->parentMaxSize +
         (pparaBuf->KS_Flag ? WM_SIZE_KS_PARENT_DATA + WM_SIZE_MP_PARENT_PADDING : 0) >
         WM_SIZE_MP_DATA_MAX)
        || (pparaBuf->childMaxSize +
            (pparaBuf->KS_Flag ? WM_SIZE_KS_CHILD_DATA + WM_SIZE_MP_CHILD_PADDING : 0) >
            WM_SIZE_MP_DATA_MAX))
    {
        WM_WARNING("Transfer data size is over %d byte.\n", WM_SIZE_MP_DATA_MAX);
        return WM_ERRCODE_INVALID_PARAM;
    }
    (void)WmCheckParentParameter(pparaBuf);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_P_PARAM, callback);

    // 指定バッファのキャッシュを書き出し
    DC_StoreRange((void *)pparaBuf, WM_PARENT_PARAM_SIZE);
    if (pparaBuf->userGameInfoLength > 0)
    {
        DC_StoreRange(pparaBuf->userGameInfo, pparaBuf->userGameInfoLength);
    }

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_P_PARAM, 1, (u32)pparaBuf);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WmCheckParentParameter

  Description:  親機設定パラメータがガイドラインに示される許容範囲内かどうか
                をチェックするデバッグ用関数。

  Arguments:    param   -   チェックする親機設定パラメータへのポインタ。

  Returns:      BOOL    -   問題ない場合はTRUEを、許容できない設定値の場合は
                            FALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL WmCheckParentParameter(const WMParentParam *param)
{
    // ユーザーゲーム情報は 112 バイトまで
    if (param->userGameInfoLength > WM_SIZE_USER_GAMEINFO)
    {
        OS_TWarning("User gameInfo length must be less than %d .\n", WM_SIZE_USER_GAMEINFO);
        return FALSE;
    }
    // ビーコン送信間隔は 10 ～ 1000
    if ((param->beaconPeriod < 10) || (param->beaconPeriod > 1000))
    {
        OS_TWarning("Beacon send period must be between 10 and 1000 .\n");
        return FALSE;
    }
    // 接続チャンネルは 1 ～ 14
    if ((param->channel < 1) || (param->channel > 14))
    {
        OS_TWarning("Channel must be between 1 and 14 .\n");
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_StartParentEx

  Description:  親機として通信を開始する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                powerSave   -   省電力モードを使用する場合はTRUE、しない場合はFALSE。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_StartParentEx(WMCallbackFunc callback, BOOL powerSave)
{
    WMErrCode result;

    // IDLEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    {
        WMArm9Buf *w9b = WMi_GetSystemWork();
#ifdef WM_DEBUG
        if (w9b->connectedAidBitmap != 0)
        {
            WM_DPRINTF("Warning: connectedAidBitmap should be 0, but %04x",
                       w9b->connectedAidBitmap);
        }
#endif
        w9b->myAid = 0;
        w9b->connectedAidBitmap = 0;
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_START_PARENT, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_START_PARENT, 1, (u32)powerSave);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartParent

  Description:  親機として通信を開始する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartParent(WMCallbackFunc callback)
{
    return WMi_StartParentEx(callback, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         WM_EndParent

  Description:  親機としての通信を停止する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndParent(WMCallbackFunc callback)
{
    WMErrCode result;

    // PARENTステート以外では実行不可
    result = WMi_CheckState(WM_STATE_PARENT);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_END_PARENT, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_END_PARENT, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartScan

  Description:  子機として親機のスキャンを開始する。
                一度の呼び出しで一台の親機情報を取得する。
                WM_EndScanを呼び出さずに連続して呼び出しても良い。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                param       -   スキャン情報を示す構造体へのポインタ。
                                スキャン結果の情報はparam->scanBufにARM7が直接書き出す
                                ので、キャッシュラインに合わせておく必要がある。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartScan(WMCallbackFunc callback, const WMScanParam *param)
{
    WMErrCode result;

    // IDLE CALSS1 SCAN ステート以外では実行不可
    result = WMi_CheckStateEx(3, WM_STATE_IDLE, WM_STATE_CLASS1, WM_STATE_SCAN);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // パラメータチェック
    if (param == NULL)
    {
        WM_WARNING("Parameter \"param\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanBuf == NULL)
    {
        WM_WARNING("Parameter \"param->scanBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((param->channel < 1) || (param->channel > 14))
    {
        WM_WARNING("Parameter \"param->channel\" must be between 1 and 14.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)(param->scanBuf) & 0x01f)
    {
        // アラインチェックは警告のみでエラーにはしない
        WM_WARNING("Parameter \"param->scanBuf\" is not 32-byte aligned.\n");
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_START_SCAN, callback);

    // ARM7にFIFOで通知
    {
        WMStartScanReq Req;

        Req.apiid = WM_APIID_START_SCAN;
        Req.channel = param->channel;
        Req.scanBuf = param->scanBuf;
        Req.maxChannelTime = param->maxChannelTime;
        Req.bssid[0] = param->bssid[0];
        Req.bssid[1] = param->bssid[1];
        Req.bssid[2] = param->bssid[2];
        Req.bssid[3] = param->bssid[3];
        Req.bssid[4] = param->bssid[4];
        Req.bssid[5] = param->bssid[5];
        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartScanEx

  Description:  子機として親機のスキャンを開始する。
                一度の呼び出しで複数台の親機情報を取得する。
                WM_EndScanを呼び出さずに連続して呼び出しても良い。

  Arguments:    callback - 非同期処理が完了した時に呼び出されるコールバック関数。
                param    - スキャン情報を示す構造体へのポインタ。
                           スキャン結果の情報はparam->scanBufにARM7が直接書き出す
                           ので、キャッシュラインに合わせておく必要がある。

  Returns:      int      - WM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartScanEx(WMCallbackFunc callback, const WMScanExParam *param)
{
    WMErrCode result;

    // IDLE CALSS1 SCAN ステート以外では実行不可
    result = WMi_CheckStateEx(3, WM_STATE_IDLE, WM_STATE_CLASS1, WM_STATE_SCAN);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // パラメータチェック
    if (param == NULL)
    {
        WM_WARNING("Parameter \"param\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanBuf == NULL)
    {
        WM_WARNING("Parameter \"param->scanBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanBufSize > WM_SIZE_SCAN_EX_BUF)
    {
        WM_WARNING
            ("Parameter \"param->scanBufSize\" must be less than or equal to WM_SIZE_SCAN_EX_BUF.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)(param->scanBuf) & 0x01f)
    {
        // アラインチェックは警告のみでエラーにはしない
        WM_WARNING("Parameter \"param->scanBuf\" is not 32-byte aligned.\n");
    }
    if (param->ssidLength > WM_SIZE_SSID)
    {
        WM_WARNING("Parameter \"param->ssidLength\" must be less than or equal to WM_SIZE_SSID.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanType != WM_SCANTYPE_ACTIVE && param->scanType != WM_SCANTYPE_PASSIVE
        && param->scanType != WM_SCANTYPE_ACTIVE_CUSTOM
        && param->scanType != WM_SCANTYPE_PASSIVE_CUSTOM)
    {
        WM_WARNING
            ("Parameter \"param->scanType\" must be WM_SCANTYPE_PASSIVE or WM_SCANTYPE_ACTIVE.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((param->scanType == WM_SCANTYPE_ACTIVE_CUSTOM
         || param->scanType == WM_SCANTYPE_PASSIVE_CUSTOM) && param->ssidMatchLength > WM_SIZE_SSID)
    {
        WM_WARNING
            ("Parameter \"param->ssidMatchLength\" must be less than or equal to WM_SIZE_SSID.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_START_SCAN_EX, callback);

    // ARM7にFIFOで通知
    {
        WMStartScanExReq Req;

        Req.apiid = WM_APIID_START_SCAN_EX;
        Req.channelList = param->channelList;
        Req.scanBuf = param->scanBuf;
        Req.scanBufSize = param->scanBufSize;
        Req.maxChannelTime = param->maxChannelTime;
        MI_CpuCopy8(param->bssid, Req.bssid, WM_SIZE_MACADDR);
        Req.scanType = param->scanType;
        Req.ssidMatchLength = param->ssidMatchLength;
        Req.ssidLength = param->ssidLength;
        MI_CpuCopy8(param->ssid, Req.ssid, WM_SIZE_SSID);

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_EndScan

  Description:  子機としてのスキャン処理を停止する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndScan(WMCallbackFunc callback)
{
    WMErrCode result;

    // SCANステート以外では実行不可
    result = WMi_CheckState(WM_STATE_SCAN);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_END_SCAN, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_END_SCAN, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartConnectEx

  Description:  子機として親機への接続を開始する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                pInfo       -   接続する親機の情報。
                                WM_StartScanにて取得した構造体を指定する。
                                この構造体の実体は強制的にキャッシュストア
                                される点に注意。
                ssid        -   親機に通知する子機情報(24Byte(WM_SIZE_CHILD_SSID)固定サイズ)
                powerSave   -   省電力モードを使用する場合はTRUE、しない場合はFALSE。
                authMode    -   Authenticationのモード選択。
                                  WM_AUTHMODE_OPEN_SYSTEM : OPEN SYSTEMモード
                                  WM_AUTHMODE_SHARED_KEY  : SHARED KEY モード

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode
WM_StartConnectEx(WMCallbackFunc callback, const WMBssDesc *pInfo, const u8 *ssid,
                  BOOL powerSave, const u16 authMode)
{
    WMErrCode result;

    // IDLE ステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // パラメータチェック
    if (pInfo == NULL)
    {
        WM_WARNING("Parameter \"pInfo\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)pInfo & 0x01f)
    {
        // アラインチェックは警告のみでエラーにはしない
        WM_WARNING("Parameter \"pInfo\" is not 32-byte aligned.\n");
    }
    if ((authMode != WM_AUTHMODE_OPEN_SYSTEM) && (authMode != WM_AUTHMODE_SHARED_KEY))
    {
        WM_WARNING
            ("Parameter \"authMode\" must be WM_AUTHMODE_OPEN_SYSTEM or WM_AUTHMODE_SHARED_KEY.\n");
    }

    // 指定バッファのキャッシュを書き出し
    DC_StoreRange((void *)pInfo, (u32)(pInfo->length * 2));

    {
        WMArm9Buf *w9b = WMi_GetSystemWork();
#ifdef WM_DEBUG
        if (w9b->connectedAidBitmap != 0)
        {
            WM_DPRINTF("Warning: connectedAidBitmap should be 0, but %04x",
                       w9b->connectedAidBitmap);
        }
#endif
        w9b->myAid = 0;
        w9b->connectedAidBitmap = 0;
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_START_CONNECT, callback);

    // ARM7にFIFOで通知
    {
        WMStartConnectReq Req;

        Req.apiid = WM_APIID_START_CONNECT;
        Req.pInfo = (WMBssDesc *)pInfo;
        if (ssid != NULL)
        {
            MI_CpuCopy8(ssid, Req.ssid, WM_SIZE_CHILD_SSID);
        }
        else
        {
            MI_CpuClear8(Req.ssid, WM_SIZE_CHILD_SSID);
        }
        Req.powerSave = powerSave;
        Req.authMode = authMode;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Disconnect

  Description:  確立されている接続を切断する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                aid         -   切断する通信相手のAID。
                                親機の場合、IDが1～15の子機を個別に切断。
                                子機の場合、IDが0の親機との通信を終了。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_Disconnect(WMCallbackFunc callback, u16 aid)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // ステートチェック
    result = WMi_CheckStateEx(5,
                              WM_STATE_PARENT, WM_STATE_MP_PARENT,
                              WM_STATE_CHILD, WM_STATE_MP_CHILD, WM_STATE_DCF_CHILD);
    WM_CHECK_RESULT(result);

    if (                               // p->status->stateのキャッシュは破棄済み
           (p->status->state == WM_STATE_PARENT) || (p->status->state == WM_STATE_MP_PARENT))
    {
        // 親機の場合
        if ((aid < 1) || (aid > WM_NUM_MAX_CHILD))
        {
            WM_WARNING("Parameter \"aid\" must be between 1 and %d.\n", WM_NUM_MAX_CHILD);
            return WM_ERRCODE_INVALID_PARAM;
        }
        DC_InvalidateRange(&(p->status->child_bitmap), 2);
        if (!(p->status->child_bitmap & (0x0001 << aid)))
        {
            WM_WARNING("There is no child that have aid %d.\n", aid);
            return WM_ERRCODE_NO_CHILD;
        }
    }
    else
    {
        // 子機の場合
        if (aid != 0)
        {
            WM_WARNING("Now child mode , so aid must be 0.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_DISCONNECT, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_DISCONNECT, 1, (u32)(0x0001 << aid));
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_DisconnectChildren

  Description:  接続確立されている子機をそれぞれ切断する。親機専用関数。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                aidBitmap   -   切断する子機のAIDビットフィールド。
                                最下位ビットは無視され、bit 1～15 がAID 1～15 の子機
                                をそれぞれ示す。
                                接続されていない子機を示すビットは無視されるので、
                                接続状況に関わらず全子機を切断する場合は0xFFFFを指定。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_DisconnectChildren(WMCallbackFunc callback, u16 aidBitmap)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // ステートチェック
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // パラメータチェック
    DC_InvalidateRange(&(p->status->child_bitmap), 2);
    if (!(p->status->child_bitmap & aidBitmap & 0xfffe))
    {
        WM_WARNING("There is no child that is included in \"aidBitmap\" %04x_.\n", aidBitmap);
        return WM_ERRCODE_NO_CHILD;
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_DISCONNECT, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_DISCONNECT, 1, (u32)aidBitmap);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/

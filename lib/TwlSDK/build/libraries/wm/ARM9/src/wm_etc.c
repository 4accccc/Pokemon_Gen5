/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_etc.c

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

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     WM_SIZE_TEMP_USR_GAME_INFO_BUF 128


/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u32 tmpUserGameInfoBuf[WM_SIZE_TEMP_USR_GAME_INFO_BUF / sizeof(u32)] ATTRIBUTE_ALIGN(32);


#ifdef  WM_ENABLE_TESTMODE
/*---------------------------------------------------------------------------*
  Name:         WM_StartTestMode

  Description:  テストモードでの通信を開始する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                signal      -   0:無変調(data=0), 1:無変調(data=1), 2:PN15段,
                                3:01パターン(スクランブルあり), 4:01パターン(スクランブルなし)
                rate        -   1:1Mbps, 2:2Mbps
                channel     -   データを送信するチャンネルを指定(1～14)。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartTestMode(WMCallbackFunc callback, u16 signal, u16 rate, u16 channel)
{
    WMErrCode result;

    // IDLEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_START_TESTMODE, callback);

    // ARM7にFIFOで通知
    {
        WMStartTestModeReq Req;

        Req.apiid = WM_APIID_START_TESTMODE;
        Req.signal = signal;
        Req.rate = rate;
        Req.channel = channel;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        if (result != WM_ERRCODE_SUCCESS)
        {
            return result;
        }
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StopTestMode

  Description:  テストモードでの通信を停止する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_StopTestMode(WMCallbackFunc callback)
{
    WMErrCode result;

    // TESTMODEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_TESTMODE);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_STOP_TESTMODE, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_STOP_TESTMODE, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartTestRxMode

  Description:  テストモードでの受信を開始する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                channel     -   データを受信するチャンネルを指定(1～14)。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartTestRxMode(WMCallbackFunc callback, u16 channel)
{
    WMErrCode result;

    // IDLEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_START_TESTRXMODE, callback);

    // ARM7にFIFOで通知
    {
        WMStartTestRxModeReq Req;

        Req.apiid = WM_APIID_START_TESTRXMODE;
        Req.channel = channel;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        if (result != WM_ERRCODE_SUCCESS)
        {
            return result;
        }
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StopTestRxMode

  Description:  テストモードでの受信を停止する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_StopTestRxMode(WMCallbackFunc callback)
{
    WMErrCode result;

    // TESTMODEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_TESTMODE_RX);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_STOP_TESTRXMODE, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_STOP_TESTRXMODE, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         WM_SetWEPKey

  Description:  暗号機能、暗号キーを設定する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                wepmode     -   0: 暗号機能なし
                                1: RC4(40bit)暗号モード
                                2: RC4(104bit)暗号モード
                                3: RC4(128bit)暗号モード
                wepkey      -   暗号キーデータ( 80バイト )へのポインタ。
                                キーデータは4つのデータで構成され、それぞれ20バイト。
                                各20バイトのうち、
                                 40ビットモードでは  5 バイト
                                104ビットモードでは 13 バイト
                                128ビットモードでは 16 バイト
                                のデータが使用される。
                                また、このデータの実体は強制的にキャッシュストアされる。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetWEPKey(WMCallbackFunc callback, u16 wepmode, const u16 *wepkey)
{
    WMErrCode result;

    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // パラメータチェック
    if (wepmode > 3)
    {
        WM_WARNING("Parameter \"wepmode\" must be less than %d.\n", 3);
        return WM_ERRCODE_INVALID_PARAM;
    }

    if (wepmode != WM_WEPMODE_NO)
    {
        // パラメータチェック
        if (wepkey == NULL)
        {
            WM_WARNING("Parameter \"wepkey\" must not be NULL.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
        if ((u32)wepkey & 0x01f)
        {
            // アラインチェックは警告のみでエラーにはしない
            WM_WARNING("Parameter \"wepkey\" is not 32-byte aligned.\n");
        }

        // 指定バッファのキャッシュを書き出し
        DC_StoreRange((void *)wepkey, WM_SIZE_WEPKEY);
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_WEPKEY, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_WEPKEY, 2, (u32)wepmode, (u32)wepkey);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetWEPKeyEx

  Description:  暗号機能、暗号キー、暗号キーIDを設定する。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                wepmode     -   0: 暗号機能なし
                                1: RC4(40bit)暗号モード
                                2: RC4(104bit)暗号モード
                                3: RC4(128bit)暗号モード
                wepkeyid    -   4つ指定したwepkeyのどれを使用するかを選択します。
                                0～3で指定します。
                wepkey      -   暗号キーデータ( 80バイト )へのポインタ。
                                キーデータは4つのデータで構成され、それぞれ20バイト。
                                各20バイトのうち、
                                 40ビットモードでは  5 バイト
                                104ビットモードでは 13 バイト
                                128ビットモードでは 16 バイト
                                のデータが使用される。
                                また、このデータの実体は強制的にキャッシュストアされる。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetWEPKeyEx(WMCallbackFunc callback, u16 wepmode, u16 wepkeyid, const u8 *wepkey)
{
    WMErrCode result;

    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // パラメータチェック
    if (wepmode > 3)
    {
        WM_WARNING("Parameter \"wepmode\" must be less than %d.\n", 3);
        return WM_ERRCODE_INVALID_PARAM;
    }

    if (wepmode != WM_WEPMODE_NO)
    {
        // パラメータチェック
        if (wepkey == NULL)
        {
            WM_WARNING("Parameter \"wepkey\" must not be NULL.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
        if (wepkeyid > 3)
        {
            WM_WARNING("Parameter \"wepkeyid\" must be less than %d.\n", 3);
        }
        if ((u32)wepkey & 0x01f)
        {
            // アラインチェックは警告のみでエラーにはしない
            WM_WARNING("Parameter \"wepkey\" is not 32-byte aligned.\n");
        }

        // 指定バッファのキャッシュを書き出し
        DC_StoreRange((void *)wepkey, WM_SIZE_WEPKEY);
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_WEPKEY_EX, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_WEPKEY_EX, 3, (u32)wepmode, (u32)wepkey, (u32)wepkeyid);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetGameInfo

  Description:  ゲーム情報を設定する。WM_SetParentParameterにより初期値はセット
                されるので、これを変更する場合に用いる。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                userGameInfo     - ユーザーゲーム情報へのポインタ。
                userGameInfoSize - ユーザーゲーム情報のサイズ。
                ggid        -   ゲームグループID
                tgid        -   テンポラリグループID
                attr        -   フラグ群。以下のフラグの論理和を設定。
                                    WM_ATTR_FLAG_ENTRY - エントリー許可
                                    WM_ATTR_FLAG_MB    - マルチブート受け付け
                                    WM_ATTR_FLAG_KS    - キーシェアリング
                                    WM_ATTR_FLAG_CS    - 連続転送モード
  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode
WM_SetGameInfo(WMCallbackFunc callback, const u16 *userGameInfo, u16 userGameInfoSize,
               u32 ggid, u16 tgid, u8 attr)
{
    WMErrCode result;

    // 親機として活動中であることを確認
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // パラメータチェック
    if (userGameInfo == NULL)
    {
        WM_WARNING("Parameter \"userGameInfo\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (userGameInfoSize > WM_SIZE_USER_GAMEINFO)
    {
        WM_WARNING("Parameter \"userGameInfoSize\" must be less than %d.\n", WM_SIZE_USER_GAMEINFO);
        return WM_ERRCODE_INVALID_PARAM;
    }

    // 指定のバッファを一旦退避
    MI_CpuCopy16((void *)userGameInfo, (void *)tmpUserGameInfoBuf, userGameInfoSize);
    DC_StoreRange((void *)tmpUserGameInfoBuf, userGameInfoSize);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_GAMEINFO, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_GAMEINFO, 5,
                             (u32)tmpUserGameInfoBuf,
                             (u32)userGameInfoSize, (u32)ggid, (u32)tgid, (u32)attr);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return result;
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetBeaconIndication

  Description:  ビーコン送受信indicateの有効/無効を切り替える。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                flag        -   0: 無効
                                1: 有効

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetBeaconIndication(WMCallbackFunc callback, u16 flag)
{
    WMErrCode result;

    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // パラメータチェック
    if ((0 != flag) && (1 != flag))
    {
        WM_WARNING("Parameter \"flag\" must be \"0\" of \"1\".\n");
        return WM_ERRCODE_INVALID_PARAM;
    }

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_BEACON_IND, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_BEACON_IND, 1, (u32)flag);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetLifeTime

  Description:  ライフタイムを設定する

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                tableNumber -   ライフタイムを設定するCAMテーブル番号(0xFFFFですべてのテーブル)
                camLifeTime -   CAMのライフタイム(100ms単位：0xFFFFで無効)
                frameLifeTime - 設定するフレームのライフタイムのビーコン間隔(100ms単位：0xFFFFで無効)
                mpLifeTime  -   MP 通信のライフタイム(100ms単位：0xFFFFで無効)

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode
WM_SetLifeTime(WMCallbackFunc callback, u16 tableNumber, u16 camLifeTime, u16 frameLifeTime,
               u16 mpLifeTime)
{
    WMErrCode result;

    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_LIFETIME, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_LIFETIME, 4,
                             (u32)tableNumber,
                             (u32)camLifeTime, (u32)frameLifeTime, (u32)mpLifeTime);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_MeasureChannel

  Description:  チャンネルの使用状況を測定する

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                ccaMode     -   CCA動作モード
                                0: キャリアセンスのみ。ED閾値は無視。
                                1: ED閾値のみ有効。
                                2: キャリアセンスとED閾値の論理積。
                                3: キャリアセンスとED閾値の論理和。
                EDThreshold -   ED閾値(0～61) -60dBm ～ -80dBm
                channel     -   調査するチャンネル(1回のMeasureChannelで1つのチャンネルのみ)
                measureTime -   調査する時間

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode
WM_MeasureChannel(WMCallbackFunc callback, u16 ccaMode, u16 edThreshold, u16 channel,
                  u16 measureTime)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // IDLEステート以外では実行不可
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_MEASURE_CHANNEL, callback);

    // ARM7にFIFOで通知
    {
        WMMeasureChannelReq Req;

        Req.apiid = WM_APIID_MEASURE_CHANNEL;
        Req.ccaMode = ccaMode;
        Req.edThreshold = edThreshold;
        Req.channel = channel;
        Req.measureTime = measureTime;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }
    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_InitWirelessCounter

  Description:  WirelessCounterを初期化する

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_InitWirelessCounter(WMCallbackFunc callback)
{
    WMErrCode result;

    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_INIT_W_COUNTER, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_INIT_W_COUNTER, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetWirelessCounter

  Description:  Wireless NICの送受信フレーム数、送受信エラーフレーム数を取得する

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_GetWirelessCounter(WMCallbackFunc callback)
{
    WMErrCode result;

    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_GET_W_COUNTER, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_GET_W_COUNTER, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetEntry

  Description:  親機として、子機からの接続受け付け可否を切り替える。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                enabled     -   エントリー許可/不許可フラグ。TRUE:許可、FALSE:不許可。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetEntry(WMCallbackFunc callback, BOOL enabled)
{
    WMErrCode result;

    // 親機以外では実行不可
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_ENTRY, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_ENTRY, 1, (u32)enabled);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SetBeaconPeriod

  Description:  Beaconの間隔を変更する

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                beaconPeriod - Beacon間隔(10～1000 TU(1024μs))

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_SetBeaconPeriod(WMCallbackFunc callback, u16 beaconPeriod)
{
    WMErrCode result;

    // 親機以外では実行不可
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_BEACON_PERIOD, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_BEACON_PERIOD, 1, (u32)beaconPeriod);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetPowerSaveMode

  Description:  PowerSaveModeを変更する

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                powerSave   -   省電力モードを使用する場合はTRUE、しない場合はFALSE。

  Returns:      WMErrCode   -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetPowerSaveMode(WMCallbackFunc callback, BOOL powerSave)
{
    WMErrCode result;

    // DCF子機ステート以外では実行不可
    result = WMi_CheckState(WM_STATE_DCF_CHILD);
    WM_CHECK_RESULT(result);

    // コールバック関数を登録
    WMi_SetCallbackTable(WM_APIID_SET_PS_MODE, callback);

    // ARM7にFIFOで通知
    result = WMi_SendCommand(WM_APIID_SET_PS_MODE, 1, (u32)powerSave);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/

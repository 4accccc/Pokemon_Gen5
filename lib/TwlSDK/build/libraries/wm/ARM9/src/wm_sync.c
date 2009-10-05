/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_sync.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"


/*---------------------------------------------------------------------------*
  Name:         WM_SetIndCallback

  Description:  WM7からの状況通知に対して呼びだされる関数を設定する。

  Arguments:    callback    -   無線ファームウェアからの状況通知時に呼び出される
                                コールバック関数を指定。

  Returns:      WMErrCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetIndCallback(WMCallbackFunc callback)
{
    WMErrCode result;
    OSIntrMode e;

    // 割込み禁止
    e = OS_DisableInterrupts();

    // 初期化済み確認
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        // 割込み禁止戻し
        (void)OS_RestoreInterrupts(e);
        return result;
    }

    // Indication用コールバックを設定
    WMi_GetSystemWork()->indCallback = callback;
    // 割込み禁止戻し
    (void)OS_RestoreInterrupts(e);

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetPortCallback

  Description:  WM7からの通信フレーム受信通知に対して呼びだされる関数を設定する。

  Arguments:    port        -   ポート番号。
                callback    -   受信通知時に呼び出されるコールバック関数。
                arg         -   コールバック関数に WMPortRecvCallback.arg として
                                渡される引数。

  Returns:      WMErrCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetPortCallback(u16 port, WMCallbackFunc callback, void *arg)
{
    WMErrCode result;
    OSIntrMode e;
    WMPortRecvCallback cb_Port;

    // パラメータチェック
#ifdef SDK_DEBUG
    if (port >= WM_NUM_OF_PORT)
    {
        WM_WARNING("Parameter \"port\" must be less than 16.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
#endif

    if (callback != NULL)
    {
        MI_CpuClear8(&cb_Port, sizeof(WMPortRecvCallback));
        cb_Port.apiid = WM_APIID_PORT_RECV;
        cb_Port.errcode = WM_ERRCODE_SUCCESS;
        cb_Port.state = WM_STATECODE_PORT_INIT;
        cb_Port.port = port;
        cb_Port.recvBuf = NULL;
        cb_Port.data = NULL;
        cb_Port.length = 0;
        cb_Port.seqNo = 0xffff;
        cb_Port.arg = arg;
        cb_Port.aid = 0;
        OS_GetMacAddress(cb_Port.macAddress);
    }

    // 割込み禁止
    e = OS_DisableInterrupts();
    // 初期化済み確認
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        // 割込み禁止戻し
        (void)OS_RestoreInterrupts(e);
        return result;
    }

    // データ受信用コールバックをセット
    {
        WMArm9Buf *p = WMi_GetSystemWork();

        p->portCallbackTable[port] = callback;
        p->portCallbackArgument[port] = arg;
    }

    if (callback != NULL)
    {
        cb_Port.connectedAidBitmap = WM_GetConnectedAIDs();
        cb_Port.myAid = WM_GetAID();
        (*callback) ((void *)&cb_Port);
    }

    // 割込み禁止戻し
    (void)OS_RestoreInterrupts(e);

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_ReadStatus

  Description:  無線ライブラリ状態を示す構造体を取得する。

  Arguments:    statusBuf   -   状態を取得する変数へのポインタ。

  Returns:      WMErrCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WM_ReadStatus(WMStatus *statusBuf)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // 初期化済み確認
    result = WMi_CheckInitialized();
    WM_CHECK_RESULT(result);

    // パラメータチェック
    if (statusBuf == NULL)
    {
        WM_WARNING("Parameter \"statusBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }

    // WMStatus構造体をCPUコピー
    DC_InvalidateRange(p->status, sizeof(WMStatus));
    MI_CpuCopyFast(p->status, statusBuf, sizeof(WMStatus));

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetMPSendBufferSize

  Description:  MP 通信の送信バッファのサイズを計算する。
                StartParent, StartConnect が終わっている必要がある。

  Arguments:    None.

  Returns:      int     -   WM_StartMP に渡す必要がある送信バッファのサイズ。
 *---------------------------------------------------------------------------*/
int WM_GetMPSendBufferSize(void)
{
    WMErrCode result;
    int     maxSendSize;
    WMArm9Buf *p = WMi_GetSystemWork();

    // ステートチェック
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_CHILD);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }

    // MP状態を確認
    DC_InvalidateRange(&(p->status->mp_flag), 4);       // ARM7ステータス領域のキャッシュを無効化
    if (p->status->mp_flag == TRUE)
    {
        WM_WARNING("Already started MP protocol. So can't execute request.\n");
        return 0;
    }

    // 計算に必要な情報をWMStatus構造体から参照
    DC_InvalidateRange(&(p->status->mp_maxSendSize), 4);
    maxSendSize = p->status->mp_maxSendSize;

    return ((maxSendSize + 31) & ~0x1f);
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetMPReceiveBufferSize

  Description:  MP 通信の受信バッファのサイズを計算する。
                StartParent, StartConnect が終わっている必要がある。

  Arguments:    None.

  Returns:      int     -   WM_StartMP に渡す必要がある受信バッファのサイズ。
 *---------------------------------------------------------------------------*/
int WM_GetMPReceiveBufferSize(void)
{
    WMErrCode result;
    BOOL    isParent;
    int     maxReceiveSize;
    int     maxEntry;
    WMArm9Buf *p = WMi_GetSystemWork();

    // ステートチェック
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_CHILD);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }

    // MP状態を確認
    DC_InvalidateRange(&(p->status->mp_flag), 4);
    if (p->status->mp_flag == TRUE)
    {
        WM_WARNING("Already started MP protocol. So can't execute request.\n");
        return 0;
    }

    // 計算に必要な情報をWMStatus構造体から参照
    DC_InvalidateRange(&(p->status->aid), 2);
    isParent = (p->status->aid == 0) ? TRUE : FALSE;
    DC_InvalidateRange(&(p->status->mp_maxRecvSize), 2);
    maxReceiveSize = p->status->mp_maxRecvSize;
    if (isParent == TRUE)
    {
        DC_InvalidateRange(&(p->status->pparam.maxEntry), 2);
        maxEntry = p->status->pparam.maxEntry;
        return (int)((sizeof(WMmpRecvHeader) - sizeof(WMmpRecvData) +
                      ((sizeof(WMmpRecvData) + maxReceiveSize - 2 + 2 /*MACBUG*/) * maxEntry)
                      + 31) & ~0x1f) * 2;
    }
    else
    {
        return (int)((sizeof(WMMpRecvBuf) + maxReceiveSize - 4 + 31) & ~0x1f) * 2;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WM_ReadMPData

  Description:  受信したデータ全体から指定子機のデータだけを抽出する。

  Arguments:    header      -   受信データ全体を示すポインタ。
                aid         -   データを抽出する子機のAID。

  Returns:      WMMpRecvData* - 該当子機からの受信データへのポインタを返す。
                                抽出に失敗した場合、NULLを返す。
 *---------------------------------------------------------------------------*/
WMMpRecvData *WM_ReadMPData(const WMMpRecvHeader *header, u16 aid)
{
    int     i;
    WMErrCode result;
    WMMpRecvData *recvdata_p[WM_NUM_MAX_CHILD]; // 各子機データの開始位置ポインタ配列(15台分)
    WMArm9Buf *p = WMi_GetSystemWork();

    // 初期化済みを確認
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return NULL;
    }

    // パラメータチェック
    if ((aid < 1) || (aid > WM_NUM_MAX_CHILD))
    {
        WM_WARNING("Parameter \"aid\" must be between 1 and %d.\n", WM_NUM_MAX_CHILD);
        return NULL;
    }

    // そのaidの子機は存在するか？
    DC_InvalidateRange(&(p->status->child_bitmap), 2);
    if (0 == (p->status->child_bitmap & (0x0001 << aid)))
    {
        return NULL;
    }

    // 受信データが存在するか？ || count フィールドが異常値でないか？
    if (header->count == 0 || header->count > WM_NUM_MAX_CHILD)
    {
        return NULL;
    }

    // 各子機データの開始位置を計算
    recvdata_p[0] = (WMMpRecvData *)(header->data);

    i = 0;
    do
    {
        // 指定されたAIDの子機データならそのポインタを返す
        if (recvdata_p[i]->aid == aid)
        {
            return recvdata_p[i];
        }
        ++i;
        recvdata_p[i] = (WMMpRecvData *)((u32)(recvdata_p[i - 1]) + header->length);
    }
    while (i < header->count);

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetAllowedChannel

  Description:  通信時に使用を許可されたチャンネルを取得する。

  Arguments:    None.

  Returns:      u16 -   使用許可チャンネルのビットフィールドを返す。最下位ビットが
                        1チャンネル、最上位ビットが16チャンネルを示す。ビットが1の
                        チャンネルが使用許可、ビットが0のチャンネルは使用禁止。
                        通常は 1～13 チャンネルの内いくつかのビットが1になった値が
                        返される。0x0000が返された場合は使用を許可されたチャンネルが
                        存在しないため、無線機能そのものを使用禁止である。
                        また、未初期化時など関数に失敗した場合は0x8000が返される。
 *---------------------------------------------------------------------------*/
u16 WM_GetAllowedChannel(void)
{
#ifdef WM_PRECALC_ALLOWEDCHANNEL
    WMErrCode result;

    #ifdef SDK_TWL // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return 0;
        }
    }
    #endif
    
    // 初期化済みを確認
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return WM_GET_ALLOWED_CHANNEL_BEFORE_INIT;
    }
    return *((u16 *)((u32)(OS_GetSystemWork()->nvramUserInfo) +
                     ((sizeof(NVRAMConfig) + 3) & ~0x00000003) + 6));
#else
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();
    
    #ifdef SDK_TWL // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return 0;
        }
    }
    #endif
    
    // 無線ハード起動済みを確認
    result = WMi_CheckIdle();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return WM_GET_ALLOWED_CHANNEL_BEFORE_INIT;
    }

    DC_InvalidateRange(&(p->status->allowedChannel), 2);
    return p->status->allowedChannel;
#endif
}

#ifdef  WM_PRECALC_ALLOWEDCHANNEL
/*---------------------------------------------------------------------------*
  Name:         WM_IsExistAllowedChannel

  Description:  通信時に使用を許可されたチャンネルが存在するかどうか確認する。
                WMライブラリを未初期化であっても正常に判定可能。

  Arguments:    None.

  Returns:      BOOL    -   使用許可されたチャンネルが存在する場合にTRUEを、
                            存在しない場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL WM_IsExistAllowedChannel(void)
{
    u16     allowedChannel;

    allowedChannel = *((u16 *)((u32)(OS_GetSystemWork()->nvramUserInfo) +
                     ((sizeof(NVRAMConfig) + 3) & ~0x00000003) + 6));
    if (allowedChannel)
    {
        return TRUE;
    }
    return FALSE;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         WM_GetLinkLevel

  Description:  通信時のリンク強度を取得する。同期関数。

  Arguments:    None.

  Returns:      WMLinkLevel -   4段階に評価したリンク強度を返す。
 *---------------------------------------------------------------------------*/
WMLinkLevel WM_GetLinkLevel(void)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // 初期化済みを確認
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return WM_LINK_LEVEL_0;        // 最低電波強度
    }

    DC_InvalidateRange(&(p->status->state), 2);
    switch (p->status->state)
    {
    case WM_STATE_MP_PARENT:
        // 親機の場合
        DC_InvalidateRange(&(p->status->child_bitmap), 2);
        if (p->status->child_bitmap == 0)
        {
            // 子機がいない場合
            return WM_LINK_LEVEL_0;    // 最低電波強度
        }
    case WM_STATE_MP_CHILD:
    case WM_STATE_DCF_CHILD:
        // 子機の場合
        DC_InvalidateRange(&(p->status->linkLevel), 2);
        return (WMLinkLevel)(p->status->linkLevel);
    }

    // 未接続の場合
    return WM_LINK_LEVEL_0;            // 最低電波強度
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetDispersionBeaconPeriod

  Description:  親機として活動する際に設定すべきビーコン間隔値を取得する。

  Arguments:    None.

  Returns:      u16 -   設定すべきビーコン間隔値(ms)。
 *---------------------------------------------------------------------------*/
u16 WM_GetDispersionBeaconPeriod(void)
{
    u8      mac[6];
    u16     ret;
    s32     i;

    OS_GetMacAddress(mac);
    for (i = 0, ret = 0; i < 6; i++)
    {
        ret += mac[i];
    }
    ret += OS_GetVBlankCount();
    ret *= 7;
    return (u16)(WM_DEFAULT_BEACON_PERIOD + (ret % 20));
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetDispersionScanPeriod

  Description:  子機として親機を探索する際に設定すべき探索限度時間を取得する。

  Arguments:    None.

  Returns:      u16 -   設定すべき探索限度時間(ms)。
 *---------------------------------------------------------------------------*/
u16 WM_GetDispersionScanPeriod(void)
{
    u8      mac[6];
    u16     ret;
    s32     i;

    OS_GetMacAddress(mac);
    for (i = 0, ret = 0; i < 6; i++)
    {
        ret += mac[i];
    }
    ret += OS_GetVBlankCount();
    ret *= 13;
    return (u16)(WM_DEFAULT_SCAN_PERIOD + (ret % 10));
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetOtherElements

  Description:  ビーコン内の拡張エレメントを取得する。
                同期関数。

  Arguments:    bssDesc - 親機情報構造体。
                          WM_StartScan(Ex)にて取得した構造体を指定する。

  Returns:      WMOtherElements - 拡張エレメント構造体。
 *---------------------------------------------------------------------------*/
WMOtherElements WM_GetOtherElements(WMBssDesc *bssDesc)
{
    WMOtherElements elems;
    u8     *p_elem;
    int     i;
    u16      curr_elem_len;
    u16      elems_len;                 // elements全体の長さ
    u16      elems_len_counter;         // elementsの長さチェック用カウンタ

    // gameInfoが入っている場合は終了
    if (bssDesc->gameInfoLength != 0)
    {
        elems.count = 0;
        return elems;
    }

    // otherElementの数を取得し、0なら終了
    elems.count = (u8)(bssDesc->otherElementCount);
    if (elems.count == 0)
        return elems;

    // elemsの数をWM_SCAN_OTHER_ELEMENT_MAX個までに制限する
    if (elems.count > WM_SCAN_OTHER_ELEMENT_MAX)
        elems.count = WM_SCAN_OTHER_ELEMENT_MAX;

    // エレメントの先頭を、まずgameInfoに設定
    p_elem = (u8 *)&(bssDesc->gameInfo);

    // elements全体の長さを取得し、チェック用カウンタを初期化
    elems_len = (u16)((bssDesc->length * sizeof(u16)) - 64);
    elems_len_counter = 0;

    // elemsの数だけループする
    for (i = 0; i < elems.count; ++i)
    {
        elems.element[i].id = p_elem[0];
        elems.element[i].length = p_elem[1];
        elems.element[i].body = (u8 *)&(p_elem[2]);

        // 今回のエレメント長を計算し、チェックカウンタに加える
        curr_elem_len = (u16)(elems.element[i].length + 2);
        elems_len_counter += curr_elem_len;

        // OS_TPrintf("eles_len        =%d\n", elems_len);
        // OS_TPrintf("eles_len_counter=%d\n", elems_len_counter);

        // elements全体の長さを超えた場合はエラーとし、
        // elementは無かったものとして通知する
        if (elems_len_counter > elems_len)
        {
            WM_WARNING("Elements length error.\n");
            elems.count = 0;
            return elems;
        }

        // 次のエレメントの先頭アドレスを計算
        p_elem = (u8 *)(p_elem + curr_elem_len);
    }

    return elems;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetNextTgid

  Description:  自動生成された一意な TGID 値を取得する。
                同期関数。

  Arguments:    None.

  Returns:      初回呼び出し時は RTC をもとに生成された TGIDを、
                次回以降は前回の返り値に 1 だけ加算した値を返す。
 *---------------------------------------------------------------------------*/
u16 WM_GetNextTgid(void)
{
    enum
    { TGID_DEFAULT = (1 << 16) };
    static u32 tgid_bak = (u32)TGID_DEFAULT;
    /* 自機固有の時間における一意性を保つよう, 初回は RTC の時刻値を使用 */
    if (tgid_bak == (u32)TGID_DEFAULT)
    {
        RTCTime rt[1];
        RTC_Init();
        if (RTC_GetTime(rt) == RTC_RESULT_SUCCESS)
        {
            tgid_bak = (u16)(rt->second + (rt->minute << 8));
        }
        else
        {
            OS_TWarning("failed to get RTC-data to create unique TGID!\n");
        }
    }
    /* 一意な値を種にしつつ毎回インクリメントする */
    tgid_bak = (u16)(tgid_bak + 1);
    return (u16)tgid_bak;
}


/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/

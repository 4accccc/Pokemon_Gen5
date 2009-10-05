/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WXC - libraries -
  File:     wxc_api.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9661 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include <nitro/wxc.h>


/*****************************************************************************/
/* declaration */

/* WXCライブラリの全ワーク変数 */
typedef struct WXCWork
{
    u32     wm_dma;
    WXCCallback system_callback;
    WXCScheduler scheduler;
    WXCUserInfo user_info[WM_NUM_MAX_CHILD + 1];
    BOOL    stopping;
    u8      beacon_count;              /* ビーコン送信回数 */
    u8      padding[3];
    WMParentParam parent_param ATTRIBUTE_ALIGN(32);
    WXCDriverWork driver ATTRIBUTE_ALIGN(32);
    WXCProtocolContext protocol[1] ATTRIBUTE_ALIGN(32);
}
WXCWork;

SDK_STATIC_ASSERT(sizeof(WXCWork) <= WXC_WORK_SIZE);


/*****************************************************************************/
/* variable */

static WXCStateCode state = WXC_STATE_END;
static WXCWork *work = NULL;


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         WXCi_ChangeState

  Description:  内部状態移行と同時にコールバック発生.

  Arguments:    stat          移行する状態
                arg           コールバック引数 (状態によって変わる)
  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_ChangeState(WXCStateCode stat, void *arg)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        state = stat;
        if (work->system_callback)
        {
            (*work->system_callback) (state, arg);
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_SeekNextBlock

  Description:  次に起動すべきブロックを検索.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_SeekNextBlock(void)
{
    /* 次のブロックを選択. */
    WXCProtocolRegistry *target =
        WXC_FindNextBlock(work->protocol, work->protocol->current_block, 0, FALSE);
    if (!target)
    {
        /* 登録されたデータが無い場合は, 仕方ないので空のブロックを選択 */
        target = WXC_FindNextBlock(work->protocol, NULL, 0, TRUE);
    }
    // 念のため、プロトコルを全くインストールしていない場合を考慮。
    if (target != NULL)
    {
        WXC_SetCurrentBlock(work->protocol, target);
        work->parent_param.ggid = target->ggid;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_DriverEventCallback

  Description:  ワイヤレスドライバからのイベントコールバック

  Arguments:    event         通知するイベント
                arg           イベントごとに定義された引数

  Returns:      イベントごとに定義された u32 のイベント結果値.
 *---------------------------------------------------------------------------*/
static u32 WXCi_DriverEventCallback(WXCDriverEvent event, void *arg)
{
    u32     result = 0;

    switch (event)
    {
    case WXC_DRIVER_EVENT_BEACON_SEND:
        /*
         * ビーコン更新タイミング (引数は WMParentParam ポインタ)
         * 起動中ならビーコン状態を更新.
         */
        if (!work->stopping && (WXC_GetStateCode() == WXC_STATE_ACTIVE))
        {
            WXC_BeaconSendHook(work->protocol, &work->parent_param);
            /* しばらく無接続なら再起動する */
            if (work->driver.peer_bitmap == 0)
            {
                if (++work->beacon_count > WXC_PARENT_BEACON_SEND_COUNT_OUT)
                {
                    work->beacon_count = 0;
                    /* スケジューラを更新し, まだ親機モードなら継続 */
                    if (WXCi_UpdateScheduler(&work->scheduler))
                    {
                    }
                    /* 子機モードであれば状態変更 */
                    else
                    {
                        WXCi_StartChild(&work->driver);
                    }
                }
            }
        }
        break;

    case WXC_DRIVER_EVENT_BEACON_RECV:
        /*
         * ビーコン検出タイミング (引数は WMBssDesc ポインタ)
         * 接続対象とみなせば TRUE を, そうでなければ FALSE を返す.
         */
        {
            WXCBeaconFoundCallback cb;
            /* ビーコンの判断をまずプロトコルに行わせる */
            cb.bssdesc = (const WMBssDesc *)arg;
            cb.matched = WXC_BeaconRecvHook(work->protocol, cb.bssdesc);
            /* その結果をふまえつつユーザアプリでも判断できるようにする */
            (*work->system_callback) (WXC_STATE_BEACON_RECV, &cb);
            result = (u32)cb.matched;
            /* 結果として接続すべきならここで TRUE を返す */
            if (result)
            {
                work->parent_param.ggid = WXC_GetCurrentBlock(work->protocol)->ggid;
            }
        }
        break;

    case WXC_DRIVER_EVENT_STATE_END:
        /*
         * ワイヤレス終了 (引数はWXCに割り当てられたワークメモリ)
         */
        work->stopping = FALSE;
        WXCi_ChangeState(WXC_STATE_END, work);
        break;

    case WXC_DRIVER_EVENT_STATE_STOP:
        /*
         * STOP ステートに遷移完了. (引数は常に NULL)
         */
        work->stopping = FALSE;
        WXCi_ChangeState(WXC_STATE_READY, NULL);
        break;

    case WXC_DRIVER_EVENT_STATE_IDLE:
        /*
         * IDLE ステートに遷移完了. (引数は常に NULL)
         */
        /* 終了準備中ならここで無線終了 */
        if (WXC_GetStateCode() != WXC_STATE_ACTIVE)
        {
            WXCi_End(&work->driver);
        }
        /* 停止準備中ならここで無線停止 */
        else if (work->stopping)
        {
            WXCi_Stop(&work->driver);
        }
        /* そうでなければ親機・子機の切り替え */
        else if (WXCi_UpdateScheduler(&work->scheduler))
        {
            /* 交換用データが複数登録されている場合はここで GGID が切り替わる */
            WXCi_SeekNextBlock();
            /* GGID に応じた親機設定初期化をプロトコルに行わせる */
            WXC_BeaconSendHook(work->protocol, &work->parent_param);
            /* 親機開始 */
            WXCi_StartParent(&work->driver);
        }
        else
        {
            WXCi_StartChild(&work->driver);
        }
        break;

    case WXC_DRIVER_EVENT_STATE_PARENT:
    case WXC_DRIVER_EVENT_STATE_CHILD:
        /*
         * MP ステートに遷移完了. (引数は常に NULL)
         */
        /* 終了準備中ならここで無線終了 */
        if (WXC_GetStateCode() != WXC_STATE_ACTIVE)
        {
            WXCi_End(&work->driver);
        }
        /* 停止準備中ならここで無線停止 */
        else if (work->stopping)
        {
            WXCi_Stop(&work->driver);
        }
        work->beacon_count = 0;
        /* 自身のユーザ情報を設定 */
        {
            WXCUserInfo *p_user = &work->user_info[work->driver.own_aid];
            p_user->aid = work->driver.own_aid;
            OS_GetMacAddress(p_user->macAddress);
        }
        break;

    case WXC_DRIVER_EVENT_CONNECTING:
        /*
         * 接続前通知. (引数は WMBssDesc ポインタ)
         */
        {
            WMBssDesc * bss = (WMBssDesc *)arg;
            WXC_CallPreConnectHook(work->protocol, bss, &bss->ssid[8]);
        }
        break;

    case WXC_DRIVER_EVENT_CONNECTED:
        /*
         * 接続通知. (引数は WMStartParentCallback か WMStartConnectCallback)
         */
        {
            const WXCProtocolRegistry *block = WXC_GetCurrentBlock(work->protocol);
            WMStartParentCallback *cb = (WMStartParentCallback *)arg;
            const BOOL is_parent = WXC_IsParentMode();
            const u16 parent_size =
                (u16)(is_parent ? work->parent_param.
                      parentMaxSize : WXCi_GetParentBssDesc(&work->driver)->gameInfo.parentMaxSize);
            const u16 child_size =
                (u16)(is_parent ? work->parent_param.
                      childMaxSize : WXCi_GetParentBssDesc(&work->driver)->gameInfo.childMaxSize);
            const u16 aid = (u16)(is_parent ? cb->aid : 0);
            WXCUserInfo *p_user = &work->user_info[aid];
            const BOOL is_valid_block = (!work->stopping && (WXC_GetStateCode() == WXC_STATE_ACTIVE) && block);

            /* まず, 接続相手のユーザ情報をコピー */
            p_user->aid = aid;
            if (is_parent)
            {
                WM_CopyBssid16(cb->macAddress, p_user->macAddress);
            }
            else
            {
                const WMBssDesc *p_bss = WXCi_GetParentBssDesc(&work->driver);
                WM_CopyBssid16(p_bss->bssid, p_user->macAddress);
            }

            /* 次に, 現在のMP設定で impl を初期化 */
            WXC_ResetSequence(work->protocol, parent_size, child_size);

            /*
             * 終了中でなく, かつ
             * RegisterData が現在有効ならここで自動登録
             */
            if (is_valid_block)
            {
                /* 自動登録するデータがある場合のみ、データを登録 */
                if ((block->send.buffer != NULL) && (block->recv.buffer != NULL))
                {
                    WXC_AddBlockSequence(work->protocol,
                                         block->send.buffer, block->send.length,
                                         block->recv.buffer, block->recv.length);
                }
            }

            /* プロトコルへ接続を通知 */
            WXC_ConnectHook(work->protocol, (u16)(1 << aid));

            /* 必要ならユーザアプリへコールバック */
            if (is_valid_block)
            {
                (*work->system_callback) (WXC_STATE_CONNECTED, p_user);
            }

            /*
             * この時点で誰も AddData() していなければ,
             * impl は !IsExecuting() になるように進行する.
             */
        }
        break;

    case WXC_DRIVER_EVENT_DISCONNECTED:
        /*
         * 切断通知.
         * ・WM_Dissconect 完了時
         * ・WM_StartParent インジケート
         * ・PortRecvCallback
         * の3箇所で呼び出されており, 重複の可能性がある.
         */
        WXC_DisconnectHook(work->protocol, (u16)(u32)arg);
        /*
         * 完了コールバック内で WXC_End() されたり
         * 全てのデータが Unregister されている場合には
         * 切断のタイミングでリセットする.
         */
        if (!WXC_GetUserBitmap())
        {
            if ((WXC_GetStateCode() == WXC_STATE_ACTIVE) ||
                (WXC_GetStateCode() == WXC_STATE_ENDING) || !WXC_GetCurrentBlock(work->protocol))
            {
                work->beacon_count = 0;
                WXCi_Reset(&work->driver);
            }
        }
        break;

    case WXC_DRIVER_EVENT_DATA_SEND:
        /* MP パケット送信フック. (引数は WXCPacketInfo ポインタ) */
        WXC_PacketSendHook(work->protocol, (WXCPacketInfo *) arg);
        break;

    case WXC_DRIVER_EVENT_DATA_RECV:
        /* MP パケット受信フック. (引数は const WXCPacketInfo ポインタ) */
        (void)WXC_PacketRecvHook(work->protocol, (const WXCPacketInfo *)arg);
        /* すでにプロトコル的に切断している場合は Reset */
        if (!work->protocol->current_block->impl->IsExecuting(work->protocol))
        {
            WXCi_Reset(&work->driver);
        }
        break;

    default:
        OS_TPanic("unchecked event implementations!");
        break;
    }

    return result;
}

/*---------------------------------------------------------------------------*
    外部関数
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WXC_Init

  Description:  すれちがい通信初期化関数.
                内部状態を初期化する.

  Arguments:    work_mem      ライブラリ内部で使用するワークメモリ.
                              WXC_WORK_SIZE バイト以上で, かつ
                              32 BYTE アラインメントされている必要がある.
                callback      ライブラリの各種システム状態を通知する
                              コールバックへのポインタ.
                dma           ライブラリ内部で使用する DMA チャンネル.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_Init(void *work_mem, WXCCallback callback, u32 dma)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    // 初回呼び出し時に各種プロトコルをインストール。
    static BOOL initialized = FALSE;
    if (!initialized)
    {
        WXC_InstallProtocolImpl(WXCi_GetProtocolImplCommon());
        WXC_InstallProtocolImpl(WXCi_GetProtocolImplWPB());
        WXC_InstallProtocolImpl(WXCi_GetProtocolImplWXC());
        initialized = TRUE;
    }
    {
        if (WXC_GetStateCode() == WXC_STATE_END)
        {
            if (((u32)work_mem & 31) != 0)
            {
                OS_TPanic("WXC work buffer must be 32-byte aligned!");
            }
            work = (WXCWork *) work_mem;
            MI_CpuClear32(work, sizeof(WXCWork));
            work->wm_dma = dma;
            work->system_callback = callback;
            WXCi_InitScheduler(&work->scheduler);
            WXC_InitProtocol(work->protocol);
            /* 親機設定を初期化 */
            work->parent_param.maxEntry = 1;    /* 現状は 1対1接続のみ対応 */
            work->parent_param.parentMaxSize = WXC_PACKET_SIZE_MAX;
            work->parent_param.childMaxSize = WXC_PACKET_SIZE_MAX;
            work->parent_param.CS_Flag = 1;     /* 連続転送 */
            WXC_InitDriver(&work->driver, &work->parent_param, WXCi_DriverEventCallback,
                           work->wm_dma);
            WXCi_ChangeState(WXC_STATE_READY, NULL);
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetParentParameter

  Description:  ワイヤレスの通信設定を指定する.
                設定内容は親機モードの状態時でのみ使用され,
                子機モードで接続した場合は親機側の設定に従う.
                この関数はオプションであり, 必要な場合に限り
                WXC_Start 関数より前に呼び出す.

  Arguments:    sendSize      親機送信サイズ.
                              WXC_PACKET_SIZE_MIN 以上かつ
                              WXC_PACKET_SIZE_MAX 以下である必要がある.
                recvSize      親機受信サイズ.
                              WXC_PACKET_SIZE_MIN 以上かつ
                              WXC_PACKET_SIZE_MAX 以下である必要がある.
                maxEntry      最大接続人数.
                              現在の実装ではこの指定は 1 のみサポートされる.

  Returns:      設定が有効であれば内部に反映して TRUE,
                そうでなければ警告文をデバッグ出力して FALSE.
 *---------------------------------------------------------------------------*/
BOOL WXC_SetParentParameter(u16 sendSize, u16 recvSize, u16 maxEntry)
{
    BOOL    ret = FALSE;

    /* 現実装では一対他の通信に未対応 */
    if (maxEntry > 1)
    {
        OS_TWarning("unsupported maxEntry. (must be 1)\n");
    }
    /* パケットサイズ判定 */
    else if ((sendSize < WXC_PACKET_SIZE_MIN) || (sendSize > WXC_PACKET_SIZE_MAX) ||
             (recvSize < WXC_PACKET_SIZE_MIN) || (recvSize > WXC_PACKET_SIZE_MAX))
    {
        OS_TWarning("packet size is out of range.\n");
    }
    /* MP 通信所要時間を判定 */
    else
    {
        /* 1回のMP通信にかかる時間に対する評価 */
        int     usec = 330 + 4 * (sendSize + 38) + maxEntry * (112 + 4 * (recvSize + 32));
        const int max_time = 5600;
        if (usec >= max_time)
        {
            OS_TWarning("specified MP setting takes %d[usec]. (should be lower than %d[usec])\n",
                        usec, max_time);
        }
        else
        {
            work->parent_param.parentMaxSize = sendSize;
            work->parent_param.childMaxSize = recvSize;
            ret = TRUE;
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_Start

  Description:  すれちがい通信開始関数.
                内部のワイヤレス駆動を開始する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_Start(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        SDK_ASSERT(WXC_GetStateCode() != WXC_STATE_END);

        if (WXC_GetStateCode() == WXC_STATE_READY)
        {
            WXCi_Reset(&work->driver);
            WXCi_ChangeState(WXC_STATE_ACTIVE, NULL);
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_Stop

  Description:  すれちがい通信停止関数.
                内部のワイヤレス駆動を停止する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_Stop(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        SDK_ASSERT(WXC_GetStateCode() != WXC_STATE_END);

        switch (WXC_GetStateCode())
        {

        case WXC_STATE_ACTIVE:
            if (!work->stopping)
            {
                work->stopping = TRUE;
                /* 現在通信中なら停止要求だけ設定しつつ完了を待つ */
                if (WXC_GetUserBitmap() == 0)
                {
                    WXCi_Stop(&work->driver);
                }
            }
            break;

        case WXC_STATE_READY:
        case WXC_STATE_ENDING:
        case WXC_STATE_END:
            /* 終了処理中か終了済みなら何もしない */
            break;

        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);

}

/*---------------------------------------------------------------------------*
  Name:         WXC_End

  Description:  すれちがい通信終了関数.
                内部のワイヤレス駆動を終了する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_End(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        switch (WXC_GetStateCode())
        {

        case WXC_STATE_READY:
        case WXC_STATE_ACTIVE:
            WXCi_ChangeState(WXC_STATE_ENDING, NULL);
            /* 現在通信中なら終了要求だけ設定しつつ完了を待つ */
            if (WXC_GetUserBitmap() == 0)
            {
                WXCi_End(&work->driver);
            }
            break;

        case WXC_STATE_ENDING:
        case WXC_STATE_END:
            /* 終了処理中か終了済みなら何もしない */
            break;

        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetStateCode

  Description:  現在のライブラリ状態を取得.

  Arguments:    None.

  Returns:      現在のライブラリ状態を示す WXCStateCode 型.
 *---------------------------------------------------------------------------*/
WXCStateCode WXC_GetStateCode(void)
{
    return state;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_IsParentMode

  Description:  現在のワイヤレス状態が親機モードか判定.
                WXC_STATE_ACTIVE の状態に限り有効.

  Arguments:    None.

  Returns:      ワイヤレス状態が親機モードなら TRUE, 子機モードなら FALSE.
 *---------------------------------------------------------------------------*/
BOOL WXC_IsParentMode(void)
{
    return WXCi_IsParentMode(&work->driver);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentParam

  Description:  現在のワイヤレス状態の WMParentParam 構造体を参照.

  Arguments:    None.

  Returns:      WMParentParam 構造体.
 *---------------------------------------------------------------------------*/
const WMParentParam *WXC_GetParentParam(void)
{
    return &work->parent_param;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentBssDesc

  Description:  現在接続対象の WMBssDesc 構造体を参照.

  Arguments:    None.

  Returns:      WMBssDesc 構造体.
 *---------------------------------------------------------------------------*/
const WMBssDesc *WXC_GetParentBssDesc(void)
{
    return WXCi_GetParentBssDesc(&work->driver);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserBitmap

  Description:  現在接続中のユーザ状況をビットマップで取得.
                

  Arguments:    None.

  Returns:      WMBssDesc 構造体.
 *---------------------------------------------------------------------------*/
u16 WXC_GetUserBitmap(void)
{
    u16     bitmap = work->driver.peer_bitmap;
    if (bitmap != 0)
    {
        bitmap = (u16)(bitmap | (1 << work->driver.own_aid));
    }
    return bitmap;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentGgid

  Description:  現在の接続で選択されているGGIDを取得.
                通信が確立されていない状況では 0 を返す.
                現在の通信状態は WXC_GetUserBitmap() 関数の返り値で判定可能.

  Arguments:    None.

  Returns:      現在の接続で選択されているGGID.
 *---------------------------------------------------------------------------*/
u32 WXC_GetCurrentGgid(void)
{
    u32     ggid = 0;
    {
        OSIntrMode bak_cpsr = OS_DisableInterrupts();
        WXCProtocolRegistry *current = WXC_GetCurrentBlock(work->protocol);
        if (current)
        {
            ggid = current->ggid;
            /* ANY であれば検出した相手の GGID を模倣する */
            if (ggid == WXC_GGID_COMMON_ANY)
            {
                ggid = work->protocol->target_ggid;
            }
        }
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    return ggid;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetOwnAid

  Description:  現在の接続における自身のAIDを取得.
                通信が確立されていない状況では不定な値を返す.
                現在の通信状態は WXC_GetUserBitmap() 関数の返り値で判定可能.

  Arguments:    None.

  Returns:      現在の接続における自身のAID.
 *---------------------------------------------------------------------------*/
u16 WXC_GetOwnAid(void)
{
    return work->driver.own_aid;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserInfo

  Description:  現在接続中のユーザ情報を取得.
                返されるポインタが指す内容を変更してはならない.

  Arguments:    aid           情報を取得するユーザのAID.

  Returns:      指定されたAIDが有効ならばユーザ情報, そうでなければ NULL
 *---------------------------------------------------------------------------*/
const WXCUserInfo *WXC_GetUserInfo(u16 aid)
{
    const WXCUserInfo *ret = NULL;

    {
        OSIntrMode bak_cpsr = OS_DisableInterrupts();
        if ((aid <= WM_NUM_MAX_CHILD) && ((WXC_GetUserBitmap() & (1 << aid)) != 0))
        {
            ret = &work->user_info[aid];
        }
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetChildMode

  Description:  ワイヤレスを子機側でしか起動させないよう設定.
                この関数は WXC_Start 関数より前に呼び出す必要がある.

  Arguments:    enable        子機側でしか起動させない場合は TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetChildMode(BOOL enable)
{
    SDK_ASSERT(WXC_GetStateCode() != WXC_STATE_ACTIVE);
    WXCi_SetChildMode(&work->scheduler, enable);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_AddData

  Description:  完了したブロックデータ交換に追加でデータを設定

  Arguments:    send_buf      送信バッファ.
                send_size     送信バッファサイズ.
                recv_buf      受信バッファ.
                recv_max      受信バッファサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_AddData(const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max)
{
    WXC_AddBlockSequence(work->protocol, send_buf, send_size, recv_buf, recv_max);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_RegisterDataEx

  Description:  交換用のデータを登録する

  Arguments:    ggid          登録データのGGID
                callback      ユーザーへのコールバック関数（データ交換完了時にコールバックされる）
                send_ptr      登録データのポインタ
                send_size     登録データのサイズ
                recv_ptr      受信バッファのポインタ
                recv_size     受信バッファのサイズ
                type          すれちがいプロトコルの種別(PIType)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_RegisterDataEx(u32 ggid, WXCCallback callback, u8 *send_ptr, u32 send_size, u8 *recv_ptr,
                        u32 recv_size, const char *type)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();

    /*
     * 同じ GGID は複数登録できないようにしておく.
     * (子機側から接続時にどれを使用してよいか判断できないため)
     */
    WXCProtocolRegistry *same_ggid = WXC_FindNextBlock(work->protocol, NULL, ggid, TRUE);
    if (same_ggid != NULL)
    {
        OS_TWarning("already registered same GGID!");
    }
    /* そうでなければ, 空いているブロックを使用 */
    else
    {
        WXCProtocolRegistry *p_data = WXC_FindNextBlock(work->protocol, NULL, 0, TRUE);
        /* 空きが無ければ失敗 */
        if (!p_data)
        {
            OS_TPanic("no more memory to register data!");
        }
        else
        {
            /* 選択されたプロトコルによって、登録する通信プロトコルを変える */
            WXCProtocolImpl *impl = WXC_FindProtocolImpl(type);
            if (!impl)
            {
                OS_TPanic("unknown protocol \"%s\"!", type);
            }
            WXC_InitProtocolRegistry(p_data, ggid, callback, impl);
            WXC_SetInitialExchangeBuffers(p_data, send_ptr, send_size, recv_ptr, recv_size);
        }
    }

    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetInitialData

  Description:  初回データ交換で毎回使用するデータブロックを指定する

  Arguments:    ggid          登録データのGGID
                send_ptr      登録データのポインタ
                send_size     登録データのサイズ
                recv_ptr      受信バッファのポインタ
                recv_size     受信バッファのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetInitialData(u32 ggid, u8 *send_ptr, u32 send_size, u8 *recv_ptr, u32 recv_size)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();

    /* 指定のブロックを検索 */
    WXCProtocolRegistry *target = WXC_FindNextBlock(work->protocol, NULL, ggid, TRUE);
    if (target)
    {
        WXC_SetInitialExchangeBuffers(target, send_ptr, send_size, recv_ptr, recv_size);
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_UnregisterData

  Description:  交換用のデータを登録から破棄する

  Arguments:    ggid          破棄するブロックに関連付けられた GGID

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_UnregisterData(u32 ggid)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();

    /* 指定のブロックを検索 */
    WXCProtocolRegistry *target = WXC_FindNextBlock(work->protocol, NULL, ggid, TRUE);
    /* 共通すれちがい通信仕様の場合も含めてもう1回検索 */
    if (!target)
    {
        target = WXC_FindNextBlock(work->protocol, NULL, (u32)(ggid | WXC_GGID_COMMON_BIT), TRUE);
    }
    if (target)
    {
        /* 現在通信に使用中であれば失敗 */
        if ((WXC_GetUserBitmap() != 0) && (target == WXC_GetCurrentBlock(work->protocol)))
        {
            OS_TWarning("specified data is now using.");
        }
        else
        {
            /* データを解放する */
            target->ggid = 0;
            target->callback = NULL;
            target->send.buffer = NULL;
            target->send.length = 0;
            target->recv.buffer = NULL;
            target->recv.length = 0;
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetSsid

  Description:  子機として接続する際の SSID を設定する.
                内部関数.

  Arguments:    buffer        設定すSSIDデータ.
                length        SSIDデータ長.
                              WM_SIZE_CHILD_SSID 以下である必要がある.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_SetSsid(const void *buffer, u32 length)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    WXC_SetDriverSsid(&work->driver, buffer, length);
    (void)OS_RestoreInterrupts(bak_cpsr);
}


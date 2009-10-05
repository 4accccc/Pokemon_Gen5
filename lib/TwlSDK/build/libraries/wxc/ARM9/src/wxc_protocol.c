/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_protocol.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

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

#include <nitro/wxc/common.h>
#include <nitro/wxc/protocol.h>


/*****************************************************************************/
/* variable */

/* 現在までに追加されているすれちがい通信プロトコル */
static WXCProtocolImpl *impl_list;


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         WXC_InitProtocol

  Description:  WXCライブラリプロトコルを初期化

  Arguments:    protocol      WXCProtocolContext 構造体.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_InitProtocol(WXCProtocolContext * protocol)
{
    protocol->current_block = NULL;
    MI_CpuClear32(protocol, sizeof(protocol));
}

/*---------------------------------------------------------------------------*
  Name:         WXC_InstallProtocolImpl

  Description:  選択可能な新しいプロトコルを追加

  Arguments:    impl          WXCProtocolImpl構造体へのポインタ.
                              この構造体はライブラリ終了時まで内部で使用される.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_InstallProtocolImpl(WXCProtocolImpl * impl)
{
    WXCProtocolImpl *p = impl_list;
    if (!p)
    {
        impl_list = impl;
        impl->next = NULL;
    }
    else
    {
        for (;; p = p->next)
        {
            if (p == impl)
            {
                break;
            }
            else if (!p->next)
            {
                p->next = impl;
                impl->next = NULL;
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXC_FindProtocolImpl

  Description:  指定の名前を持つプロトコルを検索

  Arguments:    name          プロトコル名.

  Returns:      指定の名前を持つプロトコルが存在すればそのポインタ.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl *WXC_FindProtocolImpl(const char *name)
{
    WXCProtocolImpl *p = impl_list;
    for (; p; p = p->next)
    {
        if (STD_CompareString(name, p->name) == 0)
        {
            break;
        }
    }
    return p;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_ResetSequence

  Description:  WXCライブラリプロトコルを再初期化

  Arguments:    protocol      WXCProtocolContext 構造体.
                send_max      送信パケット最大サイズ.
                recv_max      受信パケット最大サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_ResetSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max)
{
    WXC_GetCurrentBlockImpl(protocol)->Init(protocol, send_max, recv_max);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_AddBlockSequence

  Description:  ブロックデータ交換を設定

  Arguments:    protocol      WXCProtocolContext 構造体.
                send_buf      送信バッファ.
                send_size     送信バッファサイズ.
                recv_buf      受信バッファ.
                recv_max      受信バッファサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_AddBlockSequence(WXCProtocolContext * protocol,
                          const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max)
{
    int     result;
    result =
        WXC_GetCurrentBlockImpl(protocol)->AddData(protocol, send_buf, send_size, recv_buf,
                                                   recv_max);
    SDK_TASSERTMSG(result, "sequence is now busy.");
}

/*---------------------------------------------------------------------------*
  Name:         WXC_FindNextBlock

  Description:  GGID を指定してブロックデータを検索.

  Arguments:    protocol      WXCProtocolContext 構造体.
                from          検索開始位置.
                              NULL なら先頭から検索し, そうでなければ
                              指定されたブロックの次から検索する.
                ggid          検索するブロックデータに関連付けられた GGID.
                              0 なら空きブロックを示す.
                match         検索条件.
                              TRUE なら GGID の合致するものを検索対象とし,
                              FALSE なら 合致しないものを対象とする.

  Returns:      合致するブロックデータが存在すればそのポインタを,
                無ければ NULL を返す.
 *---------------------------------------------------------------------------*/
WXCProtocolRegistry *WXC_FindNextBlock(WXCProtocolContext * protocol,
                                       const WXCProtocolRegistry * from, u32 ggid, BOOL match)
{
    WXCProtocolRegistry *target;

    /* NULL なら先頭から検索 */
    if (!from)
    {
        from = &protocol->data_array[WXC_REGISTER_DATA_MAX - 1];
    }

    target = (WXCProtocolRegistry *) from;
    for (;;)
    {
        BOOL    eq;
        /* 次の要素が配列の終端に達したらループ */
        if (++target >= &protocol->data_array[WXC_REGISTER_DATA_MAX])
        {
            target = &protocol->data_array[0];
        }
        /* 検索条件の一致するブロックが存在すれば終了 */
        eq = (target->ggid == ggid);
        if ((match && eq) || (!match && !eq))
        {
            break;
        }
        /* 全要素を検索済みなら終了 */
        if (target == from)
        {
            target = NULL;
            break;
        }
    }
    return target;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconSendHook

  Description:  ビーコン更新タイミングで呼び出されるフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_param       次回のビーコンに使用する WMParentParam 構造体.
                              この関数の内部で必要に応じて変更する.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param)
{
    WXC_GetCurrentBlockImpl(protocol)->BeaconSend(protocol, p_param);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconRecvHook

  Description:  スキャンされた各ビーコンに対して呼び出されるフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_desc        スキャンされた WMBssDesc 構造体

  Returns:      接続対象とみなせば TRUE を, そうでなければ FALSE を返す.
 *---------------------------------------------------------------------------*/
BOOL WXC_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc)
{
    BOOL    ret = FALSE;

    /* GGID 0 は空きブロックとして扱われるので除外 */
    u32 ggid = p_desc->gameInfo.ggid;
    if (ggid != 0)
    {
        /* 登録済みの GGID から一致するものを検索 */
        WXCProtocolRegistry *found = NULL;
        int i;
        for (i = 0; i < WXC_REGISTER_DATA_MAX; ++i)
        {
            WXCProtocolRegistry *p = &protocol->data_array[i];
            /* 厳密に一致 */
            if (p->ggid == ggid)
            {
                found = p;
                break;
            }
            /* 双方とも共通すれちがい */
            else if (WXC_IsCommonGgid(ggid) && WXC_IsCommonGgid(p->ggid))
            {
                /* いずれかが中継所であれば一致 */
                const BOOL is_target_any = (ggid == WXC_GGID_COMMON_PARENT);
                const BOOL is_current_any = (p->ggid == WXC_GGID_COMMON_ANY);
                if (is_target_any)
                {
                    if (!is_current_any)
                    {
                        ggid = p->ggid;
                        found = p;
                        break;
                    }
                }
                else
                {
                    if (is_current_any)
                    {
                        found = p;
                        break;
                    }
                }
            }
        }
        /* 一致すればプロトコル内でも判定処理を行う */
        if (found)
        {
            ret = found->impl->BeaconRecv(protocol, p_desc);
            /* GGID もプロトコルも一致すれば選択を切り替える */
            if (ret)
            {
                WXC_SetCurrentBlock(protocol, found);
                protocol->target_ggid = ggid;
            }
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketSendHook

  Description:  MP パケット送信のタイミングで呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                packet        送信パケット情報を設定する WXCPacketInfo ポインタ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet)
{
    WXC_GetCurrentBlockImpl(protocol)->PacketSend(protocol, packet);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketRecvHook

  Description:  MP パケット受信のタイミングで呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                packet        受信パケット情報を参照する WXCPacketInfo ポインタ.

  Returns:      1回のデータ交換が完了したら TRUE.
 *---------------------------------------------------------------------------*/
BOOL WXC_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet)
{
    int     ret = FALSE;

    ret = WXC_GetCurrentBlockImpl(protocol)->PacketRecv(protocol, packet);
    /*
     * 1回のデータ交換が完了.
     * ここでは, そのまま通信を続ければやがて !IsExecuting() になるはず.
     */
    if (ret)
    {
        /* 転送完了をユーザコールバックへ通知 */
        WXCCallback callback = protocol->current_block->callback;
        if (callback)
        {
            (*callback) (WXC_STATE_EXCHANGE_DONE, &protocol->recv);
        }
        /*
         * コールバック内で AddData() されていれば, 通信継続するはず.
         * そうでなければ前述の通り !IsExecuting() になるように処理が進む.
         */
    }

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_ConnectHook

  Description:  通信対象の接続検知時に呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                bitmap        接続された通信先の AID ビットマップ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_ConnectHook(WXCProtocolContext * protocol, u16 bitmap)
{
#pragma unused(protocol)
#pragma unused(bitmap)
}

/*---------------------------------------------------------------------------*
  Name:         WXC_DisconnectHook

  Description:  通信対象の切断検知時に呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                bitmap        切断された通信先の AID ビットマップ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_DisconnectHook(WXCProtocolContext * protocol, u16 bitmap)
{
#pragma unused(protocol)
#pragma unused(bitmap)
}

/*---------------------------------------------------------------------------*
  Name:         WXC_CallPreConnectHook

  Description:  通信対象への接続前に呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_desc        接続対象の WMBssDesc 構造体
                ssid          設定するためのSSIDを格納するバッファ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_CallPreConnectHook(WXCProtocolContext * protocol, WMBssDesc *p_desc, u8 *ssid)
{
    WXCProtocolImpl * const impl = WXC_GetCurrentBlockImpl(protocol);
    if (impl->PreConnectHook)
    {
        impl->PreConnectHook(protocol, p_desc, ssid);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXC_InitProtocolRegistry

  Description:  指定の登録構造体に GGID とコールバックを関連付ける

  Arguments:    p_data        登録に使用する WXCProtocolRegistry 構造体
                ggid          設定する GGID
                callback      ユーザーへのコールバック関数
                              (NULL の場合は設定解除)
                impl          採用する通信プロトコル

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_InitProtocolRegistry(WXCProtocolRegistry * p_data, u32 ggid, WXCCallback callback,
                              WXCProtocolImpl * impl)
{
    p_data->ggid = ggid;
    p_data->callback = callback;
    p_data->impl = impl;
    p_data->send.buffer = NULL;
    p_data->send.length = 0;
    p_data->recv.buffer = NULL;
    p_data->recv.length = 0;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetInitialExchangeBuffers

  Description:  初回のデータ交換で毎回自動的に使用するデータを設定

  Arguments:    p_data        登録に使用する WXCProtocolRegistry 構造体
                send_ptr      登録データのポインタ
                send_size     登録データのサイズ
                recv_ptr      受信バッファのポインタ
                recv_size     受信バッファのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetInitialExchangeBuffers(WXCProtocolRegistry * p_data, u8 *send_ptr, u32 send_size,
                                   u8 *recv_ptr, u32 recv_size)
{
    p_data->send.buffer = send_ptr;
    p_data->send.length = (u32)send_size;

    p_data->recv.buffer = recv_ptr;
    p_data->recv.length = (u32)recv_size;
}

/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_protocol_impl_wxc.c

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
#include <nitro/wxc/wxc_protocol_impl_wxc.h>


/*****************************************************************************/
/* declaration */

static void WXCi_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param);
static BOOL WXCi_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc);
static void WXCi_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet);
static BOOL WXCi_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet);
static void WXCi_InitSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max);
static BOOL WXCi_AddData(WXCProtocolContext * protocol, const void *send_buf, u32 send_size,
                         void *recv_buf, u32 recv_max);
static BOOL WXCi_IsExecuting(WXCProtocolContext * protocol);


/*****************************************************************************/
/* variable */

/* WXC プロトコル用のワークバッファ */
static WXCImplWorkWxc impl_wxc_work;

/* WXC プロトコルインタフェース */
static WXCProtocolImpl impl_wxc = {
    "WXC",
    WXCi_BeaconSendHook,
    WXCi_BeaconRecvHook,
    NULL,
    WXCi_PacketSendHook,
    WXCi_PacketRecvHook,
    WXCi_InitSequence,
    WXCi_AddData,
    WXCi_IsExecuting,
    &impl_wxc_work,
};


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         WXCi_GetProtocolImplWXC

  Description:  WXCすれちがい通信プロトコルのインタフェースを取得。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl* WXCi_GetProtocolImplWXC(void)
{
    return &impl_wxc;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_BeaconSendHook

  Description:  ビーコン更新タイミングで呼び出されるフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_param       次回のビーコンに使用する WMParentParam 構造体.
                              この関数の内部で必要に応じて変更する.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param)
{
#pragma unused(protocol)
#pragma unused(p_param)

    /*
     * 現在 WXC では特に UserGameInfo を使用していない.
     * 今後, GGID のネットマスク化や共通 GGID モードなど導入された場合
     * 必要に応じて拡張する予定.
     */
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_BeaconRecvHook

  Description:  スキャンされた各ビーコンに対して呼び出されるフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_desc        スキャンされた WMBssDesc 構造体

  Returns:      接続対象とみなせば TRUE を, そうでなければ FALSE を返す.
 *---------------------------------------------------------------------------*/
BOOL WXCi_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc)
{
#pragma unused(protocol)
#pragma unused(p_desc)

    /* 現在 WXC では GGID のみで区別する「ローカルモード」のみ導入 */

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_PacketSendHook

  Description:  MP パケット送信のタイミングで呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                packet        送信パケット情報を設定する WXCPacketInfo ポインタ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    WXCBlockHeader *p_header = (WXCBlockHeader *) packet->buffer;

    WXC_PACKET_LOG("--SEND:ACK=(%3d,%d,%04X),REQ=(%3d,%d,%04X)\n",
                   wxc_work->ack.phase, wxc_work->ack.command, wxc_work->ack.index,
                   wxc_work->req.phase, wxc_work->req.command, wxc_work->req.index);

    /* 送信バッファにパケットヘッダ部分を設定 */
    p_header->req = wxc_work->req;
    p_header->ack = wxc_work->ack;

    /* フェーズが同じ場合のみ応答データ送信 */
    if (wxc_work->ack.phase == wxc_work->req.phase)
    {
        u8     *p_body = packet->buffer + sizeof(WXCBlockHeader);

        switch (wxc_work->ack.command)
        {
        case WXC_BLOCK_COMMAND_INIT:
            /* 初期情報(サイズ + チェックサム)を送信 */
            WXC_PACKET_LOG("       INIT(%6d)\n", protocol->send.length);
            *(u16 *)(p_body + 0) = (u16)protocol->send.length;
            *(u16 *)(p_body + 2) = protocol->send.checksum;
            break;
        case WXC_BLOCK_COMMAND_SEND:
            /* 最後に要求されたインデックスを送信 */
            {
                int     offset = (wxc_work->ack.index * wxc_work->send_unit);
                u32     len = (u32)(protocol->send.length - offset);
                if (len > wxc_work->send_unit)
                {
                    len = wxc_work->send_unit;
                }
                MI_CpuCopy8((const u8 *)protocol->send.buffer + offset, p_body, len);
            }
            break;
        }
    }

    /* パケットサイズを指定 */
    packet->length = (u16)MATH_ROUNDUP(sizeof(WXCBlockHeader) + wxc_work->send_unit, 2);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_MergeBlockData

  Description:  受信したブロックデータ断片を保存.

  Arguments:    protocol      WXCProtocolContext 構造体.
                index         受信データのインデックス.
                src           受信データバッファ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_MergeBlockData(WXCProtocolContext * protocol, int index, const void *src)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    if (index < wxc_work->recv_total)
    {
        u32    *bmp = wxc_work->recv_bitmap_buf + (index >> 5);
        u32     bit = (u32)(1 << (index & 31));
        if ((*bmp & bit) == 0)
        {
            int     offset = (index * wxc_work->recv_unit);
            u32     len = (u32)(protocol->recv.length - offset);
            if (len > wxc_work->recv_unit)
            {
                len = wxc_work->recv_unit;
            }
            /* ここで recv.buffer が NULL だったら保存しない */
            if (protocol->recv.buffer != NULL)
            {
                MI_CpuCopy8(src, (u8 *)protocol->recv.buffer + offset, len);
            }
            *bmp |= bit;
            /* 受信完了なら DONE */
            if (--wxc_work->recv_rest == 0)
            {
                wxc_work->req.command = WXC_BLOCK_COMMAND_DONE;
            }
            /* そうでなければ未受信のインデックスを検索 */
            else
            {
                int     i;
                /* 前回要求したインデックスを基点とする */
                int     count = wxc_work->recent_index[0];
                int     last_count = count;
                if (last_count >= wxc_work->recv_total)
                {
                    last_count = (int)wxc_work->recv_total - 1;
                }
                for (;;)
                {
                    /* 終端に達したら先頭へループ */
                    if (++count >= wxc_work->recv_total)
                    {
                        count = 0;
                    }
                    /* 検索が一周した場合は履歴から再選択 */
                    if (count == last_count)
                    {
                        count = wxc_work->recent_index[WXC_RECENT_SENT_LIST_MAX - 1];
                        break;
                    }
                    /* 未受信か判定 */
                    if ((*(wxc_work->recv_bitmap_buf + (count >> 5)) & (1 << (count & 31))) == 0)
                    {
                        /* 最近要求したインデックスでないか判定 */
                        for (i = 0; i < WXC_RECENT_SENT_LIST_MAX; ++i)
                        {
                            if (count == wxc_work->recent_index[i])
                            {
                                break;
                            }
                        }
                        if (i >= WXC_RECENT_SENT_LIST_MAX)
                        {
                            break;
                        }
                    }
                }
                /* インデックス要求履歴を更新 */
                for (i = WXC_RECENT_SENT_LIST_MAX; --i > 0;)
                {
                    wxc_work->recent_index[i] = wxc_work->recent_index[i - 1];
                }
                wxc_work->recent_index[0] = (u16)count;
                wxc_work->req.index = wxc_work->recent_index[0];
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_PacketRecvHook

  Description:  MP パケット受信のタイミングで呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                packet        受信パケット情報を参照する WXCPacketInfo ポインタ.

  Returns:      1回のデータ交換が完了したら TRUE.
 *---------------------------------------------------------------------------*/
BOOL WXCi_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    int     ret = FALSE;

    WXCBlockHeader *p_header = (WXCBlockHeader *) packet->buffer;

    if (packet->length >= wxc_work->recv_unit)
    {
        WXC_PACKET_LOG("--RECV:REQ=(%3d,%d,%04X),ACK=(%3d,%d,%04X)\n",
                       p_header->req.phase, p_header->req.command, p_header->req.index,
                       p_header->ack.phase, p_header->ack.command, p_header->ack.index);

        /* フェーズが同じ場合のみ相手の要求データを保存 */
        if (p_header->req.phase == wxc_work->req.phase)
        {
            wxc_work->ack = p_header->req;
        }

        /* フェーズが同じ場合のみ相手の応答データを参照 */
        if (p_header->ack.phase == wxc_work->req.phase)
        {
            u8     *p_body = packet->buffer + sizeof(WXCBlockHeader);

            /* コマンドごとの受信処理 */
            switch (p_header->ack.command)
            {
            case WXC_BLOCK_COMMAND_QUIT:
                /* プロトコルの切断 */
                wxc_work->executing = FALSE;
                break;
            case WXC_BLOCK_COMMAND_INIT:
                /* 初期情報(サイズ + チェックサム)を受信 */
                protocol->recv.length = *(u16 *)(p_body + 0);
                protocol->recv.checksum = *(u16 *)(p_body + 2);
                wxc_work->recv_total =
                    (u16)((protocol->recv.length + wxc_work->recv_unit - 1) / wxc_work->recv_unit);
                wxc_work->recv_rest = wxc_work->recv_total;
                wxc_work->req.index = 0;
                /* サイズが 0 の場合は WXC_BLOCK_COMMAND_DONE に移行 */
                if (wxc_work->recv_total > 0)
                {
                    wxc_work->req.command = WXC_BLOCK_COMMAND_SEND;
                }
                else
                {
                    wxc_work->req.command = WXC_BLOCK_COMMAND_DONE;
                }
                WXC_PACKET_LOG("       INIT(%6d)\n", protocol->recv.length);
                break;
            case WXC_BLOCK_COMMAND_SEND:
                /* 実際に応答された受信データを格納 */
                WXCi_MergeBlockData(protocol, p_header->ack.index, p_body);
                break;
            }
        }

        /* 双方のデータ交換完了 */
        if ((p_header->ack.phase == wxc_work->req.phase) &&
            (p_header->ack.command == WXC_BLOCK_COMMAND_DONE) &&
            (wxc_work->ack.command == WXC_BLOCK_COMMAND_DONE))
        {
            /* データ交換フェーズをインクリメントして再初期化 */
            ++wxc_work->req.phase;
            /* そのまま通信を続ければやがて !executing になるようにしておく */
            wxc_work->req.command = WXC_BLOCK_COMMAND_QUIT;
            ret = TRUE;
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_InitSequence

  Description:  WXCライブラリプロトコルを再初期化する関数

  Arguments:    protocol      WXCProtocolContext 構造体.
                send_max      送信パケット最大サイズ.
                recv_max      受信パケット最大サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_InitSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    /* 送受信バッファの初期化 */
    protocol->send.buffer = NULL;
    protocol->send.length = 0;
    protocol->send.checksum = 0;
    protocol->recv.buffer = NULL;
    protocol->recv.length = 0;
    protocol->recv.buffer_max = 0;

    /* ブロック転送の分割サイズを計算 */
    wxc_work->send_unit = (u16)(send_max - sizeof(WXCBlockHeader));
    wxc_work->recv_unit = (u16)(recv_max - sizeof(WXCBlockHeader));

    /* 送信管理情報の初期化 */
    {
        int     i;
        for (i = 0; i < WXC_RECENT_SENT_LIST_MAX; ++i)
        {
            wxc_work->recent_index[i] = 0;
        }
    }
    /* 受信管理情報の初期化 */
    wxc_work->req.phase = 0;
    wxc_work->recv_total = 0;

    /*
     * なんら AddData() されないで通信が開始されれば
     * !IsExecuting() へ向かうように初期化しておくこと.
     */
    wxc_work->req.command = WXC_BLOCK_COMMAND_QUIT;
    /*
     * ただし相手への ack にそんなことをすると
     * ただちに終了してしまうので, 無応答(IDLE) としておくこと.
     */
    wxc_work->ack.phase = 0;
    wxc_work->ack.command = WXC_BLOCK_COMMAND_IDLE;

    MI_CpuClear32(wxc_work->recv_bitmap_buf, sizeof(wxc_work->recv_bitmap_buf));

    wxc_work->executing = TRUE;

}

/*---------------------------------------------------------------------------*
  Name:         WXCi_AddData

  Description:  ブロックデータ交換を設定する関数。

  Arguments:    protocol      WXCProtocolContext 構造体.
                send_buf      送信バッファ.
                send_size     送信バッファサイズ.
                recv_buf      受信バッファ.
                recv_max      受信バッファサイズ.

  Returns:      登録できる状況なら、設定した後 TRUE を返す.
 *---------------------------------------------------------------------------*/
BOOL WXCi_AddData(WXCProtocolContext * protocol, const void *send_buf, u32 send_size,
                  void *recv_buf, u32 recv_max)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    if (wxc_work->req.command == WXC_BLOCK_COMMAND_QUIT)
    {
        wxc_work->req.command = WXC_BLOCK_COMMAND_INIT;

        protocol->send.buffer = (void *)send_buf;
        protocol->send.length = (u16)send_size;
        protocol->send.checksum = MATH_CalcChecksum8(send_buf, send_size);

        protocol->recv.buffer = recv_buf;
        protocol->recv.buffer_max = (u16)recv_max;
        MI_CpuClear32(wxc_work->recv_bitmap_buf, sizeof(wxc_work->recv_bitmap_buf));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IsExecuting

  Description:  現在データ交換中かを判定.

  Arguments:    None.

  Returns:      現在データ交換中ならば TRUE.
 *---------------------------------------------------------------------------*/
BOOL WXCi_IsExecuting(WXCProtocolContext * protocol)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    return wxc_work->executing;
}


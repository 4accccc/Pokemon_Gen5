/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_protocol_impl_common.c.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

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
/* declaration */

/* 共通すれちがい通信プロトコルのビーコンフォーマット構造体 */
typedef struct WXCCommonProtocolBeaconFormat
{
    u32     send_length;    /* 送信データサイズ (ゲームごとに固定) */
}
WXCCommonProtocolBeaconFormat;


/* 共通すれちがい通信プロトコルの接続時SSIDフォーマット構造体 */
typedef struct WXCCommonProtocolSsidFormat
{
    char    header[4];      /* バージョン情報 "WXC1" */
    u32     ggid;           /* 子機から申告する GGID */
}
WXCCommonProtocolSsidFormat;


/* 通常コマンドパケットフォーマット */
typedef struct PacketCommandFormat
{
    u16     segment:1;  /* always 0 */
    u16     ack:1;
    u16     turn:1;
    u16     phase:5;
    u16     checksum:8;
    u16     recv_arg;
    u16     send_arg;
}
PacketCommandFormat;

/* データブロック交換用のセグメントフォーマット */
typedef struct PacketSegmentFormat
{
    u16     segment:1;  /* always 1 */
    u16     recv_arg:15;
    u16     turn:1;
    u16     send_arg:15;
    u8      buffer[0];
}
PacketSegmentFormat;

/* 共通すれちがい通信プロトコル用の情報構造体 */
typedef struct WXCCommonProtocolImplContext
{
    /* ビーコン送受信設定 */
    WXCCommonProtocolBeaconFormat beacon_format;
    /* MP 通信設定 */
    u32     parent_send;
    u32     child_send;
    BOOL    executing;
    /* プロトコル制御 */
    int     req_phase;
    u32     req_turn;
    int     ack_phase;
    /* データ交換処理 */
    u32     own_packet;
    u32     peer_packet;
    u32     req_index;
    u32     ack_index;
    u32     recv_count;
    u32     recv_total;
    u32     recv_bitset[WXC_MAX_DATA_SIZE / (8 * 4) + 1];

//#define DEBUG_CALCSPEED

#if defined(DEBUG_CALCSPEED)
    /* 計速用デバッグメンバ */
    OSTick  tick_count;
#endif
}
WXCCommonProtocolImplContext;

static void WXCi_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param);
static BOOL WXCi_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc);
static void WXCi_PreConnectHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc, u8 *ssid);
static void WXCi_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet);
static BOOL WXCi_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet);
static void WXCi_InitSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max);
static BOOL WXCi_AddData(WXCProtocolContext * protocol, const void *send_buf, u32 send_size,
                         void *recv_buf, u32 recv_max);
static BOOL WXCi_IsExecuting(WXCProtocolContext * protocol);


/*****************************************************************************/
/* constant */

/* データ交換プロトコルの状態遷移 */
enum
{
    PHASE_IDLE,
    PHASE_QUIT,
    PHASE_INIT,
    PHASE_DATA,
    PHASE_DONE
};


/*****************************************************************************/
/* variable */

/* WXC プロトコル用のワークバッファ */
static WXCCommonProtocolImplContext work_common ATTRIBUTE_ALIGN(32);

/* WXC プロトコルインタフェース */
static WXCProtocolImpl impl_common =
{
    "COMMON",
    WXCi_BeaconSendHook,
    WXCi_BeaconRecvHook,
    WXCi_PreConnectHook,
    WXCi_PacketSendHook,
    WXCi_PacketRecvHook,
    WXCi_InitSequence,
    WXCi_AddData,
    WXCi_IsExecuting,
    &work_common,
};


/*****************************************************************************/
/* function */

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
    /*
     * このフックは, 通常のゲームタイトルまたは
     * 親機モード専用の中継所でのみ呼び出される.
     */
    WXCCommonProtocolImplContext *work = (WXCCommonProtocolImplContext*)WXC_GetCurrentBlockImpl(protocol)->impl_work;
    WXCProtocolRegistry * const reg = WXC_GetCurrentBlock(protocol);

    work->beacon_format.send_length = reg->send.length;
    p_param->userGameInfo = (u16*)&work->beacon_format;
    p_param->userGameInfoLength = sizeof(work->beacon_format);
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
    /*
     * このフックが呼び出されるのはゲームタイトルの子機モード時のみ.
     * WXC_GGID_COMMON_ANYを使用する中継所ではユーザコールバックへ通知される.
     */
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_PreConnectHook

  Description:  通信対象への接続前に呼び出されるフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_desc        接続対象の WMBssDesc 構造体
                ssid          設定するためのSSIDを格納するバッファ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_PreConnectHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc,
                         u8 ssid[WM_SIZE_CHILD_SSID])
{
#pragma unused(p_desc)
    /*
     * このフックが呼び出されるのはゲームタイトルの子機モード時のみ.
     * WXC_GGID_COMMON_ANYを使用する中継所ではユーザコールバックへ通知される.
     */
    WXCCommonProtocolSsidFormat *p = (WXCCommonProtocolSsidFormat*)ssid;
    WXCProtocolRegistry * const reg = WXC_GetCurrentBlock(protocol);
    p->header[0] = 'W';
    p->header[1] = 'X';
    p->header[2] = 'C';
    p->header[3] = '1';
    p->ggid = reg->ggid;
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
    WXCCommonProtocolImplContext *work = (WXCCommonProtocolImplContext*)WXC_GetCurrentBlockImpl(protocol)->impl_work;

    protocol->send.buffer = NULL;
    protocol->send.length = 0;
    protocol->send.buffer_max = 0;
    protocol->send.checksum = 0;
    protocol->recv.buffer = NULL;
    protocol->recv.length = 0;
    protocol->recv.buffer_max = 0;
    protocol->recv.checksum = 0;

    work->parent_send = send_max;
    work->child_send = recv_max;

    work->req_phase = PHASE_QUIT;
    work->req_turn = 0;
    work->ack_phase = PHASE_IDLE;

    work->executing = TRUE;
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
    BOOL    retval = FALSE;

    WXCCommonProtocolImplContext *work = (WXCCommonProtocolImplContext*)WXC_GetCurrentBlockImpl(protocol)->impl_work;

    if (work->req_phase == PHASE_QUIT)
    {
        retval = TRUE;
        protocol->send.buffer = (void*)send_buf;
        protocol->send.buffer_max = (u16)send_size;
        protocol->send.length = (u16)send_size;
        protocol->send.checksum = MATH_CalcChecksum8(send_buf, send_size);
        protocol->recv.buffer = recv_buf;
        protocol->recv.buffer_max = (u16)recv_max;
        /* 受信サイズとチェックサムはデータ交換後に確定 */
        work->req_phase = PHASE_INIT;
        work->ack_phase = PHASE_IDLE;
#if defined(DEBUG_CALCSPEED)
        if (OS_IsTickAvailable())
        {
            work->tick_count = OS_GetTick();
        }
#endif

    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IsExecuting

  Description:  現在データ交換中かを判定.

  Arguments:    None.

  Returns:      現在データ交換中ならば TRUE.
 *---------------------------------------------------------------------------*/
BOOL WXCi_IsExecuting(WXCProtocolContext * protocol)
{
    WXCCommonProtocolImplContext *work = (WXCCommonProtocolImplContext*)WXC_GetCurrentBlockImpl(protocol)->impl_work;
    return work->executing;
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
    WXCCommonProtocolImplContext *work = (WXCCommonProtocolImplContext*)WXC_GetCurrentBlockImpl(protocol)->impl_work;

    if ((work->req_phase == PHASE_DATA) || (work->req_phase == PHASE_DONE))
    {
        PacketSegmentFormat *format = (PacketSegmentFormat*)packet->buffer;
        format->segment = TRUE;
        format->turn = work->req_turn;
        format->recv_arg = work->req_index;
        format->send_arg = work->ack_index;
        if (work->ack_index != 0x7FFF)
        {
            u32     segment = work->own_packet;
            u32     offset = segment * work->ack_index;
            u32     rest = protocol->send.length - offset;
            MI_CpuCopy8((u8*)protocol->send.buffer + offset, format->buffer, MATH_MIN(segment, rest));
        }
    WXC_PACKET_LOG("--- [send] %d:F:%04X:%04X\n", format->turn, format->recv_arg, format->send_arg);
    }
    else
    {
        PacketCommandFormat *format = (PacketCommandFormat*)packet->buffer;
        format->segment = FALSE;
        format->ack = (u8)(work->ack_phase == work->req_phase);
        format->turn = work->req_turn;
        format->phase = (u8)work->req_phase;
        format->checksum = (u8)protocol->send.checksum;
        format->recv_arg = (u16)protocol->recv.buffer_max;
        format->send_arg = (u16)protocol->send.length;
    WXC_PACKET_LOG("--- [send] %d:%d:%04X:%04X\n", format->turn, format->phase, format->recv_arg, format->send_arg);
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
    BOOL    retval = FALSE;

    WXCCommonProtocolImplContext *work = (WXCCommonProtocolImplContext*)WXC_GetCurrentBlockImpl(protocol)->impl_work;

    PacketCommandFormat format;
    BOOL    ack = FALSE;
    BOOL    turn = !work->req_turn;
    u32     recv_arg;
    u32     send_arg;
    u16     checksum = 0;

    /* まず相手のコマンドを解析する */
    if (packet->length < sizeof(format))
    {
        OS_TWarning("not enough length.\n");
        return FALSE;
    }
    MI_CpuCopy8(packet->buffer, &format, sizeof(format));
    /* データセグメントパケット */
    if (format.segment)
    {
        PacketSegmentFormat segment;
        MI_CpuCopy8(packet->buffer, &segment, sizeof(segment));
        turn = segment.turn;
        recv_arg = segment.recv_arg;
        send_arg = segment.send_arg;
        work->ack_phase = ((recv_arg == 0x7FFF) && (send_arg == 0x7FFF)) ?
            PHASE_DONE : PHASE_DATA;
        WXC_PACKET_LOG("---                            [recv] %d:F:%04X:%04X\n", segment.turn, segment.recv_arg, segment.send_arg);
    }
    else
    {
        work->ack_phase = format.phase;
        ack = format.ack;
        turn = format.turn;
        recv_arg = format.recv_arg;
        send_arg = format.send_arg;
        checksum = format.checksum;
        WXC_PACKET_LOG("---                            [recv] %d:%d:%04X:%04X\n", format.turn, format.phase, format.recv_arg, format.send_arg);
    }

    /* turn が異なる場合, 自身が DONE 状態ならデータ交換完了 */
    if (turn != work->req_turn)
    {
        if (work->req_phase == PHASE_DONE)
        {
            retval = TRUE;
        }
    }
    /* turn が同じ場合, データ交換処理を継続 */
    else
    {
        switch (work->ack_phase)
        {
        case PHASE_INIT:
            if (work->req_phase != PHASE_INIT)
            {
                break;
            }
            /* 送受信サイズを保存 */
            protocol->send.buffer_max = recv_arg;
            protocol->recv.checksum = checksum;
            protocol->recv.length = send_arg;
            /* どちらかのバッファサイズを超える設定なら QUIT へ */
            if ((protocol->send.length > protocol->send.buffer_max) ||
                (protocol->recv.length > protocol->recv.buffer_max))
            {
                OS_TWarning("not enough buffer length for data exchange.\n");
                work->req_phase = PHASE_QUIT;
            }
            else if (!ack)
            {
                break;
            }
            /* 応答があれば DATA へ */
        case PHASE_DATA:
            /* データブロック交換開始準備 */
            if (work->req_phase == PHASE_INIT)
            {
                work->req_phase = PHASE_DATA;
                if (packet->length == work->child_send)
                {
                    work->own_packet = work->parent_send;
                    work->peer_packet = work->child_send;
                }
                else
                {
                    work->own_packet = work->child_send;
                    work->peer_packet = work->parent_send;
                }
                work->own_packet -= sizeof(PacketSegmentFormat);
                work->peer_packet -= sizeof(PacketSegmentFormat);
                work->req_index = 0;
                work->ack_index = 0;
                work->recv_count = 0;
                work->recv_total = (u32)((protocol->recv.length  + work->peer_packet
                                         - 1) / work->peer_packet);
                MI_CpuFill32(work->recv_bitset, 0x00, sizeof(work->recv_bitset));
            }
            if (work->ack_phase != PHASE_DATA)
            {
                break;
            }
            /* データブロック交換処理 */
            if (work->req_phase == PHASE_DATA)
            {
                /* 次回送信のために今回の要求番号を保存 */
                work->ack_index = recv_arg;
                /* 受信バッファをマージ */
                if (send_arg != 0x7FFF)
                {
                    u32     pos = (send_arg >> 5UL);
                    u32     bit = (send_arg & 31UL);
                    if ((work->recv_bitset[pos] & (1 << bit)) == 0)
                    {
                        const u8 *argument = &packet->buffer[sizeof(PacketSegmentFormat)];
                        u32     segment = work->peer_packet;
                        u32     offset = segment * send_arg;
                        u32     rest = protocol->recv.length - offset;
                        MI_CpuCopy8(argument, (u8*)protocol->recv.buffer + offset,
                                    MATH_MIN(segment, rest));
                        work->recv_bitset[pos] |= (1 << bit);
                        ++work->recv_count;
                    }
                }
                /* 転送未完なら次の要求インデックスを計算 */
                if (work->recv_count < work->recv_total)
				{
                    u32     index = work->req_index;
                    u32     pos = (index >> 5UL);
                    u32     bit = (index & 31UL);
                    for (;;)
                    {
                        if (++bit >= 32)
                        {
                            ++pos, bit = 0;
                            for (; work->recv_bitset[pos] == (u32)~0; ++pos)
                            {
                            }
                            bit = MATH_CTZ((u32)~work->recv_bitset[pos]);
                        }
                        if ((pos << 5UL) + bit >= work->recv_total)
                        {
                            pos = 0, bit = 0;
                        }
                        if ((work->recv_bitset[pos] & (1 << bit)) == 0)
                        {
                            break;
                        }
                    }
                    work->req_index = (pos << 5UL) + bit;
                }
                /* 転送完了なら要求インデックスをクローズ */
                else
                {
                    work->req_index = 0x7FFF;
                    /* 双方とも転送完了なら DONE へ */
                    if (work->ack_index == 0x7FFF)
                    {
                        work->req_phase = PHASE_DONE;
                    }
                }
            }
            break;
        case PHASE_DONE:
            if (work->req_phase == PHASE_DATA)
            {
                work->ack_index = recv_arg;
                /* 双方とも転送完了なら DONE へ */
                if ((work->ack_index == 0x7FFF) && (work->req_index == 0x7FFF))
                {
                    work->req_phase = PHASE_DONE;
                }
            }
            if (work->req_phase == PHASE_DONE)
            {
                retval = TRUE;
            }
            break;
        case PHASE_QUIT:
            /* こちらも QUIT へ */
            work->req_phase = PHASE_QUIT;
            /* 応答があれば通信終了可能 */
            if (ack)
            {
                work->executing = FALSE;
            }
            break;
        case PHASE_IDLE:
        default:
            /* 無視 */
            break;
        }
    }
    if (retval)
    {
#if defined(SDK_DEBUG)
        /* チェックサム確認 */
        u8      checksum = MATH_CalcChecksum8(protocol->recv.buffer, protocol->recv.length);
        if (protocol->recv.checksum != checksum)
        {
            OS_TWarning("checksum error!\n");
        }
#endif
#if defined(DEBUG_CALCSPEED)
        /* 計速 (送受信データのうち大きな方を対象とする) */
        if (OS_IsTickAvailable())
        {
            OSTick diff = OS_GetTick() - work->tick_count;
            u64    milli = OS_TicksToMilliSeconds(diff);
            u64    length = MATH_MAX(protocol->send.length, protocol->recv.length);
            WXC_PACKET_LOG("data-exchange : %d[B/s]\n", (length * 1000) / milli);
        }
#endif
        work->req_turn = !work->req_turn;
        work->req_phase = PHASE_QUIT;
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_GetProtocolImplCommon

  Description:  共通すれちがい通信プロトコルのインタフェースを取得。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl* WXCi_GetProtocolImplCommon(void)
{
    return &impl_common;
}


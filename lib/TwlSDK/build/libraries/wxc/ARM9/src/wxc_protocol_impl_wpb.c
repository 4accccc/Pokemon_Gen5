/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_protocol_impl_wpb.c

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
#include <nitro/wxc/wxc_protocol_impl_wpb.h>

#include <nitro/wxc.h>                 /* use WXC_IsParentMode() */


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

static void pass_copy_to_structure(u8 *buf, PassBuffer * pb);


/*****************************************************************************/
/* variable */

static WXCImplWorkWpb impl_work_wpb;

WXCProtocolImpl impl_wpb = {
    "WPB",
    WXCi_BeaconSendHook,
    WXCi_BeaconRecvHook,
    NULL,
    WXCi_PacketSendHook,
    WXCi_PacketRecvHook,
    WXCi_InitSequence,
    WXCi_AddData,
    WXCi_IsExecuting,
    &impl_work_wpb,
};


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         WXCi_GetProtocolImplWPB

  Description:  WPBすれちがい通信プロトコルのインタフェースを取得。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl* WXCi_GetProtocolImplWPB(void)
{
    return &impl_wpb;
}

/*---------------------------------------------------------------------------*
  Name:         PrintPassBuffer

  Description:  受信内容をデバッグ出力.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintPassBuffer(void *buf, const char *message)
{
#ifdef SDK_FINALROM
#pragma unused(message)
#endif

    static PassBuffer pass;
    pass_copy_to_structure((u8 *)buf, &pass);

    OS_TPrintf("%s(req=%02X,res=%02X)\n", message, pass.req_count, pass.res_count);
    if (pass.req_count == pass.req_count)
    {
        pass.req_count = pass.req_count;
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_init_recv_bitmap

  Description:  受信履歴ビットマップの初期化

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_data_init_recv_bitmap(PassCtrl * pass_ctrl)
{
    pass_ctrl->recv_bitmap = 0;
    pass_ctrl->recv_bitmap_index = 0;
}

/*---------------------------------------------------------------------------*
  Name:         pass_InitBuf

  Description:  PassBuffer構造体の初期化

  Arguments:    pb - PassBuffer構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_InitBuf(PassBuffer * pb)
{
    pb->res_count = REQUEST_NONE;
    pb->req_count = 0;
    MI_CpuClear8(pb->buf, PASS_BUFFER_SIZE);
}

/*---------------------------------------------------------------------------*
  Name:         pass_ResetData

  Description:  に呼び出される関数。

  Arguments:    None.

  Returns:      none.
 *---------------------------------------------------------------------------*/
static void pass_ResetData(WXCImplWorkWpb * wxc_work)
{
    PassCtrl *pc;
    pc = &wxc_work->pass_ctrl;

    pass_InitBuf(&(pc->send_buf));
    pass_InitBuf(&(pc->recv_buf));
    pass_data_init_recv_bitmap(pc);
    pc->pre_send_count = REQUEST_NONE;
    pc->reset_done = TRUE;
    pc->send_done = FALSE;
    pc->recv_done = FALSE;
    pc->hardware_buffer_count = 0;
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

    /* gameInfoに共通すれちがい情報を埋め込む処理
     *p_param->userGameInfo と p_param->userGameInfoLength を書き換え
     */

    static u16 gameInfo[2] ATTRIBUTE_ALIGN(32);
    u16     gameInfoLength = sizeof gameInfo;

    /* gameInfo の先頭 4 バイトは予約 (0) */
    MI_CpuClear16(gameInfo, sizeof gameInfo);

    if (gameInfoLength > WM_SIZE_USER_GAMEINFO)
    {
        SDK_ASSERT(FALSE);
    }

    if (gameInfoLength)
    {
        p_param->userGameInfo = gameInfo;
    }
    p_param->userGameInfoLength = gameInfoLength;
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

    BOOL    ret = FALSE;

    /* ここで共通すれちがいか判断   
     * userGameInfo が 4 バイト以上あり、最初の 4 バイトを u32 として
     * 最上位ビットが 0 なら共通すれ違いに対応
     */

    if (p_desc->gameInfoLength >=
        (char *)&p_desc->gameInfo.ggid - (char *)&p_desc->gameInfo
        + sizeof p_desc->gameInfo.ggid &&
        p_desc->gameInfo.userGameInfoLength >= 4 &&
        (*(u32 *)p_desc->gameInfo.userGameInfo & 1 << 31) == 0)
    {
        ret = TRUE;
    }

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_get_recv_bitmap

  Description:  受信履歴ビットマップをチェックする

  Arguments:    seq_no - シーケンス番号

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL pass_data_get_recv_bitmap(WXCProtocolContext * protocol, int seq_no)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    if (seq_no < pass_ctrl->recv_bitmap_index)
    {
        return TRUE;
    }
    if (seq_no >= pass_ctrl->recv_bitmap_index + 32)
    {
        return FALSE;
    }
    if (pass_ctrl->recv_bitmap & 1 << seq_no % 32)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_get_next_count

  Description:  次に相手に要求するシーケンス番号を得る

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static u16 pass_data_get_next_count(WXCProtocolContext * protocol)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pc = &wxc_work->pass_ctrl;

    int     count;

    if (pc->recv_bitmap_index >= wxc_work->my_pass_data.total_count)
    {
        return REQUEST_DONE;           /* すべて受信済み */
    }
    count = pc->pre_send_count;
    for (;;)
    {
        count++;
        if (count >= wxc_work->my_pass_data.total_count || count >= pc->recv_bitmap_index + 32)
        {
            count = pc->recv_bitmap_index;
        }
        if (!pass_data_get_recv_bitmap(protocol, count))
        {
            pc->pre_send_count = count;
            return (u16)count;
        }
        if (count == pc->pre_send_count)
        {
            /* ここに来ることはないはず */
            OS_TPrintf("Error ! %d %d %d %08X\n", pc->pre_send_count, pc->recv_bitmap_index,
                       wxc_work->my_pass_data.total_count, pc->recv_bitmap);
            return REQUEST_DONE;       /* すべて受信済み */
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_copy_to_buffer

  Description:  PassBuffer構造体からWM送信バッファにコピーする

  Arguments:    pb  - PassBuffer構造体
                buf - WM送信バッファ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_copy_to_buffer(PassBuffer * pb, u8 *buf)
{
    int     i;

    *buf++ = (u8)(((pb->req_count) >> 8) & 0xff);       /* HI */
    *buf++ = (u8)(pb->req_count & 0xff);        /* LO */

    *buf++ = (u8)(((pb->res_count) >> 8) & 0xff);       /* HI */
    *buf++ = (u8)(pb->res_count & 0xff);        /* LO */

    for (i = 0; i < PASS_BUFFER_SIZE; i++)
    {
        *buf++ = pb->buf[i];
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_DataToBuf

  Description:  ユーザー送信バッファからPassBuffer構造体にコピーする

  Arguments:    seq_no - シーケンス番号
                pb     - PassBuffer構造体
                buf    - ユーザー送信バッファ

  Returns:      None.    {TRUE, TRUE, FALSE, TRUE},

 *---------------------------------------------------------------------------*/
static void pass_DataToBuf(WXCProtocolContext * protocol, int seq_no, PassBuffer * pb,
                           PassData * pd)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    const u8 *src;
    u8     *dest;

    pb->res_count = (u16)seq_no;

    if (seq_no != REQUEST_DONE && seq_no != REQUEST_NONE && seq_no != REQUEST_BYE)
    {
        src = pd->user_send_data + (seq_no * wxc_work->send_unit);
        dest = pb->buf;
        if (seq_no == wxc_work->my_pass_data.total_count - 1)
        {
            int     mod = wxc_work->my_pass_data.size % wxc_work->send_unit;
            if (mod)
            {
                MI_CpuCopy8(src, dest, (u32)mod);
            }
            else
            {
                MI_CpuCopy8(src, dest, wxc_work->send_unit);
            }
        }
        else
        {
            MI_CpuCopy8(src, dest, wxc_work->send_unit);
        }
    }
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
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    /* WPB は親機として子機と通信する場合と、子機として親機と通信する場合で処理が異なる */
    if (WXC_IsParentMode())
    {
        int     send_size = 0;
        int     send_buf_count;
        u8     *send_buf = packet->buffer;

        /* 自局のリクエスト番号のセット */
        if (pass_ctrl->recv_done == TRUE)
        {
            if (pass_ctrl->reset_done)
            {
                pass_ctrl->send_buf.req_count = REQUEST_DONE;
            }
            else
            {
                pass_ctrl->send_buf.req_count = REQUEST_BYE;
            }
        }
        else
        {
            pass_ctrl->send_buf.req_count = pass_data_get_next_count(protocol);
            if (pass_ctrl->send_buf.req_count == REQUEST_DONE)
            {
                pass_ctrl->recv_done = TRUE;
            }
        }

        /* 相手のリクエストに対するデータのセット */
        if (pass_ctrl->send_done)
        {
            send_buf_count = REQUEST_NONE;
        }
        else
        {
            send_buf_count = pass_ctrl->recv_buf.req_count;
        }
        pass_DataToBuf(protocol, send_buf_count, &(pass_ctrl->send_buf), &wxc_work->my_pass_data);

        /* send_bufから送信バッファにコピー */
        pass_copy_to_buffer(&(pass_ctrl->send_buf), send_buf);

#ifdef DEBUG
        OS_TPrintf("parent send->%x req->%x\n", send_buf_count, pass_ctrl->send_buf.req_count);
#endif
        send_size = PASS_PACKET_SIZE;

        /* パケットサイズを指定 */
        packet->length = (u16)MATH_ROUNDUP(4 + wxc_work->send_unit, 2);

#ifdef DEBUG
        PrintPassBuffer(packet->buffer, "parent send");
#endif
    }
    else
    {
        int     peer_request;

        /* 自局のリクエスト番号をセット */
        if (pass_ctrl->recv_done == TRUE)
        {
            if (pass_ctrl->reset_done)
            {
                pass_ctrl->send_buf.req_count = REQUEST_DONE;
            }
            else
            {
                pass_ctrl->send_buf.req_count = REQUEST_BYE;
            }
        }
        else
        {
            pass_ctrl->send_buf.req_count = pass_data_get_next_count(protocol);
            if (pass_ctrl->send_buf.req_count == REQUEST_DONE)
            {
                pass_ctrl->recv_done = TRUE;
            }
        }

        /* 相手局のリクエスト番号に対してデータをセット */
        peer_request = (int)(pass_ctrl->recv_buf.req_count);
        pass_DataToBuf(protocol, peer_request, &(pass_ctrl->send_buf), &wxc_work->my_pass_data);

        /* send_bufから送信バッファにコピー */
        pass_copy_to_buffer(&(pass_ctrl->send_buf), packet->buffer);

#ifdef DEBUG
        OS_TPrintf("child send->%x req->%x\n", peer_request, pass_ctrl.send_buf.req_count);
#endif
        /* パケットサイズを指定 */
        packet->length = (u16)MATH_ROUNDUP(4 + wxc_work->send_unit, 2);
#ifdef DEBUG
        PrintPassBuffer(packet->buffer, "child send");
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_copy_to_structure

  Description:  WM受信バッファからPassBuffer構造体にコピーする

  Arguments:    buf - WM受信バッファ
                pb  - PassBuffer構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_copy_to_structure(u8 *buf, PassBuffer * pb)
{
    int     i;

    pb->req_count = (u16)(((u32)(*buf++)) << 8);        /* HI */
    pb->req_count += (u16)(*buf++);    /* LO */

    pb->res_count = (u16)(((u32)(*buf++)) << 8);        /* HI */
    pb->res_count += (u16)(*buf++);    /* LO */

    for (i = 0; i < PASS_BUFFER_SIZE; i++)
    {
        pb->buf[i] = *buf++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_set_recv_bitmap

  Description:  受信履歴ビットマップをセットする

  Arguments:    aid    - AID(AIDごとに受信バッファを管理しているため)
                seq_no - シーケンス番号

  Returns:      BOOL   - FALSE/すでにチェック済み
 *---------------------------------------------------------------------------*/
static BOOL pass_data_set_recv_bitmap(WXCProtocolContext * protocol, int seq_no)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    if (seq_no < pass_ctrl->recv_bitmap_index)
    {
        return FALSE;
    }
    if (seq_no >= pass_ctrl->recv_bitmap_index + 32)
    {
        return FALSE;
    }
    if (pass_ctrl->recv_bitmap & 1 << seq_no % 32)
    {
        return FALSE;
    }
    pass_ctrl->recv_bitmap |= 1 << seq_no % 32;
    while (pass_ctrl->recv_bitmap & 1 << pass_ctrl->recv_bitmap_index % 32)
    {
        pass_ctrl->recv_bitmap &= ~(1 << pass_ctrl->recv_bitmap_index++ % 32);
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         pass_BufToData

  Description:  PassBuffer構造体からユーザー受信バッファにコピーする

  Arguments:    pb  - PassBuffer構造体
                buf - ユーザー受信バッファ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_BufToData(WXCProtocolContext * protocol, PassBuffer * pb, PassCtrl * pctrl)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    int     res_count;
    u8     *src, *dest;

    res_count = (int)pb->res_count;
    src = pb->buf;

    if (pctrl->user_recv_buffer == NULL)
    {
        return;
    }

    dest = pctrl->user_recv_buffer + (res_count * PASS_BUFFER_SIZE);
    if (res_count == wxc_work->my_pass_data.total_count - 1)
    {
        int     mod = wxc_work->my_pass_data.size % wxc_work->recv_unit;
        if (mod)
        {
            MI_CpuCopy8(src, dest, (u32)mod);
        }
        else
        {
            MI_CpuCopy8(src, dest, wxc_work->recv_unit);
        }
    }
    else
    {
        MI_CpuCopy8(src, dest, wxc_work->recv_unit);
    }
}

static void disconnect_callback(PassCtrl * pass_ctrl)
{
    if (!(pass_ctrl->reset_done == FALSE && pass_ctrl->recv_done))
    {
        pass_ctrl->reset_done = FALSE;
        pass_ctrl->recv_done = TRUE;
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
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wpb_work->pass_ctrl;

    /* WPB は親機として子機と通信する場合と、子機として親機と通信する場合で処理が異なる */
    if (WXC_IsParentMode())
    {
        if (packet->buffer == NULL || packet->length == 0)
        {
            return FALSE;              /* 通信を継続する */
        }

        PrintPassBuffer(packet->buffer, "parent recv");

        /* まず受信バッファからpass_recv_bufにコピー */
        pass_copy_to_structure(((u8 *)packet->buffer), &(pass_ctrl->recv_buf));
        if (pass_ctrl->recv_buf.req_count == REQUEST_BYE)
        {
#ifdef DEBUG
            OS_TPrintf("parent : get REQUEST_BYE\n");
#endif
            wpb_work->executing = FALSE;        /* 通信を切断する */
            return TRUE;
        }
        if (pass_ctrl->reset_done == TRUE)
        {
            if (pass_ctrl->recv_done == FALSE)
            {
#ifdef DEBUG
                OS_TPrintf("parent recv->%x\n", pass_ctrl->recv_buf.res_count);
#endif
                if (pass_ctrl->recv_buf.res_count < wpb_work->my_pass_data.total_count)
                {
                    /* 受信履歴にチェックを入れる */
                    if (TRUE == pass_data_set_recv_bitmap(protocol, pass_ctrl->recv_buf.res_count))
                    {
                        /* 受信データをセーブ */
                        pass_BufToData(protocol, &(pass_ctrl->recv_buf), pass_ctrl);
                    }
                }
            }
            else
            {
                if (pass_ctrl->recv_buf.req_count == REQUEST_DONE)
                {
                    pass_ctrl->send_done = TRUE;
                }
                if (pass_ctrl->send_done == TRUE)
                {

                    /* こちらも送信データ終了の場合 */
                    if (pass_ctrl->hardware_buffer_count < (HARDWARE_BUFFER_DUMMY_COUNT * 2))
                    {
                        /* ４回くるまで保留 */
                        pass_ctrl->hardware_buffer_count++;
                        return FALSE;  /* 通信を継続する */
                    }

                    pass_ctrl->reset_done = FALSE;

                    /* 完了 */
                    return TRUE;
                }
            }
        }
        return FALSE;                  /* 通信を継続する */
    }
    else
    {
        PrintPassBuffer(packet->buffer, "child recv");

        /* 親機として子機と通信するプロトコル */
        if (packet->buffer == NULL || packet->length == 0)
        {
            return FALSE;              /* 通信を継続する */
        }

#ifdef DEBUG
        OS_TPrintf("child recv->%x\n", pass_ctrl->recv_buf.res_count);
#endif

        /* まず受信バッファからpass_recv_bufにコピー */
        pass_copy_to_structure(((u8 *)packet->buffer), &(pass_ctrl->recv_buf));
        if (pass_ctrl->recv_buf.req_count == REQUEST_BYE)
        {
#ifdef DEBUG
            OS_TPrintf("child: get REQUEST_BYE\n");
#endif
            wpb_work->executing = FALSE;        /* 通信を切断する */
            return TRUE;
        }
        if (pass_ctrl->reset_done == TRUE)
        {
            if (packet->length < PASS_PACKET_SIZE)
            {
                return FALSE;          /* 通信を継続する */
            }
            if (pass_ctrl->recv_done == FALSE)
            {
                if (pass_ctrl->recv_buf.res_count < wpb_work->my_pass_data.total_count)
                {
                    /* 受信履歴にチェックを入れる */
                    if (TRUE == pass_data_set_recv_bitmap(protocol, pass_ctrl->recv_buf.res_count))
                    {
                        pass_BufToData(protocol, &(pass_ctrl->recv_buf), pass_ctrl);
                    }
                }
            }
            else
            {
                if (pass_ctrl->recv_buf.req_count == REQUEST_DONE)
                {
                    pass_ctrl->send_done = TRUE;
                }
                if (pass_ctrl->send_done == TRUE)
                {

                    /* 自局も送信データ終了の場合 */
                    if (pass_ctrl->hardware_buffer_count < HARDWARE_BUFFER_DUMMY_COUNT)
                    {
                        /* ２回くるまで保留
                           pass_ctrl->hardware_buffer_count++;
                           return FALSE;        /* 通信を継続する */
                    }

                    pass_ctrl->reset_done = FALSE;

                    /* 完了 */
                    return TRUE;
                }
            }
        }
        return FALSE;                  /* 通信を継続する */
    }
    return FALSE;
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
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    /* 4 は WPB パケットヘッダの長さ.
     * このプロトコルに適用 (WPB は 512byte)
     * ここでは、ParentParam の値を書き換えないように注意!!
     */
    wpb_work->send_unit = (u16)(send_max - 4);  /* 512 - 4 = 508 のはず */
    wpb_work->recv_unit = (u16)(recv_max - 4);  /* 512 - 4 = 508 のはず */

    /* 受信管理情報の初期化 */
    MI_CpuClear32(wpb_work->recv_bitmap_buf, sizeof(wpb_work->recv_bitmap_buf));

    /* WPB 関係の初期化 */
    wpb_work->my_pass_data.total_count = 0;
    wpb_work->my_pass_data.size = 0;
    wpb_work->my_pass_data.user_send_data = NULL;

    wpb_work->executing = TRUE;
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
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    /* データバッファの初期化 */
    pass_ResetData(WXC_GetCurrentBlockImpl(protocol)->impl_work);

    /* protocol 共通メンバの 送受信バッファも関連付けておく */
    protocol->send.buffer = (void *)send_buf;
    protocol->send.length = (u16)send_size;
    protocol->send.checksum = MATH_CalcChecksum8(send_buf, send_size);
    protocol->recv.buffer = recv_buf;
    protocol->recv.buffer_max = (u16)recv_max;
    MI_CpuClear32(wpb_work->recv_bitmap_buf, sizeof(wpb_work->recv_bitmap_buf));

    /* WPB 側のバッファ設定 */
    wpb_work->my_pass_data.total_count =
        (int)(recv_max / PASS_BUFFER_SIZE) + ((recv_max % PASS_BUFFER_SIZE) ? 1 : 0);
    wpb_work->my_pass_data.size = (int)send_size;
    wpb_work->my_pass_data.user_send_data = send_buf;

    wpb_work->pass_ctrl.user_recv_buffer = recv_buf;

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IsExecuting

  Description:  現在データ交換中かを判定.

  Arguments:    None.

  Returns:      現在データ交換中ならば TRUE.
 *---------------------------------------------------------------------------*/
BOOL WXCi_IsExecuting(WXCProtocolContext * protocol)
{
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    return wpb_work->executing;
}

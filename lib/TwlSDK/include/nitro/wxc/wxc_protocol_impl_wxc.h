/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     wxc_protocol_impl_wxc.h

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_WXC_PROTOCOL_IMPL_WXC_H_
#define	NITRO_WXC_PROTOCOL_IMPL_WXC_H_

#include <nitro.h>


/*****************************************************************************/
/* constant */


#ifdef  __cplusplus
extern "C" {
#endif


// wxc 用の ImplWork 構造体
// 前までは protocol.h の WXCProtocolContext にあったもの
typedef struct
{
    WXCBlockInfo req;                  /* 自身からの要求 */
    WXCBlockInfo ack;                  /* 相手への応答 */

    u32     recv_total;                /* 受信パケット総数 */
    u32     recv_rest;                 /* 受信パケット残数 */

    /* 受信管理情報 */
    u32     recv_bitmap_buf[WXC_RECV_BITSET_SIZE];

    u16     send_unit;                 /* 送信パケット単位サイズ */
    u16     recv_unit;                 /* 受信パケット単位サイズ */

    /* 送信管理情報 */
    u16     recent_index[WXC_RECENT_SENT_LIST_MAX];

    BOOL    executing;
} WXCImplWorkWxc;

extern WXCProtocolImpl wxc_protocol_impl_wxc;


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_PROTOCOL_IMPL_WXC_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

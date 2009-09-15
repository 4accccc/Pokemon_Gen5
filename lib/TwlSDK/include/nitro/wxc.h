/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     wxc.h

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

#ifndef	NITRO_WXC_H_
#define	NITRO_WXC_H_


#include <nitro.h>

#include <nitro/wxc/common.h>
#include <nitro/wxc/protocol.h>
#include <nitro/wxc/driver.h>
#include <nitro/wxc/scheduler.h>

#include <nitro/wxc/wxc_protocol_impl_wxc.h>


/*****************************************************************************/
/* constant */

/* ライブラリに必要な内部ワークメモリのサイズ */
#define WXC_WORK_SIZE              0xA000


/*****************************************************************************/
/* function */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         WXC_Init

  Description:  ライブラリを初期化する.

  Arguments:    work          ライブラリ内部で使用するワークメモリ.
                              WXC_WORK_SIZE バイト以上で, かつ
                              32 BYTE アラインメントされている必要がある.
                callback      ライブラリの各種システム状態を通知する
                              コールバックへのポインタ.
                dma           ライブラリ内部で使用する DMA チャンネル.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_Init(void *work, WXCCallback callback, u32 dma);

/*---------------------------------------------------------------------------*
  Name:         WXC_Start

  Description:  ライブラリのワイヤレス駆動を開始する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_Start(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_Stop

  Description:  ライブラリのワイヤレス駆動を停止する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_Stop(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_End

  Description:  ライブラリを終了する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_End(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetStateCode

  Description:  現在のライブラリ状態を取得.

  Arguments:    None.

  Returns:      現在のライブラリ状態を示す WXCStateCode 型.
 *---------------------------------------------------------------------------*/
WXCStateCode WXC_GetStateCode(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_IsParentMode

  Description:  現在のワイヤレス状態が親機モードか判定.
                WXC_STATE_ACTIVE の状態に限り有効.

  Arguments:    None.

  Returns:      ワイヤレス状態が親機モードなら TRUE, 子機モードなら FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WXC_IsParentMode(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentParam

  Description:  現在のワイヤレス状態の WMParentParam 構造体を参照.
                WXC_STATE_ACTIVE かつ WXC_IsParentMode() が
                TRUE の状態に限り有効.

  Arguments:    None.

  Returns:      WMParentParam 構造体.
 *---------------------------------------------------------------------------*/
const WMParentParam *WXC_GetParentParam(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentBssDesc

  Description:  現在接続対象の WMBssDesc 構造体を参照.
                WXC_STATE_ACTIVE かつ WXC_IsParentMode() が
                FALSE の状態に限り有効.

  Arguments:    None.

  Returns:      WMBssDesc 構造体.
 *---------------------------------------------------------------------------*/
const WMBssDesc *WXC_GetParentBssDesc(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserBitmap

  Description:  現在接続中のユーザ状況をビットマップで取得.
                通信が確立されていない状況では 0 を返す.

  Arguments:    None.

  Returns:      WMBssDesc 構造体.
 *---------------------------------------------------------------------------*/
u16     WXC_GetUserBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentGgid

  Description:  現在の接続で選択されているGGIDを取得.
                通信が確立されていない状況では 0 を返す.
                現在の通信状態は WXC_GetUserBitmap() 関数の返り値で判定可能.

  Arguments:    None.

  Returns:      現在の接続で選択されているGGID.
 *---------------------------------------------------------------------------*/
u32     WXC_GetCurrentGgid(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetOwnAid

  Description:  現在の接続における自身のAIDを取得.
                通信が確立されていない状況では不定な値を返す.
                現在の通信状態は WXC_GetUserBitmap() 関数の返り値で判定可能.

  Arguments:    None.

  Returns:      現在の接続における自身のAID.
 *---------------------------------------------------------------------------*/
u16     WXC_GetOwnAid(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserInfo

  Description:  現在接続中のユーザ情報を取得.
                返されるポインタが指している情報の内容を変更してはならない.

  Arguments:    aid           情報を取得するユーザのAID.

  Returns:      指定されたAIDが有効ならばユーザ情報, そうでなければ NULL
 *---------------------------------------------------------------------------*/
const WXCUserInfo *WXC_GetUserInfo(u16 aid);

/*---------------------------------------------------------------------------*
  Name:         WXC_SetChildMode

  Description:  ワイヤレスを子機側としてしか起動させないよう設定.
                この関数は WXC_Start 関数より前に呼び出す必要がある.

  Arguments:    enable        子機側でしか起動させない場合は TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_SetChildMode(BOOL enable);

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
BOOL    WXC_SetParentParameter(u16 sendSize, u16 recvSize, u16 maxEntry);

/*---------------------------------------------------------------------------*
  Name:         WXC_RegisterDataEx

  Description:  交換用のデータを登録する

  Arguments:    ggid          登録データのGGID
                callback      ユーザーへのコールバック関数（データ交換完了時にコールバックされる）
                send_ptr      登録データのポインタ
                send_size     登録データのサイズ
                recv_ptr      受信バッファのポインタ
                recv_size     受信バッファのサイズ
                type          プロトコルを示す文字列

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_RegisterDataEx(u32 ggid, WXCCallback callback, u8 *send_ptr, u32 send_size,
                           u8 *recv_ptr, u32 recv_size, const char *type);

#define WXC_RegisterData(...) SDK_OBSOLETE_FUNCTION("WXC_RegisterData() is obsolete. use WXC_RegisterCommonData()")

/*---------------------------------------------------------------------------*
  Name:         WXC_RegisterCommonData

  Description:  交換用のデータを共通すれちがい通信仕様で登録する

  Arguments:    ggid          登録データのGGID
                callback      ユーザーへのコールバック関数（データ交換完了時にコールバックされる）
                send_ptr      登録データのポインタ
                send_size     登録データのサイズ
                recv_ptr      受信バッファのポインタ
                recv_size     受信バッファのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXC_RegisterCommonData(u32 ggid, WXCCallback callback,
                                          u8 *send_ptr, u32 send_size,
                                          u8 *recv_ptr, u32 recv_size)
{
    WXC_RegisterDataEx((u32)(ggid | WXC_GGID_COMMON_BIT), callback, send_ptr,
                             send_size, recv_ptr, recv_size, "COMMON");
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
void    WXC_SetInitialData(u32 ggid, u8 *send_ptr, u32 send_size, u8 *recv_ptr, u32 recv_size);

/*---------------------------------------------------------------------------*
  Name:         WXC_AddData

  Description:  完了したブロックデータ交換に追加でデータを設定

  Arguments:    send_buf      送信バッファ.
                send_size     送信バッファサイズ.
                recv_buf      受信バッファ.
                recv_max      受信バッファサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_AddData(const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max);

/*---------------------------------------------------------------------------*
  Name:         WXC_UnregisterData

  Description:  交換用のデータを登録から破棄する

  Arguments:    ggid          破棄するブロックに関連付けられた GGID

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_UnregisterData(u32 ggid);

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetSsid

  Description:  子機として接続する際の SSID を設定する.
                内部関数.

  Arguments:    buffer        設定すSSIDデータ.
                length        SSIDデータ長.
                              WM_SIZE_CHILD_SSID 以下である必要がある.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXCi_SetSsid(const void *buffer, u32 length);


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

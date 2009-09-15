/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_child.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MB_MB_CHILD_H_
#define NITRO_MB_MB_CHILD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb/mb.h>

/* ---------------------------------------------------------------------

        型定義

   ---------------------------------------------------------------------*/

//---------------------------------------------------------
// 子機ダウンロードの状態
//---------------------------------------------------------

typedef enum
{
    MB_COMM_CSTATE_NONE,               // 初期化前状態
    MB_COMM_CSTATE_INIT_COMPLETE,      // MB子機初期化完了状態
    MB_COMM_CSTATE_CONNECT,            // 親機への接続が完了した状態
    MB_COMM_CSTATE_CONNECT_FAILED,     // 親機への接続が失敗した状態
    MB_COMM_CSTATE_DISCONNECTED_BY_PARENT,      // 親機から切断された状態
    MB_COMM_CSTATE_REQ_ENABLE,         // MPが確立してデータリクエストが可能になった状態
    MB_COMM_CSTATE_REQ_REFUSED,        // リクエストに対して親機からKICKされた状態
    MB_COMM_CSTATE_DLINFO_ACCEPTED,    // 親機にリクエストが受理された状態
    MB_COMM_CSTATE_RECV_PROCEED,       // ダウンロードデータの受信開始状態
    MB_COMM_CSTATE_RECV_COMPLETE,      // ダウンロードデータの受信完了状態
    MB_COMM_CSTATE_BOOTREQ_ACCEPTED,   // 親機からのブート要求受信状態
    MB_COMM_CSTATE_BOOT_READY,         // 親機との通信を切断してブート完了した状態
    MB_COMM_CSTATE_CANCELED,           // 途中でキャンセルされ、親機との切断が完了した状態
    MB_COMM_CSTATE_CANCELLED = MB_COMM_CSTATE_CANCELED,
    MB_COMM_CSTATE_AUTHENTICATION_FAILED,       // 
    MB_COMM_CSTATE_MEMBER_FULL,        // 親機へのエントリー数が定員を超えていた場合の状態
    MB_COMM_CSTATE_GAMEINFO_VALIDATED, // 親機のビーコン受信状態
    MB_COMM_CSTATE_GAMEINFO_INVALIDATED,        // 既に取得していた親機のビーコンの状態が不完全な状態になった場合
    MB_COMM_CSTATE_GAMEINFO_LOST,      // 親機のビーコンを見失った状態の通知
    MB_COMM_CSTATE_GAMEINFO_LIST_FULL, // これ以上親機を取得できなくなった状態の通知
    MB_COMM_CSTATE_ERROR,              // 途中でエラーが発生した状態

    MB_COMM_CSTATE_FAKE_END,           // フェイク子機の完了通知(mb_fake_child使用時のみ遷移する状態)

    /*  内部使用の列挙値です。
       この状態には遷移しません。 */
    MB_COMM_CSTATE_WM_EVENT = 0x80000000
}
MBCommCState;

#define MBCommCStateCallback    MBCommCStateCallbackFunc
/*  子機イベント通知コールバック型 */
typedef void (*MBCommCStateCallbackFunc) (u32 status, void *arg);


/* ---------------------------------------------------------------------
        マルチブートライブラリ(MB)API - 子機用
   ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MB_SetRejectGgid

  Description:  受信を拒否する GGID を指定します。
                enable に TRUE を指定すると、配信親機の GGID と mask の論理積が
                ggid と mask の論理積が一致した場合に 配信親機のビーコンは受信しないようになります。

  Arguments:    ggid         除外対象の GGID
                mask         GGID のマスク
                enable       有効にするならば TRUE, 無効にするならば FALSE.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MB_SetRejectGgid(u32 ggid, u32 mask, BOOL enable);

#ifdef __cplusplus
}
#endif


#endif // NITRO_MB_MB_CHILD_H_

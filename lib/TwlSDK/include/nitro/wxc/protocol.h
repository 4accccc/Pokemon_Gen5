/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     protocol.h

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

#ifndef	NITRO_WXC_PROTOCOL_H_
#define	NITRO_WXC_PROTOCOL_H_


#include <nitro.h>
#include <nitro/wxc/common.h>


/*****************************************************************************/
/* constant */

/* コマンドタイプ */
#define WXC_BLOCK_COMMAND_IDLE      0  /* 要求/応答がない状態 */
#define WXC_BLOCK_COMMAND_INIT      1  /* データサイズとチェックサム送受信 */
#define WXC_BLOCK_COMMAND_SEND      2  /* データブロック送受信 */
#define WXC_BLOCK_COMMAND_STOP      3  /* 送受信キャンセル */
#define WXC_BLOCK_COMMAND_DONE      4  /* 送受信完了 */
#define WXC_BLOCK_COMMAND_QUIT      5  /* 通信終了要求 (切断) */

/* 1 回の MP パケットサイズ */
#define WXC_PACKET_BUFFRER_MIN      (int)(WXC_PACKET_SIZE_MIN - sizeof(WXCBlockHeader))
/* データ交換に使用する最大データ長 (32kB) */
#define WXC_MAX_DATA_SIZE           (32 * 1024)
#define WXC_RECV_BITSET_SIZE        ((((WXC_MAX_DATA_SIZE + WXC_PACKET_BUFFRER_MIN - 1) / WXC_PACKET_BUFFRER_MIN) + 31) / 32)

/* ブロックデータ応答時の重複防止用インデックス履歴 */
#define WXC_RECENT_SENT_LIST_MAX    2

/* 登録可能なデータブロックの総数 */
#define WXC_REGISTER_DATA_MAX       16


/*****************************************************************************/
/* declaration */

/* ブロックデータ交換の送受信情報 */
typedef struct WXCBlockInfo
{
    u8      phase;                     /* ブロックデータ交換のフェーズ */
    u8      command;                   /* 今回のフェーズでのコマンド */
    u16     index;                     /* 要求するシーケンス番号 */
}
WXCBlockInfo;

/* ブロックデータ交換のパケットヘッダ */
typedef struct WXCBlockHeader
{
    WXCBlockInfo req;                  /* 送信元からの要求 */
    WXCBlockInfo ack;                  /* 送信先への応答 */
}
WXCBlockHeader;

SDK_STATIC_ASSERT(sizeof(WXCBlockHeader) == 8);


/* ブロックデータ情報構造体 */
typedef struct WXCBlockDataFormat
{
    void   *buffer;                    /* バッファへのポインタ */
    u32     length;                    /* データサイズ */
    u32     buffer_max;                /* バッファサイズ (送信データなら length と一致) */
    u16     checksum;                  /* チェックサム (MATH_CalcChecksum8() を使用) */
    u8      padding[2];
}
WXCBlockDataFormat;


struct WXCProtocolContext;


/* WXC で対応可能な通信プロトコルのインタフェースを規定する構造体 */
typedef struct WXCProtocolImpl
{
    /* プロトコル種別を示す文字列 */
    const char *name;
    /* ビーコン送受信時のフック関数 */
    void    (*BeaconSend) (struct WXCProtocolContext *, WMParentParam *);
    BOOL    (*BeaconRecv) (struct WXCProtocolContext *, const WMBssDesc *);
    /* 接続前に呼び出されるフック関数 */
    void    (*PreConnectHook) (struct WXCProtocolContext *, const WMBssDesc *, u8 ssid[WM_SIZE_CHILD_SSID]);
    /* MPパケット送受信時のフック関数 */
    void    (*PacketSend) (struct WXCProtocolContext *, WXCPacketInfo *);
    BOOL    (*PacketRecv) (struct WXCProtocolContext *, const WXCPacketInfo *);
    /* 接続時の初期化関数 */
    void    (*Init) (struct WXCProtocolContext *, u16, u16);
    /* データブロック登録関数 */
    BOOL    (*AddData) (struct WXCProtocolContext *, const void *, u32, void *, u32);
    /* 現在データ交換中か判定する関数 */
    BOOL    (*IsExecuting) (struct WXCProtocolContext *);
    /* 必要に応じて各プロトコルで使用するワークエリアへのポインタ */
    void   *impl_work;
    /* 単方向リストとして管理するためのポインタ */
    struct WXCProtocolImpl *next;
}
WXCProtocolImpl;

/* 登録されたプロトコル動作を規定する構造体 */
typedef struct WXCProtocolRegistry
{
    /* 将来的に単方向リスト管理するためのポインタ */
    struct WXCProtocolRegistry *next;
    /* プロトコル情報 */
    u32     ggid;
    WXCCallback callback;
    WXCProtocolImpl *impl;
    /* 初期自動送受信用データ */
    WXCBlockDataFormat send;           /* 送信バッファ */
    WXCBlockDataFormat recv;           /* 受信バッファ */
}
WXCProtocolRegistry;


/* WXCライブラリの通信プロトコル制御情報 */
typedef struct WXCProtocolContext
{
    WXCBlockDataFormat send;           /* 送信バッファ */
    WXCBlockDataFormat recv;           /* 受信バッファ */
    /*
     * 交換用送受信データ情報.
     * ここは最終的に配列でなくリストにする.
     */
    WXCProtocolRegistry *current_block;
    WXCProtocolRegistry data_array[WXC_REGISTER_DATA_MAX];
    u32     target_ggid;
}
WXCProtocolContext;


/*****************************************************************************/
/* function */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         WXC_InitProtocol

  Description:  WXCライブラリプロトコルを初期化

  Arguments:    protocol      WXCProtocolContext 構造体.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_InitProtocol(WXCProtocolContext * protocol);

/*---------------------------------------------------------------------------*
  Name:         WXC_InstallProtocolImpl

  Description:  選択可能な新しいプロトコルを追加

  Arguments:    impl          WXCProtocolImpl構造体へのポインタ.
                              この構造体はライブラリ終了時まで内部で使用される.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_InstallProtocolImpl(WXCProtocolImpl * impl);

/*---------------------------------------------------------------------------*
  Name:         WXC_FindProtocolImpl

  Description:  指定の名前を持つプロトコルを検索

  Arguments:    name          プロトコル名.

  Returns:      指定の名前を持つプロトコルが存在すればそのポインタ.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl *WXC_FindProtocolImpl(const char *name);

/*---------------------------------------------------------------------------*
  Name:         WXC_ResetSequence

  Description:  WXCライブラリプロトコルを再初期化

  Arguments:    protocol      WXCProtocolContext 構造体.
                send_max      送信パケット最大サイズ.
                recv_max      受信パケット最大サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_ResetSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max);

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
void    WXC_AddBlockSequence(WXCProtocolContext * protocol,
                             const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentBlock

  Description:  現在選択されているブロックデータを取得.

  Arguments:    protocol      WXCProtocolContext 構造体.

  Returns:      現在選択されているブロックデータ.
 *---------------------------------------------------------------------------*/
static inline WXCProtocolRegistry *WXC_GetCurrentBlock(WXCProtocolContext * protocol)
{
    return protocol->current_block;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentBlockImpl

  Description:  現在選択されているブロックデータのプロトコルインタフェースを取得.

  Arguments:    protocol      WXCProtocolContext 構造体.

  Returns:      現在選択されているブロックデータ.
 *---------------------------------------------------------------------------*/
static inline WXCProtocolImpl *WXC_GetCurrentBlockImpl(WXCProtocolContext * protocol)
{
    return protocol->current_block->impl;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetCurrentBlock

  Description:  指定のブロックデータを選択する.

  Arguments:    protocol      WXCProtocolContext 構造体.
                target        選択するデータブロック.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXC_SetCurrentBlock(WXCProtocolContext * protocol, WXCProtocolRegistry * target)
{
    protocol->current_block = target;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_FindNextBlock

  Description:  GGID を指定してブロックデータを検索.

  Arguments:    protocol      WXCProtocolContext 構造体.
                from          検索開始位置.
                              NULL なら先頭から検索し, そうでなければ
                              指定されたブロックの次から検索する.
                ggid          検索するブロックデータに関連付けられた GGID.
                match         検索条件.
                              TRUE なら GGID の合致するものを検索対象とし,
                              FALSE なら 合致しないものを対象とする.

  Returns:      合致するブロックデータが存在すればそのポインタを,
                無ければ NULL を返す.
 *---------------------------------------------------------------------------*/
WXCProtocolRegistry *WXC_FindNextBlock(WXCProtocolContext * protocol,
                                       const WXCProtocolRegistry * from, u32 ggid, BOOL match);

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconSendHook

  Description:  ビーコン更新タイミングで呼び出されるフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_param       次回のビーコンに使用する WMParentParam 構造体.
                              この関数の内部で必要に応じて変更する.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param);

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconRecvHook

  Description:  スキャンされた各ビーコンに対して呼び出されるフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_desc        スキャンされた WMBssDesc 構造体

  Returns:      接続対象とみなせば TRUE を, そうでなければ FALSE を返す.
 *---------------------------------------------------------------------------*/
BOOL    WXC_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc);

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketSendHook

  Description:  MP パケット送信のタイミングで呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                packet        送信パケット情報を設定する WXCPacketInfo ポインタ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet);

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketRecvHook

  Description:  MP パケット受信のタイミングで呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                packet        受信パケット情報を参照する WXCPacketInfo ポインタ.

  Returns:      プロトコル的に相手が切断したら TRUE.
 *---------------------------------------------------------------------------*/
BOOL    WXC_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet);

/*---------------------------------------------------------------------------*
  Name:         WXC_ConnectHook

  Description:  通信対象の接続検知時に呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                bitmap        接続された通信先の AID ビットマップ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_ConnectHook(WXCProtocolContext * protocol, u16 bitmap);

/*---------------------------------------------------------------------------*
  Name:         WXC_DisconnectHook

  Description:  通信対象の切断検知時に呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                bitmap        切断された通信先の AID ビットマップ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_DisconnectHook(WXCProtocolContext * protocol, u16 bitmap);

/*---------------------------------------------------------------------------*
  Name:         WXC_CallPreConnectHook

  Description:  通信対象への接続前に呼び出すフック.

  Arguments:    protocol      WXCProtocolContext 構造体.
                p_desc        接続対象の WMBssDesc 構造体
                ssid          設定するためのSSIDを格納するバッファ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_CallPreConnectHook(WXCProtocolContext * protocol, WMBssDesc *p_desc, u8 *ssid);

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
void    WXC_InitProtocolRegistry(WXCProtocolRegistry * p_data, u32 ggid, WXCCallback callback,
                                 WXCProtocolImpl * impl);

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
void    WXC_SetInitialExchangeBuffers(WXCProtocolRegistry * p_data, u8 *send_ptr, u32 send_size,
                                      u8 *recv_ptr, u32 recv_size);

WXCProtocolImpl* WXCi_GetProtocolImplCommon(void);
WXCProtocolImpl* WXCi_GetProtocolImplWPB(void);
WXCProtocolImpl* WXCi_GetProtocolImplWXC(void);


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_PROTOCOL_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

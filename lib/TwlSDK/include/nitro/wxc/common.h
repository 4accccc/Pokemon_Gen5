/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     common.h

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

#ifndef	NITRO_WXC_COMMON_H_
#define	NITRO_WXC_COMMON_H_


#include <nitro.h>


/*****************************************************************************/
/* macro */

/*
 * デバッグ出力切り替え. (標準では無効になっています)
 * コマンドラインからこのシンボルで0(無効)または1(有効)に指定可能です.
 * 変更の反映には WXC ライブラリのリビルドが必要であることにご注意ください.
 */
#if !defined(WXC_DEBUG_OUTPUT)
#define     WXC_DEBUG_OUTPUT    0
#endif

/* 廃止予定 */
#define     WXC_PACKET_LOG      WXC_DEBUG_LOG
#define     WXC_DRIVER_LOG      WXC_DEBUG_LOG

/* コンパイル時正当性チェック (ライブラリ内部で使用) */
#if defined(SDK_COMPILER_ASSERT)
#define SDK_STATIC_ASSERT  SDK_COMPILER_ASSERT
#else
#define SDK_STATIC_ASSERT(expr) \
    extern void sdk_compiler_assert ## __LINE__ ( char is[(expr) ? +1 : -1] )
#endif


/*****************************************************************************/
/* constant */

typedef enum
{
    /* WXC_GetStatus 関数で取得可能な内部状態 */
    WXC_STATE_END,                     /* WXC_End 関数による終了処理完了 */
    WXC_STATE_ENDING,                  /* WXC_End 関数以降かつ 終了処理実行中 */
    WXC_STATE_READY,                   /* WXC_Init 関数以降かつ WXC_Start 関数以前 */
    WXC_STATE_ACTIVE,                  /* WXC_Start 関数以降でワイヤレスが有効な状態 */

    /* システムコールバックに通知される内部イベント */
    WXC_STATE_CONNECTED,               /* 子機接続 (親機側のみ発生, 引数はWMStartParentCallback) */
    WXC_STATE_EXCHANGE_DONE,           /* データ交換完了 (引数はユーザが指定した受信バッファ) */
    WXC_STATE_BEACON_RECV              /* ビーコン受信 (引数はWXCBeaconFoundCallback構造体) */
}
WXCStateCode;

/* 親機モードのビーコン設定 */
#define WXC_BEACON_PERIOD            90 /* [ms] */
#define WXC_PARENT_BEACON_SEND_COUNT_OUT (900 / WXC_BEACON_PERIOD)

/* ライブラリのMP送受信パケットサイズ */
#define WXC_PACKET_SIZE_MIN          20
#define WXC_PACKET_SIZE_MAX          WM_SIZE_MP_DATA_MAX

/* 共通すれちがい通信用の GGID */
#define WXC_GGID_COMMON_BIT          0x80000000UL
#define WXC_GGID_COMMON_ANY          (u32)(WXC_GGID_COMMON_BIT | 0x00000000UL)
#define WXC_GGID_COMMON_PARENT       (u32)(WXC_GGID_COMMON_BIT | 0x00400120UL)


/*****************************************************************************/
/* declaration */

/* WXC ライブラリのコールバック関数プロトタイプ */
typedef void (*WXCCallback) (WXCStateCode stat, void *arg);

/* 各AIDごとにWXCが保持しているユーザ情報 (WXC_GetUserInfo() で取得) */
typedef struct WXCUserInfo
{
    u16     aid;
    u8      macAddress[6];
}
WXCUserInfo;

/* MP 送受信パケットの操作に使用する構造体 */
typedef struct WXCPacketInfo
{
    /*
     * 送信用バッファまたは受信バッファ.
     * 送信時はこのバッファに実際の送信データを格納して返す.
     */
    u8     *buffer;
    /*
     * 送信可能最大サイズまたは受信データサイズ.
     * 送信時はここに実際の送信サイズを格納して返す.
     */
    u16     length;
    /*
     * 現在接続されている AID ビットマップ.
     * (ビット0 の有無で自身が親機か子機か判断可能)
     * 送信時はここに実際の送信対象を格納して返す.
     */
    u16     bitmap;
}
WXCPacketInfo;

/* WXC_STATE_BEACON_RECV コールバックの引数 */
typedef struct WXCBeaconFoundCallback
{
    /*
     * 指定のプロトコルに合致するフォーマットであれば TRUE,
     * そうでない場合には FALSE として与えられる.
     * このビーコンの送信元へ接続する場合はこのメンバを TRUE にして返す.
     */
    BOOL    matched;
    /*
     * 今回検出されたビーコン.
     */
    const WMBssDesc *bssdesc;
}
WXCBeaconFoundCallback;


/*****************************************************************************/
/* function */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
    WXC ユーティリティ
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
  Name:         WXC_DEBUG_LOG

  Description:  WXC ライブラリ内部のデバッグ出力関数.
                この関数は SDK_WEAK_SYMBOL で修飾されており,
                標準では OS_TPrintf と等価な処理になります.
                アプリケーション固有のデバッグシステムなどで
                この同名関数をオーバーライドすることができます.

  Arguments:    format        デバッグ出力内容を示す書式文字列
                ...           可変引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if (WXC_DEBUG_OUTPUT == 1)
void    WXC_DEBUG_LOG(const char *format, ...);
#else
#define     WXC_DEBUG_LOG(...) ((void)0)
#endif

/*---------------------------------------------------------------------------*
  Name:         WXC_GetWmApiName

  Description:  WM 関数の名前文字列を取得

  Arguments:    id          WM 関数の種類を示す WMApiid 列挙値

  Returns:      WM 関数の名前文字列.
 *---------------------------------------------------------------------------*/
const char *WXC_GetWmApiName(WMApiid id);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetWmErrorName

  Description:  WM エラーコードの名前文字列を取得

  Arguments:    err         WM エラーコードを示す WMErrCode 列挙値

  Returns:      WM エラーコードの名前文字列.
 *---------------------------------------------------------------------------*/
const char *WXC_GetWmErrorName(WMErrCode err);

/*---------------------------------------------------------------------------*
  Name:         WXC_CheckWmApiResult

  Description:  WM 関数の呼び出し返り値を判定し,
                エラーであれば詳細をデバッグ出力.

  Arguments:    id          WM 関数の種類
                err         関数から返されたエラーコード

  Returns:      正当な返り値であれば TRUE, そうでなければ FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WXC_CheckWmApiResult(WMApiid id, WMErrCode err);

/*---------------------------------------------------------------------------*
  Name:         WXC_CheckWmCallbackResult

  Description:  WM コールバックの返り値を判定し,
                エラーであれば詳細をデバッグ出力.

  Arguments:    arg         WM 関数のコールバックへ返された引数

  Returns:      正当な結果値であれば TRUE, そうでなければ FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WXC_CheckWmCallbackResult(void *arg);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetNextAllowedChannel

  Description:  指定されたチャンネルの次に使用可能なチャンネルを取得.
                最上位チャンネルの次は最下位チャンネルへループする.
                0 を指定すれば最下位のチャンネルを取得.

  Arguments:    ch            今回のチャンネル位置 (1から14, あるいは0)

  Returns:      次に使用可能なチャンネル. (1から14)
 *---------------------------------------------------------------------------*/
u16     WXC_GetNextAllowedChannel(int ch);

/*---------------------------------------------------------------------------*
  Name:         WXC_IsCommonGgid

  Description:  指定されたGGIDが共通すれちがい通信プロトコルGGIDか判定.

  Arguments:    gid          判定するGGID

  Returns:      共通すれちがい通信プロトコルGGIDであればTRUE.
 *---------------------------------------------------------------------------*/
SDK_INLINE
BOOL    WXC_IsCommonGgid(u32 ggid)
{
    return ((ggid & WXC_GGID_COMMON_BIT) != 0);
}


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_COMMON_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

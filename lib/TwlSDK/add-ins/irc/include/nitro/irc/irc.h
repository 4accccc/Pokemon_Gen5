/*---------------------------------------------------------------------------*
  Project:  TwlSDK - IRC - include
  File:     irc.h

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-17#$
  $Rev: 10767 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_IRC_IRC_H_
#define NITRO_IRC_IRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/irc/irc_internal.h>


#define IRC_DEBUG   (1) // 非デバッグ時は定義を消しておいてください


#ifdef IRC_DEBUG
#define PRINTF OS_Printf
#else
#define PRINTF TRUE ? (void)0 : OS_Printf
#endif

/*---------------------------------------------------------------------------*/
/* constants */

#define IRC_TRNS_MAX (180)   // 一度に送れる転送バイト数(旧カードの場合は 128 に変更する必要がある)

/*---------------------------------------------------------------------------*/
/* declarations */

// エラーコード
typedef enum
{
    IRC_ERROR_NONE,             // エラーなし
    //IRC_ERROR_NOTCONNECT,     // 非接続中
    IRC_ERROR_TIMEOUT,          // タイムアウトエラー
    //IRC_ERROR_CHECKSUM,       // チェックサムエラー->リトライをしてタイムアウトした場合のみエラー
    IRC_ERROR_NUM

} IRCError;


// コールバック関数の型定義
typedef void (*IRCConnectCallback)(u8 id, BOOL isSender);
typedef void (*IRCRecvCallback)(u8 *data, u8 size, u8 command, u8 unitId);
typedef void (*IRCShutdownCallback)(BOOL isError);

/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         IRC_Init

  Description:  ライブラリの初期化とユニット番号を設定するための関数

  Arguments:    unitId : 通信の識別に使われる機器 ID
                         任天堂から発行された機器 ID を指定してください

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Init(u8 unitId);

/*---------------------------------------------------------------------------*
  Name:         IRC_Move

  Description:  毎フレーム呼ぶプロセス関数

  Arguments:    None.

  Returns:      IRCError 型のエラー番号を返します
 *---------------------------------------------------------------------------*/
IRCError IRC_Move(void);

/*---------------------------------------------------------------------------*
  Name:         IRC_Send

  Description:  データを送信するための関数

  Arguments:    buf     : 送信したいデータの先頭アドレス
                          コマンドのみ送りたい場合は NULL を指定してください
                size    : 送信するデータのバイト数
                command : 通信内容を識別するためのコマンド番号
                          偶数のみ使用可能ですが、内容はアプリケーションで自由に設定してください

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Send(const u8 *buf, u8 size, u8 command);

/*---------------------------------------------------------------------------*
  Name:         IRC_SetConnectCallback

  Description:  接続コールバックを設定するための関数

  Arguments:    callback : 接続時に呼び出されるコールバック

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetConnectCallback(IRCConnectCallback callback);

/*---------------------------------------------------------------------------*
  Name:         IRC_SetRecvCallback

  Description:  データ受信コールバックを設定するための関数

  Arguments:    callback : データ受信時に呼び出されるコールバック

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetRecvCallback(IRCRecvCallback callback);

/*---------------------------------------------------------------------------*
  Name:         IRC_SetShutdownCallback

  Description:  切断コールバックを設定するための関数

  Arguments:    callback : 切断時に呼び出されるコールバック

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetShutdownCallback(IRCShutdownCallback callback);

/*---------------------------------------------------------------------------*
  Name:         IRC_Connect

  Description:  接続要求するための関数
                本関数を呼んだ方(この要求を受けた方と逆)がセンダとなります

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Connect(void);

/*---------------------------------------------------------------------------*
  Name:         IRC_IsConnect

  Description:  通信リンクが張れているかどうか確認するための関数

  Arguments:    None.

  Returns:      コネクションが張れ、通信可能であれば TRUE を
                そうでなければ FALSE を返します
 *---------------------------------------------------------------------------*/
BOOL IRC_IsConnect(void);

/*---------------------------------------------------------------------------*
  Name:         IRC_IsSender

  Description:  通信開始を行った端末(センダ側)かどうか調べるための関数
                (接続中のみ有効)

  Arguments:    None.

  Returns:      先に接続要求を出した側であれば TRUE を
                そうでなければ FALSE を返します
 *---------------------------------------------------------------------------*/
BOOL IRC_IsSender(void);

/*---------------------------------------------------------------------------*
  Name:         IRC_Shutdown

  Description:  通信処理を終了するための関数

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Shutdown(void);

/*---------------------------------------------------------------------------*
  Name:         IRC_Check

  Description:  新型 IRC カードかを判別するための関数

  Arguments:    None.

  Returns:      新型 IRC カードなら TRUE を
                そうでなければ FALSE を返します
 *---------------------------------------------------------------------------*/
BOOL IRC_Check(void);


// 動作には必須でない関数
/*---------------------------------------------------------------------------*
  Name:         IRC_SetUnitNumber

  Description:  ユニット番号を設定するための関数

  Arguments:    unitId : 通信の識別に使われる機器 ID
                         任天堂から発行された機器 ID を指定してください

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetUnitNumber(u8 unitId);

/*---------------------------------------------------------------------------*
  Name:         IRC_GetUnitNumber

  Description:  通信相手のユニット番号を確認するための関数

  Arguments:    None.

  Returns:      通信相手のユニット番号を返します(切断時は 0xFF)
 *---------------------------------------------------------------------------*/
u8 IRC_GetUnitNumber(void);

/*---------------------------------------------------------------------------*
  Name:         IRC_GetRetryCount

  Description:  トータルのリトライ回数を取得するための関数(デバッグ用)

  Arguments:    None.

  Returns:      リトライ回数を返します
 *---------------------------------------------------------------------------*/
u32 IRC_GetRetryCount(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NITRO_IRC_IRC_H_

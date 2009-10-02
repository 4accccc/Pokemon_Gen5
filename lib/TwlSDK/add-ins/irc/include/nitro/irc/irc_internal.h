/*---------------------------------------------------------------------------*
  Project:  TwlSDK - IRC - include
  File:     irc_interrnal.h

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
#ifndef NITRO_IRC_IRC_INTERNAL_H_
#define NITRO_IRC_IRC_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* constants */

#define IRC_SUM_VALUE       (0x80)  // チェックサム初期値

/*---------------------------------------------------------------------------*/
/* declarations */

typedef enum
{
    IRC_ST_DISCONNECT,
    IRC_ST_WAIT_RESPONSE,   // コネクトコマンドを発行してレスポンスを待っている状態(仮センダー)
    IRC_ST_WAIT_ACK,        // レスポンスを発行してACKを待っている状態(仮レシーバー)
    IRC_ST_SENDER,
    IRC_ST_RECEIVER,
    IRC_ST_NUM

} IRC_STATUS;

typedef struct
{
    u8    command;    // コマンド
    u8    unitId;     // UNIT_ID
    u8    sumL;       // チェックサム上位
    u8    sumH;       // チェックサム下位
    //u32 uniqueId;

} IRCHeader;

typedef struct
{
    IRC_STATUS status;
    BOOL flg_connect;

} IRCStatus;


// 通信コマンドリスト(システムコマンドのみ)
typedef enum
{
    // システムコマンド
    IR_RESET    = (0xF2),
    IR_SHUTDOWN = (0xF4),
    IR_RETRY    = (0xF6),

    IR_ACK      = (0xF8),   // ACK
    IR_RESPONSE = (0xFA),   // レスポンス
    IR_CONNECT  = (0xFC),   // コネクト
    IR_NUM

} IRCommand;



void IRCi_Init(void);
void IRCi_End(void);

/*---------------------------------------------------------------------------*
  Name:         IRCi_Read

  Description:  IRC からデータ読み込みを行う関数

  Arguments:    buf  : 受信バッファ

  Returns:      受信サイズ
 *---------------------------------------------------------------------------*/
u8 IRCi_Read(u8 *buf);

/*---------------------------------------------------------------------------*
  Name:         IRCi_Write

  Description:  IRC にそのままデータ送信を行う関数(ヘッダはIRC レイヤで作る)

  Arguments:    data  : 書き込むデータのアドレス
                size  : 書き込むデータのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRCi_Write(u8 *data, u8 size);

/*---------------------------------------------------------------------------*
  Name:         IRCi_Check

  Description:  IRC カードチェックを行う関数

  Arguments:    None.

  Returns:      新型 IRC なら 0xAA を返す
 *---------------------------------------------------------------------------*/
u8 IRCi_Check(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NITRO_IRC_IRC_INTERNAL_H_

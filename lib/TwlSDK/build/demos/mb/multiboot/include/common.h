/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     common.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#if !defined(NITRO_MB_DEMO_MULTIBOOT_DATA_H_)
#define NITRO_MB_DEMO_MULTIBOOT_DATA_H_


#include <nitro.h>
#include <nitro/mb.h>


/*
 * multiboot デモ全体の共通定義.
 */


/******************************************************************************/
/* constant */

/*
 * ゲームグループ ID.
 *
 * MBGameRegistry 構造体において個々のファイルへ
 * GGID を設定できるようになったため,
 * MB_Init() での指定は将来的に意味を成さなくなるかもしれません.
 */
#define SDK_MAKEGGID_SYSTEM(num)    (0x003FFF00 | (num))
#define MY_GGID         SDK_MAKEGGID_SYSTEM(0x20)

/*
 * 親機が配信するチャンネル.
 *
 * マルチブート子機は全ての可能なチャンネルを巡回しますので
 * WM ライブラリが許す値(現状 1, 7, 13)のいずれかを使用するようにしてください.
 *
 * ユーザアプリケーションにおいて通信の混雑を避ける場合は可変にします.
 * チャンネル変更のタイミングについてはアプリケーションの裁量になりますが,
 * 例えば, レスポンスの悪さに対しユーザが「再スタート」を押すなどの
 * 契機を設けるといった方法が考えられます.
 */
#define PARENT_CHANNEL  13

/* 表示する子機リスト最大数 */
#define DISP_LIST_NUM   (15)

/* MB ライブラリに割り当てる DMA 番号 */
#define MB_DMA_NO       2

/* このデモが登録するゲーム総数 */
#define GAME_PROG_MAX   2

/* このデモが提供するマルチブートゲームプログラム情報 */
extern const MBGameRegistry mbGameList[GAME_PROG_MAX];

/* このデモのゲームシーケンス状態値 */
enum
{
    STATE_NONE,
    STATE_MENU,
    STATE_MB_PARENTINIT,
    STATE_MB_PARENT
};


/******************************************************************************/
/* variable */

/* MB ライブラリへ割り当てるワーク領域 */
extern u32 cwork[MB_SYSTEM_BUF_SIZE / sizeof(u32)];

/* このデモのゲームシーケンス状態 */
extern u8 prog_state;


/******************************************************************************/
/* function */

/* キートリガ検出 */
u16     ReadKeySetTrigger(u16 keyset);

/* min - max の範囲でval値を offset分ローテーション */
BOOL    RotateU8(u8 *val, u8 min, u8 max, s8 offset);

/* 親機初期化 */
void    ParentInit(void);

/* 1 フレームごとの親機メイン処理 */
void    ParentMain(void);

/* ファイルバッファポインタを初期化 */
void    InitFileBuffer(void);

/* 親機MP送信サイズをセット */
void    SetParentSendSize(u16 p_size);

/* 親機MP送信サイズを取得 */
u16     GetParentSendSize(void);

/* 子機最大接続数をセット */
void    SetMaxEntry(u8 num);

/* 子機最大接続数を取得 */
u8      GetMaxEntry(void);

/* 親機のオート操作設定 */
void    SetAutoOperation(u8 fAccept, u8 fSend, u8 fBoot);

#endif /* !defined(NITRO_MB_DEMO_MULTIBOOT_DATA_H_) */

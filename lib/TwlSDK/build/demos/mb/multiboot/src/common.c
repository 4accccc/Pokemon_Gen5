/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     common.c

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

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif


#include "common.h"


/*
 * このデモ全体で使用する共通機能.
 */


/******************************************************************************/
/* constant */

/* このデモがダウンロードさせるプログラム情報の配列 */
const MBGameRegistry mbGameList[GAME_PROG_MAX] = {
    {
     "/em.srl",                        // マルチブート子機バイナリのファイルパス
     L"edgeDemo",                      // ゲーム名
     L"edgemarking demo\ntesttesttest", // ゲーム内容説明
     "/data/icon.char",                // アイコンキャラクタデータのファイルパス
     "/data/icon.plt",                 // アイコンパレットデータのファイルパス
     0x12123434,                       // GameGroupID(GGID)
     16,                               // 最大プレイ人数
     },
    {
     "/pol_toon.srl",
     L"PolToon",
     L"toon rendering",
     "/data/icon.char",
     "/data/icon.plt",
     0x56567878,
     8,
     },
};


/******************************************************************************/
/* variable */

/* MB ライブラリへ割り当てるワーク領域 */
u32     cwork[MB_SYSTEM_BUF_SIZE / sizeof(u32)];

/* このデモのゲームシーケンス状態 */
u8      prog_state;


/******************************************************************************/
/* function */

/* キートリガ検出 */
u16 ReadKeySetTrigger(u16 keyset)
{
    static u16 cont = 0xffff;          /* IPL 内での A ボタン押下対策 */
    u16     trigger = (u16)(keyset & (keyset ^ cont));
    cont = keyset;
    return trigger;
}

/* min - max の範囲でval値を offset分ローテーション */
BOOL RotateU8(u8 *val, u8 min, u8 max, s8 offset)
{
    int     lVal = (int)*val - min;
    int     div = max - min + 1;

    if (div == 0)
        return FALSE;

    lVal = (lVal + offset + div) % div + min;

    *val = (u8)lVal;
    return TRUE;
}

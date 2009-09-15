/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-pm
  File:     user.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#include <nitro/wxc.h>
#include "user.h"
#include "common.h"
#include "dispfunc.h"


/*****************************************************************************/
/* constant */

/* テスト用の GGID */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x53)
#define GGID_ORG_2                            SDK_MAKEGGID_SYSTEM(0x54)

/* テスト用送受信データサイズ */
#define DATA_SIZE (1024 * 20)


/*****************************************************************************/
/* variable */

/* 交換成功回数 */
static int data_exchange_count = 0;

/* テスト用データバッファ */
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

static u8 send_data2[DATA_SIZE];
static u8 recv_data2[DATA_SIZE];

extern int passby_endflg;
extern BOOL passby_ggid[MENU_MAX];     // すれちがい通信をするGGID

/*---------------------------------------------------------------------------*
  Name:         CalcHash

  Description:  簡易チェック用のハッシュ計算

  Arguments:    src           計算対象のバッファ

  Returns:      計算値.
 *---------------------------------------------------------------------------*/
static u8 CalcHash(const u8 *src)
{
    int     i;
    u8      sum = 0;
    for (i = 0; i < DATA_SIZE - 1; i++)
    {
        sum ^= src[i];
    }
    return sum;
}

/*---------------------------------------------------------------------------*
  Name:         user_callback

  Description:  データ交換完了コールバック

  Arguments:    stat          通知ステータス. (常に WXC_STATE_EXCHANGE_DONE)
                arg           通知引数. (受信データバッファ)

  Returns:      計算値.
 *---------------------------------------------------------------------------*/
static void user_callback(WXCStateCode stat, void *arg)
{
#pragma unused(stat)

    const WXCBlockDataFormat * block = (const WXCBlockDataFormat *)arg;
    u8      sum = 0;
    u8     *data = (u8 *)block->buffer;

    ++data_exchange_count;

    /* debug 受信データの表示 */
    sum = CalcHash(data);

    if (sum == data[DATA_SIZE - 1])
    {
        BgPrintf((s16)1, (s16)5, WHITE, "sum OK 0x%02X %6d %6d sec", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
        BgPrintf((s16)1, (s16)6, WHITE, "%s", data);
    }
    else
    {
        BgPrintf((s16)1, (s16)5, WHITE, "sum NG 0x%02X %6d %6d sec", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
    }
    MI_CpuClear32(data, DATA_SIZE);
    
    // 一回通信したら終了するようにする
    
    if( WXC_IsParentMode() == TRUE)
    {
        passby_endflg = 1;
    }
    else
    {
        passby_endflg = 2;
    }
}


/*---------------------------------------------------------------------------*
  Name:         system_callback

  Description:  WXC ライブラリのシステムコールバック

  Arguments:    stat          通知ステータス. (常に WXC_STATE_EXCHANGE_DONE)
                arg           通知引数. (受信データバッファ)

  Returns:      計算値.
 *---------------------------------------------------------------------------*/
static void system_callback(WXCStateCode state, void *arg)
{
    switch (state)
    {
    case WXC_STATE_READY:
        /*
         * WXC_Init 関数呼び出しの内部から発生.
         * arg は常に NULL.
         */
        break;

    case WXC_STATE_ACTIVE:
        /*
         * WXC_Start 関数呼び出しの内部から発生.
         * arg は常に NULL.
         */
        break;

    case WXC_STATE_ENDING:
        /*
         * WXC_End 関数呼び出しの内部から発生.
         * arg は常に NULL.
         */
        break;

    case WXC_STATE_END:
        /*
         * WXC_End 関数による終了処理完了時に発生.
         * arg は WXC_Init 関数で割り当てた内部ワークメモリ.
         * この時点でワークメモリはユーザに解放されています.
         */
        {
            void *system_work = arg;
            OS_Free(system_work);
        }
        break;

    case WXC_STATE_EXCHANGE_DONE:
        /*
         * データ交換完了. (現状は発生しません)
         */
        break;
    }
}


void User_Init(void)
{
    /* ライブラリ内部状態初期化 */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /*
     * データブロックの登録.
     * 現状, 送受信データサイズは自他とも同じとする.
     */
    // フラグが立ってれば登録
    if(passby_ggid[0] == TRUE)
    {
        (void)OS_SPrintf((char *)send_data, "---------------------");
        send_data[DATA_SIZE - 2] = (u8)GGID_ORG_1;
        send_data[DATA_SIZE - 1] = CalcHash(send_data);
        WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);
    }
    
    /*
     * テストとしてもう1個データブロック登録.
     * 現仕様では各々のデータブロックを時分割で選択し,
     * そのときに同一GGIDを持つ子機とデータ交換する.
     */
    // フラグが立ってれば登録
    
    if(passby_ggid[1] == TRUE)
    {
        (void)OS_SPrintf((char *)send_data2, "/////////////////////");
        send_data2[DATA_SIZE - 2] = (u8)GGID_ORG_2;
        send_data2[DATA_SIZE - 1] = CalcHash(send_data2);
        WXC_RegisterCommonData(GGID_ORG_2, user_callback, send_data2, DATA_SIZE, recv_data2, DATA_SIZE);
    }
    
    /* ライブラリのワイヤレスを起動 */
    WXC_Start();
}

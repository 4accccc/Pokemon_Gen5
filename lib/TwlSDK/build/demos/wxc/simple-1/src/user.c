/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - simple-1
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
#include "font.h"
#include "user.h"
#include "print.h"

static int data_exchange_count = 0;

/**** デバッグ用データバッファ ****/
#define DATA_SIZE 1024*20
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

/* テスト用の GGID */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x51)

extern u16 gScreen[32 * 32];           // 仮想スクリーン
u16 temp_gScreen[32 * 32];           // 仮想スクリーン


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

/* すれちがい通信でデータ交換が完了したときにかかるコールバック関数 */
static void user_callback(WXCStateCode stat, void *ptr)
{
#pragma unused(stat)

    const WXCBlockDataFormat * block = (const WXCBlockDataFormat *)ptr;
    u8      sum = 0;
    u8     *data = (u8 *)block->buffer;

    ++data_exchange_count;

    //gScreenバッファを一列ずらす
    MI_CpuClearFast((void *)temp_gScreen, sizeof(temp_gScreen));
    MI_CpuCopyFast((void *)(&gScreen[(5 * 32)]), (void *)(&temp_gScreen[(7 * 32)]), sizeof(u16)*32*14);
    MI_CpuCopyFast((void *)temp_gScreen, (void *)gScreen, sizeof(gScreen));

    /* debug 受信データの表示 */
    sum = CalcHash(data);

    if (sum == data[DATA_SIZE - 1])
    {
        PrintString(1, 5, 0x1, "sum OK 0x%02X %6d %6d sec", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
        PrintString(1, 6, 0x1, "%s", data);
    }
    else
    {
        PrintString(1, 5, 0x1, "sum NG 0x%02X sum 0x0%02X %6d sec", sum, data[DATA_SIZE - 1],
                OS_TicksToSeconds(OS_GetTick()));
    }
    MI_CpuClear32(data, DATA_SIZE);
    
    // 一回通信したら終了するようにする
    (void)WXC_End();
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
    u8      MacAddress[6];

    /* ライブラリ内部状態初期化 */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /* 時局データのセット */
    OS_GetMacAddress(MacAddress);
    (void)OS_SPrintf((char *)send_data, "data %02X:%02X:%02X:%02X:%02X:%02X",
                  MacAddress[0], MacAddress[1], MacAddress[2],
                  MacAddress[3], MacAddress[4], MacAddress[5]);

    send_data[DATA_SIZE - 1] = CalcHash(send_data);

    /* 自局データの登録 */
    /* 相手局データ受信用のバッファ、データサイズは自局データサイズと同じ */
    WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);
    
    /* ライブラリのワイヤレスを起動 */
    WXC_Start();
}


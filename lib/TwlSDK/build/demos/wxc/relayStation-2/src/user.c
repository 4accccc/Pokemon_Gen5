/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - relayStation-2
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
#include "dispfunc.h"


/*****************************************************************************/
/* variable */

/* 中継所がリレーモードなら TRUE */
BOOL    station_is_relay;

/* 交換用データバッファ */
u8      send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

/* 上記送信データの元の所有者 */
u8      send_data_owner[6];

/* 交換成功回数 */
static int data_exchange_count = 0;


/*****************************************************************************/
/* function */

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

    /* 引数には受信データの情報が与えられる */
    const WXCBlockDataFormat *recv = (const WXCBlockDataFormat *)arg;

    ++data_exchange_count;

    /*
     * 中継所がリレーモードなら, 送信すべきデータを
     * たったいま受信したデータで再登録する.
     */
    if (station_is_relay)
    {
        /* 受信相手の MAC アドレスを保存 */
        const WXCUserInfo *info = WXC_GetUserInfo((u16)((WXC_GetOwnAid() == 0) ? 1 : 0));
        MI_CpuCopy8(info->macAddress, send_data_owner, 6);
        /* 交換用データを再登録 */
        MI_CpuCopy8(recv->buffer, send_data, sizeof(send_data));
        MI_CpuClear32(recv->buffer, DATA_SIZE);
        WXC_SetInitialData(GGID_ORG_1, send_data, DATA_SIZE, recv_data, DATA_SIZE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         system_callback

  Description:  WXC ライブラリのシステムコールバック

  Arguments:    stat          通知ステータス.
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
            void   *system_work = arg;
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

/*---------------------------------------------------------------------------*
  Name:         User_Init

  Description:  WXC ライブラリに関するユーザ側初期化処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void User_Init(void)
{
    /* WXCライブラリ内部状態初期化 */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /* 中継所モードでは子機にしかならない */
    WXC_SetChildMode(TRUE);

    /*
     * 初期データブロックの登録.
     * これ以降はリレーモードで受信した場合以外
     * 登録データが更新されることは無い.
     */
    OS_GetMacAddress(send_data_owner);
    (void)OS_SPrintf((char *)send_data, "data %02X:%02X:%02X:%02X:%02X:%02X",
                     send_data_owner[0], send_data_owner[1], send_data_owner[2],
                     send_data_owner[3], send_data_owner[4], send_data_owner[5]);
    send_data[DATA_SIZE - 1] = CalcHash(send_data);
    WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);

    /* WXCライブラリのワイヤレスを起動 */
    WXC_Start();
}

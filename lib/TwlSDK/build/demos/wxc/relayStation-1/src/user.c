/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - relayStation-1
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

/* テスト用送受信データサイズ */
#define DATA_SIZE (1024 * 20)

/* 中継所で対応する GGDI 一覧 */
static u32 ggid_list[] =
{
    GGID_ORG_1,
    GGID_ORG_2,
	0,
} ;


/*****************************************************************************/
/* variable */

/* 交換成功回数 */
static int data_exchange_count = 0;

/* 送受信用データバッファ */
static u8 send_data[2][DATA_SIZE];
static u8 recv_data[2][DATA_SIZE];


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
  Name:         CreateData

  Description:  送信用データを作成.

  Arguments:    buf           データ格納先.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CreateData(void *buf)
{
	*(OSTick*)buf = OS_GetTick();
	OS_GetMacAddress((u8*)buf + sizeof(OSTick));
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
    u8     *recv = (u8 *)block->buffer;

    ++data_exchange_count;

    /* 受信データの表示 */
	{
	    const u8 *dst_mac = recv + sizeof(OSTick);
		u8      sum = CalcHash(recv);
		if (sum == recv[DATA_SIZE - 1])
		{
			BgPrintf((s16)1, (s16)5, WHITE, "sum OK 0x%02X %6d %6d sec", sum, data_exchange_count,
					OS_TicksToSeconds(OS_GetTick()));
			BgPrintf((s16)1, (s16)6, WHITE, "  (from %02X:%02X:%02X:%02X:%02X:%02X)\n\n",
						dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		}
		else
		{
			BgPrintf((s16)1, (s16)5, WHITE, "sum NG 0x%02X %6d %6d sec", sum, data_exchange_count,
					OS_TicksToSeconds(OS_GetTick()));
		}
	}

    /* 次回の交換用データバッファ設定 */
    {
		/* 対応する GGID ごとに独立してバッファを管理 */
		const u32 ggid = WXC_GetCurrentGgid();
		int index = 0;
		for ( index = 0 ; (ggid_list[index]|WXC_GGID_COMMON_BIT) != ggid ; ++index )
		{
		}
		/*
		 * 今回受信したデータについて,
		 * ・次回の送信に使うか(中継モード)
		 * ・ここで読み捨てるか(配信モード)
		 * 選択する.
		 */
		if (keep_flg)
		{
			CreateData(send_data[index]);
		}
		else
		{
		    MI_CpuCopy8(recv, send_data[index], sizeof(send_data[index]));
		}
		/*
		 * データの再設定.
		 * WXC_RegisterCommonData() の送信データに NULL を指定しておき
		 * CONNECTED の通知段階で WXC_AddData() する方法と同じ.
		 */
		send_data[index][DATA_SIZE - 2] = (u8)ggid;
		send_data[index][DATA_SIZE - 1] = CalcHash(send_data[index]);
		MI_CpuClear32(recv, DATA_SIZE);
		WXC_SetInitialData(ggid, send_data[index], DATA_SIZE, recv_data[index], DATA_SIZE);
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
	int	index;

    /* ライブラリ内部状態初期化 */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /* 中継所モードでは子機にしかならない */
    WXC_SetChildMode(TRUE);

    /* 各 GGID ごとの初回データブロック登録 */
    for ( index = 0 ; ggid_list[index] ; ++index )
    {
        if(passby_ggid[index])
        {
            CreateData(send_data[index]);
            send_data[index][DATA_SIZE - 2] = (u8)ggid_list[index];
            send_data[index][DATA_SIZE - 1] = CalcHash(send_data[index]);
            WXC_RegisterCommonData(ggid_list[index], user_callback, send_data[index], DATA_SIZE, recv_data[index], DATA_SIZE);
        }
    }

    /* ライブラリのワイヤレスを起動 */
    WXC_Start();
}

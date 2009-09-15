/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - unregister-1
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
#include "font.h"
#include "text.h"


/*****************************************************************************/
/* constant */

/* テスト用の GGID */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x55)

/* テスト用送受信データサイズ */
#define DATA_SIZE (1024 * 20)


/*****************************************************************************/
/* variable */

/* 交換成功回数 */
static int data_exchange_count = 0;

/* テスト用データバッファ */
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];


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
    u8      sum = 0;
    u8     *data = (u8 *)block->buffer;

	SDK_ASSERT(stat == WXC_STATE_EXCHANGE_DONE);
    ++data_exchange_count;

    /* debug 受信データの表示 */
    sum = CalcHash(data);

    if (sum == data[DATA_SIZE - 1])
    {
        mprintf("sum OK 0x%02X %6d %6d sec\n", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));

		/* 受信したデータが自分のものか再送か判定 */
		{
			u8 mac[6];
			const u8 *dst_mac = data + sizeof(OSTick);
			int i;

			OS_GetMacAddress(mac);
			for ( i = 0 ; (i < sizeof(mac)) && (dst_mac[i] == mac[i]) ; ++i )
			{
			}
			/* 自分が以前に送ったデータなら, 再度データ生成 */
			if(i >= sizeof(mac))
			{
				mprintf("  (own data before %6d sec)\n",
					OS_TicksToSeconds(OS_GetTick() - *(OSTick*)data));
				CreateData(send_data);
			}
			/* 新しい相手のデータならそのまま返信 */
			else
			{
				mprintf("  (from %02X:%02X:%02X:%02X:%02X:%02X)\n\n",
					dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
				MI_CpuCopy8(data, send_data, sizeof(send_data));
			}
			/*
			 * データを Unregister して再度 Register するために一時停止.
			 * 現在のライブラリでは WXC_SetInitialData() を使用することによって
			 * ワイヤレスを一時停止せずともこの場で同等の処理が可能だが,
			 * 本サンプルは WXC_UnregisterData() の使用方法を示すものなので
			 * あえてこの方法を採用している.
			 */
			WXC_Stop();
		}
    }
    else
    {
        mprintf("sum NG 0x%02X sum 0x0%02X %6d sec\n", sum, data[DATA_SIZE - 1],
                OS_TicksToSeconds(OS_GetTick()));
    }

    MI_CpuClear32(data, DATA_SIZE);
}

/*---------------------------------------------------------------------------*
  Name:         system_callback

  Description:  WXC ライブラリのシステムコールバック

  Arguments:    stat          通知ステータス.
                arg           通知引数. (通知ステータスによって意味が異なる)

  Returns:      計算値.
 *---------------------------------------------------------------------------*/
static void system_callback(WXCStateCode state, void *arg)
{
	switch (state)
	{
	case WXC_STATE_READY:
		/*
		 * WXC_Init 関数呼び出しの内部から、またはWXC_Stop関数の完了時点で発生.
		 * arg は常に NULL.
		 */

		/* 既存の登録データがあればここで破棄 */
		if (data_exchange_count > 0)
		{
			WXC_UnregisterData(GGID_ORG_1);
		}
		/* 次回の交換用データを新たに登録 */
		send_data[DATA_SIZE - 2] = (u8)GGID_ORG_1;
		send_data[DATA_SIZE - 1] = CalcHash(send_data);
		WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);
		/* ライブラリのワイヤレスを起動 */
		WXC_Start();
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

	case WXC_STATE_CONNECTED:
		/*
		 * 新規の接続が発生した場合に発生.
		 * arg は接続対象を示す WXCUserInfo 構造体へのポインタ.
		 */
		{
			const WXCUserInfo * user = (const WXCUserInfo *)arg;
			OS_TPrintf("connected(%2d):%02X:%02X:%02X:%02X:%02X:%02X\n",
				user->aid,
				user->macAddress[0], user->macAddress[1], user->macAddress[2],
				user->macAddress[3], user->macAddress[4], user->macAddress[5]);
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
    /* 初回の交換用データを初期化 */
    data_exchange_count = 0;
    CreateData(send_data);
    /* ライブラリ内部状態初期化 */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);
}

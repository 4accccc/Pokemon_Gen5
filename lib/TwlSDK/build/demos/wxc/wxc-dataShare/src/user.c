/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-dataShare
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
#include "disp.h"
#include "gmain.h"
#include "wh.h"

static int data_exchange_count = 0;

/**** デバッグ用データバッファ ****/
#define DATA_SIZE 1024*20
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

/* テスト用の GGID */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x52)
/*---------------------------------------------------------------------------*
    外部変数定義
 *---------------------------------------------------------------------------*/
extern int passby_endflg;
extern WMBssDesc bssdesc;
extern WMParentParam parentparam;
extern u8 macAddress[6];

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
        OS_TPrintf("sum OK 0x%02X %6d %6d sec\n", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
        OS_TPrintf("%s\n", data);
    }
    else
    {
        OS_TPrintf("sum NG 0x%02X sum 0x0%02X %6d sec", sum, data[DATA_SIZE - 1],
               OS_TicksToSeconds(OS_GetTick()));
    }
    MI_CpuClear32(data, DATA_SIZE);
    
    // 一回通信したら終了するようにする
    
    if( WXC_IsParentMode() == TRUE)
    {
        const WMParentParam *param;
        
        passby_endflg = 1;
        
        // 親の情報を取得しておく
        param = WXC_GetParentParam();
        MI_CpuCopyFast( param, &parentparam, sizeof(WMParentParam) );        
    }
    else
    {
        const WMBssDesc *bss;
        
        passby_endflg = 2;
        // 親のMACアドレスをGet
        bss = WXC_GetParentBssDesc();
        MI_CpuCopyFast( bss, &bssdesc, sizeof(WMBssDesc) ); 
    }
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
            MI_CpuCopy16( user->macAddress, macAddress, sizeof(u8)*6 ); 
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


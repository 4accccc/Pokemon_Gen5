/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-dataShare
  File:     gmain.c

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

#include "gmain.h"
#include "common.h"
#include "disp.h"
#include "wh.h"



//============================================================================
//  変数定義
//============================================================================

//-----------------------
// データシェアリング用
//-----------------------
static u8 sSendBuf[256] ATTRIBUTE_ALIGN(32);    // 送信バッファ( 要32バイトアライン )
static u8 sRecvBuf[256] ATTRIBUTE_ALIGN(32);    // 受信バッファ( 要32バイトアライン )
static BOOL sRecvFlag[1 + WM_NUM_MAX_CHILD];    // 受信フラグ
static GShareData *sShareDataPtr;

//============================================================================
//  関数定義
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         GInitDataShare

  Description:  データシェアリング用バッファの初期化設定

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GInitDataShare(void)
{
    sShareDataPtr = (GShareData *) sSendBuf;

    DC_FlushRange(sSendBuf, 4);
    DC_WaitWriteBufferEmpty();
}


/*---------------------------------------------------------------------------*
  Name:         GStepDataShare

  Description:  データシェアリングの同期を1つ進めます。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL GStepDataShare(s32 frame)
{
    sShareDataPtr->count = (u32)frame;

    switch (WH_GetSystemState())
    {
        //------------------------------
        // データシェアリング通信
    case WH_SYSSTATE_DATASHARING:
        {
            u16 i;
            u8 *adr;

            if (!WH_StepDS(sSendBuf))
            {
                // データシェア通信失敗
                return FALSE;
            }

            for (i = 0; i < 16; ++i)
            {
                adr = (u8 *)WH_GetSharedDataAdr(i);
                if (adr != NULL)
                {
                    MI_CpuCopy8(adr, &(sRecvBuf[i * sizeof(GShareData)]), sizeof(GShareData));
                    sRecvFlag[i] = TRUE;
                }
                else
                {
                    sRecvFlag[i] = FALSE;
                }
            }
        }
        break;
    }
    return TRUE;
}






/*---------------------------------------------------------------------------*
  Name:         GMain

  Description:  親子共通メインルーチン

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void GMain(void)
{

    BgPrintStr(4, 3, 0x4, "Send:     %04x-%04x", sShareDataPtr->key, sShareDataPtr->count);
    BgPutString(4, 5, 0x4, "Receive:");
    {
        s32 i;

        for (i = 0; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            if (sRecvFlag[i])
            {
                GShareData *sd;
                sd = (GShareData *) (&(sRecvBuf[i * sizeof(GShareData)]));

                BgPrintStr(4, (s16)(6 + i), 0x4,
                           "Player%02d: %04x-%04x", i, sd->key, sd->count & 0xffff);
            }
            else
            {
                BgPutString(4, (s16)(6 + i), 0x7, "No child");
            }
        }
    }
}

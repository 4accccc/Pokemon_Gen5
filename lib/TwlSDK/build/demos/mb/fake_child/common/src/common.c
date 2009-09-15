/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - fake_child
  File:     common.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
*---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include "common.h"
#include "disp.h"


static void VBlankIntr(void);

/*
 * このデモ全体で使用する共通機能.
 */
static u16 padPress;
static u16 padTrig;

/*---------------------------------------------------------------------------*
  Name:         ReadKey

  Description:  キーの読み込み処理

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void ReadKey(void)
{
    u16     currData = PAD_Read();

    padTrig = (u16)(~padPress & currData);
    padPress = currData;
}

/*---------------------------------------------------------------------------*
  Name:         GetPressKey

  Description:  押下キー取得

  Arguments:    None

  Returns:      押下されているキーのビットマップ
 *---------------------------------------------------------------------------*/
u16 GetPressKey(void)
{
    return padPress;
}


/*---------------------------------------------------------------------------*
  Name:         GetTrigKey

  Description:  キートリガ取得

  Arguments:    None

  Returns:      キートリガのビットマップ
 *---------------------------------------------------------------------------*/
u16 GetTrigKey(void)
{
    return padTrig;
}


/*---------------------------------------------------------------------------*
  Name:         CommonInit

  Description:  共通初期化関数

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CommonInit(void)
{
    /* OS 初期化 */
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();

    /* GX 初期化 */
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // 割り込み設定
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)GX_VBlankIntr(TRUE);
    
    // キーを一回空読み
    ReadKey();
}


/*---------------------------------------------------------------------------*
  Name:         InitAllocateSystem

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    // OS_Initは呼ばれているという前提
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}




/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  キートリガ取得

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    DispVBlankFunc();

    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - tips - banner - TWLBanner-anim1
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <DEMO.h>

static void InitDEMOSystem();
static void InitInteruptSystem();

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数です。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OS_Init();
    InitInteruptSystem();
    InitDEMOSystem();
    OS_Printf("*** start TWLBanner-anim1 demo\n");
    
    for (;;)
    {
        DEMODrawText( 8, 0, "TWLBanner-anim1 demo");
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  コンソールの画面出力用の表示設定を行います。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // 画面表示の初期化。
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  割り込みを初期化します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - tick-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5205 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "data.h"

static GXOamAttr oamBak[128];

void    VBlankIntr(void);
void    ObjSet(int objNo, int x, int y, int charNo, int paletteNo);

//---- OBJ 表示用に 0～9,A～F のコードを返す
inline int ObjChar(u32 cnt, int shift)
{
    u32     d = (cnt >> shift) & 0xf;
    return (int)((d < 10) ? '0' + d : 'A' + d - 10);
}

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    //================ 初期化
    OS_Init();
    OS_InitTick();                     // システムクロック初期化

    GX_Init();

    //================ 設定
    //---- 電源全てオン
    GX_SetPower(GX_POWER_ALL);

    //----  Vブランク割込許可
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    //---- Vブランク発生設定
    (void)GX_VBlankIntr(TRUE);

    //---- VRAM クリア
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    //---- OAMとパレットクリア
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);

    //---- バンクＡをＯＢＪへ
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_A);

    //---- グラフィックス表示モードにする
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);

    //---- OBJの表示のみON
    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);

    //---- 32KバイトのOBJで2Dマップモードで使用
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    //---- データロード
    MI_DmaCopy32(3, sampleCharData, (void *)HW_OBJ_VRAM, sizeof(sampleCharData));
    MI_DmaCopy32(3, samplePlttData, (void *)HW_OBJ_PLTT, sizeof(samplePlttData));

    GX_DispOn();


    //================ メインループ
    while (1)
    {
        u16     keyData;
        u32     hi, low;
        OSTick  timerCnt;

        //---- Vブランク割込終了待ち
        OS_WaitVBlankIntr();

        //---- 非表示OBJは画面外へ移動
        MI_DmaFill32(3, oamBak, 0xc0, sizeof(oamBak));

        //---- パッドデータ読み込み
        keyData = PAD_Read();

        //---- システムクロック値取得
        timerCnt = OS_GetTick();

        //---- システムクロック値表示
        hi = (u32)(timerCnt >> 32);
        low = (u32)(timerCnt & 0xffffffff);

        ObjSet(0, 50, 100, ObjChar(hi, 28), 2);
        ObjSet(1, 60, 100, ObjChar(hi, 24), 2);
        ObjSet(2, 70, 100, ObjChar(hi, 20), 2);
        ObjSet(3, 80, 100, ObjChar(hi, 16), 2);
        ObjSet(4, 90, 100, ObjChar(hi, 12), 2);
        ObjSet(5, 100, 100, ObjChar(hi, 8), 2);
        ObjSet(6, 110, 100, ObjChar(hi, 4), 2);
        ObjSet(7, 120, 100, ObjChar(hi, 0), 2);

        ObjSet(8, 140, 100, ObjChar(low, 28), 2);
        ObjSet(9, 150, 100, ObjChar(low, 24), 2);
        ObjSet(10, 160, 100, ObjChar(low, 20), 2);
        ObjSet(11, 170, 100, ObjChar(low, 16), 2);
        ObjSet(12, 180, 100, ObjChar(low, 12), 2);
        ObjSet(13, 190, 100, ObjChar(low, 8), 2);
        ObjSet(14, 200, 100, ObjChar(low, 4), 2);
        ObjSet(15, 210, 100, ObjChar(low, 0), 2);

        //---- A ボタンを押したらシステムクロックちょっと進める
        if (keyData & PAD_BUTTON_A)
        {
            OS_SetTick(OS_GetTick() + (u64)0x20000000000ULL);
        }
    }
}

//--------------------------------------------------------------------------------
//  OBJ セット
//
void ObjSet(int objNo, int x, int y, int charNo, int paletteNo)
{
    G2_SetOBJAttr((GXOamAttr *)&oamBak[objNo],
                  x,
                  y,
                  0,
                  GX_OAM_MODE_NORMAL,
                  FALSE,
                  GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_8x8, GX_OAM_COLOR_16, charNo, paletteNo, 0);
}


//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    //---- ＯＡＭの更新
    DC_FlushRange(oamBak, sizeof(oamBak));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    MI_DmaCopy32(3, oamBak, (void *)HW_OAM, sizeof(oamBak));

    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*====== End of main.c ======*/

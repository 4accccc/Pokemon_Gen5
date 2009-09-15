/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - simple-2
  File:     dispfunc.c

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
#include <string.h>

#include "dispfunc.h"

// --- global value
GXOamAttr oamBak[128];
/*
	=================================================================================

	BG文字描画 (BG0固定)

	=================================================================================
*/


#define BG_LINES	(24)
#define PROMPT		">"

// --- static value
static u16 vscr[32 * 32] ATTRIBUTE_ALIGN(32);   // 仮想スクリーン
static u16 vscr_sub[32 * 32] ATTRIBUTE_ALIGN(32);       // 仮想スクリーン(sub)
static u16 cur_line = 0;

// --- static functions
static void BgiPutChar(s16 x, s16 y, u8 palette, char c, u8 lcd);
static void BgiPutStringN(s16 x, s16 y, u8 palette, char *text, u32 num, u8 lcd);


static void BgiPutChar(s16 x, s16 y, u8 palette, char c, u8 lcd)
{
    if (lcd)
    {
        vscr[((y * 32) + x) % (32 * 32)] = (u16)((palette << 12) | c);
    }
    else
    {
        vscr_sub[((y * 32) + x) % (32 * 32)] = (u16)((palette << 12) | c);
    }
}

static void BgiPutStringN(s16 x, s16 y, u8 palette, char *text, u32 num, u8 lcd)
{
    s32 i;

    for (i = 0; i < num; i++)
    {
        if (text[i] == 0x00)
        {
            break;
        }

        BgiPutChar((s16)(x + i), y, palette, text[i], lcd);
    }

}

/*
	--- initialize
*/
void BgInitForPrintStr(void)
{
    // main lcd
    GX_SetBankForBG(GX_VRAM_BG_64_E);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf000,      // SCR ベースブロック 31
                     GX_BG_CHARBASE_0x00000,    // CHR ベースブロック 0
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);

    GX_LoadBG0Char(sampleCharData, 0, sizeof(sampleCharData));
    GX_LoadBGPltt(samplePlttData, 0, sizeof(samplePlttData));

    MI_CpuFillFast((void *)vscr, 0, sizeof(vscr));
    DC_FlushRange(vscr, sizeof(vscr));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(vscr, 0, sizeof(vscr));

    // sub lcd
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf800,     // SCR ベースブロック 31
                      GX_BG_CHARBASE_0x10000,   // CHR ベースブロック 0
                      GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);

    GXS_LoadBG0Char(sampleCharData, 0, sizeof(sampleCharData));
    GXS_LoadBGPltt(samplePlttData, 0, sizeof(samplePlttData));
    MI_CpuFillFast((void *)vscr_sub, 0, sizeof(vscr_sub));
    DC_FlushRange(vscr_sub, sizeof(vscr_sub));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GXS_LoadBG0Scr(vscr_sub, 0, sizeof(vscr_sub));

    cur_line = 0;
}

/* 
	--- for main LCD
*/

// Format付き文字列出力
void BgPrintf(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char temp[32 * 2 + 2];

    MI_CpuClear8(&temp[0], sizeof(temp));

    va_start(vlist, text);
    (void)OS_VSNPrintf(temp, 32 * 2 + 2, text, vlist);
    va_end(vlist);

    BgPutString(x, y, palette, temp);
}


// 文字列出力
void BgPutString(s16 x, s16 y, u8 palette, char *text)
{
    BgPutStringN(x, y, palette, text, 32);
}

// 文字数制限付き文字列出力
void BgPutStringN(s16 x, s16 y, u8 palette, char *text, u32 num)
{
    BgiPutStringN(x, y, palette, text, num, 1);
}


// 1文字出力
void BgPutChar(s16 x, s16 y, u8 palette, char c)
{
    BgiPutChar(x, y, palette, c, 1);
}

// BGのクリア
void BgClear(void)
{
    MI_CpuClearFast((void *)vscr, sizeof(vscr));
}

/*
	--- v blank process
*/

void BgScrSetVblank(void)
{
    // 仮想スクリーンをVRAMに反映
    DC_FlushRange(vscr, sizeof(vscr));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(vscr, 0, sizeof(vscr));
    DC_FlushRange(vscr_sub, sizeof(vscr_sub));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GXS_LoadBG0Scr(vscr_sub, 0, sizeof(vscr_sub));
}


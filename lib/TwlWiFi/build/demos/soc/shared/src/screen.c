/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - shared
  File:     screen.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#include "screen.h"
#include "output.h"

/*****************************************************************************/

#define SCR_SIZE_X 32
#define SCR_SIZE_Y 32

/* variable */
static u16  main_text_scr[SCR_SIZE_Y][SCR_SIZE_X] ATTRIBUTE_ALIGN(32);
static BOOL main_text_modified;
static GXRgb bitmap_frame[GX_LCD_SIZE_X * GX_LCD_SIZE_Y] ATTRIBUTE_ALIGN(32);
static BOOL bitmap_modified;

/*****************************************************************************/

/* function */

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V ブランク コールバック.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
    FlushScreen();
    FlushDebugOutput();
}

//  MAIN BG MAP
//
// BG3 ... ビットマップの表示
// BG0 ... 文字の描画

//  MAIN VRAM (A) MAP
//
//  +---------------+-- 20000h
//  |   BG1 scr     |
//  |---------------|   1E800h
//  |   BG1 chr     |   
//  |---------------|   18000h
//  |   BG3 bmp     |   
//  +---------------+-- 00000h
//

//  SUB BG MAP
//
// BG0 ... 

//  SUB VRAM (HI) MAP
//
//  +---------------+-- 0C000h
//  |   BG0 scr     |   
//  |---------------|   0B800h
//  |   BG0 chr     |   
//  +---------------+-- 00000h
//

/*---------------------------------------------------------------------------*
  Name:         InitScreen

  Description:  表示設定初期化.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitScreen(void)
{
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    /* 内部メモリクリア */
    main_text_modified = FALSE;
    MI_CpuClearFast(main_text_scr, sizeof(main_text_scr));

    /* VRAM クリア */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void*)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void*)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void*)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    /*
     * VRAM-A  = main-BG
     * VRAM-HI = sub-BG
     */
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_48_HI);

    /*
     * メイン画面:
     *   BG0 = text-BG
     */
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_4, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_BG3);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x18000, GX_BG_EXTPLTT_01);
    GX_SetBGScrOffset(GX_BGSCROFFSET_0x10000);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    G2_SetBG0Offset(0, 0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));

    // main-BMP-BG
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
    G2_SetBG3Priority(1);

    /*
     * サブ画面:
     *   BG0 = text-BG
     */
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xb800, GX_BG_CHARBASE_0x00000,
                      GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    G2S_SetBG0Offset(0, 0);
    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));

    /* 表示有効化 */
    GX_DispOn();
    GXS_DispOn();

    /* Vブランク設定 */
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    /* 画面の上下切り替え */
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
}

/*---------------------------------------------------------------------------*
  Name:         ClearScreen

  Description:  テキストスクリーンの消去.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ClearScreen(void)
{
    MI_CpuClearFast(main_text_scr, sizeof(main_text_scr));
    main_text_modified = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         PadkeyClearScreen

  Description:  Padkeyが使用する部分のテキストスクリーンの消去.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PadkeyClearScreen(void)
{
    MI_CpuClearFast(main_text_scr[0], sizeof(main_text_scr[0]));
    MI_CpuClearFast(main_text_scr[4], sizeof(main_text_scr[4]));
    MI_CpuClearFast(main_text_scr[23], sizeof(main_text_scr[23]));
    main_text_modified = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  テキストスクリーンに文字列を表示.

  Arguments:    x             グリッド単位の X 座標
                y             グリッド単位の Y 座標
                palette       パレットインデックス
                format        書式文字列

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PrintString(int x, int y, int palette, const char* format, ...)
{
    va_list vlist;
    char    temp[512 + 1];
    int     i;
    int     num = sizeof(temp) - 1;
    int     px = 0;

    va_start(vlist, format);
    (void)OS_VSNPrintf(temp, sizeof(temp)-2, format, vlist);
    va_end(vlist);

    for (i = 0; (num > 0) && temp[i]; ++i)
    {
        switch(temp[i])
        {
          case '\0':
            break;
          case '\r':
            if (temp[i+1] == '\n')
            {
                i++, --num;
            }
            // fall through
          case '\n':
            y++;
            px = 0;
            break;
          default:
            if ((x + px < SCR_SIZE_X) && (y < 24))
            {
                main_text_scr[y][x + px] = (u16) ((palette << 12) | (u8) temp[i]);
                num--, px++;
            }
            break;
        }
    }

    main_text_modified = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         PrintDebugString

  Description:  サブ画面テキストスクリーンにデバッグ文字列を表示.

  Arguments:    s             出力文字列.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PrintDebugString(const char* s, ...)
{
    char    tmp[256];
    va_list va;
    va_start(va, s);
    (void)OS_VSNPrintf(tmp, sizeof(tmp), s, va);
    va_end(va);
    DebugOutput(tmp);
}

/*---------------------------------------------------------------------------*
  Name:         FlushScreen

  Description:  テキストスクリーンをこの場でVRAMへ反映.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FlushScreen(void)
{
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    BOOL        modified = main_text_modified;
    BOOL       bmodified = bitmap_modified;
    if (modified)
    {
        //PrintString(0, 0, COLOR_WHITE, "%s", GetTestName());
        main_text_modified = FALSE;
    }
    if (bmodified)
    {
        bitmap_modified = FALSE;
    }
    (void)OS_RestoreInterrupts(bak_cpsr);

    /* バックグラウンドでの自動更新を考え, ここで割り込みは禁止しない */
    if (modified)
    {
        DC_FlushRange(main_text_scr, sizeof(main_text_scr));
        GX_LoadBG0Scr(main_text_scr, 0, sizeof(main_text_scr));
    }

    if (bmodified)
    {
        DC_FlushRange(bitmap_frame, sizeof(bitmap_frame));
        GX_LoadBG3Bmp(bitmap_frame, 0, sizeof(bitmap_frame));
    }
}

/*---------------------------------------------------------------------------*
  Name:         ShowNavigater

  Description:  テキストスクリーンの最下行に、オペレータを誘導するメッセージ
                を表示する。毎フレーム呼び出すことにより、点滅表示される。

  Arguments:    text    -   表示するメッセージテキスト。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ShowNavigater(const char* text)
{
    char    temp[33] = { "                                " };
    s32     i;

    for (i = 0; i < 31; i++)
    {
        if (text[i] == 0x00)
        {
            break;
        }

        temp[i + 1] = text[i];
    }

    PrintString(0, 22, GetFlickerColor(), temp);
}

/*---------------------------------------------------------------------------*
  Name:         GetFlickerColor

  Description:  点滅用の文字列色を取得する。

  Arguments:    None.

  Returns:      int     -   パレット番号
 *---------------------------------------------------------------------------*/
int GetFlickerColor(void)
{
    return (int)(5 + ((OS_GetVBlankCount() / 30) % 2));
}

/*---------------------------------------------------------------------------*
  Name:         PrintStep

  Description:  テキストスクリーンの指定位置に、手順文字列を表示。
                文字数と行幅に応じて適宜改行する。

  Arguments:    x       -   表示原点 x 座標。
                y       -   表示原点 y 座標。
                palette -   表示パレット色。 (COLOR_*)
                step    -   現在のステップ番号。
                comment -   テスト手順を示すコメント文字列。

  Returns:      表示に要した行数.
 *---------------------------------------------------------------------------*/
int PrintStep(int x, int y, int palette, int step, const char* comment)
{
    static const char   blank[32 + 1] = "                                ";
    char        c;
    int         len, base;
    const int   oy = y;
    const int   limit = 29 - x - 4;

    for (; y < 23; ++y)
    {
        if (y == oy)
        {
            PrintString(x, y, palette, " %d. ", step);
        }
        else
        {
            PrintString(x, y, palette, "    ");
        }

        /* 1 行に表示可能な文字数を検索 */
        len = 0;
        base = 0;
        for (;;)
        {
            /* 空白 */
            while (((c = comment[len]) != '\0') && ((c == ' ') || (c == '\t')))
            {
                ++len;
            }

            if (base == 0)
            {
                comment += len;
                len = 0;
            }

            /* トークン */
            while (((c = comment[len]) != '\0') && ((c != ' ') && (c != '\t')))
            {
                ++len;
            }

            if (len > limit)
            {
                break;
            }

            base = len;
            if (c == '\0')
            {
                break;
            }
        }

        PrintString(x + 4, y, palette, "%.*s%.*s", base, comment, limit - base, blank);
        comment += base;
        if (comment[0] == '\0')
        {
            break;
        }
    }

    return y - oy + 1;
}

/*---------------------------------------------------------------------------*
  Name:         FillRect

  Description:  ビットマップに矩形を描画

  Arguments:    x     : 描画 X 座標
                y     : 描画 Y 座標
                wx    : 描画 X 幅
                wy    : 描画 Y 幅
                color : 設定する色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FillRect(int x, int y, int wx, int wy, GXRgb color)
{
    if ((x < GX_LCD_SIZE_X) && (y < GX_LCD_SIZE_Y))
    {
        if (wx > GX_LCD_SIZE_X - x)
        {
            wx = GX_LCD_SIZE_X - x;
        }
        if (wy > GX_LCD_SIZE_Y - y)
        {
            wy = GX_LCD_SIZE_Y - y;
        }
        if ((wx > 0) && (wy > 0))
        {
            GXRgb *dst = &bitmap_frame[x + y * GX_LCD_SIZE_X];
            while (--wy >= 0)
            {
                MI_CpuFill16(dst, color, wx * sizeof(GXRgb));
                dst += GX_LCD_SIZE_X;
            }
            bitmap_modified = TRUE;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DrawFrame

  Description:  ビットマップに枠を描画

  Arguments:    x      : 描画 X 座標
                y      : 描画 Y 座標
                wx     : 描画 X 幅
                wy     : 描画 Y 幅
                color : 設定する色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DrawFrame(int x, int y, int wx, int wy, GXRgb color)
{
    FillRect(x, y, wx, 1, color);
    FillRect(x, y + wy - 1, wx, 1, color);
    FillRect(x, y + 1, 1, wy - 1, color);
    FillRect(x + wx - 1, y + 1, 1, wy - 1, color);
}

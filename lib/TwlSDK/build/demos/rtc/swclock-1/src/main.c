/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - demos - swclock-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-02#$
  $Rev: 2048 $
  $Author: tokunaga_eiji $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#include <twl/rtc.h>
#else
#include <nitro.h>
#include <nitro/rtc.h>
#endif

#include    "font.h"


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // キーリピート開始までのフレーム数
#define     KEY_REPEAT_SPAN     10     // キーリピートの間隔フレーム数


/*---------------------------------------------------------------------------*
    文字列定数定義
 *---------------------------------------------------------------------------*/
// 曜日
const char *StrWeek[7] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// キー入力情報
typedef struct KeyInformation
{
    u16     cnt;                       // 未加工入力値
    u16     trg;                       // 押しトリガ入力
    u16     up;                        // 離しトリガ入力
    u16     rep;                       // 押し維持リピート入力

}
KeyInformation;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void AlarmIntrCallback(void);

static void KeyRead(KeyInformation * pKey);
static void ClearString(void);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void ColorString(s16 x, s16 y, s16 length, u8 palette);


/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u16 gScreen[32 * 32];           // 仮想スクリーン
static KeyInformation gKey;            // キー入力

static RTCDate   gRtcDate;             // RTC から取得する日付
static RTCTime   gRtcTime;             // RTC から取得する時刻
static RTCDate   gSWClockDate;         // ソフトウェアクロックから取得する日付
static RTCTimeEx gSWClockTimeEx;       // ソフトウェアクロックから取得する時刻

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // 各種初期化
    OS_Init();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // 表示設定初期化
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    // 文字列表示用に２Ｄ表示設定
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf800,      // SCR ベースブロック 31
                     GX_BG_CHARBASE_0x00000,    // CHR ベースブロック 0
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // 割込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();


    //****************************************************************
    // RTC初期化
    RTC_Init();
    // チック初期化
    OS_InitTick();
    // ソフトウェアクロック初期化
    (void)RTC_InitSWClock();
    //****************************************************************

    // LCD表示開始
    GX_DispOn();
    GXS_DispOn();

    // デバッグ文字列出力
    OS_Printf("ARM9: SWClock demo started.\n");

    // キー入力情報取得の空呼び出し(IPL での A ボタン押下対策)
    KeyRead(&gKey);

    // メインループ
    while (TRUE)
    {
        // キー入力情報取得
        KeyRead(&gKey);

        // スクリーンクリア
        ClearString();

        //****************************************************************
        // RTC から日付・時刻を読み出し
        (void)RTC_GetDateTime(&gRtcDate, &gRtcTime);
        // ソフトウェアクロックから日付・時刻を読み出し
        (void)RTC_GetDateTimeExFromSWClock(&gSWClockDate, &gSWClockTimeEx);
        //****************************************************************

        // RTC から取得した日付を表示
        PrintString(1, 3, 0xf, "RTCDate:");
        PrintString(10, 3, 0xf, "%04d/%02d/%02d/%s",
                    gRtcDate.year + 2000, gRtcDate.month, gRtcDate.day, StrWeek[gRtcDate.week]);
        // ソフトウェアクロックから取得した日付を表示
        PrintString(1, 5, 0xf, "SWCDate:");
        PrintString(10, 5, 0xf, "%04d/%02d/%02d/%s",
                    gSWClockDate.year + 2000, gSWClockDate.month, gSWClockDate.day, StrWeek[gSWClockDate.week]);

        // RTC から取得した時刻を表示
        PrintString(1, 8, 0xf, "RTCTime:");
        PrintString(10, 8, 0xf, "%02d:%02d:%02d", gRtcTime.hour, gRtcTime.minute, gRtcTime.second);
        // ソフトウェアクロックから取得した時刻を表示
        PrintString(1, 10, 0xf, "SWCTime:");        
        PrintString(10, 10, 0xf, "%02d:%02d:%02d:%03d", gSWClockTimeEx.hour, gSWClockTimeEx.minute,
                    gSWClockTimeEx.second, gSWClockTimeEx.millisecond);

        // ソフトウェアクロックから取得した 2000年1月1日0時00分からの総チック値を表示
        PrintString(1, 13, 0xf, "SWCTick:");
        PrintString(10, 13, 0xf, "%llu", RTC_GetSWClockTick() );        
        
        // ボタン操作説明文を表示
        PrintString(1, 19, 0x3, "START > Synclonize SWC with RTC.");
        PrintString(1, 20, 0x3, "A     > Go to sleep.");
        PrintString(1, 21, 0x3, "B     > Wake up from sleep.");


        // ボタン入力操作
        // START キー
        if ((gKey.trg) & PAD_BUTTON_START)
        {
            OS_TPrintf("Synchronizing SWC with RTC.\n");
            (void)RTC_SyncSWClock();
        }

        // Ａ ボタン
        if (gKey.trg & PAD_BUTTON_A)
        {
            OS_TPrintf("Going to sleep... Press B button to wake up.\n");
            PM_GoSleepMode( PM_TRIGGER_KEY, PM_PAD_LOGIC_AND, PAD_BUTTON_B );
        }

        // Ｖブランク待ち
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みベクトル。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // 仮想スクリーンをVRAMに反映
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // IRQ チェックフラグをセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  キー入力情報を編集する。
                押しトリガ、離しトリガ、押し継続リピートを検出。

  Arguments:    pKey  - 編集するキー入力情報構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInformation * pKey)
{
    static u16 repeat_count[12];
    int     i;
    u16     r;

    r = PAD_Read();
    pKey->trg = 0x0000;
    pKey->up = 0x0000;
    pKey->rep = 0x0000;

    for (i = 0; i < 12; i++)
    {
        if (r & (0x0001 << i))
        {
            if (!(pKey->cnt & (0x0001 << i)))
            {
                pKey->trg |= (0x0001 << i);     // 押しトリガ
                repeat_count[i] = 1;
            }
            else
            {
                if (repeat_count[i] > KEY_REPEAT_START)
                {
                    pKey->rep |= (0x0001 << i); // 押し継続リピート
                    repeat_count[i] = KEY_REPEAT_START - KEY_REPEAT_SPAN;
                }
                else
                {
                    repeat_count[i]++;
                }
            }
        }
        else
        {
            if (pKey->cnt & (0x0001 << i))
            {
                pKey->up |= (0x0001 << i);      // 離しトリガ
            }
        }
    }
    pKey->cnt = r;                     // 未加工キー入力
}

/*---------------------------------------------------------------------------*
  Name:         ClearString

  Description:  仮想スクリーンをクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearString(void)
{
    MI_CpuClearFast((void *)gScreen, sizeof(gScreen));
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  仮想スクリーンに文字列を配置する。文字列は32文字まで。

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                palette - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0x0000;
    for (i = 0;; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        gScreen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ColorString

  Description:  仮想スクリーンに配置した文字列の色を変更する。

  Arguments:    x       - 色変更を開始する x 座標( × 8 ドット )。
                y       - 色変更を開始する y 座標( × 8 ドット )。
                length  - 連続して色変更する文字数。
                palette - 文字の色をパレット番号で指定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ColorString(s16 x, s16 y, s16 length, u8 palette)
{
    s32     i;
    u16     temp;
    s32     index;

    if (length < 0)
        return;

    for (i = 0; i < length; i++)
    {
        index = ((y * 32) + x + i) % (32 * 32);
        temp = gScreen[index];
        temp &= 0x0fff;
        temp |= (palette << 12);
        gScreen[index] = temp;
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

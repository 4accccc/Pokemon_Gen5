/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - demos - switchover
  File:     wireless_main.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9665 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include    <nitro.h>
#include    <nitro/wm.h>
#include    <nitro/wvr.h>

#include    "font.h"
#include    "common.h"
#include    "wh.h"


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // キーリピート開始までのフレーム数
#define     KEY_REPEAT_SPAN     10     // キーリピートの間隔フレーム数

#define     PICTURE_FRAME_PER_GAME_FRAME    1

#define     DEFAULT_GGID            0x003fff60
#define     NUM_MAX_CHILD           15
#define     DEFAULT_CHAN            1
#define     PARENT_DATA_SIZE        4
#define     CHILD_DATA_SIZE         4

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// キー入力情報
typedef struct KeyInfo
{
    u16     cnt;                       // 未加工入力値
    u16     trg;                       // 押しトリガ入力
    u16     up;                        // 離しトリガ入力
    u16     rep;                       // 押し維持リピート入力

}
KeyInfo;


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void StartUpCallback(void *arg, WVRResult result);

static void ModeSelect(void);          // 親機/子機 選択画面
static void ModeError(void);           // エラー表示画面
static void ModeWorking(void);         // ビジー画面
static void ModeParent(void);          // 親機 通信画面
static void ModeChild(void);           // 子機 通信画面
static void VBlankIntr(void);          // Vブランク割込みハンドラ

// データ受信時に呼び出される関数
static void MpReceiveCallback(u16 aid, u16 *data, u16 length);

// 汎用サブルーチン
static void KeyRead(KeyInfo * pKey);
static void ClearString(void);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void ColorString(s16 x, s16 y, s16 length, u8 palette);


/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u16 gScreen[32 * 32];           // 仮想スクリーン
static KeyInfo gKey;                   // キー入力
static u32 gFrame;                     // フレームカウンタ

// 表示用送受信バッファ
static u32 gSendBuf[1024] ATTRIBUTE_ALIGN(32);
static u32 gRecvBuf[1 + WM_NUM_MAX_CHILD][1024];
static BOOL gRecvFlag[1 + WM_NUM_MAX_CHILD];

static volatile u8 startCheck;

/*---------------------------------------------------------------------------*
  Name:         WirelessMain

  Description:  無線通信の初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WirelessMain(void)
{
    GX_DispOff();
    GXS_DispOff();

    /* VRAM, OAM, パレットRAM の全クリア */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    /* V ブランク割り込みハンドラ設定 */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // 文字列表示用に２Ｄ表示設定
    GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf800,      // SCR ベースブロック 31
                     GX_BG_CHARBASE_0x00000,    // CHR ベースブロック 0
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // LCD表示開始
    GX_DispOn();
    GXS_DispOn();

    //************************************
    // 無線ライブラリ駆動開始
    {
        startCheck = 0;
        if (WVR_RESULT_OPERATING != WVR_StartUpAsync(GX_VRAM_ARM7_128_D, StartUpCallback, NULL))
        {
            OS_TPanic("WVR_StartUpAsync failed. \n");
        }
        while (!startCheck)
        {
        }
    }
    //************************************

    //************************************
    // 無線初期化
    if (!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }
    WH_SetGgid(DEFAULT_GGID);
    WH_SetReceiver(MpReceiveCallback);
    //************************************

    // デバッグ文字列出力
    OS_Printf("ARM9: ichneumon test program started.\n");

    // キー入力情報取得の空呼び出し(IPL での A ボタン押下対策)
    KeyRead(&gKey);

    // メインループ
    for (gFrame = 0; TRUE; gFrame++)
    {
        // キー入力情報取得
        KeyRead(&gKey);

        if (gKey.trg & PAD_BUTTON_START)
        {
            WH_Finalize();
            while (WH_GetSystemState() != WH_SYSSTATE_IDLE)
            {
                OS_WaitVBlankIntr();
            }
            if (!WH_End())
            {
                OS_Panic("WH_End failed.");
            }
            while (WH_GetSystemState() != WH_SYSSTATE_STOP)
            {
            }
            break;
        }
        // スクリーンクリア
        ClearString();

        // 通信状態により処理を振り分け
        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_IDLE:
            ModeSelect();
            break;
        case WH_SYSSTATE_ERROR:
        case WH_SYSSTATE_CONNECT_FAIL:
            WH_Reset();
            break;
        case WH_SYSSTATE_FATAL:
            ModeError();
            break;
        case WH_SYSSTATE_BUSY:
        case WH_SYSSTATE_SCANNING:
            ModeWorking();
            break;
        case WH_SYSSTATE_CONNECTED:
            switch (WH_GetConnectMode())
            {
            case WH_CONNECTMODE_MP_PARENT:
                ModeParent();
                break;
            case WH_CONNECTMODE_MP_CHILD:
                ModeChild();
                break;
            }
            break;
        }

        // Ｖブランク待ち
        OS_WaitVBlankIntr();
    }

    //************************************
    // 無線ライブラリ駆動終了
    {
        startCheck = 0;
        if (WVR_RESULT_OPERATING != WVR_TerminateAsync(StartUpCallback, NULL))
        {
            OS_TPanic("WVR_TerminateAsync failed. \n");
        }
        while (!startCheck)
        {
        }
    }
    //************************************

}

/*---------------------------------------------------------------------------*
  Name:         StartUpCallback

  Description:  無線駆動制御ライブラリの非同期的な処理終了が通知される
                コールバック関数。

  Arguments:    arg     -   WVR_StartUpAsyncコール時に指定した引数。未使用。
                result  -   非同期関数の処理結果。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void StartUpCallback(void *arg, WVRResult result)
{
#pragma unused( arg )

    if (result != WVR_RESULT_SUCCESS)
    {
        OS_TPanic("WVR_StartUpAsync error.[%08xh]\n", result);
    }
    startCheck = 1;
}

/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  親機/子機 選択画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    PrintString(3, 10, 0xf, "Push A to connect as PARENT");
    PrintString(3, 12, 0xf, "Push B to connect as CHILD");

    if (gKey.trg & PAD_BUTTON_A)
    {
        //********************************
        s32     i;

        for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            gRecvFlag[i] = FALSE;
        }

        (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, DEFAULT_CHAN);
        //********************************
    }
    else if (gKey.trg & PAD_BUTTON_B)
    {
        static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        //********************************
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_MP_CHILD, ANY_PARENT, DEFAULT_CHAN);
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  エラー表示画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeError(void)
{
    PrintString(5, 10, 0x1, "======= ERROR! =======");
    PrintString(5, 13, 0xf, " Fatal error occured.");
    PrintString(5, 14, 0xf, "Please reboot program.");
}

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  ビジー画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeWorking(void)
{
    PrintString(9, 11, 0xf, "Now working...");

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  親機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeParent(void)
{
    PrintString(8, 1, 0x2, "Parent mode");
    PrintString(4, 3, 0x4, "Send:     %08X", gSendBuf[0]);
    PrintString(4, 5, 0x4, "Receive:");
    {
        s32     i;

        for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            if (gRecvFlag[i])
            {
                PrintString(5, (s16)(6 + i), 0x4, "Child%02d: %08X", i, gRecvBuf[i][0]);
            }
            else
            {
                PrintString(5, (s16)(6 + i), 0x7, "No child");
            }
        }
    }

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  子機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeChild(void)
{
    PrintString(8, 1, 0x2, "Child mode");
    PrintString(4, 3, 0x4, "Send:     %08X", gSendBuf[0]);
    PrintString(4, 5, 0x4, "Receive:");
    PrintString(5, 7, 0x4, "Parent:  %08X", gRecvBuf[0][0]);

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
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
    // 新規データ送信開始
    if (WH_GetSystemState() == WH_SYSSTATE_CONNECTED)
    {
        switch (WH_GetConnectMode())
        {
        case WH_CONNECTMODE_MP_PARENT:
            gSendBuf[0] = gFrame;
            //********************************
            (void)WH_SendData(gSendBuf, PARENT_DATA_SIZE, NULL);
            //********************************
            break;
        case WH_CONNECTMODE_MP_CHILD:
            gSendBuf[0] = gFrame;
            //********************************
            (void)WH_SendData(gSendBuf, CHILD_DATA_SIZE, NULL);
            //********************************
            break;
        }
    }

    // 仮想スクリーンをVRAMに反映
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // IRQ チェックフラグをセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         MpReceiveCallback

  Description:  MPデータ受信時に呼び出される関数。

  Arguments:    aid     - 送信元子機の aid( 0 の場合は親機からのデータ )
                data    - 受信データへのポインタ (NULL で切断通知)
                length  - 受信データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MpReceiveCallback(u16 aid, u16 *data, u16 length)
{
#pragma unused( length )
    SDK_MAX_ASSERT(aid, 15);

    if (data != NULL)
    {
        SDK_ASSERT(length >= 4);
        gRecvFlag[aid] = TRUE;
        // コピー元は2バイトアライン(4バイトアラインでない)
        if (aid == 0)
        {
            // 親機から受信した場合
            MI_CpuCopy8(data, &(gRecvBuf[aid][0]), PARENT_DATA_SIZE);
        }
        else
        {
            MI_CpuCopy8(data, &(gRecvBuf[aid][0]), CHILD_DATA_SIZE);
        }
    }
    else
    {
        gRecvFlag[aid] = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  キー入力情報を編集する。
                押しトリガ、離しトリガ、押し継続リピートを検出。

  Arguments:    pKey  - 編集するキー入力情報構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInfo * pKey)
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

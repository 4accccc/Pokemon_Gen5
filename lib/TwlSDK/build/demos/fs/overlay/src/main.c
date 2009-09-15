/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - FS - overlay
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "DEMO.h"
#include "func.h"


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         LoadOverlayHigh

  Description:  もっとも簡単な手順でオーバーレイをロードします。
                通常はこの処理で充分です。

  Arguments:    id : オーバーレイID

  Returns:      ロード処理の成否
 *---------------------------------------------------------------------------*/
static BOOL LoadOverlayHigh(MIProcessor target, FSOverlayID id)
{
    return FS_LoadOverlay(target, id);
}

/*---------------------------------------------------------------------------*
  Name:         LoadOverlayMiddle

  Description:  やや冗長な手順でオーバーレイをロードします。
                オーバーレイ情報の取得処理と実際のモジュール読み込み処理を
                分割して行う必要がある場合はこのようなシーケンスになります。

  Arguments:    id : オーバーレイID

  Returns:      ロード処理の成否
 *---------------------------------------------------------------------------*/
static BOOL LoadOverlayMiddle(MIProcessor target, FSOverlayID id)
{
    BOOL            retval = FALSE;
    FSOverlayInfo   info;
    if (FS_LoadOverlayInfo(&info, target, id))
    {
        if (FS_LoadOverlayImage(&info))
        {
            FS_StartOverlay(&info);
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         LoadOverlayLow

  Description:  もっとも低水準なAPIのみを組み合わせてオーバーレイをロードします。
                WFSライブラリを使用してワイヤレス経由でオーバーレイを取得するなど、
                全体の進捗をアプリケーション側で詳細に管理したい場合は
                このシーケンスをご参考ください。

  Arguments:    id : オーバーレイID

  Returns:      ロード処理の成否
 *---------------------------------------------------------------------------*/
static BOOL LoadOverlayLow(MIProcessor target, FSOverlayID id)
{
    BOOL            retval = FALSE;
    FSOverlayInfo   info;
    if (FS_LoadOverlayInfo(&info, target, id))
    {
        FSFile  file;
        FS_InitFile(&file);
        (void)FS_LoadOverlayImageAsync(&info, &file);
        (void)FS_WaitAsync(&file);
        (void)FS_CloseFile(&file);
        FS_StartOverlay(&info);
        retval = TRUE;
    }
    return retval;
}

// High,Middle,Lowいずれのロード処理も等価です。
// ご使用になるシーンを考慮された上で適切な方法をご選択ください。
#define	LoadOverlay(id)	LoadOverlayLow(MI_PROCESSOR_ARM9, id)

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  アプリケーションメインエントリ

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    FS_Init(FS_DMA_NOT_USE);

    OS_TPrintf("--------------------------------\n"
               "overlay sample.\n");

    {
        // LSFファイルに記述したオーバーレイIDは明示的に宣言する必要があります。
        // (実際のシンボルはリンク時にLCFテンプレートファイルが自動生成します)
        FS_EXTERN_OVERLAY(main_overlay_1);
        FS_EXTERN_OVERLAY(main_overlay_2);
        FS_EXTERN_OVERLAY(main_overlay_3);

        /* *INDENT-OFF* */
        static struct
        {
            FSOverlayID     id;
            const char     *name;
        }
        list[] =
        {
            { FS_OVERLAY_ID(main_overlay_1), "main_overlay_1", },
            { FS_OVERLAY_ID(main_overlay_2), "main_overlay_2", },
            { FS_OVERLAY_ID(main_overlay_3), "main_overlay_3", },
        };
        static const int    overlay_max = sizeof(list) / sizeof(*list);
        /* *INDENT-ON* */

        int     i;
        int     loaded_overlay = -1;
        int     current = 0;

        // 各オーバーレイ上の関数のアドレスを表示します。
        // 特に、func_1とfunc_2が同じアドレスである点に注目してください。
        OS_TPrintf("func_1() : addr = 0x%08X.\n", func_1);
        OS_TPrintf("func_2() : addr = 0x%08X.\n", func_2);
        OS_TPrintf("func_3() : addr = 0x%08X.\n", func_3);

        // オーバーレイを選択し、ロードします。
        for (;;)
        {
            DEMOReadKey();
            // Aボタンで現在選択しているオーバーレイをロード/アンロードします。
            if (DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                if (loaded_overlay == -1)
                {
                    (void)LoadOverlay(list[current].id);
                    loaded_overlay = current;
                }
                else
                {
                    (void)FS_UnloadOverlay(MI_PROCESSOR_ARM9, list[loaded_overlay].id);
                    loaded_overlay = -1;
                }
            }
            // Bボタンで現在のオーバーレイ上にある関数を1回呼び出します。
            if (DEMO_IS_TRIG(PAD_BUTTON_B))
            {
                // 複数のオーバーレイの領域が競合する時、
                // FSライブラリは最後にロードされたもの以外の動作を保証しません。
                // オーバーレイ上の関数を使用する際はこの点にご注意ください。
                if (loaded_overlay == 0)
                {
                    func_1();
                }
                else if (loaded_overlay == 1)
                {
                    func_2();
                }
                else if (loaded_overlay == 2)
                {
                    func_3();
                }
            }
            // 上下キーでカーソルを移動します。
            if (DEMO_IS_TRIG(PAD_KEY_DOWN))
            {
                if (++current >= overlay_max)
                {
                    current -= overlay_max;
                }
            }
            else if (DEMO_IS_TRIG(PAD_KEY_UP))
            {
                if (--current < 0)
                {
                    current += overlay_max;
                }
            }
            // 画面表示
            {
                int     ox = 10;
                int     oy = 60;
                DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
                DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
                DEMODrawText(10, 10, "UP&DOWN :select overlay");
                DEMODrawText(10, 20, "A button:load/unload overlay");
                DEMODrawText(10, 30, "B button:call function");
                DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
                DEMODrawFrame(ox, oy, 240, 10 + overlay_max * 10, GX_RGBA( 0, 31, 0, 1));
                for (i = 0; i < overlay_max; ++i)
                {
                    BOOL    focus = (i == current);
                    BOOL    loaded = (list[i].id == loaded_overlay);
                    DEMOSetBitmapTextColor((GXRgb)(focus ? GX_RGBA(31, 31, 0, 1) : GX_RGBA(31, 31, 31, 1)));
                    DEMODrawText(ox + 10, oy + 5 + i * 10, "%s%s %s",
                                 focus ? ">" : " ", list[i].name, loaded ? "(loaded)" : "");
                }
            }
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}

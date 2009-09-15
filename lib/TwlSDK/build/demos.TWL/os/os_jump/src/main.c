/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - os - os_jump
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-08#$
  $Rev: 8901 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <stdlib.h>

#include "screen.h"

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/
#define KEY_REPEAT_START    25  // キーリピート開始までのフレーム数
#define KEY_REPEAT_SPAN     10  // キーリピートの間隔フレーム数

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

// キー入力情報
typedef struct KeyInfo
{
    u16 cnt;    // 未加工入力値
    u16 trg;    // 押しトリガ入力
    u16 up;     // 離しトリガ入力
    u16 rep;    // 押し維持リピート入力
} KeyInfo;

// キー入力
static KeyInfo  gKey;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/

static void VBlankIntr(void);
static void InitInterrupts(void);
static void InitHeap(void);

static void ReadKey(KeyInfo* pKey);

/*---------------------------------------------------------------------------*/

void TwlMain(void)
{
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    InitHeap();
    InitScreen();
    InitInterrupts();

    GX_DispOn();
    GXS_DispOn();

    ClearScreen();

    // キー入力情報取得の空呼び出し(IPL での A ボタン押下対策)
    ReadKey(&gKey);

    while(TRUE)
    {
        // キー入力情報取得
        ReadKey(&gKey);

        // メイン画面クリア
        ClearScreen();
        
        PutMainScreen(0, 0, 0xff, "OS_IsRebooted = %s", OS_IsRebooted()? "TRUE":"FALSE");
        
        PutSubScreen(0, 0, 0xff, "APPLICATION JUMP DEMO");
        PutSubScreen(0, 1, 0xff, "  A: OS_JumpToSystemMenu");
        PutSubScreen(0, 2, 0xff, "  B: OS_RebootSystem");
        PutSubScreen(0, 3, 0xff, "  X: OS_JumpToEULAViewer");
        PutSubScreen(0, 4, 0xff, "  Y: OS_JumpToInternetSetting");
        PutSubScreen(0, 5, 0xff, "  R: OS_JumpToWirelessSetting");
        
        if (gKey.trg & PAD_BUTTON_A)
        {
            if (FALSE == OS_JumpToSystemMenu() )
            {
                //ジャンプに失敗
                OS_Panic("OS_JumpToSystemMenu() is Failed");
            }
        }

        if (gKey.trg & PAD_BUTTON_B)
        {
            if (FALSE == OS_RebootSystem() )
            {
                //ジャンプに失敗
                OS_Panic("OS_RebootSystem() is Failed");
            }
        }

        if (gKey.trg & PAD_BUTTON_X)
        {
            if (FALSE == OS_JumpToEULAViewer() )
            {
                //ジャンプに失敗
                OS_Panic("OS_JumpToEULAViewer() is Failed");
            }
        }

        if (gKey.trg & PAD_BUTTON_Y)
        {
            if (FALSE == OS_JumpToInternetSetting() )
            {
                //ジャンプに失敗
                OS_Panic("OS_JumpToInternetSetting() is Failed");
            }
        }
        if (gKey.trg & PAD_BUTTON_R)
        {
            if (FALSE == OS_JumpToWirelessSetting() )
            {
                //ジャンプに失敗
                OS_Panic("OS_JumpToWirelessSetting() is Failed");
            }
        }
        // Ｖブランク待ち ( スレッド対応 )
        OS_WaitVBlankIntr();
    }

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みハンドラ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // テキスト表示を更新
    UpdateScreen();

    // IRQ チェックフラグをセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitInterrupts

  Description:  割り込み設定を初期化する。
                V ブランク割り込みを許可し、割り込みハンドラを設定する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInterrupts(void)
{
    // V ブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // 割り込み許可
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitHeap

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitHeap(void)
{
    void*           tempLo;
    OSHeapHandle    hh;

    // メインメモリ上のアリーナにヒープをひとつ作成
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        // ヒープ作成に失敗した場合は異常終了
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         ReadKey

  Description:  キー入力情報を取得し、入力情報構造体を編集する。
                押しトリガ、離しトリガ、押し継続リピートトリガ を検出する。

  Arguments:    pKey  - 編集するキー入力情報構造体を指定する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadKey(KeyInfo* pKey)
{
    static u16  repeat_count[12];
    int         i;
    u16         r;

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
                    repeat_count[i] = (u16) (KEY_REPEAT_START - KEY_REPEAT_SPAN);
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

    pKey->cnt = r;  // 未加工キー入力
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

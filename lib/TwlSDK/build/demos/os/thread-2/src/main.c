/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - thread-2
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-02#$
  $Rev: 5266 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define STACK_SIZE     1024
#define THREAD1_PRIO   1
#define THREAD2_PRIO   2

OSThread thread1;
OSThread thread2;

u32 stack1[STACK_SIZE / sizeof(u32)];
u32 stack2[STACK_SIZE / sizeof(u32)];

void VBlankIntr(void);
void proc1(void *arg);
void proc2(void *arg);

//================================================================================
//      割り込みを利用したスレッドの切り替えのサンプル
//      IRQ ハンドラ内でスレッドを起動する。
//      ただし実際にスレッドの切り替えが起こるのは IRQ ルーチン終了まで延期される．
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    s32     n;

    OS_Init();
    OS_InitThread();

    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, NULL, stack2 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD2_PRIO);

    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);

    //================ 設定
#ifdef SDK_ARM9
    //---- グラフィクス初期化
    GX_Init();
#endif // SDK_ARM9

    //----  Vブランク割込許可
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    //---- Vブランク発生設定
    (void)GX_VBlankIntr(TRUE);

    //================ メインループ
    for (n = 0; n < 5; n++)
    {
        //---- Vブランク割込終了待ち
        OS_WaitVBlankIntr();

        OS_Printf("main\n");
    }

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理 (IRQ 内処理)
//
void VBlankIntr(void)
{
    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);

    //---- スレッド起動
    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);
}

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#pragma unused(arg)
    while (1)
    {
        OS_Printf("---- Thread1 sleep\n");
        OS_SleepThread(NULL);
        OS_Printf("---- Thread1 wakeup\n");
    }
}

//--------------------------------------------------------------------------------
//    proc2
//
void proc2(void *arg)
{
#pragma unused(arg)
    while (1)
    {
        OS_Printf("---- Thread2 sleep\n");
        OS_SleepThread(NULL);
        OS_Printf("---- Thread2 wakeup\n");
    }
}

/*====== End of main.c ======*/

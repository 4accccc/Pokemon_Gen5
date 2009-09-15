/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - CARD - card-mrom
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


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  メイン エントリポイント.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    OS_InitTick();
    CARD_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // 大量のリードアクセスを 10 回繰り返して転送速度を計測。
    // DMA転送は割り込みハンドラのオーバーヘッドがかかるのでCPU転送を使用する。
    {
        enum
        { BUFFER_SIZE = 1 * 1024 * 1024 };
        static u8   buf[BUFFER_SIZE];
        int         i;
        OSTick      t;
        const u16   id = (u16)OS_GetLockID();

        CARD_LockRom(id);
        for (i = 0; i < 10; ++i)
        {
            t = OS_GetTick();
            CARD_ReadRom(MI_DMA_NOT_USE, CARD_GetOwnRomHeader()->main_rom_offset, buf, BUFFER_SIZE);
            t = OS_GetTick() - t;
            /* n[B/s] = 1[MB] / t[us] */
            OS_Printf("[%2d] ... %10.6f[B/s]\n", i + 1,
                      1.0 * BUFFER_SIZE / OS_TicksToMicroSeconds(t));
        }
        CARD_UnlockRom(id);

        OS_TPrintf("[end]\n");
    }

    OS_Terminate();
}

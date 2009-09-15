/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - OS - forKorea-1
  File:     main.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include "DEMO.h"


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
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
    DEMOStartDisplay();

    OS_Printf("forKorea-1 test started.\n");

    {
        OSOwnerInfo ownerInfo;
        int         frame;

        OS_GetOwnerInfo(&ownerInfo);

        for (frame = 0; ; ++frame)
        {
            DEMOClearString();

            DEMOPutString(4, 4, "Frame : %d", frame);
            DEMOPutString(4, 8, "LanguageCode : %d", ownerInfo.language);

            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

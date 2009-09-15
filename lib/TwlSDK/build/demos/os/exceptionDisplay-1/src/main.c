/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - exceptionDisplay-1
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

void    myExceptionCallback(u32 context, void *arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
void NitroMain(void)
{
    OS_Init();

	OS_EnableUserExceptionHandlerOnDebugger();
    OS_SetUserExceptionHandler(myExceptionCallback, (void *)0);

    //---- ここでキャッシュをフラッシュするのは、
    //     例外が発生するとプロテクションユニットが変更されるので
    //     例外に関するアドレス等は確実に書き込んでおきたいから。
    DC_FlushAll();
    DC_WaitWriteBufferEmpty();


    //---- not ocurred exception by div0
    OS_Printf("now calculate to divide by 0.\n");
    {
        int     a = 1;
        int     b = 0;
        volatile int c = a / b;
    }
    OS_Printf("not occurred exception.\n");

    //---- force to occur exception
    OS_Printf("now force to occur exception...\n");
    asm
    {
        /* *INDENT-OFF* */
        ldr      r0, =123
        ldr      r1, =0
        ldr      r2, =256
        ldr      r3, =1000
        ldr      r4, =2000
        ldr      r5, =3000
        ldr      r6, =4000
        ldr      r7, =5000
        mov      r8, pc
        ldr      r9, =0xff
        ldr      r10, =0xee
        ldr      r12, =0x123

#ifdef SDK_ARM9
        ldr      r11, [r1,#0]
#else  //SDK_ARM9
        dcd      0x06000010

#endif //SDK_ARM9
        
        /* *INDENT-ON* */
    }
    OS_Printf("not occurred exception.\n");

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         myExceptionCallback

  Description:  user callback for exception

  Arguments:    context :
                arg     :

  Returns:      None
 *---------------------------------------------------------------------------*/
void myExceptionCallback(u32 context, void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( context, arg )
#endif

    OS_Printf("---- USER EXCEPTION CALLBACK\n");
    OS_Printf("context=%x  arg=%x\n", context, arg);

    OS_Printf("---- Thread LIST\n");
    OS_DumpThreadList();
}

#include <nitro/codereset.h>

/*====== End of main.c ======*/

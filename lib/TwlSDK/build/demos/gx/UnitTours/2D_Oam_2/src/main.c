/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_Oam_2
  File:     main.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that rotates an OBJ:
//
// USAGE:
//   LEFT, L: Turn left
//   RIGHT,R: Turn right
//
// HOWTO:
// 1. Set up the character/palette/attribute data same as 2D_Oam_1.
// 2. Set up a matrix by G2_SetOBJAffine().
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"

static GXOamAttr sOamBak[128];         // buffer for OAM

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     rotate = 0;

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and make OBJ visible.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayOBJOnly();

    //---------------------------------------------------------------------------
    // Transmitting the character data and the palette data
    //---------------------------------------------------------------------------
    GX_LoadOBJ(d_64_256_obj_schDT, 0, sizeof(d_64_256_obj_schDT));
    GX_LoadOBJPltt(d_64_256_obj_sclDT, 0, sizeof(d_64_256_obj_sclDT));

    MI_DmaFill32(3, sOamBak, 192, sizeof(sOamBak));     // OAMバッファをクリアー

    DEMOStartDisplay();
    //---------------------------------------------------------------------------
    // Main Loop
    //---------------------------------------------------------------------------
    while (1)
    {
        DEMOReadKey();

        if (DEMO_IS_PRESS(PAD_KEY_RIGHT))
            rotate += 256;
        if (DEMO_IS_PRESS(PAD_KEY_LEFT))
            rotate -= 256;
        if (DEMO_IS_PRESS(PAD_BUTTON_R))
            rotate += 256;
        if (DEMO_IS_PRESS(PAD_BUTTON_L))
            rotate -= 256;
        if (DEMO_IS_TRIG(PAD_BUTTON_SELECT))
            rotate = 0;

        G2_SetOBJAttr(&sOamBak[0],     // a pointer to the attributes
                      60,              // x
                      35,              // y
                      0,               // priority
                      GX_OAM_MODE_NORMAL,       // OBJ mode
                      FALSE,           // mosaic
                      GX_OAM_EFFECT_AFFINE,     // flip/affine/no display/affine(double)
                      GX_OAM_SHAPE_64x64,       // size and shape
                      GX_OAM_COLORMODE_256,     // OBJ character data are in 256 colors format
                      0,               // character name
                      0,               // color param
                      0                // affine param
            );

        {
            fx16    sinVal, cosVal;
            MtxFx22 mtx;
            GXOamAffine *tmp = (GXOamAffine *)(&sOamBak[0]);

            sinVal = FX_SinIdx(rotate);
            cosVal = FX_CosIdx(rotate);
            mtx._00 = cosVal;
            mtx._01 = sinVal;
            mtx._10 = -sinVal;
            mtx._11 = cosVal;
            G2_SetOBJAffine(tmp, &mtx);
        }

        // Store the data onto the main memory, and invalidate the cache.
        DC_FlushRange(sOamBak, sizeof(sOamBak));
        /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
        // DC_WaitWriteBufferEmpty();

#ifdef SDK_AUTOTEST
        rotate = 4098;                 // default params for testing.
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x2F6D8647);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        OS_WaitVBlankIntr();           // Waiting the end of VBlank interrupt

        GX_LoadOAM(sOamBak, 0, sizeof(sOamBak));
    }
}

//---------------------------------------------------------------------------
// VBlank interrupt function:
//
// Interrupt handlers are registered on the interrupt table by OS_SetIRQFunction.
// OS_EnableIrqMask selects IRQ interrupts to enable, and
// OS_EnableIrq enables IRQ interrupts.
// Notice that you have to call 'OS_SetIrqCheckFlag' to check a VBlank interrupt.
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

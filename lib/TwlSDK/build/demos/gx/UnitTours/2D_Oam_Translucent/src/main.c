/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_Oam_Translucent
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-30#$
  $Rev: 9157 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that use OBJ with alpha blending effect.
// 
// This sample displays two spheres on the display.
// One of the sphere is transparent.
// 
// USAGE:
//   UP, DOWN   : Change rate of transparent.
//   
// HOWTO:
// 1. Set GX_OAM_MODE_XLU to OAM attribute to use transparent OBJ.
// 2. Set transparent object by G2_SetBlendAlpha().
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"
//---------------------------------------------------------------------------
//  ŠTàF
//        ‚n‚`‚lƒoƒbƒtƒ@—Ìˆæ
//---------------------------------------------------------------------------
static GXOamAttr s_OAMBuffer[128];

//---------------------------------------------------------------------------
//  ŠTàF
//  @@@‚uƒuƒ‰ƒ“ƒNŠ„‚èž‚Ýˆ—
//  à–¾F
//  @@@‚uƒuƒ‰ƒ“ƒNŠ„‚èž‚Ý‚ªs‚í‚ê‚½‚±‚Æ‚ðŠm”F‚·‚é‚½‚ß‚Ìƒtƒ‰ƒO‚ð—§‚Ä‚Ü‚·B
//
//        ‹¤’Ê‰Šú‰»iDEMOInitCommon()j‚ÅA
//        E‚h‚q‚pŠ„‚èž‚Ý‚Ì‘I‘ð(OS_SetIrqMask())B
//        E‚h‚q‚pŠ„‚èž‚Ý‚ðs‚¤‚±‚ÌŠÖ”‚Ì“o˜^(OS_SetIRQFunction())B
//        E‚h‚q‚pŠ„‚èž‚Ý‚Ì‹–‰Â‚ÌÝ’è(OS_EnableIrq())B
//        ‚ªs‚í‚ê‚é‚±‚Æ‚ÅA‚±‚ÌŠÖ”‚ª‚uƒuƒ‰ƒ“ƒN‚Ì‚Æ‚«‚ÉŒÄ‚Î‚ê‚Ü‚·B
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // VBlank Š„‚èž‚Ý‚ÌŠm”Fƒtƒ‰ƒO‚ðÝ’è
}

//---------------------------------------------------------------------------
//  ŠTàF
//        ”¼“§–¾‚n‚a‚i‚Ì•\Ž¦
//  à–¾F
//        ”¼“§–¾‚Ì‚n‚a‚iƒLƒƒƒ‰ƒNƒ^[‚ð•\Ž¦‚·‚éB
//        ¶‘¤‚É•\Ž¦‚³‚ê‚Ä‚¢‚éOBJ‚Í’Êí•\Ž¦‚ÅA‰E‘¤‚É•\Ž¦‚³‚ê‚Ä‚¢‚éOBJ‚Í”¼“§–¾
//      OBJB
//  ‘€ìF
//        UP,DOWN: ”¼“§–¾OBJ‚ÌƒuƒŒƒ“ƒfƒBƒ“ƒOŒW”‚ð•ÏX‚Å‚«‚éB
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     eva = 12;

    //---------------------
    // ‰Šú‰»
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayOBJOnly();

    /* ƒLƒƒƒ‰ƒNƒ^[ƒf[ƒ^[‚ÆƒpƒŒƒbƒgƒf[ƒ^“Ç‚Ýž‚Ý */
    GX_LoadOBJ(d_64_256_obj_schDT, 0, sizeof(d_64_256_obj_schDT));
    GX_LoadOBJPltt(d_64_256_obj_sclDT, 0, sizeof(d_64_256_obj_sclDT));

    DEMOStartDisplay();

    //---------------------
    //  ƒƒCƒ“ƒ‹[ƒv
    //---------------------
    while (1)
    {
        /* ƒpƒbƒhî•ñ‚Ì“Ç‚Ýž‚Ý */
        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // code for auto test
        {
            const EXTKeys keys[8] = { {PAD_KEY_DOWN, 5}, {0, 0} };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        if (DEMO_IS_PRESS(PAD_KEY_UP))
        {
            if (++eva > 0x10)
            {
                eva = 0x10;
            }
            OS_Printf("eva=%d\n", eva);
        }
        else if (DEMO_IS_PRESS(PAD_KEY_DOWN))
        {
            if (--eva < 0x00)
            {
                eva = 0x00;
            }
            OS_Printf("eva=%d\n", eva);
        }

        /* ’Êí‚n‚a‚i‚Ì‚n‚`‚lƒAƒgƒŠƒrƒ…[ƒg‚ðÝ’è */
        G2_SetOBJAttr(&s_OAMBuffer[0], // ‚P”Ô–Ú‚Ì‚n‚`‚l‚ðŽg—p
                      0,               // ‚wˆÊ’u
                      0,               // ‚xˆÊ’u
                      0,               // —Dæ‡ˆÊ
                      GX_OAM_MODE_NORMAL,       // ’ÊíOBJ
                      FALSE,           // ƒ‚ƒUƒCƒN–³Œø
                      GX_OAM_EFFECT_NONE,       // “ÁŽêŒø‰Ê–³‚µ
                      GX_OAM_SHAPE_64x64,       // ‚n‚a‚i‚Ì‘å‚«‚³
                      GX_OAM_COLOR_256, // ‚Q‚T‚UF
                      0,               // ƒLƒƒƒ‰–¼
                      0, 0);

        /* ”¼“§–¾‚n‚a‚i‚Ì‚n‚`‚lƒAƒgƒŠƒrƒ…[ƒg‚ðÝ’è */
        G2_SetOBJAttr(&s_OAMBuffer[1], // ‚Q”Ô–Ú‚Ì‚n‚`‚l‚ðŽg—p
                      64,              // ‚wˆÊ’u
                      0,               // ‚xˆÊ’u
                      0,               // —Dæ‡ˆÊ
                      GX_OAM_MODE_XLU, // ”¼“§–¾OBJ
                      FALSE,           // ƒ‚ƒUƒCƒN–³Œø
                      GX_OAM_EFFECT_NONE,       // “ÁŽêŒø‰Ê–³‚µ
                      GX_OAM_SHAPE_64x64,       // ‚n‚a‚i‚Ì‘å‚«‚³
                      GX_OAM_COLOR_256, // ‚Q‚T‚UF
                      0,               // ƒLƒƒƒ‰–¼
                      0, 0);
        /* ƒ¿ƒuƒŒƒ“ƒfƒBƒ“ƒOÝ’è */
        G2_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE,       // ‘æˆê‘ÎÛ–Ê‚ÍŽw’è‚È‚µ
                         GX_BLEND_PLANEMASK_BD, // ‘æ“ñ‘ÎÛ–Ê‚ÍƒoƒbƒNƒhƒƒbƒv
                         eva,          // ‘æˆê‘ÎÛ–Ê‚Ö‚ÌƒuƒŒƒ“ƒhŒW”‚ÌÝ’è
                         0);           // 

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x80C7021F);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        /* ƒLƒƒƒbƒVƒ…‚ðƒƒCƒ“ƒƒ‚ƒŠ[‚É“f‚«o‚· */
        DC_FlushRange(s_OAMBuffer, sizeof(s_OAMBuffer));
        /* DMA‘€ì‚ÅIOƒŒƒWƒXƒ^‚ÖƒAƒNƒZƒX‚·‚é‚Ì‚ÅƒLƒƒƒbƒVƒ…‚Ì Wait ‚Í•s—v */
        // DC_WaitWriteBufferEmpty();

        /* ‚u‚a‚Œ‚‚Ž‚‹‘Ò‚¿ */
        OS_WaitVBlankIntr();

        /* ‚n‚`‚l‚Ö“]‘— */
        GX_LoadOAM(s_OAMBuffer,        // OAMƒoƒbƒtƒ@‚ðOAM‚Ö“]‘—
                   0, sizeof(s_OAMBuffer));
        MI_DmaFill32(3,                // OAMƒoƒbƒtƒ@‚ðƒNƒŠƒA[
                     s_OAMBuffer, 192, sizeof(s_OAMBuffer));
    }
}

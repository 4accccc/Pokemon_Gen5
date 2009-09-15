/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - DSP - g711
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/g711.h>

#include <DEMO.h>


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    // OSの初期化。
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    FS_Init(FS_DMA_NOT_USE);
    SNDEX_Init();

    // 画面表示の初期化。
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    // DSPコンポーネントをロード。
    {
        FSFile  file[1];
        int     slotB;
        int     slotC;
        // このサンプルでは全てのWRAMをDSPのために確保しているが
        // 実際には必要なスロット数だけあればよい。
        (void)MI_FreeWram_B(MI_WRAM_ARM9);
        (void)MI_CancelWram_B(MI_WRAM_ARM9);
        (void)MI_FreeWram_C(MI_WRAM_ARM9);
        (void)MI_CancelWram_C(MI_WRAM_ARM9);
        slotB = 0xFF;
        slotC = 0xFF;
        // このサンプルでは簡易化のため静的メモリ上のファイルを使用する。
        DSP_OpenStaticComponentG711(file);
        if (!DSP_LoadG711(file, slotB, slotC))
        {
            OS_TPanic("failed to load G.711 DSP-component! (lack of WRAM-B/C)");
        }
    }

    // G.711が正しく符号化・復号できることを確認。
    {
        enum { sample_wave_max = 512 * 1024 };
        static u8   encbuf[sample_wave_max] ATTRIBUTE_ALIGN(4);
        static u8   srcbuf[sample_wave_max] ATTRIBUTE_ALIGN(4);
        static u8   dstbuf[sample_wave_max] ATTRIBUTE_ALIGN(4);
        u32         length;
        // まず変換対象のPCM波形を用意する。
        {
            FSFile  file[1];
            u32     chunk;
            FS_InitFile(file);
            if (!FS_OpenFileEx(file, "rom:/fanfare.32.wav", FS_FILEMODE_R) ||
                !FS_SeekFile(file, 0x24, FS_SEEK_SET) ||
                (FS_ReadFile(file, &chunk, 4) != 4) ||
                (chunk != (('d'<<0)|('a'<<8)|('t'<<16) |('a'<<24))) ||
                (FS_ReadFile(file, &length, 4) != 4) ||
                (FS_ReadFile(file, srcbuf, (s32)length) != length))
            {
                OS_TPanic("cannot prepare sample waveform!");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(srcbuf, length);
        }
        // 単位をサンプル数へ統一。
        length /= sizeof(u16);
        // 得られたPCM波形をA-law形式で符号化。
        DSP_EncodeG711(encbuf, srcbuf, length, DSP_AUDIO_CODEC_MODE_G711_ALAW);
        // この場で完了を待つ。
        DSP_WaitForG711();
        // 得られたA-law形式をPCM波形に復号。
        DSP_DecodeG711(dstbuf, encbuf, length, DSP_AUDIO_CODEC_MODE_G711_ALAW);
        // この場で完了を待つ。
        DSP_WaitForG711();

        // G.711とは直接関係ないが、Aボタンを押したらサウンドを再生させてみる。
        (void)SNDEX_SetDSPMixRate(0);
        for (;;)
        {
            DEMOReadKey();
            if (DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                DSP_PlaySound(srcbuf, length*2, FALSE); // length はサンプル数なので、バイト単位に修正
            }
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    // 不要になればDSPコンポーネントをアンロード。
    DSP_UnloadG711();

    // ごく単純なサンプルなので画面表示すべきものはない。
    OS_Terminate();
}

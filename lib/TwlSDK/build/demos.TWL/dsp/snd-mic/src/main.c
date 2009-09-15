/*---------------------------------------------------------------------------*
  Project:  TwlSDK - DSP - demos.TWL - snd-mic
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9562 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/


#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/g711.h>

#include <DEMO.h>


/*---------------------------------------------------------------------------*/
/* constants */

#define TEST_BUFFER_SIZE    (1024 * 32)
#define WAV_BUFFER_SIZE     (1024 * 512)


/*---------------------------------------------------------------------------*/
/* variables */

// サウンド再生テスト用の音声バッファ。
static u8           gWavData[WAV_BUFFER_SIZE] ATTRIBUTE_ALIGN(4);
static u32          gWavSize;

// マイクサンプリングテスト用のリングバッファ。
static u8           gDspData[TEST_BUFFER_SIZE] ATTRIBUTE_ALIGN(2);

// DSPとの比較用に実行するMICライブラリのサンプリング制御情報。
static MICAutoParam gMicAutoParam;
static u8           gMicData[TEST_BUFFER_SIZE] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         NormalizeRingBuffer

  Description:  リングバッファから最新のHW_LCD_HEIGHTサンプル分だけ取得。

  Arguments:    dst      : サンプル格納先。
                type     : サンプル種別を表すMICSamplingType型。
                buffer   : サンプルの格納されたリングバッファ先頭アドレス。
                position : 最後のサンプリング位置。(バイト単位)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NormalizeRingBuffer(void *dst, MICSamplingType type, const void *buffer, int position)
{
    const u8   *src = (const u8 *)buffer;
    u8         *tmp = (u8 *)dst;
    int         smp = 1;
    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
        smp = sizeof(u8);
        break;
    case MIC_SAMPLING_TYPE_12BIT:
    case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
        smp = sizeof(u16);
        break;
    }
    {
        int     max = TEST_BUFFER_SIZE / smp;
        int     ofs = (position / smp - (HW_LCD_HEIGHT - 1)) & (max - 1);
        int     pos = 0;
        while (pos < HW_LCD_HEIGHT)
        {
            int     n = MATH_MIN(HW_LCD_HEIGHT - pos, max - ofs);
            MI_CpuCopy8(&src[ofs * smp], &tmp[pos * smp], (u32)(n * smp));
            ofs = (ofs + n) & (max - 1);
            pos += n;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DrawWaveGraph

  Description:  三角ポリゴンで波形を描画

  Arguments:    type   : サンプル種別を表すMICSamplingType型。
                color  : 波形の色。
                buffer : 指定したサンプル種別で格納された波形配列。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawWaveGraph(MICSamplingType type, GXRgb color, const void *buffer)
{
    int     i;
    int     bits = 8;
    BOOL    isSigned = FALSE;
    switch (type)
    {
    default:
    case MIC_SAMPLING_TYPE_8BIT:
        bits = 8, isSigned = FALSE;
        break;
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
        bits = 8, isSigned = TRUE;
        break;
    case MIC_SAMPLING_TYPE_12BIT:
    case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
        bits = 16, isSigned = FALSE;
        break;
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
        bits = 16, isSigned = TRUE;
        break;
    }
    for (i = 0; i < HW_LCD_HEIGHT - 1; ++i)
    {
        u8      cur8 = (u8)((bits == 8) ? ((const u8*)buffer)[i] : (((const u16*)buffer)[i] >> 8));
        u8      nxt8 = (u8)((bits == 8) ? ((const u8*)buffer)[i + 1] : (((const u16*)buffer)[i + 1] >> 8));
        int     cur = isSigned ? (((s8)cur8) + 128) : cur8;
        int     nxt = isSigned ? (((s8)nxt8) + 128) : nxt8;
        fx16    fsx = (fx16)(((cur - 128) * 0x1000) / 128);
        fx16    fsy = (fx16)(((96 - i) * 0x1000) / 96);
        fx16    fex = (fx16)(((nxt - 128) * 0x1000) / 128);
        fx16    fey = (fx16)(((96 - i + 1) * 0x1000) / 96);
        G3_Begin(GX_BEGIN_TRIANGLES);
        {
            G3_Color(color);
            G3_Vtx(fsx, fsy, 0);
            G3_Color(color);
            G3_Vtx(fex, fey, 0);
            G3_Color(color);
            G3_Vtx(fsx, fsy, 1);
        }
        G3_End();
    }
}

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
    G3X_InitMtxStack();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    OS_TPrintf("A button: start DSP sampling\n");
    OS_TPrintf("B button: stop DSP sampling\n");
    OS_TPrintf("X button: pause rendering\n");
    OS_TPrintf("Y button: play oneshot sound\n");
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMODrawText(0, 170, "--- DSP");
    DEMOSetBitmapTextColor(GX_RGBA(31, 31,  0, 1));
    DEMODrawText(0, 180, "--- MIC");
    DEMOSetBitmapTextColor(GX_RGBA( 0, 31,  0, 1));
    DEMODrawText(220,   5, "OLD");
    DEMODrawText(220, 180, "NEW");

    // マイクモジュールの電源をON。
    (void)PM_SetAmp(PM_AMP_ON);

    // MICライブラリの自動サンプリングを開始。
    {
        MIC_Init();
        gMicAutoParam.type = MIC_SAMPLING_TYPE_12BIT;
        gMicAutoParam.buffer = (void *)gMicData;
        gMicAutoParam.size = TEST_BUFFER_SIZE;
        gMicAutoParam.rate = MIC_SAMPLING_RATE_32730;
        gMicAutoParam.loop_enable = TRUE;
        gMicAutoParam.full_callback = NULL;
        for (;;)
        {
            MICResult   result = MIC_StartLimitedSampling(&gMicAutoParam);
            if (result == MIC_RESULT_SUCCESS)
            {
                break;
            }
            else if ((result == MIC_RESULT_BUSY) || (result == MIC_RESULT_SEND_ERROR))
            {
                OS_Sleep(1);
            }
            else if (result == MIC_RESULT_ILLEGAL_STATUS)
            {
                OS_TWarning("Already started sampling.\n");
                break;
            }
            else if (result == MIC_RESULT_ILLEGAL_PARAMETER)
            {
                OS_TWarning("Illegal parameter to start automatic sampling.\n");
                break;
            }
            else
            {
                OS_TPanic("MIC_StartLimtedSampling() replied fatal error (%d).\n", result);
            }
        }
    }

    // DSPコンポーネントをロード。
    // (どのコンポーネントでもよいのでここではG.711を使用)
    {
        (void)MI_FreeWram_B(MI_WRAM_ARM9);
        (void)MI_CancelWram_B(MI_WRAM_ARM9);
        (void)MI_FreeWram_C(MI_WRAM_ARM9);
        (void)MI_CancelWram_C(MI_WRAM_ARM9);
        {
            FSFile  file[1];
            DSP_OpenStaticComponentG711(file);
            if (!DSP_LoadG711(file, 0xFF, 0xFF))
            {
                OS_TPanic("can't allocate WRAM Slot");
            }
        }
    }

    // サウンド再生テスト用のPCM波形を準備。
    {
        FSFile  file[1];
        u32     chunk;
        FS_InitFile(file);
        if (!FS_OpenFileEx(file, "rom:/fanfare.32.wav", FS_FILEMODE_R) ||
            !FS_SeekFile(file, 0x24, FS_SEEK_SET) ||
            (FS_ReadFile(file, &chunk, 4) != 4) ||
            (chunk != (('d'<<0)|('a'<<8)|('t'<<16) |('a'<<24))) ||
            (FS_ReadFile(file, &gWavSize, 4) != 4) ||
            (FS_ReadFile(file, gWavData, (s32)gWavSize) != gWavSize))
        {
            OS_TPanic("cannot prepare sample waveform!");
        }
        (void)FS_CloseFile(file);
        DC_StoreRange(gWavData, gWavSize);
    }

    // DSPサウンドを100%に設定。
    (void)SNDEX_SetDSPMixRate(0);

    // メインループ。
    for (;;)
    {
        static BOOL pauseRendering = FALSE;

        DEMOReadKey();

        // A/BボタンでDSPサンプリングを開始/停止。
        if (DEMO_IS_TRIG(PAD_BUTTON_A))
        {
            DSP_StartSampling(gDspData, sizeof(gDspData));
        }
        if (DEMO_IS_TRIG(PAD_BUTTON_B))
        {
            DSP_StopSampling();
        }
        // Xボタンで波形描画を一時停止。
        if (DEMO_IS_TRIG(PAD_BUTTON_X))
        {
            pauseRendering = !pauseRendering;
        }
        // Yボタンでサウンドを再生。
        if (DEMO_IS_TRIG(PAD_BUTTON_Y))
        {
            DSP_PlaySound(gWavData, gWavSize, FALSE);
        }

        // 比較のため、MICとDSPのサンプリング位置をなるべく同時に取得する。
        {
            const u8   *dsp, *mic;

            // DSP内部のサンプリングバッファと定期的に同期する必要があるが
            // この処理はCPUで行われるためキャッシュの無効化は不要。
            DSP_SyncSamplingBuffer();
            dsp = (const u8 *)DSP_GetLastSamplingAddress();
            mic = (const u8 *)MIC_GetLastSamplingAddress();
            // MICライブラリはARM7が直接メインメモリに書き込むため
            // 明示的な同期は不要だがキャッシュの無効化が必要。
            DC_InvalidateRange(gMicData, sizeof(gMicData));

            // 両者の最新波形を描画。
            if (!pauseRendering)
            {
                G3X_Reset();
                G3_Identity();
                G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);

                {
                    u8          tmp[HW_LCD_HEIGHT * sizeof(u16)];
                    const u8   *src = gDspData;
                    MICSamplingType type = MIC_SAMPLING_TYPE_SIGNED_12BIT;
                    NormalizeRingBuffer(tmp, type, src, dsp - src);
                    DrawWaveGraph(type, GX_RGB(31, 31, 31), tmp);
                }

                if (mic != NULL)
                {
                    u8          tmp[HW_LCD_HEIGHT * sizeof(u16)];
                    const u8   *src = gMicData;
                    MICSamplingType type = gMicAutoParam.type;
                    NormalizeRingBuffer(tmp, type, src, mic - src);
                    DrawWaveGraph(type, GX_RGB(31, 31,  0), tmp);
                }

                G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
            }
        }

        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }

    OS_Terminate();
}

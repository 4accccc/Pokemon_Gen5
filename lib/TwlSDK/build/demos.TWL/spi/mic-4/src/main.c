/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - spi - mic-4
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-11#$
  $Rev: 8401 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    A sample that controls microphone sampling, and output sampling data.

    USAGE:
        UP , DOWN    : Control sampling span.
        LEFT , RIGHT : Control sampling bit range. ( 8bit or 12 bit etc. )
        A            : start / stop sampling. (toggle)
        B            : force-stop. (stop and ignore rest data)
        SELECT       : Terminate program.

    HOWTO:
        1. Initialize MIC library and SNDEX library.
        2. When sampling is stopped, you can change parameters
           and start automatic microphone sampling.
           Debug-output is sampling data for tool "mic2wav.exe".
        3. Debug-output log can make its waveform files by tool "mic2wav.exe".
           > $(TWLSDK_ROOT)/tools/bin/mic2wav.exe [logfile] [,directory]
           Each sampling data (separeted by A-Button) creates a waveform file
           in "[directory]/%08X.wav".

    NOTE:
        1. The speed of debug-output is later than sampling.
           When you stop sampling, then please wait all data is printed.
 *---------------------------------------------------------------------------*/

#include    <twl.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     TEST_BUFFER_SIZE    (1024 * 1024)   // 1M bytes
#define     RETRY_MAX_COUNT     8

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void     StartSampling(const MICAutoParam* param);
static void     StopSampling(void);
static SNDEXFrequency   GetI2SFrequency(void);
static void     SetI2SFrequency(SNDEXFrequency freq);
static void     SwitchI2SFrequency(void);

static void     StepUpSamplingRate(void);
static void     StepDownSamplingRate(void);
static void     OnSamplingDone(MICResult result, void* arg);
static void     OutputSampleWave(void* dat, MICSamplingType type);
static void     StartSamplingOutput(void);
static void     StopSamplingOutput(void);

static void     SetDrawData(void *address, MICSamplingType type);
static void     VBlankIntr(void);
static void     Init3D(void);
static void     Draw3D(void);
static void     DrawLine(s16 sx, s16 sy, s16 ex, s16 ey);

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static MICAutoParam gMicAutoParam;
static u8           gDrawData[192];
static u8           gMicData[TEST_BUFFER_SIZE] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);

static volatile BOOL g_sample_busy  =   FALSE;
static void*        g_record_smps   =   NULL;

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    /* 各種初期化 */
    OS_Init();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    /* 表示設定初期化 */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    /* 3D関連初期化 */
    Init3D();

    /* 割込み設定 */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    /* 関連ライブラリ初期化 */
    {
        (void)PM_SetAmp(PM_AMP_ON);
        SNDEX_Init();
        MIC_Init();
    
        gMicAutoParam.type      =   MIC_SAMPLING_TYPE_12BIT;
        gMicAutoParam.buffer    =   (void *)gMicData;
        gMicAutoParam.size      =   TEST_BUFFER_SIZE;
        if ((OS_IsRunOnTwl() == TRUE) && (GetI2SFrequency() == SNDEX_FREQUENCY_47610))
        {
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_23810;
        }
        else
        {
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_16360;
        }
        gMicAutoParam.loop_enable   =   FALSE;
        gMicAutoParam.full_callback =   OnSamplingDone;
    }

    /* 内部変数初期化 */
    MI_CpuFill8(gDrawData, 0x80, sizeof(gDrawData));

    /* LCD表示開始 */
    GX_DispOn();
    GXS_DispOn();

    /* デバッグ文字列出力 */
    OS_Printf("# mic-4 demo started.\n");
    OS_Printf("#   up/down    -> change sampling rate\n");
    OS_Printf("#   left/right -> change bit range\n");
    OS_Printf("#   A          -> start / stop\n");
    OS_Printf("#   B          -> force-stop\n");
    OS_Printf("#   select     -> terminate\n");

    /* キー入力情報取得の空呼び出し(IPL での A ボタン押下対策) */
    (void)PAD_Read();

    {
        u16     keyOld  =   0;
        u16     keyTrg;
        u16     keyNow;
    
        /* メインループ */
        while (TRUE)
        {
            /* キー入力情報取得 */
            keyNow  =   PAD_Read();
            keyTrg  =   (u16)((keyOld ^ keyNow) & keyNow);
            keyOld  =   keyNow;
        
            /* A によりサンプリング開始/停止 */
            if (keyTrg & PAD_BUTTON_A)
            {
                if (!g_sample_busy)
                {
                    g_sample_busy = TRUE;
                    g_record_smps = gMicData;
                    StartSamplingOutput();
                    StartSampling(&gMicAutoParam);
                }
                else
                {
                    StopSampling();
                    OnSamplingDone(MIC_RESULT_SUCCESS, NULL);
                }
            }
            /* B によりサンプリング停止、残データ無視 */
            if (keyTrg & PAD_BUTTON_B)
            {
                if (g_sample_busy)
                {
                    StopSampling();
                    OnSamplingDone(MIC_RESULT_SUCCESS, NULL);
                }
                if (g_record_smps)
                {
                    StopSamplingOutput();
                }
            }
            /* SELECT によりプログラムを強制終了 */
            if (keyTrg & PAD_BUTTON_SELECT)
            {
                OS_Exit(0);
            }
            /* 可変パラメータ変更 */
            if (!g_record_smps)
            {
                /* サンプリング種別(ビット幅)変更 */
                if (keyTrg & (PAD_KEY_LEFT | PAD_KEY_RIGHT))
                {
                    if (gMicAutoParam.type == MIC_SAMPLING_TYPE_8BIT)
                    {
                        gMicAutoParam.type = MIC_SAMPLING_TYPE_12BIT;
                    }
                    else
                    {
                        gMicAutoParam.type = MIC_SAMPLING_TYPE_8BIT;
                    }
                    StartSamplingOutput();
                }
                /* サンプリングレート変更 */
                if (keyTrg & PAD_KEY_UP)
                {
                    StepUpSamplingRate();
                    StartSamplingOutput();
                }
                if (keyTrg & PAD_KEY_DOWN)
                {
                    StepDownSamplingRate();
                    StartSamplingOutput();
                }
            }
        
            /* サンプリング結果をログ出力 */
            OutputSampleWave(MIC_GetLastSamplingAddress(), gMicAutoParam.type);
        
            /* 波形描画 */
            if (g_sample_busy)
            {
                SetDrawData(MIC_GetLastSamplingAddress(), gMicAutoParam.type);
                Draw3D();
            }
        
            /* Ｖブランク待ち */
            OS_WaitVBlankIntr();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StartSampling

  Description:  MIC_StartLimitedSampling 関数を呼び出し、エラー処理を行う。

  Arguments:    param   -   マイク API 関数に引き渡すパラメータ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StartSampling(const MICAutoParam* param)
{
    MICResult   result;
    s32         retry   =   0;

    while (TRUE)
    {
        result  =   MIC_StartLimitedSampling(param);
        switch (result)
        {
        case MIC_RESULT_SUCCESS:            // 成功
            return;
        case MIC_RESULT_BUSY:               // 他スレッドからマイク操作中
        case MIC_RESULT_SEND_ERROR:         // PXI キューが一杯
            if (++ retry <= RETRY_MAX_COUNT)
            {
                OS_Sleep(1);
                continue;
            }
            OS_TWarning("Retry count overflow.\n");
            return;
        case MIC_RESULT_ILLEGAL_STATUS:     // 状態が自動サンプリング停止中でない
            OS_TWarning("Already started sampling.\n");
            return;
        case MIC_RESULT_ILLEGAL_PARAMETER:  // 指定したパラメータがサポート外
            OS_TWarning("Illegal parameter to start automatic sampling.\n");
            return;
        default:                            // その他の致命的なエラー
            OS_Panic("Fatal error (%d).\n", result);
            /* never return */
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StopSampling

  Description:  MIC_StopLimitedSampling 関数を呼び出し、エラー処理を行う。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StopSampling(void)
{
    MICResult   result;
    s32         retry   =   0;

    while (TRUE)
    {
        result  =   MIC_StopLimitedSampling();
        switch (result)
        {
        case MIC_RESULT_SUCCESS:            // 成功
        case MIC_RESULT_ILLEGAL_STATUS:     // 状態が自動サンプリング中でない
            return;
        case MIC_RESULT_BUSY:               // 他スレッドからマイクを操作中
        case MIC_RESULT_SEND_ERROR:         // PXI キューが一杯
            if (++ retry <= RETRY_MAX_COUNT)
            {
                OS_Sleep(1);
                continue;
            }
            OS_TWarning("Retry count overflow.\n");
            return;
        default:                            // その他の致命的なエラー
            OS_Panic("Fatal error (%d).\n", result);
            /* never return */
        }
    }
}

#include <twl/ltdmain_begin.h>
/*---------------------------------------------------------------------------*
  Name:         GetI2SFrequency

  Description:  I2S 動作周波数を取得する。

  Arguments:    None.

  Returns:      SDNEXFrequency  -   I2S 動作周波数を返す。
 *---------------------------------------------------------------------------*/
static SNDEXFrequency
GetI2SFrequency(void)
{
    SNDEXResult     result;
    SNDEXFrequency  freq;
    s32             retry   =   0;

    while (TRUE)
    {
        result  =   SNDEX_GetI2SFrequency(&freq);
        switch (result)
        {
        case SNDEX_RESULT_SUCCESS:          // 成功
            return freq;
        case SNDEX_RESULT_EXCLUSIVE:        // 排他エラー
        case SNDEX_RESULT_PXI_SEND_ERROR:   // PXI キューが一杯
            if (++ retry <= RETRY_MAX_COUNT)
            {
                OS_Sleep(1);
                continue;
            }
            OS_TWarning("Retry count overflow.\n");
            break;
        case SNDEX_RESULT_BEFORE_INIT:      // 初期化前
        case SNDEX_RESULT_ILLEGAL_STATE:    // 状態異常
            OS_TWarning("Illegal state to get I2S frequency.\n");
            break;
        case SNDEX_RESULT_FATAL_ERROR:      // 致命的なエラー
        default:
            OS_Panic("Fatal error (%d).\n", result);
            /* never return */
        }
    }
    return SNDEX_FREQUENCY_32730;
}

/*---------------------------------------------------------------------------*
  Name:         SetI2SFrequency

  Description:  I2S 動作周波数を変更する。

  Arguments:    freq    -   I2S 動作周波数を指定する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SetI2SFrequency(SNDEXFrequency freq)
{
    SNDEXResult     result;
    s32             retry   =   0;

    while (TRUE)
    {
        result  =   SNDEX_SetI2SFrequency(freq);
        switch (result)
        {
        case SNDEX_RESULT_SUCCESS:          // 成功
            return;
        case SNDEX_RESULT_EXCLUSIVE:        // 排他エラー
        case SNDEX_RESULT_PXI_SEND_ERROR:   // PXI キューが一杯
            if (++ retry <= RETRY_MAX_COUNT)
            {
                OS_Sleep(1);
                continue;
            }
            OS_TWarning("Retry count overflow.\n");
            return;
        case SNDEX_RESULT_BEFORE_INIT:      // 初期化前
        case SNDEX_RESULT_ILLEGAL_STATE:    // 状態異常
            OS_TWarning("Illegal state to set I2S frequency.\n");
            return;
        case SNDEX_RESULT_INVALID_PARAM:    // 引数異常
            OS_TWarning("Could not set I2S frequency into %d.\n", freq);
            return;
        case SNDEX_RESULT_FATAL_ERROR:      // 致命的なエラー
        default:
            OS_Panic("Fatal error (%d)\n", result);
            /* never return */
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         SwitchI2SFrequency

  Description:  マイクサンプリング周波数に応じて I2S 動作周波数を変更する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SwitchI2SFrequency(void)
{
    SNDEXFrequency  freq;

    freq    =   GetI2SFrequency();
    switch (gMicAutoParam.rate)
    {
    case MIC_SAMPLING_RATE_8180:
    case MIC_SAMPLING_RATE_10910:
    case MIC_SAMPLING_RATE_16360:
    case MIC_SAMPLING_RATE_32730:
        if (freq != SNDEX_FREQUENCY_32730)
        {
            SetI2SFrequency(SNDEX_FREQUENCY_32730);
        }
        break;
    case MIC_SAMPLING_RATE_11900:
    case MIC_SAMPLING_RATE_15870:
    case MIC_SAMPLING_RATE_23810:
    case MIC_SAMPLING_RATE_47610:
        if (freq != SNDEX_FREQUENCY_47610)
        {
            SetI2SFrequency(SNDEX_FREQUENCY_47610);
        }
        break;
    }
}

#include <twl/ltdmain_end.h>

/*---------------------------------------------------------------------------*
  Name:         StepUpSamplingRate

  Description:  マイクサンプリング周波数を一段階増加させる。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StepUpSamplingRate(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        switch (gMicAutoParam.rate)
        {
        case MIC_SAMPLING_RATE_8180:    // 32.73k / 4
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_10910;
            break;
        case MIC_SAMPLING_RATE_10910:   // 32.73k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_11900;
            break;
        case MIC_SAMPLING_RATE_11900:   // 47.61k / 4
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_15870;
            break;
        case MIC_SAMPLING_RATE_15870:   // 47.61k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_16360;
            break;
        case MIC_SAMPLING_RATE_16360:   // 32.73k / 2
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_23810;
            break;
        case MIC_SAMPLING_RATE_23810:   // 47.61k / 2
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_32730;
            break;
        case MIC_SAMPLING_RATE_32730:   // 32.73k / 1
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_47610;
            break;
        case MIC_SAMPLING_RATE_47610:   // 47.61k / 1
        default:
            /* Do nothing */
            return;
        }
        SwitchI2SFrequency();
    }
    else
    {
        switch (gMicAutoParam.rate)
        {
        case MIC_SAMPLING_RATE_8180:    // 32.73k / 4
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_10910;
            break;
        case MIC_SAMPLING_RATE_10910:   // 32.73k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_11900;
            break;
        case MIC_SAMPLING_RATE_11900:   // 47.61k / 4
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_15870;
            break;
        case MIC_SAMPLING_RATE_15870:   // 47.61k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_16360;
            break;
        case MIC_SAMPLING_RATE_16360:   // 32.73k / 2
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_23810;
            break;
        case MIC_SAMPLING_RATE_23810:   // 47.61k / 2
        default:
            /* Do nothing */
            return;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StepUpSamplingRate

  Description:  マイクサンプリング周波数を一段階減少させる。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StepDownSamplingRate(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        switch (gMicAutoParam.rate)
        {
        case MIC_SAMPLING_RATE_47610:   // 47.61k / 1
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_32730;
            break;
        case MIC_SAMPLING_RATE_32730:   // 32.73k / 1
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_23810;
            break;
        case MIC_SAMPLING_RATE_23810:   // 47.61k / 2
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_16360;
            break;
        case MIC_SAMPLING_RATE_16360:   // 32.73k / 2
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_15870;
            break;
        case MIC_SAMPLING_RATE_15870:   // 47.61k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_11900;
            break;
        case MIC_SAMPLING_RATE_11900:   // 47.61k / 4
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_10910;
            break;
        case MIC_SAMPLING_RATE_10910:   // 32.73k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_8180;
            break;
        case MIC_SAMPLING_RATE_8180:    // 32.73k / 4
        default:
            /* Do nothing */
            return;
        }
        SwitchI2SFrequency();
    }
    else
    {
        switch (gMicAutoParam.rate)
        {
        case MIC_SAMPLING_RATE_23810:   // 47.61k / 2
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_16360;
            break;
        case MIC_SAMPLING_RATE_16360:   // 32.73k / 2
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_15870;
            break;
        case MIC_SAMPLING_RATE_15870:   // 47.61k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_11900;
            break;
        case MIC_SAMPLING_RATE_11900:   // 47.61k / 4
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_10910;
            break;
        case MIC_SAMPLING_RATE_10910:   // 32.73k / 3
            gMicAutoParam.rate  =   MIC_SAMPLING_RATE_8180;
            break;
        case MIC_SAMPLING_RATE_8180:    // 32.73k / 4
        default:
            /* Do nothing */
            return;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         OnSamplingDone

  Description:  自動サンプリング完了時に状態フラグを更新する。

  Arguments:    result  -   未使用。
                arg     -   未使用。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
OnSamplingDone(MICResult result, void* arg)
{
#pragma unused(result, arg)

    if (g_sample_busy)
    {
        g_sample_busy = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OutputSampleWave

  Description:  サンプリングデータをデバッグ出力する。

  Arguments:    dat  -  出力するデータの先頭アドレス。
                type -  マイクサンプリング種別。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
OutputSampleWave(void* dat, MICSamplingType type)
{
    /* 16 サンプル単位で 1 回に 2 行まで.
       それ以上の量を出力すると、ログが欠落することがある. */
    enum    { smps_per_line = 16, max_line_per_frame = 2 };
    char    buf[1 + (((MATH_MAX(sizeof(u8), sizeof(u16)) * 2) + 1) * smps_per_line) + 2];
    char*   s;
    int     lines, i, j;
    u32     sample;
    u8      temp;

    if (!g_record_smps || !dat)
    {
        return;
    }

    DC_InvalidateRange((void*)g_record_smps, (u32)((u8*)dat - (u8*)g_record_smps));
    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
        /* 8 bit サンプリングの場合 */
        for (lines = 0; (lines < max_line_per_frame) && ((u8*)g_record_smps + smps_per_line <= (u8*)dat); lines ++)
        {
            s   =   buf;
            *s ++   =   '|';
            for (i = 0; i < smps_per_line; i ++)
            {
                sample  =   (u32)(((u8*)g_record_smps)[i]);
                for (j = 0; j < (sizeof(u8) * 2); j ++)
                {
                    temp    =   (u8)((sample >> (((sizeof(u8) * 2) - 1 - j) * 4)) & 0x0f);
                    *s ++   =   (char)((temp < 10) ? ('0' + temp) : ('A' + temp - 10));
                }
                *s ++   =   ',';
            }
            *s ++   =   '\n';
            *s      =   '\0';
            OS_FPutString(OS_PRINT_OUTPUT_CURRENT, buf);
            g_record_smps   =   (void*)((u8*)g_record_smps + smps_per_line);
        }
        break;
    case MIC_SAMPLING_TYPE_12BIT:
        /* 12 bit サンプリングの場合 */
        for (lines = 0; (lines < max_line_per_frame) && ((u16*)g_record_smps + smps_per_line <= (u16*)dat); lines ++)
        {
            s   =   buf;
            *s ++   =   '|';
            for (i = 0; i < smps_per_line; i ++)
            {
                sample  =   (u32)(((u16*)g_record_smps)[i]);
                for (j = 0; j < (sizeof(u16) * 2); j ++)
                {
                    temp    =   (u8)((sample >> (((sizeof(u16) * 2) - 1 - j) * 4)) & 0x0f);
                    *s ++   =   (char)((temp < 10) ? ('0' + temp) : ('A' + temp - 10));
                }
                *s ++   =   ',';
            }
            *s ++   =   '\n';
            *s      =   '\0';
            OS_FPutString(OS_PRINT_OUTPUT_CURRENT, buf);
            g_record_smps   =   (void*)((u16*)g_record_smps + smps_per_line);
        }
        break;
    }

    if (!g_sample_busy && g_record_smps)
    {
        if (((u8*)g_record_smps + (smps_per_line * max_line_per_frame)) >= (u8*)dat)
        {
            StopSamplingOutput();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StartSamplingOutput

  Description:  サンプリング出力開始を示す文字列をデバッグ出力する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StartSamplingOutput(void)
{
    s32     range   =   0;
    s32     rate    =   0;

    switch (gMicAutoParam.type)
    {
    case MIC_SAMPLING_TYPE_8BIT:    range = 8;      break;
    case MIC_SAMPLING_TYPE_12BIT:   range = 16;     break;
    }

    switch (gMicAutoParam.rate)
    {
    case MIC_SAMPLING_RATE_32730:   rate = 32730;   break;
    case MIC_SAMPLING_RATE_16360:   rate = 16360;   break;
    case MIC_SAMPLING_RATE_10910:   rate = 10910;   break;
    case MIC_SAMPLING_RATE_8180:    rate = 8180;    break;
    case MIC_SAMPLING_RATE_47610:   rate = 47610;   break;
    case MIC_SAMPLING_RATE_23810:   rate = 23810;   break;
    case MIC_SAMPLING_RATE_15870:   rate = 15870;   break;
    case MIC_SAMPLING_RATE_11900:   rate = 11900;   break;
    }

    OS_Printf("$rate=%d\n$bits=%d\n", rate, range);
}

/*---------------------------------------------------------------------------*
  Name:         StopSamplingOutput

  Description:  サンプリング出力終了を示す文字列をデバッグ出力する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StopSamplingOutput(void)
{
    OS_Printf("$end\n\n");
    g_record_smps = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         SetDrawData

  Description:  現状での最新サンプリング済みデータを表示に反映させるバッファに
                格納する。

  Arguments:    address - コンポーネントによって最新のサンプリングデータが
                          格納されたメインメモリ上の位置。
                type    - サンプリング種別(ビット幅)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetDrawData(void *address, MICSamplingType type)
{
    s32     i;

    /* サンプリングが一回も行われていない場合は何もせずに終了
       (マイクと無関係のメモリのキャッシュを破棄してしまう為) */
    if ((address < gMicData) || (address >= (gMicData + TEST_BUFFER_SIZE)))
    {
        return;
    }

    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
        /* 8 bit サンプリングの場合 */
        {
            u8     *p;

            p = (u8 *)((u32)address - 191);
            if (p < gMicData)
            {
                p = (u8 *)((u32)p + TEST_BUFFER_SIZE);
            }
            DC_InvalidateRange((void *)((u32)p & ~(HW_CACHE_LINE_SIZE - 1)), HW_CACHE_LINE_SIZE);
            for (i = 0; i < 192; i++)
            {
                gDrawData[i] = *p;
                p++;
                if ((u32)p >= (u32)(gMicData + TEST_BUFFER_SIZE))
                {
                    p -= TEST_BUFFER_SIZE;
                }
                if (((u32)p % HW_CACHE_LINE_SIZE) == 0)
                {
                    DC_InvalidateRange(p, HW_CACHE_LINE_SIZE);
                }
            }
        }
        break;
    case MIC_SAMPLING_TYPE_12BIT:
        /* 12 ビットサンプリングの場合 */
        {
            u16    *p;

            p = (u16 *)((u32)address - 382);
            if ((u32)p < (u32)gMicData)
            {
                p = (u16 *)((u32)p + TEST_BUFFER_SIZE);
            }
            DC_InvalidateRange((void *)((u32)p & ~(HW_CACHE_LINE_SIZE - 1)), HW_CACHE_LINE_SIZE);
            for (i = 0; i < 192; i++)
            {
                gDrawData[i] = (u8)((*p >> 8) & 0x00ff);
                p++;
                if ((u32)p >= (u32)(gMicData + TEST_BUFFER_SIZE))
                {
                    p = (u16 *)((u32)p - TEST_BUFFER_SIZE);
                }
                if (((u32)p % HW_CACHE_LINE_SIZE) == 0)
                {
                    DC_InvalidateRange(p, HW_CACHE_LINE_SIZE);
                }
            }
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みベクトル。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    /* IRQ チェックフラグをセット */
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         Init3D

  Description:  3Dにて表示するための初期化処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Init3D(void)
{
    G3X_Init();
    G3X_InitMtxStack();
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
    G3X_AlphaTest(FALSE, 0);
    G3X_AntiAlias(TRUE);
    G3X_EdgeMarking(FALSE);
    G3X_SetFog(FALSE, (GXFogBlend)0, (GXFogSlope)0, 0);
    G3X_SetClearColor(0, 0, 0x7fff, 63, FALSE);
    G3_ViewPort(0, 0, 255, 191);
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
}

/*---------------------------------------------------------------------------*
  Name:         Draw3D

  Description:  3D表示にて波形を表示

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Draw3D(void)
{
    G3X_Reset();
    G3_Identity();
    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);

    {
        s32     i;

        if ((gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_8BIT) ||
            (gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_12BIT) ||
            (gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF))
        {
            for (i = 0; i < 191; i++)
            {
                DrawLine((s16)((s8)gDrawData[i]),
                         (s16)i, (s16)((s8)gDrawData[i + 1]), (s16)(i + 1));
            }
        }
        else
        {
            for (i = 0; i < 191; i++)
            {
                DrawLine((s16)(gDrawData[i]), (s16)i, (s16)(gDrawData[i + 1]), (s16)(i + 1));
            }
        }
    }

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

/*---------------------------------------------------------------------------*
  Name:         DrawLine

  Description:  三角ポリゴンで線を描画

  Arguments:    sx  - 描画する線の開始点のｘ座標
                sy  - 描画する線の開始点のｙ座標
                ex  - 描画する線の終点のｘ座標
                ey  - 描画する線の終点のｙ座標

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey)
{
    fx16    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fx16    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fx16    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fx16    fey = (fx16)(((96 - ey) * 0x1000) / 96);

    G3_Begin(GX_BEGIN_TRIANGLES);
    {
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fsx, fsy, 0);
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fex, fey, 0);
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fsx, fsy, 1);
    }
    G3_End();
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/

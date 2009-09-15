/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - spi - spiMonkey3
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

/*---------------------------------------------------------------------------*
    A sample which controls microphone input, touch panel input, and sound
    output in parallel.
    Since TWL has a hardware to control SPI inputs, Software is not imposed
    limitation to use SPI devices in parallel.

    USAGE:
        UP , DOWN    : Switch microphone sampling span.
        LEFT , RIGHT : Switch microphone sampling bit range.
        SEL , STA    : Switch microphone sampling loop enable or disable.

    HOWTO:
        1. Initialize MIC library and SNDEX library.
        2. Start automatic sampling of microphone by default configration.
        3. Initialize TP library with getting calibration parameters.
        4. Start automatic sampling of touch panel by fixed configration.
        5. Initialize SND library.
        6. Start sounding.
        7. When you switch parameters about automatic sampling of microphone,
           first, stop automatic sampling. Then, if necessary, stop sounding
           and switch I2S frequency. After all, edit parameters and start
           automatic sampling of microphone again.
 *---------------------------------------------------------------------------*/

#include    <twl.h>
#include    "snd_data.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     BUFFER_SIZE_MIC         (256 * 1024)   // 256K bytes
#define     RETRY_MAX_COUNT         8
#define     START_VCOUNT_TP         0
#define     FREQUENCE_TP            4
#define     BUFFER_SIZE_TP          (FREQUENCE_TP + 1)

/* マイクサンプリング種別文字列 */
static const char* bitRangeString[MIC_SAMPLING_TYPE_MAX] =
{
    "unsigned 8",
    "unsigned 12",
    "signed 8",
    "signed 12",
    "unsigned 12 (filter off)",
    "signed 12 (filter off)"
};

/* フォントデータ */
static const u32    fontChar[8 * 4];
static const u32    fontPltt[8 * 4];

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void     StartMicSampling(const MICAutoParam* param);
static void     StopMicSampling(void);
static void     AdjustMicSampling(u32 rate);
static SNDEXFrequency   GetI2SFrequency(void);
static void     SetI2SFrequency(SNDEXFrequency freq);
static void     SwitchI2SFrequency(void);

static void     StepUpSamplingRate(void);
static void     StepDownSamplingRate(void);

static void     VBlankIntr(void);

static void     InitDrawMicData(void);
static void     SetDrawMicData(void *address, MICSamplingType type);
static void     DrawMicData(void);
static void     DrawLine(s16 sx, s16 sy, s16 ex, s16 ey, GXRgb color);
static void     PrintMicSamplingParam(void);

static void     InitDrawTpData(void);
static void     SetDrawTpData(void);
static void     DrawTpData(void);

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u16          fontScr[32 * 32] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);

static MICAutoParam gMicAutoParam;
static u8           gMicData[BUFFER_SIZE_MIC] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);
static u8           gDrawMicData[192];

static TPData       gTpData[BUFFER_SIZE_TP];
static TPData       gDrawTpData[FREQUENCE_TP];

static volatile BOOL    g_sample_busy   =   FALSE;
static u32              g_snd_async_tag;

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

    /* 表示関連初期化 */
    InitDrawMicData();
    InitDrawTpData();

    /* 割込み設定 */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    /* マイク関連ライブラリ初期化 */
    {
        (void)PM_SetAmp(PM_AMP_ON);
        SNDEX_Init();
        MIC_Init();
    
        gMicAutoParam.type = MIC_SAMPLING_TYPE_8BIT;
        gMicAutoParam.buffer = (void *)gMicData;
        gMicAutoParam.size = BUFFER_SIZE_MIC;
        if ((OS_IsRunOnTwl() == TRUE) && (GetI2SFrequency() == SNDEX_FREQUENCY_47610))
        {
            gMicAutoParam.rate = MIC_SAMPLING_RATE_23810;
        }
        else
        {
            gMicAutoParam.rate = MIC_SAMPLING_RATE_16360;
        }
        gMicAutoParam.loop_enable = TRUE;
        gMicAutoParam.full_callback = NULL;
        StartMicSampling((const MICAutoParam*)&gMicAutoParam);
    }

    /* タッチパネル初期化 */
    {
        TPCalibrateParam    calibrate;
    
        TP_Init();
        if (TRUE == TP_GetUserInfo(&calibrate))
        {
            TP_SetCalibrateParam(&calibrate);
            if (0 != TP_RequestAutoSamplingStart(START_VCOUNT_TP, FREQUENCE_TP, gTpData, BUFFER_SIZE_TP))
            {
                OS_Panic("Could not start auto sampling of touch panel.\n");
            }
        }
        else
        {
            OS_Panic("Could not get userInfo about touch panel calibration.\n");
        }
    }

    /* サウンド初期化 */
    SND_Init();
    SND_AssignWaveArc((SNDBankData*)sound_bank_data, 0, (SNDWaveArc*)sound_wave_data);
    SND_StartSeq(0, sound_seq_data, 0, (SNDBankData*)sound_bank_data);
    
    /* 内部変数初期化 */
    MI_CpuFill8(gDrawMicData, 0x80, 192);

    /* LCD表示開始 */
    GX_DispOn();
    GXS_DispOn();

    /* デバッグ文字列出力 */
    OS_Printf("spiMonkey3 demo started.\n");
    OS_Printf("   up/down    -> switch microphone sampling rate\n");
    OS_Printf("   left/right -> switch microphone sampling bit range\n");
    OS_Printf("   select     -> switch microphone sampling loop state\n");
    OS_Printf("\n");
    PrintMicSamplingParam();

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
        
            if (g_sample_busy == FALSE)
            {
                /* サンプリング種別(ビット幅)変更 */
                if (keyTrg & PAD_KEY_LEFT)
                {
                    StopMicSampling();
                    gMicAutoParam.type
                        =   (MICSamplingType)((gMicAutoParam.type + MIC_SAMPLING_TYPE_MAX - 1) % MIC_SAMPLING_TYPE_MAX);
                    StartMicSampling((const MICAutoParam*)&gMicAutoParam);
                    PrintMicSamplingParam();
                }
                if (keyTrg & PAD_KEY_RIGHT)
                {
                    StopMicSampling();
                    gMicAutoParam.type
                            =   (MICSamplingType)((gMicAutoParam.type + 1) % MIC_SAMPLING_TYPE_MAX);
                    StartMicSampling((const MICAutoParam*)&gMicAutoParam);
                    PrintMicSamplingParam();
                }
            
                /* サンプリングレート変更 */
                if (keyTrg & PAD_KEY_UP)
                {
                    StepUpSamplingRate();
                    PrintMicSamplingParam();
                    if (OS_IsRunOnTwl() == TRUE)
                    {
                        g_sample_busy   =   TRUE;
                        SND_PauseSeq(0, TRUE);
                        g_snd_async_tag =   SND_GetCurrentCommandTag();
                    }
                }
                if (keyTrg & PAD_KEY_DOWN)
                {
                    StepDownSamplingRate();
                    PrintMicSamplingParam();
                    if (OS_IsRunOnTwl() == TRUE)
                    {
                        g_sample_busy   =   TRUE;
                        SND_PauseSeq(0, TRUE);
                        g_snd_async_tag =   SND_GetCurrentCommandTag();
                    }
                }
            
                /* バッファ一杯時のループ可否変更 */
                if (keyTrg & (PAD_BUTTON_SELECT | PAD_BUTTON_START))
                {
                    StopMicSampling();
                    gMicAutoParam.loop_enable = (gMicAutoParam.loop_enable + 1) % 2;
                    StartMicSampling((const MICAutoParam*)&gMicAutoParam);
                    PrintMicSamplingParam();
                }
            }
            else
            {
                if (OS_IsRunOnTwl() == TRUE)
                {
                    if (SND_IsFinishedCommandTag(g_snd_async_tag) == TRUE)
                    {
                        StopMicSampling();
                        SwitchI2SFrequency();
                        StartMicSampling((const MICAutoParam*)&gMicAutoParam);
                        SND_PauseSeq(0, FALSE);
                        g_sample_busy   =   FALSE;
                    }
                }
                else
                {
                    g_sample_busy   =   FALSE;
                }
            }
        
            /* サンプリング結果を表示用バッファに編集 */
            SetDrawMicData(MIC_GetLastSamplingAddress(), gMicAutoParam.type);
            SetDrawTpData();
        
            /* マイクサンプリング波形描画 */
            DrawMicData();
        
            /* サウンドメイン処理 */
            while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
            {
            }
            (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
        
            /* Ｖブランク待ち */
            OS_WaitVBlankIntr();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StartMicSampling

  Description:  MIC_StartLimitedSampling 関数を呼び出し、エラー処理を行う。

  Arguments:    param   -   マイク API 関数に引き渡すパラメータ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StartMicSampling(const MICAutoParam* param)
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
  Name:         StopMicSampling

  Description:  MIC_StopLimitedSampling 関数を呼び出し、エラー処理を行う。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StopMicSampling(void)
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

/*---------------------------------------------------------------------------*
  Name:         AdjustMicSampling

  Description:  MIC_AdjustLimitedSampling 関数を呼び出し、エラー処理を行う。

  Arguments:    rate    -   サンプリング周期を指定する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
AdjustMicSampling(u32 rate)
{
    MICResult   result;
    s32         retry   =   0;

    while (TRUE)
    {
        result  =   MIC_AdjustLimitedSampling(rate);
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
        case MIC_RESULT_ILLEGAL_PARAMETER:  // 指定したパラメータがサポート外
            OS_TWarning("Illegal parameter to adjust automatic sampling rate.\n");
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
        AdjustMicSampling(gMicAutoParam.rate);
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
        AdjustMicSampling(gMicAutoParam.rate);
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

    /* タッチパネル接触位置表示 */
    DrawTpData();
}

/*---------------------------------------------------------------------------*
  Name:         InitDrawMicData

  Description:  マイクサンプリング波形を 3D 表示するための初期化処理を行う。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitDrawMicData(void)
{
    /* メインディスプレイ 3D 表示設定 */
    G3X_Init();
    G3X_InitMtxStack();
    G3X_AlphaTest(FALSE, 0);
    G3X_AntiAlias(TRUE);
    G3X_EdgeMarking(FALSE);
    G3X_SetFog(FALSE, (GXFogBlend)0, (GXFogSlope)0, 0);
    G3X_SetClearColor(GX_RGB(0, 0, 0), 0, 0x7fff, 63, FALSE);
    G3_ViewPort(0, 0, 255, 191);
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
}

/*---------------------------------------------------------------------------*
  Name:         SetDrawMicData

  Description:  最新のマイクサンプリング済みデータを表示用バッファに編集する。

  Arguments:    address - コンポーネントによって最新のサンプリングデータが格納
                          されたメインメモリ上の位置。
                type    - サンプリング種別(ビット幅)。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetDrawMicData(void *address, MICSamplingType type)
{
    s32     i;

    /* サンプリングが一回も行われていない場合は何もせずに終了
       (マイクと無関係のメモリのキャッシュを破棄してしまう為) */
    if ((address < gMicData) || (address >= (gMicData + BUFFER_SIZE_MIC)))
    {
        return;
    }

    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
        /* 8 bit サンプリングの場合 */
        {
            u8     *p;

            p = (u8 *)((u32)address - 191);
            if (p < gMicData)
            {
                p = (u8 *)((u32)p + BUFFER_SIZE_MIC);
            }
            DC_InvalidateRange((void *)((u32)p & ~(HW_CACHE_LINE_SIZE - 1)), HW_CACHE_LINE_SIZE);
            for (i = 0; i < 192; i++)
            {
                gDrawMicData[i] = *p;
                p++;
                if ((u32)p >= (u32)(gMicData + BUFFER_SIZE_MIC))
                {
                    p -= BUFFER_SIZE_MIC;
                }
                if (((u32)p % HW_CACHE_LINE_SIZE) == 0)
                {
                    DC_InvalidateRange(p, HW_CACHE_LINE_SIZE);
                }
            }
        }
        break;
    case MIC_SAMPLING_TYPE_12BIT:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
    case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
        /* 12 ビットサンプリングの場合 */
        {
            u16    *p;

            p = (u16 *)((u32)address - 382);
            if ((u32)p < (u32)gMicData)
            {
                p = (u16 *)((u32)p + BUFFER_SIZE_MIC);
            }
            DC_InvalidateRange((void *)((u32)p & ~(HW_CACHE_LINE_SIZE - 1)), HW_CACHE_LINE_SIZE);
            for (i = 0; i < 192; i++)
            {
                gDrawMicData[i] = (u8)((*p >> 8) & 0x00ff);
                p++;
                if ((u32)p >= (u32)(gMicData + BUFFER_SIZE_MIC))
                {
                    p = (u16 *)((u32)p - BUFFER_SIZE_MIC);
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
  Name:         DrawMicData

  Description:  マイクサンプリング波形を 3D 表示する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawMicData(void)
{
    s32     i;

    G3X_Reset();
    G3_Identity();
    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);

    if ((gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_8BIT) ||
        (gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_12BIT) ||
        (gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF))
    {
        for (i = 0; i < 191; i++)
        {
            DrawLine((s16)((s8)gDrawMicData[i]), (s16)i, (s16)((s8)gDrawMicData[i + 1]), (s16)(i + 1), GX_RGB(31, 31, 31));
        }
    }
    else
    {
        for (i = 0; i < 191; i++)
        {
            DrawLine((s16)(gDrawMicData[i]), (s16)i, (s16)(gDrawMicData[i + 1]), (s16)(i + 1), GX_RGB(31, 31, 31));
        }
    }
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

/*---------------------------------------------------------------------------*
  Name:         DrawLine

  Description:  三角ポリゴンで線を描画する。

  Arguments:    sx      -   描画する線の開始点のｘ座標。
                sy      -   描画する線の開始点のｙ座標。
                ex      -   描画する線の終点のｘ座標。
                ey      -   描画する線の終点のｙ座標。
                color   -   描画する線の色。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey, GXRgb color)
{
    fx16    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fx16    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fx16    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fx16    fey = (fx16)(((96 - ey) * 0x1000) / 96);

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

/*---------------------------------------------------------------------------*
  Name:         PrintMicSamplingParam

  Description:  マイクのサンプリング設定をプリント出力する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintMicSamplingParam(void)
{
    s32     range = 0;

    OS_Printf(" sampling-rate: ");
    switch (gMicAutoParam.rate)
    {
    case MIC_SAMPLING_RATE_32730:   OS_Printf("32.73kHz");  break;
    case MIC_SAMPLING_RATE_16360:   OS_Printf("16.36kHz");  break;
    case MIC_SAMPLING_RATE_10910:   OS_Printf("10.91kHz");  break;
    case MIC_SAMPLING_RATE_8180:    OS_Printf(" 8.18kHz");  break;
    case MIC_SAMPLING_RATE_47610:   OS_Printf("47.61kHz");  break;
    case MIC_SAMPLING_RATE_23810:   OS_Printf("23.81kHz");  break;
    case MIC_SAMPLING_RATE_15870:   OS_Printf("15.87kHz");  break;
    case MIC_SAMPLING_RATE_11900:   OS_Printf("11.90kHz");  break;
    }

    OS_Printf(" , loop: %s", (gMicAutoParam.loop_enable ? " on" : "off"));

    OS_Printf(" , bit-range: %s\n",
            bitRangeString[gMicAutoParam.type % MIC_SAMPLING_TYPE_MAX]);
}

/*---------------------------------------------------------------------------*
  Name:         Prepare2DScreen

  Description:  タッチパネル接触位置を表示するための 2D スクリーンの初期化処理
                を行う。

  Arguments:    scr     -   編集するスクリーンデータの先頭アドレスを指定。
                pltt    -   接触位置を示す線に用いるパレット番号を指定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
Prepare2DScreen(u16* scr, u16 pltt)
{
    s32     i;
    s32     j;

    scr[0]  =   (u16)(0x0003 | ((pltt & 0x000f) << 12));
    for (j = 1; j < 32; j ++)
    {
        scr[j]  =   (u16)(0x0001 | ((pltt & 0x000f) << 12));
    }
    for (i = 1; i < 32; i ++)
    {
        scr[i * 32] =   (u16)(0x0002 | ((pltt & 0x000f) << 12));
        for (j = 1; j < 32; j ++)
        {
            scr[(i * 32) + j]   =   (u16)((pltt & 0x000f) << 12);
        }
    }
    DC_StoreRange(scr, (32 * 32 * sizeof(u16)));
}

/*---------------------------------------------------------------------------*
  Name:         InitDrawTpData

  Description:  タッチパネル接触位置を 2D 表示するための初期化処理を行う。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitDrawTpData(void)
{
    /* サブディスプレイ 2D 表示設定 */
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_32_H);
    GXS_SetGraphicsMode(GX_BGMODE_0);

    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
            GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2S_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
            GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2S_SetBG2ControlText(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
            GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000);
    G2S_SetBG3ControlText(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
            GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000);
    Prepare2DScreen(fontScr, 0);
    GXS_LoadBG0Scr(fontScr, 0, sizeof(fontScr));
    Prepare2DScreen(fontScr, 1);
    GXS_LoadBG1Scr(fontScr, 0, sizeof(fontScr));
    Prepare2DScreen(fontScr, 2);
    GXS_LoadBG2Scr(fontScr, 0, sizeof(fontScr));
    Prepare2DScreen(fontScr, 3);
    GXS_LoadBG3Scr(fontScr, 0, sizeof(fontScr));
    G2S_SetBG0Priority(0);
    G2S_SetBG1Priority(1);
    G2S_SetBG2Priority(2);
    G2S_SetBG3Priority(3);

    GXS_LoadBGPltt(fontPltt, 0, sizeof(fontPltt));
    GXS_LoadBG0Char(fontChar, 0, sizeof(fontChar));
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3);
}

/*---------------------------------------------------------------------------*
  Name:         SetDrawTpData

  Description:  最新のタッチパネルサンプリング済みデータを表示画面座標に変換し
                表示用バッファに編集する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SetDrawTpData(void)
{
    s32     i;
    s32     index;
    s32     lastIndex;

    lastIndex   =   TP_GetLatestIndexInAuto();
    for (i = 0; i < FREQUENCE_TP; i ++)
    {
        index   =   (lastIndex + BUFFER_SIZE_TP - i) % BUFFER_SIZE_TP;
        if ((gTpData[index].touch == TP_TOUCH_ON) && (gTpData[index].validity == TP_VALIDITY_VALID))
        {
            gDrawTpData[i].touch    =   TP_TOUCH_ON;
            TP_GetCalibratedPoint(&gDrawTpData[i], &gTpData[index]);
        }
        else
        {
            gDrawTpData[i].touch    =   TP_TOUCH_OFF;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DrawTpData

  Description:  タッチパネル接触位置を 2D 表示する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
DrawTpData(void)
{
    int     plane   =   GX_PLANEMASK_NONE;

    if (gDrawTpData[0].touch == TP_TOUCH_ON)
    {
        G2S_SetBG0Offset((256 - gDrawTpData[0].x), (256 - gDrawTpData[0].y));
        plane   |=  GX_PLANEMASK_BG0;
    }
    if (gDrawTpData[1].touch == TP_TOUCH_ON)
    {
        G2S_SetBG1Offset((256 - gDrawTpData[1].x), (256 - gDrawTpData[1].y));
        plane   |=  GX_PLANEMASK_BG1;
    }
    if (gDrawTpData[2].touch == TP_TOUCH_ON)
    {
        G2S_SetBG2Offset((256 - gDrawTpData[2].x), (256 - gDrawTpData[2].y));
        plane   |=  GX_PLANEMASK_BG2;
    }
    if (gDrawTpData[3].touch == TP_TOUCH_ON)
    {
        G2S_SetBG3Offset((256 - gDrawTpData[3].x), (256 - gDrawTpData[3].y));
        plane   |=  GX_PLANEMASK_BG3;
    }
    GXS_SetVisiblePlane(plane);
}

/*---------------------------------------------------------------------------*
    フォントデータ
 *---------------------------------------------------------------------------*/
static const u32    fontChar[8 * 4] =
{
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x11111111, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000001, 0x00000001, 0x00000001, 0x00000001,
    0x00000001, 0x00000001, 0x00000001, 0x00000001,
    0x11111111, 0x00000001, 0x00000001, 0x00000001,
    0x00000001, 0x00000001, 0x00000001, 0x00000001
};

static const u32    fontPltt[8 * 4] =
{
    0x7fff0000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x5ef70000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x3def0000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1ce70000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};


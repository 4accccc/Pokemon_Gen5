/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - spi - mic-3
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

/*---------------------------------------------------------------------------*
    A sample that controls microphone sampling.

    USAGE:
        UP , DOWN    : Control sampling span.
        LEFT , RIGHT : Control sampling bit range. ( 8bit or 12 bit etc. )
        SEL , STA    : Control loop sampling enable or disable.

    HOWTO:
        1. Initialize MIC library and SNDEX library.
        2. Start automatic sampling of microphone by default configuration.
        3. When you switch parameters, first, stop sampling. Then, if necessary,
           switch I2S frequency. After all, edit parameters and start automatic
           sampling again.
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
static void     AdjustSampling(u32 rate);
static SNDEXFrequency   GetI2SFrequency(void);
static void     SetI2SFrequency(SNDEXFrequency freq);
static void     SwitchI2SFrequency(void);

static void     StepUpSamplingRate(void);
static void     StepDownSamplingRate(void);
static void     SetDrawData(void *address, MICSamplingType type);
static void     PrintfVariableData(void);

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
    
        gMicAutoParam.type      =   MIC_SAMPLING_TYPE_8BIT;
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
        gMicAutoParam.loop_enable   =   TRUE;
        gMicAutoParam.full_callback =   NULL;
        StartSampling((const MICAutoParam*)&gMicAutoParam);
    }

    /* 内部変数初期化 */
    MI_CpuFill8(gDrawData, 0x80, sizeof(gDrawData));

    /* LCD表示開始 */
    GX_DispOn();
    GXS_DispOn();

    /* デバッグ文字列出力 */
    OS_Printf("mic-3 demo started.\n");
    OS_Printf("   up/down      -> change sampling rate\n");
    OS_Printf("   left/right   -> change bit range\n");
    OS_Printf("   start/select -> change loop setting\n");
    OS_Printf("\n");
    PrintfVariableData();

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
        
            /* サンプリング種別(ビット幅)変更 */
            if (keyTrg & PAD_KEY_LEFT)
            {
                StopSampling();
                gMicAutoParam.type
                    =   (MICSamplingType)((gMicAutoParam.type + MIC_SAMPLING_TYPE_MAX - 1) % MIC_SAMPLING_TYPE_MAX);
                StartSampling((const MICAutoParam*)&gMicAutoParam);
                PrintfVariableData();
            }
            if (keyTrg & PAD_KEY_RIGHT)
            {
                StopSampling();
                gMicAutoParam.type
                        =   (MICSamplingType)((gMicAutoParam.type + 1) % MIC_SAMPLING_TYPE_MAX);
                StartSampling((const MICAutoParam*)&gMicAutoParam);
                PrintfVariableData();
            }
        
            /* サンプリングレート変更 */
            if (keyTrg & PAD_KEY_UP)
            {
                StepUpSamplingRate();
                PrintfVariableData();
            }
            if (keyTrg & PAD_KEY_DOWN)
            {
                StepDownSamplingRate();
                PrintfVariableData();
            }
        
            /* バッファ一杯時のループ可否変更 */
            if (keyTrg & (PAD_BUTTON_SELECT | PAD_BUTTON_START))
            {
                StopSampling();
                gMicAutoParam.loop_enable = (gMicAutoParam.loop_enable + 1) % 2;
                StartSampling((const MICAutoParam*)&gMicAutoParam);
                PrintfVariableData();
            }
        
            /* 波形描画 */
            SetDrawData(MIC_GetLastSamplingAddress(), gMicAutoParam.type);
            Draw3D();
        
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

/*---------------------------------------------------------------------------*
  Name:         AdjustSampling

  Description:  MIC_AdjustLimitedSampling 関数を呼び出し、エラー処理を行う。

  Arguments:    rate    -   サンプリング周期を指定する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
AdjustSampling(u32 rate)
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
        StopSampling();
        SwitchI2SFrequency();
        StartSampling((const MICAutoParam*)&gMicAutoParam);
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
        AdjustSampling(gMicAutoParam.rate);
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
        StopSampling();
        SwitchI2SFrequency();
        StartSampling((const MICAutoParam*)&gMicAutoParam);
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
        AdjustSampling(gMicAutoParam.rate);
    }
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
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
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
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
    case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
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
  Name:         PrintfVariableData

  Description:  可変なサンプリング設定をプリント出力する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static const char* bitRangeString[MIC_SAMPLING_TYPE_MAX] =
{
    "unsigned 8",
    "unsigned 12",
    "signed 8",
    "signed 12",
    "unsigned 12 (filter off)",
    "signed 12 (filter off)"
};

/*---------------------------------------------------------------------------*/
static void PrintfVariableData(void)
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

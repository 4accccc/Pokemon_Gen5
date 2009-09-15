/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - snd - extraFunc
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-10#$
  $Rev: 8925 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    A sample that use SNDEX library.

    USAGE:
        UP , DOWN    : Switch item to control.
        LEFT , RIGHT : Set config related to selected item.

    HOWTO:
        1. Initialize SNDEX library.
        2. Call API to set/get each config.
 *---------------------------------------------------------------------------*/

#include    <twl.h>
#include    "font.h"
#include    "snd_data.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     INDEX_MAX   4

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u16      screen[32 * 32] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);
static s32      index   =   0;

static SNDEXMute        mute    =   SNDEX_MUTE_OFF;
static SNDEXFrequency   freq    =   SNDEX_FREQUENCY_32730;
static u8               rate    =   0;
static u8               volume  =   0;
static BOOL             shutter =   FALSE;


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL     CheckResult     (SNDEXResult result);
static void     UpdateScreen    (void);
static void     VBlankIntr      (void);
static void     PrintString     (s16 x, s16 y, u8 palette, char *text, ...);
static void VolumeSwitchCallback(SNDEXResult result, void* arg);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
TwlMain (void)
{
    /* 初期化 */
    OS_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    /* 表示設定初期化 */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void*)HW_OAM, GX_LCD_SIZE_Y, HW_OAM_SIZE);
    MI_CpuClearFast((void*)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void*)HW_DB_OAM, GX_LCD_SIZE_Y, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void*)HW_DB_PLTT, HW_DB_PLTT_SIZE);
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
            GX_BG_COLORMODE_16,
            GX_BG_SCRBASE_0xf800,      // SCR ベースブロック 31
            GX_BG_CHARBASE_0x00000,    // CHR ベースブロック 0
            GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    MI_CpuClearFast((void*)screen, sizeof(screen));
    DC_FlushRange(screen, sizeof(screen));
    GX_LoadBG0Scr(screen, 0, sizeof(screen));

    /* 割込み設定 */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    /* サウンド初期化 */
    SND_Init();
    SND_AssignWaveArc((SNDBankData*)sound_bank_data, 0, (SNDWaveArc*)sound_wave_data);
    SND_StartSeq(0, sound_seq_data, 0, (SNDBankData*)sound_bank_data);

    /* サウンド拡張機能初期化 */
    SNDEX_Init();
    (void)CheckResult(SNDEX_GetMute(&mute));
    (void)CheckResult(SNDEX_GetI2SFrequency(&freq));
    (void)CheckResult(SNDEX_GetDSPMixRate(&rate));
    (void)CheckResult(SNDEX_GetVolume(&volume));
    UpdateScreen();
    
    /* 音量調整ボタン押下時のコールバック関数を設定 */
    SNDEX_SetVolumeSwitchCallback(VolumeSwitchCallback, NULL);

    /* LCD 表示開始 */
    GX_DispOn();
    GXS_DispOn();

    {
        u16     keyOld  =   PAD_Read();
        u16     keyTrg;
        u16     keyNow;
        SNDEXHeadphone hp;
    
        /* メインループ */
        while (TRUE)
        {
            /* キー入力情報取得 */
            keyNow  =   PAD_Read();
            keyTrg  =   (u16)((keyOld ^ keyNow) & keyNow);
            keyOld  =   keyNow;
        
            /* 入力に応じて各種操作 */
            if (keyTrg & PAD_KEY_UP)
            {
                index   =   (index + INDEX_MAX - 1) % INDEX_MAX;
            }
            if (keyTrg & PAD_KEY_DOWN)
            {
                index   =   (index + 1) % INDEX_MAX;
            }
            if (keyTrg & PAD_KEY_RIGHT)
            {
                switch (index)
                {
                case 0: // Mute control
                    if (mute == SNDEX_MUTE_OFF)
                    {
                        if (CheckResult(SNDEX_SetMute(SNDEX_MUTE_ON)) == TRUE)
                        {
                            mute    =   SNDEX_MUTE_ON;
                        }
                    }
                    break;
                case 1: // I2S frequency control
                    if (freq == SNDEX_FREQUENCY_32730)
                    {
                        if (CheckResult(SNDEX_SetI2SFrequency(SNDEX_FREQUENCY_47610)) == TRUE)
                        {
                            freq    =   SNDEX_FREQUENCY_47610;
                        }
                    }
                    break;
                case 2: // DSP mix rate control
                    if (rate < SNDEX_DSP_MIX_RATE_MAX)
                    {
                        if (CheckResult(SNDEX_SetDSPMixRate((u8)(rate + 1))) == TRUE)
                        {
                            rate ++;
                        }
                    }
                    break;
                case 3: // Volume control
                    if (volume < SNDEX_VOLUME_MAX)
                    {
                        if (CheckResult(SNDEX_SetVolume((u8)(volume + 1))) == TRUE)
                        {
                            volume ++;
                        }
                    }
                    break;
                }
            }
            if (keyTrg & PAD_KEY_LEFT)
            {
                switch (index)
                {
                case 0: // Mute control
                    if (mute == SNDEX_MUTE_ON)
                    {
                        if (CheckResult(SNDEX_SetMute(SNDEX_MUTE_OFF)) == TRUE)
                        {
                            mute    =   SNDEX_MUTE_OFF;
                        }
                    }
                    break;
                case 1: // I2S frequency control
                    if (freq == SNDEX_FREQUENCY_47610)
                    {
                        if (CheckResult(SNDEX_SetI2SFrequency(SNDEX_FREQUENCY_32730)) == TRUE)
                        {
                            freq    =   SNDEX_FREQUENCY_32730;
                        }
                    }
                    break;
                case 2: // DSP mix rate control
                    if (rate > SNDEX_DSP_MIX_RATE_MIN)
                    {
                        if (CheckResult(SNDEX_SetDSPMixRate((u8)(rate - 1))) == TRUE)
                        {
                            rate --;
                        }
                    }
                    break;
                case 3: // Volume control
                    if (volume > SNDEX_VOLUME_MIN)
                    {
                        if (CheckResult(SNDEX_SetVolume((u8)(volume - 1))) == TRUE)
                        {
                            volume --;
                        }
                    }
                    break;
                }
            }
            if (keyTrg & PAD_BUTTON_A)
            {
                /* ヘッドフォン接続判定 */
                if (SNDEX_PXI_RESULT_SUCCESS == SNDEX_IsConnectedHeadphone(&hp))
                {
                    if (hp == SNDEX_HEADPHONE_CONNECT)
                    {
                        OS_TPrintf("Headphone is connected.\n");
                    }
                    else
                    {
                        OS_TPrintf("Headphone is unconnected.\n");
                    }
                }
            }
        
            /* サウンドメイン処理 */
            while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
            {
            }
            (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
            
            /* ボリュームの変更に volume 値を追従させる */
            (void)CheckResult(SNDEX_GetVolume(&volume));
            
            /* 画面更新 */
            UpdateScreen();
        
            /* V ブランク待ち */
            OS_WaitVBlankIntr();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CheckResult

  Description:  SNDEX ライブラリの API 呼び出し結果を確認する。

  Arguments:    result  -   API より返された戻り値を指定する。

  Returns:      BOOL    -   API 呼び出しが成功した場合に TRUE を返す。
                            何らかの理由で失敗した場合に FALSE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
CheckResult (SNDEXResult result)
{
    switch (result)
    {
    case SNDEX_RESULT_SUCCESS:
        return TRUE;
    case SNDEX_RESULT_BEFORE_INIT:
        /* ライブラリ初期化前。
            各 API を使用する前に、SNDEX_Init() 関数を呼び出す必要があります。 */
        OS_TWarning("Not initialized.\n");
        break;
    case SNDEX_RESULT_INVALID_PARAM:
        /* 不明なパラメータ。
            API に与えている引数を見直して下さい。 */
        OS_TWarning("Invalid parameter.\n");
        break;
    case SNDEX_RESULT_EXCLUSIVE:
        /* 排他制御中。
            非同期型 API を連続で呼び出したり、複数スレッドから非同期に
            SNDEX ライブラリを使用したりしないで下さい。 */
        OS_TWarning("Overlapped requests.\n");
        break;
    case SNDEX_RESULT_ILLEGAL_STATE:
        /* 状態異常。
            ハードウェアが TWL でない場合、SNDEX ライブラリは使用できません。
            例外ハンドラ内から同期型の関数は呼び出せません。
            CODEC を DS モードにしている場合は、音声の強制出力状態に切替えられません。
            CODEC を DS モードにしている場合、音声の強制出力状態である場合、
            マイクの自動サンプリング中である場合は、I2S 周波数を変更できません。 */
        OS_TWarning("In unavailable state.\n");
        break;
    case SNDEX_RESULT_PXI_SEND_ERROR:
        /* PXI 送信エラー。
            ARM9 から ARM7 へのデータ送信キューが飽和しています。ARM7 がキュー内の
            データを消化するまでしばらく待ってからリトライして下さい。 */
        OS_TWarning("PXI queue full.\n");
        break;
    case SNDEX_RESULT_DEVICE_ERROR:
        /* デバイス操作に失敗。
            音量の取得、変更は CPU 外部デバイス (マイコン) へのアクセスを伴いますが、
            このアクセスに失敗しています。数回リトライして症状が改善しない場合は、
            マイコンの暴走やハードウェアの故障など、ソフト的には復旧不可能な原因で
            ある可能性が高いため、"成功" とみなして処理を進めて下さい。 */
        OS_TWarning("Micro controller unusual.\n");
        return TRUE;
    case SNDEX_RESULT_FATAL_ERROR:
    default:
        /* FATAL エラー。
            ライブラリのロジック上は発生し得ません。ライブラリの内部状態管理を無視して
            ARM7 に直接 PXI コマンドを発行した場合や、メモリ破壊によって内部状態管理に
            不整合が生じた場合、想定外の ARM7 コンポーネントと組み合わせた場合などに
            発生する可能性があります。 */
        OS_TWarning("Fatal error.\n");
        break;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         UpdateScreen

  Description:  画面表示内容を更新する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
UpdateScreen (void)
{
    /* 仮想スクリーンをクリア */
    MI_CpuClearFast((void*)screen, sizeof(screen));

    /* 仮想スクリーン上に文字列を編集 */
    PrintString(2, 1, 0xf, "Mute     : %d/1 [ %s ]", mute, ((mute == SNDEX_MUTE_OFF) ? "OFF" : "ON"));
    PrintString(2, 3, 0xf, "I2S freq : %d/1 [ %s ]", freq, ((freq == SNDEX_FREQUENCY_32730) ? "32K" : "48K"));
    PrintString(2, 5, 0xf, "DSP mix  : %d/8", rate);
    PrintString(2, 7, 0xf, "Volume   : %d/%d", volume, (u8)SNDEX_VOLUME_MAX);
    
    PrintString(0, 11, 0xe, "- [A] : Check Headphone");
    PrintString(0, 13, 0xe, "- [Volume Button] : Print log");

    /* 選択項目の表示色を変更 */
    PrintString(0, (s16)(index * 2 + 1), 0xf, ">");
    {
        s32     i;
        s32     j;
    
        for (i = 0; i < 32; i ++)
        {
            j   =   ((index * 2 + 1) * 32) + i;
            screen[j]   &=  (u16)(~(0xf << 12));
            screen[j]   |=  (u16)(0x4 << 12);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みベクトル。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
VBlankIntr (void)
{
    /* 仮想スクリーンを VRAM に反映 */
    DC_FlushRange(screen, sizeof(screen));
    GX_LoadBG0Scr(screen, 0, sizeof(screen));

    /* IRQ チェックフラグをセット */
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  仮想スクリーンに文字列を配置する。文字列は32文字まで。

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                palette - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
PrintString (s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *((u16*)(&temp[32]))    =   0x0000;
    for (i = 0; ; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        screen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         VolumeSwitchCallback

  Description:  TWL本体の音量調整ボタン押下時に実行されるコールバック関数

  Arguments:    result  -   音量調整ボタン押下操作による結果。
                arg     -   この関数がコールバック関数として呼ばれた際に渡される引数。
                

  Returns:      なし
 *---------------------------------------------------------------------------*/
static void VolumeSwitchCallback(SNDEXResult result, void* arg)
{
#pragma unused( arg )
#pragma unused( result )

    /* コールバック処理中、SNDEX ライブラリの関数は使用できません。*
     * SNDEX_RESULT_EXCLUSIVE が返ります。                         */
    OS_TPrintf("volume switch pressed.\n");
}

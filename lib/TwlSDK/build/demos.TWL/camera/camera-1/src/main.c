/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - camera - camera-1
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-15#$
  $Rev: 10750 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/camera.h>
#include "DEMOBitmap.h"

#define NDMA_NO      1           // 使用するNDMA番号(0-3)
#define WIDTH       256         // イメージの幅
#define HEIGHT      192         // イメージの高さ

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // 一ラインの転送バイト数

static void VBlankIntr(void);
static BOOL CameraInit(void);
static void CameraIntrVsync(CAMERAResult result);
static void CameraIntrError(CAMERAResult result);
static void CameraIntrReboot(CAMERAResult result);
static void CameraDmaRecvIntr(void* arg);

static CAMERASelect current;
static CAMERAFlip flipIn, flipOut;
static BOOL flipFlag = FALSE;
static BOOL switchFlag = FALSE;
static u32 stabilizedCount;

static int wp;  // カメラからデータを取り込み中のバッファ
static int rp;  // 最後のVRAMにコピーしたバッファ
static BOOL wp_pending; // 取り込みを中断した (再び同じバッファに取り込む)
static u16 buffer[2][WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);
static void PendingCapture(void)
{
    wp_pending = TRUE;
}

// 文字表示
static void PutString( char *format, ... )
{
    u16             *dest = G2_GetBG1ScrPtr();
    char            temp[32+1];
    int             i;
    va_list         va;

    va_start(va, format);
    (void)OS_VSNPrintf(temp, sizeof(temp), format, va);
    va_end(va);

    for (i = 0; i < 32 && temp[i]; i++)
    {
        dest[i] = (u16)((u8)temp[i] | (1 << 12));
    }
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    CAMERAResult result;

    // 初期化
    OS_Init();
    OS_InitThread();
    GX_Init();
    OS_InitTick();
    OS_InitAlarm();

    // GXでDMAを使わない (旧DMAはカメラDMAの邪魔になる)
    (void)GX_SetDefaultDMA(GX_DMA_NOT_USE);

    // VRAMクリア
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    GX_SetBankForLCDC(GX_VRAM_LCDC_B);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_A, 128 * 1024);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_B, 128 * 1024);

    // ダイレクトビットマップ表示モード＆文字表示
    GX_SetBankForBG(GX_VRAM_BG_256_AB);         // VRAM-A,BバンクをＢＧに割り当る
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_4, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3);

    G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GX_BG_EXTPLTT_01);
    G2_SetBG1Priority(1);
    G2_BG1Mosaic(FALSE);

    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x20000);
    G2_SetBG3Priority(3);
    G2_BG3Mosaic(FALSE);

    // 文字の読み込み
    {
        static const GXRgb pal[16] = { GX_RGB(0, 0, 0), GX_RGB(31, 31, 31), };
        GX_LoadBG1Char(DEMOAsciiChr, 0x00000, sizeof(DEMOAsciiChr));
        GX_LoadBGPltt(pal, 0x0000, sizeof(pal));
    }
    wp = 0;
    rp = 1;
    wp_pending = TRUE;
    stabilizedCount = 0;

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableInterrupts();

    OS_WaitVBlankIntr();
    GX_DispOn();

    // カメラ初期化
    current = CAMERA_SELECT_IN;
    flipIn  = CAMERA_FLIP_HORIZONTAL;
    flipOut = CAMERA_FLIP_NONE;
    flipFlag = FALSE;

    (void)CameraInit();

    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // DMA割り込み設定
    (void)OS_EnableIrqMask(OS_IE_NDMA1);

    // カメラVSYNC割り込みコールバック
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // カメラエラー割り込みコールバック
    CAMERA_SetBufferErrorCallback(CameraIntrError);

    // カメラの再起動完了コールバック
    CAMERA_SetRebootCallback(CameraIntrReboot);

    // キャプチャに使用するDMAを準備しておく
    MI_StopNDma(NDMA_NO);
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);

    // カメラスタート
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
    OS_TPrintf("Camera is shooting a movie...\n");

    while (1)
    {
        u16 pad;
        u16 trg;
        static u16 old = 0xffff;    // ignore the trigger by first data
        static int camera_end_flag = FALSE;
        static int camera_stop_capture_flag = FALSE;
        static int camera_notactivate_flag = FALSE;

        OS_WaitVBlankIntr();

        pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;

        if (camera_end_flag == FALSE)
        {
            if (trg & PAD_BUTTON_Y)
            {
                flipFlag = TRUE;
            }
            if (trg & PAD_BUTTON_X)
            {
                switchFlag = TRUE;
            }
            if (camera_stop_capture_flag == FALSE)
            {
                if (trg & PAD_BUTTON_B)
                {
                    static BOOL standby = FALSE;
                    standby ^= 1;
                    if (standby)
                    {
                        // 一旦カメラの使用を停止して、次回また使用する予定であれば、わざわざ CAMERA_End を呼ばなくとも Activate NONE にするだけで十分
                        // StopCapture よりも前に Activate を NONE にすると、次回 Activate ON にするまで IsBusy が TRUE となる恐れがある
                        CAMERA_StopCapture();
                        while(CAMERA_IsBusy())
                        {
                        }
                        MI_StopNDma(NDMA_NO);
                        result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
                        if(result == CAMERA_RESULT_FATAL_ERROR)
                            OS_Panic("CAMERA FATAL ERROR\n");

                        if(result == CAMERA_RESULT_ILLEGAL_STATUS)
                            continue;

                        camera_notactivate_flag = TRUE;
                    }
                    else
                    {
                        result = CAMERA_I2CActivate(current);
                        if(result == CAMERA_RESULT_FATAL_ERROR)
                            OS_Panic("CAMERA FALTAL ERROR\n");
                        if(result == CAMERA_RESULT_ILLEGAL_STATUS)
                            continue;
                        stabilizedCount = 0;

                        CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
                        CAMERA_ClearBuffer();
                        CAMERA_StartCapture();
                        while (CAMERA_IsBusy() == FALSE)
                        {
                        }
                        camera_notactivate_flag = FALSE;
                    }
                    OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
                }
            }
            if (camera_notactivate_flag == FALSE)
            {
                if (trg & PAD_BUTTON_START)
                {
                    if (CAMERA_IsBusy())
                    {
                        // パラメータ変更したい場合は、CAMERA_IsBusy() を FALSE にするため、あらかじめカメラを起動した後に
                        // キャプチャを停止する必要がある
                        // カメラを使用しなくなる場合は、カメラがスタンバイ状態であってもキャプチャを停止しても構わない
                        result = CAMERA_I2CActivate(current);
                        if(result == CAMERA_RESULT_FATAL_ERROR)
                            OS_Panic("CAMERA FATAL ERROR\n");
                        if(result == CAMERA_RESULT_ILLEGAL_STATUS)
                            continue;

                        OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
                        OS_TPrintf("call CAMERA_Stop()... ");
                        CAMERA_StopCapture();
                        while (CAMERA_IsBusy())
                        {
                        }
                        OS_TPrintf("Camera was stopped.\n");
                        MI_StopNDma(NDMA_NO);
                        stabilizedCount = 0;
                        camera_stop_capture_flag = TRUE;
                    }
                    else
                    {
                        CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
                        CAMERA_ClearBuffer();
                        CAMERA_StartCapture();
                        OS_TPrintf("Camera is shooting a movie...\n");
                        while (CAMERA_IsBusy() == FALSE)
                        {
                        }
                        camera_stop_capture_flag = FALSE;
                    }
                }
            }
        }
        // カメラを使用しない場合は CAMERA_I2CActivate(CAMERA_SELECT_NONE) を呼び出せばよい
        if (trg & PAD_BUTTON_SELECT)
        {
            if (camera_end_flag == FALSE)
            {
                OS_TPrintf("call CAMERA_End()... ");
                CAMERA_StopCapture();
                while(CAMERA_IsBusy())
                {
                }
                MI_StopNDma(NDMA_NO);
                CAMERA_End();
                OS_TPrintf("Done.\n");
                camera_stop_capture_flag = TRUE;
                camera_notactivate_flag = TRUE;
                camera_end_flag = TRUE;
            }
            else
            {
                if(CameraInit() == TRUE)
                {
                    // 最初のInit時に登録した各種コールバックの再登録
                    CAMERA_SetVsyncCallback(CameraIntrVsync);
                    CAMERA_SetBufferErrorCallback(CameraIntrError);
                    CAMERA_SetRebootCallback(CameraIntrReboot);

                    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
                    CAMERA_ClearBuffer();
                    CAMERA_StartCapture();
                    OS_TPrintf("Camera is shooting a movie...\n");
                    while (CAMERA_IsBusy() == FALSE)
                    {
                    }
                    camera_end_flag = FALSE;
                    camera_notactivate_flag = FALSE;
                    camera_stop_capture_flag = FALSE;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
    if (wp == rp)
    {
        rp ^= 1;
        GX_LoadBG3Scr(buffer[rp], 0, BYTES_PER_LINE * HEIGHT);
        DC_InvalidateRange(buffer[rp], BYTES_PER_LINE * HEIGHT);
    }
}

//--------------------------------------------------------------------------------
//    カメラの初期化処理 (InitとI2C関連のみ)
//
BOOL CameraInit(void)
{
    CAMERAResult result;
    OSTick begin = OS_GetTick();

    result = CAMERA_Init();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_Init was failed.");
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;

    OS_TPrintf("CAMERA_Init took %d msec\n", (int)OS_TicksToMilliSeconds(OS_GetTick()-begin));

    result = CAMERA_I2CFlip(CAMERA_SELECT_IN, flipIn);
    if (result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_I2CFlip was failed. (%d)\n", result);
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;

    result = CAMERA_I2CFlip(CAMERA_SELECT_OUT, flipOut);
    if (result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_I2CFlip was failed. (%d)\n", result);
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;

    result = CAMERA_I2CActivate(current);
    if (result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_I2CActivate was failed. (%d)\n", result);
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;
    stabilizedCount = 0;

    return TRUE;
}

//--------------------------------------------------------------------------------
//    カメラ割り込み処理 (エラー時とVsync時に発生)
//
void CameraIntrError(CAMERAResult result)
{
#pragma unused(result)
    OS_TPrintf("Error was occurred.\n");
    // カメラ停止処理
    CAMERA_StopCapture();
    MI_StopNDma(NDMA_NO);
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    CAMERA_ClearBuffer();
    wp_pending = TRUE;      // 次回も同じフレームを使用する
    CAMERA_StartCapture();
}

// 再起動完了コールバック
void CameraIntrReboot(CAMERAResult result)
{
    if(result == CAMERA_RESULT_FATAL_ERROR)
    {
        return; // カメラが再起動処理を行っても復帰することができなかった
    }
    CameraIntrError(result); // DMAの同期がずれているかもしれないので仕切り直し
}

static void DoneCallback(CAMERAResult result, void* arg)
{
    const char* str = arg;
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("%s was failed. (%d)\n", arg, result);
    }
}

void CameraIntrVsync(CAMERAResult result)
{
#pragma unused(result)
    if(stabilizedCount <= 30)
        stabilizedCount++;
}

#define FPS_SAMPLES 4
void CameraDmaRecvIntr(void* arg)
{
#pragma unused(arg)
    CAMERAResult result;

    MI_StopNDma(NDMA_NO);

    if(flipFlag == TRUE)
    {
        CAMERAFlip *pFlip = (current == CAMERA_SELECT_IN ? &flipIn : &flipOut);
        *pFlip = (CAMERAFlip)((*pFlip + 1) % CAMERA_FLIP_MAX);
        result = CAMERA_I2CFlipAsync(current, *pFlip, DoneCallback, "CAMERA_I2CFlipAsync");
        if (result == CAMERA_RESULT_FATAL_ERROR)
            OS_TPanic("CAMERA_I2CFlipAsync was failed. (%d)\n", result);
        if(result == CAMERA_RESULT_ILLEGAL_STATUS)
            return;

        flipFlag = FALSE;
    }
    else if(switchFlag == TRUE)
    {
        current = (current == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
        result = CAMERA_I2CActivateAsync(current, DoneCallback, "CAMERA_I2CActivateAsync");
        if (result == CAMERA_RESULT_FATAL_ERROR)
            OS_TPanic("CAMERA_I2CActivateAsync was failed. (%d)\n", result);
        if(result == CAMERA_RESULT_ILLEGAL_STATUS)
            return;
        stabilizedCount = 0;
        switchFlag = FALSE;
    }

    // 次のフレームのキャプチャを開始する
    if (wp_pending)
    {
        wp_pending = FALSE;
    }
    else
    {
        // カメラが安定するまで、キャプチャ結果は画面に表示しない
        // このデモでは異常な色を回避するために最低限待つ必要のある 4 カメラフレーム待っているが、
        // 自動露出が安定するまで待ちたい場合は、リファレンスにあるように屋内で 14、屋外で 30 フレーム待つ必要があります。
        if(stabilizedCount > 4)
        {
            wp ^= 1;
        }
    }
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);

    // フレームレートの表示
    {
        static OSTick begin = 0;
        static int uspf[FPS_SAMPLES] = { 0 };
        static int count = 0;
        int i;
        int sum = 0;
        OSTick end = OS_GetTick();
        if (begin)  // 初回を除く
        {
            uspf[count] = (int)OS_TicksToMicroSeconds(end - begin);
            count = (count + 1) % FPS_SAMPLES;
        }
        begin = end;
        // 平均値の算定
        for (i = 0; i < FPS_SAMPLES; i++)
        {
            if (uspf[i] == 0)  break;
            sum +=  uspf[i];
        }
        if (sum)
        {
            int mfps = (int)(1000000000LL * i / sum);
            PutString("%2d.%03d fps", mfps / 1000, mfps % 1000);
        }
    }
}

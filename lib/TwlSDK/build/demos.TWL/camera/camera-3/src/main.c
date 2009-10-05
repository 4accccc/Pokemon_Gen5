/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - camera - camera-3
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

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

static u32 stabilizedCount;

static BOOL switchFlag;

/////
//  描画よりも表示のFPSが大きい前提でのトリプルバッファシーケンス
/////

static int wp;  // カメラからデータを取り込み中のバッファ
static int rp;  // 表示中のバッファ
static BOOL wp_pending; // 取り込みを中断した (再び同じバッファに取り込む)
static GXBGBmpScrBase dispSrc[] =
{
    GX_BG_BMPSCRBASE_0x20000,
    GX_BG_BMPSCRBASE_0x40000,
    GX_BG_BMPSCRBASE_0x60000,
};
static void* dispAddr[] =
{
    (void*)(HW_BG_VRAM + 0x20000),
    (void*)(HW_BG_VRAM + 0x40000),
    (void*)(HW_BG_VRAM + 0x60000),
};
static CAMERASelect current;
static void DebugReport(void)
{
    const char* const str[] =
    {
        "HW_BG_VRAM + 0x20000",
        "HW_BG_VRAM + 0x40000",
        "HW_BG_VRAM + 0x60000"
    };
    OSIntrMode enabled = OS_DisableInterrupts();
    int wp_bak = wp;
    int rp_bak = rp;
    (void)OS_RestoreInterrupts(enabled);
    OS_TPrintf("\nCapture to %s\tDisplay from %s\n", str[wp_bak], str[rp_bak]);
}
static GXBGBmpScrBase GetNextDispSrc(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    GXBGBmpScrBase base;
    int next = (rp + 1) % 3;
    if (next != wp) // 次バッファが書き込み中でない
    {
        rp = next;
    }
    base = dispSrc[rp];
    (void)OS_RestoreInterrupts(enabled);
    //DebugReport();
    return base;
}
static void* GetNextCaptureAddr(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    void* addr;
    int next = (wp + 1) % 3;
    if (wp_pending)         // 同じバッファに取り込みし直す
    {
        wp_pending = FALSE;
    }
    else if (next != rp)    // 次バッファが読み込み中でない
    {
        // カメラが安定するまで、キャプチャ結果は画面に表示しない
        // このデモでは異常な色を回避するために最低限待つ必要のある 4 カメラフレーム待っているが、
        // 自動露出が安定するまで待ちたい場合は、リファレンスにあるように屋内で 14、屋外で 30 フレーム待つ必要があります。
        if(stabilizedCount > 4)
        {
            wp = next;
        }
    }
    else    // 取り込むべきバッファがない (fpsからみてあり得ない)
    {
        OS_TPrintf("ERROR: there is no unused frame to capture.\n");
    }
    addr = dispAddr[wp];
    (void)OS_RestoreInterrupts(enabled);
    //DebugReport();
    return addr;
}
static void PendingCapture(void)
{
    wp_pending = TRUE;
}

// 文字表示 (1行だけ)
static u16 text_buffer[ 32 ] ATTRIBUTE_ALIGN(32);
static void PutString( char *format, ... )
{
    u16             *dest = text_buffer;
    char            temp[32+1];
    int             i;
    va_list         va;

    va_start(va, format);
    (void)OS_VSNPrintf(temp, sizeof(temp), format, va);
    va_end(va);

    MI_CpuClearFast(text_buffer, sizeof(text_buffer));
    for (i = 0; i < 32 && temp[i]; i++)
    {
        dest[i] = (u16)((u8)temp[i] | (1 << 12));
    }
    DC_StoreRange(text_buffer, sizeof(text_buffer));
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
    GX_SetBankForLCDC(GX_VRAM_LCDC_C);
    GX_SetBankForLCDC(GX_VRAM_LCDC_D);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_A, 128 * 1024);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_B, 128 * 1024);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_C, 128 * 1024);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_D, 128 * 1024);

    // ダイレクトビットマップ表示モード＆文字表示
    GX_SetBankForBG(GX_VRAM_BG_512_ABCD);         // VRAM-A,B,C,DバンクをＢＧに割り当る
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_4, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3);

    G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GX_BG_EXTPLTT_01);
    G2_SetBG1Priority(1);
    G2_BG1Mosaic(FALSE);

    wp = 0;
    rp = 2;
    wp_pending = TRUE;
    stabilizedCount = 0;
    switchFlag = FALSE;
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GetNextDispSrc());
    G2_SetBG3Priority(3);
    G2_BG3Mosaic(FALSE);

    // 文字の読み込み
    {
        static const GXRgb pal[16] = { GX_RGB(0, 0, 0), GX_RGB(31, 31, 31), };
        GX_LoadBG1Char(DEMOAsciiChr, 0x00000, sizeof(DEMOAsciiChr));
        GX_LoadBGPltt(pal, 0x0000, sizeof(pal));
    }

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
    (void)CameraInit();

    // DMA割り込み設定
    (void)OS_EnableIrqMask(OS_IE_NDMA1);

    // カメラVSYNC割り込みコールバック
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // カメラエラー割り込みコールバック
    CAMERA_SetBufferErrorCallback(CameraIntrError);

    // カメラの再起動完了コールバック
    CAMERA_SetRebootCallback(CameraIntrReboot);

    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // キャプチャスタート
    CAMERA_DmaRecvAsync(NDMA_NO, GetNextCaptureAddr(), CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
    OS_TPrintf("Camera is shooting a movie...\n");

    while (1)
    {
        u16 pad;
        u16 trg;
        static u16 old = 0xffff;
        static BOOL standby = FALSE;

        OS_WaitVBlankIntr();

        pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;

        if (trg & PAD_BUTTON_A)
        {
            DebugReport();
        }
        if (trg & PAD_BUTTON_X)
        {
            switchFlag = TRUE;
        }
        if (trg & PAD_BUTTON_B)
        {
            standby ^= 1;
            if (standby)
            {
                // StopCapture よりも前に Activate を NONE にすると、次回 Activate ON にするまで IsBusy が TRUE となる恐れがある
                CAMERA_StopCapture();
                while(CAMERA_IsBusy())
                {
                }
                MI_StopNDma(NDMA_NO);
                result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
                if(result == CAMERA_RESULT_FATAL_ERROR)
                    OS_Panic("CAMERA FATAL ERROR\n");
            }
            else
            {
                result = CAMERA_I2CActivate(current);
                if(result == CAMERA_RESULT_FATAL_ERROR)
                    OS_Panic("CAMERA FATAL ERROR\n");
                stabilizedCount = 0;
                CAMERA_DmaRecvAsync(NDMA_NO, GetNextCaptureAddr(), CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
                CAMERA_ClearBuffer();
                CAMERA_StartCapture();
            }
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
        }
    }
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GetNextDispSrc());
    GX_LoadBG1Scr(text_buffer, 0, sizeof(text_buffer));
}

//--------------------------------------------------------------------------------
//    カメラの初期化処理 (InitとI2C関連のみ)
//
BOOL CameraInit(void)
{
    CAMERAResult result;
    result = CAMERA_Init();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_Init was failed.");
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
//    カメラ割り込み処理 (エラー時とVsync時の両方で発生)
//
#define FPS_SAMPLES 4
void CameraIntrError(CAMERAResult result)
{
#pragma unused(result)
    OS_TPrintf("Error was occurred.\n");
    // カメラ停止処理
    CAMERA_StopCapture();
    MI_StopNDma(NDMA_NO);
    CAMERA_ClearBuffer();
    PendingCapture();       // 次回も同じフレームを使用する
    CAMERA_DmaRecvAsync(NDMA_NO, GetNextCaptureAddr(), CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    CAMERA_StartCapture();
}

void CameraIntrReboot(CAMERAResult result)
{
    if(result == CAMERA_RESULT_FATAL_ERROR)
    {
        return; // カメラが再起動処理を行っても復帰することができなかった
    }
    CameraIntrError(result); // DMAの同期がずれているかもしれないので仕切り直し
}

void CameraIntrVsync(CAMERAResult result)
{
#pragma unused(result)
    if(stabilizedCount <= 30)
        stabilizedCount++;
    if(switchFlag)
    {
        current = (current == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
        OS_TPrintf("call CAMERA_I2CActivate(%s)... ", (current == CAMERA_SELECT_IN ? "CAMERA_SELECT_IN" : "CAMERA_SELECT_OUT"));
        result = CAMERA_I2CActivate(current);
        if(result == CAMERA_RESULT_FATAL_ERROR)
            OS_Panic("CAMERA FATAL ERROR\n");
        OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
        stabilizedCount = 0;
        switchFlag = FALSE;
    }
}

void CameraDmaRecvIntr(void* arg)
{
#pragma unused(arg)
    MI_StopNDma(NDMA_NO);

    if (CAMERA_IsBusy() == TRUE)
    {
        if (MI_IsNDmaBusy(NDMA_NO)) // 画像の転送が終わっているかチェック
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            MI_StopNDma(NDMA_NO);
        }
        // 次のフレームのキャプチャを開始する
        CAMERA_DmaRecvAsync(NDMA_NO, GetNextCaptureAddr(), CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    }

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

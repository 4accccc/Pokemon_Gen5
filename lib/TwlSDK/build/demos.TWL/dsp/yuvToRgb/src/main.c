/*---------------------------------------------------------------------------*
  Project:  TwlSDK - DSP - demos - yuvToRgb
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

/*---------------------------------------------------------------------------
    カメラから YUV422 形式で取得した画像を、DSP を用いて
    RGB555 形式に変換して画面に表示するデモです。
    
    START ボタンで終了します。
 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/dsp.h>

#include <twl/camera.h>

#include <DEMO.h>
#include <twl/dsp/common/graphics.h>

/* 設定 */
#define DMA_NO_FOR_FS          3    // FS_Init 用
#define CAMERA_NEW_DMA_NO      1    // CAMERAで使用するNew DMA番号

#define CAM_WIDTH   256            // カメラが取得する画像の幅
#define CAM_HEIGHT  192            // カメラが取得する画像の高さ

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(CAM_WIDTH)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(CAM_WIDTH)    // 一ラインの転送バイト数

/*---------------------------------------------------------------------------*
 プロトタイプ宣言
*---------------------------------------------------------------------------*/
void VBlankIntr(void);
static void CameraIntrVsync(CAMERAResult result);
static void CameraIntrError(CAMERAResult result);
static void CameraIntrReboot(CAMERAResult result);
static void InitializeGraphics(void);
static void InitializeCamera(void);
static void WriteScreenBuffer(u16 *data, u32 width, u32 height, u16 *scr);
static void ConvertCallbackFunc(void);

/*---------------------------------------------------------------------------*
 内部変数定義
*---------------------------------------------------------------------------*/
// メイン・サブ画面のスクリーンバッファ
// カメラが取得した画像、及びDSP による変換結果を格納するバッファ
static u16 TmpBuf[2][CAM_WIDTH * CAM_HEIGHT] ATTRIBUTE_ALIGN(32);

static BOOL StartRequest = FALSE;
static OSTick StartTick;       // DSP による処理時間を計測するための変数

static int wp;                  // カメラからデータを取り込み中のバッファ
static int rp;                  // 最後のVRAMにコピーしたバッファ
static BOOL wp_pending;         // 取り込みを中断した (再び同じバッファに取り込む)

static BOOL IsConvertNow = FALSE;      // YUV -> RGB 変換を行っているかどうか

/*---------------------------------------------------------------------------*
 Name:         TwlMain

 Description:  初期化及びメインループ。

 Arguments:    None.

 Returns:      None.
*---------------------------------------------------------------------------*/
void TwlMain(void)
{
    FSFile file;
    
    DEMOInitCommon();
    OS_InitThread();
    OS_InitTick();
    OS_InitAlarm();

    DEMOInitVRAM();
    InitializeGraphics();
    InitializeCamera();

    DEMOStartDisplay();
    
    // GXでDMAを使わない (旧DMAはカメラDMAの邪魔になる)
    (void)GX_SetDefaultDMA(GX_DMA_NOT_USE);

    // 最初はROMヘッダに従って WRAM が何かに割り当てられている可能性があるので、クリアする
    (void)MI_FreeWram_B( MI_WRAM_ARM9 );
    (void)MI_CancelWram_B( MI_WRAM_ARM9 );
    (void)MI_FreeWram_C( MI_WRAM_ARM9 );
    (void)MI_CancelWram_C( MI_WRAM_ARM9 );
    
    FS_Init(DMA_NO_FOR_FS);

    (void)OS_EnableInterrupts();

    // グラフィックスコンポーネントのロード
    DSP_OpenStaticComponentGraphics(&file);
    if(!DSP_LoadGraphics(&file, 0xFF, 0xFF))
    {
        OS_TPanic("failed to load graphics DSP-component! (lack of WRAM-B/C)");
    }
    
    // カメラスタート
    wp = 0;
    rp = 1;
    wp_pending = TRUE;
    StartRequest = TRUE;
    CameraIntrVsync(CAMERA_RESULT_SUCCESS);
    OS_TPrintf("Camera is shooting a movie...\n");
    OS_TPrintf("Press A Button.\n");
    
    while (1)
    {
        DEMOReadKey();
        
        if (DEMO_IS_TRIG( PAD_BUTTON_START ))
        {
            break;    // 終了
        }
        
        if (wp == rp && !IsConvertNow)
        {
            rp ^= 1;
            DC_FlushRange(TmpBuf[rp], BYTES_PER_LINE * CAM_HEIGHT);
            GX_LoadBG3Scr(TmpBuf[rp], 0, BYTES_PER_LINE * CAM_HEIGHT);
        }
        
        OS_WaitVBlankIntr();           // Waiting the end of VBlank interrupt
    }
    
    OS_TPrintf("demo end.\n");

    // グラフィックスコンポーネントのアンロード
    DSP_UnloadGraphics();
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

// カメラ割り込み
void CameraIntrError(CAMERAResult result)
{
#pragma unused(result)
    OS_TPrintf("Error was occurred.\n");
    // 停止処理
    CAMERA_StopCapture();           // カメラ停止
    CAMERA_ClearBuffer();           // クリア
    MI_StopNDma(CAMERA_NEW_DMA_NO); // DMA停止
    wp_pending = TRUE;              // 次回も同じフレームを使用する
    StartRequest = TRUE;            // カメラ再開要求
}

void CameraIntrReboot(CAMERAResult result)
{
    if(result == CAMERA_RESULT_FATAL_ERROR)
    {
        return; // カメラが再起動処理を行っても復帰することができなかった
    }
    // カメラスタート
    CAMERA_ClearBuffer();
    MI_StopNDma(CAMERA_NEW_DMA_NO);
    wp_pending = TRUE;              // 次回も同じフレームを使用する
    StartRequest = TRUE;            // カメラ再開要求
}


void CameraIntrVsync(CAMERAResult result)
{
#pragma unused(result)
    // 以降はVsync時の処理
    if (StartRequest)
    {
        CAMERA_ClearBuffer();
        CAMERA_StartCapture();
        StartRequest = FALSE;
    }

    if (CAMERA_IsBusy() == FALSE)   // done to execute stop command?
    {
    }
    else
    {
        if (MI_IsNDmaBusy(CAMERA_NEW_DMA_NO))    // NOT done to capture last frame?
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            MI_StopNDma(CAMERA_NEW_DMA_NO);  // DMA停止
        }
        // start to capture for next frame
        if (wp_pending)
        {
            wp_pending = FALSE;
        }
        else
        {
            // 更新バッファ変更
            wp ^= 1;
            IsConvertNow = TRUE;
            StartTick = OS_GetTick();
            (void)DSP_ConvertYuvToRgbAsync(TmpBuf[rp ^1], TmpBuf[rp ^1], CAM_WIDTH * CAM_HEIGHT * sizeof(u16), ConvertCallbackFunc);
        }

        CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, TmpBuf[wp], CAMERA_GetBytesAtOnce(CAM_WIDTH), CAMERA_GET_FRAME_BYTES(CAM_WIDTH, CAM_HEIGHT), NULL, NULL);
    }
}

static void InitializeGraphics()
{
    // VRAM 割り当て
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BG0_AS_2D);
    GX_SetVisiblePlane( GX_PLANEMASK_BG3 );
    GXS_SetGraphicsMode( GX_BGMODE_4 );
    GXS_SetVisiblePlane( GX_PLANEMASK_BG3 );
    
    GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);  // スクリーンオフセット値を設定
    GX_SetBGCharOffset(GX_BGCHAROFFSET_0x20000);  // キャラクタベースオフセット値を設定
    
    G2_BlendNone();
    G2S_BlendNone();
    GX_Power2DSub(TRUE);    // サブ2Dグラフィックエンジンをオフ
    
    // main-BG
    // BG3: scr 96KB
    {
        G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
        G2_SetBG3Priority(2);
        G2_BG3Mosaic(FALSE);
    }
    
    // sub-BG
    // BG3: scr 96KB
    {
        G2S_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
        G2S_SetBG3Priority(2);
        G2S_BG3Mosaic(FALSE);
    }
}

static void InitializeCamera(void)
{
    CAMERAResult result;
    
    result = CAMERA_Init();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_Init was failed.");

    result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
    if (result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_I2CActivate was failed. (%d)\n", result);

    // カメラVSYNC割り込みコールバック
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // カメラエラー割り込みコールバック
    CAMERA_SetBufferErrorCallback(CameraIntrError);
    
    // カメラの再起動完了コールバック
    CAMERA_SetRebootCallback(CameraIntrReboot);

    CAMERA_SetOutputFormat(CAMERA_OUTPUT_YUV);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(CAM_WIDTH));
}

/* YUV -> RGB のコンバート終了時に呼ばれるコールバック関数 */
static void ConvertCallbackFunc(void)
{
    OS_TPrintf("[Async]time: %d microsec.\n", OS_TicksToMicroSeconds(OS_GetTick() - StartTick));
    IsConvertNow = FALSE;
}


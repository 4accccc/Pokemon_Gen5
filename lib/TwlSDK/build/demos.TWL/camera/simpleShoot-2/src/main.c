/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CAMERA - demos - simpleShoot-2
  File:     main.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-04-21#$
  $Rev: 10434 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "DEMO.h"
#include <twl/camera.h>
#include <twl/dsp.h>
#include <twl/dsp/common/g711.h>

#include    "snd_data.h"

#define NDMA_NO      1           // 使用するNDMA番号(0-3)
#define WIDTH       256         // イメージの幅
#define HEIGHT      192         // イメージの高さ

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // 一ラインの転送バイト数

void VBlankIntr(void);
static BOOL CameraInit(void);
static void CameraIntrVsync(CAMERAResult result);
static void CameraIntrError(CAMERAResult result);
static void CameraIntrReboot(CAMERAResult result);
static void CameraDmaRecvIntr(void* arg);

static int wp;  // カメラからデータを取り込み中のバッファ
static int rp;  // 最後のVRAMにコピーしたバッファ
static BOOL wp_pending; // 取り込みを中断した (再び同じバッファに取り込む)
static u32 stabilizedCount;
static u16 buffer[2][WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);

static BOOL switchFlag;

#define STRM_BUF_SIZE 1024*64

static char StrmBuf_shutter[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u32 len_shutter;
static char StrmBuf_begin[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u32 len_begin;
static char StrmBuf_end[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u32 len_end;

// ファイル名
static const char filename_shutter[] = "shutter_sound/shutter_sound_32730.wav";
static const char filename_begin[] = "videorec_sound/videorec_sound_begin_32730.wav";
static const char filename_end[] = "videorec_sound/videorec_sound_end_32730.wav";

static void DebugReport(void)
{
    OS_TPrintf("\nCapture to No.%d\tDisplay from No.%d\n", wp, rp);
}
static void PendingCapture(void)
{
    wp_pending = TRUE;
}

// 撮影した画像を格納するバッファ
static u16 shoot_buffer[WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);
static int shoot_flag = 0;

// RECアイコン
extern u16 _binary_rec_icon_nbfs[];

static u16* movie_buffer; // 動画用バッファ
static u32  movie_buffer_max_count = 0;
static u32  movie_buffer_count = 0;
static int  movie_flag = 0;

static CAMERASelect current_camera = CAMERA_SELECT_IN;

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
    if (wp == rp)
    {
        rp ^= 1;
        MI_CpuCopyFast(buffer[rp], (void*)HW_LCDC_VRAM_A, BYTES_PER_LINE * HEIGHT);
        if(movie_flag == 1)
        {
            int i = 0;
            MI_CpuCopy(_binary_rec_icon_nbfs+1, (void*)(HW_LCDC_VRAM_A + sizeof(u16)*1), sizeof(u16) * 30);
            for(i = 1; i < 12; i++)
                MI_CpuCopy(_binary_rec_icon_nbfs+32*i, (void*)(HW_LCDC_VRAM_A + sizeof(u16)*256*i), sizeof(u16) * 32);
            MI_CpuCopy(_binary_rec_icon_nbfs+32*12+1, (void*)(HW_LCDC_VRAM_A + sizeof(u16)*256*12+sizeof(u16)*1), sizeof(u16) * 30);
        }
        DC_InvalidateRange(buffer[rp], BYTES_PER_LINE * HEIGHT);
        if(movie_flag == 1)
        {
            MI_CpuCopyFast(buffer[rp], (void*)(movie_buffer+(WIDTH*HEIGHT*movie_buffer_count)), BYTES_PER_LINE * HEIGHT);
            movie_buffer_count++;
            if(movie_buffer_count > movie_buffer_max_count)
                movie_flag = 2;
        }
    }
    if(shoot_flag == 1)
    {
        MI_CpuCopyFast(buffer[rp], (void*)shoot_buffer, BYTES_PER_LINE * HEIGHT);
        shoot_flag = 2;
    }
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

    result = CAMERA_I2CEffect(CAMERA_SELECT_IN, CAMERA_EFFECT_NONE);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CEffect was failed. (%d)\n", result);
    }

    result = CAMERA_I2CActivate(current_camera);
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
#define PRINT_RATE  32
void CameraIntrError(CAMERAResult result)
{
#pragma unused(result)
    OS_TPrintf("Error was occurred.\n");
    // カメラ停止処理
    CAMERA_StopCapture();
    MI_StopNDma(NDMA_NO);
    CAMERA_ClearBuffer();
    wp_pending = TRUE;      // 次回も同じフレームを使用する
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
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
        current_camera = (current_camera == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
        result = CAMERA_I2CActivate(current_camera);
        if(result == CAMERA_RESULT_FATAL_ERROR)
            OS_Panic("CAMERA FATAL ERROR\n");
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
        OS_TPrintf(".");
        if (MI_IsNDmaBusy(NDMA_NO)) // 画像の転送が終わっているかチェック
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            MI_StopNDma(NDMA_NO);
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
    }

    {
        static OSTick begin = 0;
        static int count = 0;
        if (begin == 0)
        {
            begin = OS_GetTick();
        }
        else if (++count == PRINT_RATE)
        {
            OSTick uspf = OS_TicksToMicroSeconds(OS_GetTick() - begin) / count;
            int mfps = (int)(1000000000LL / uspf);
            OS_TPrintf("%2d.%03d fps\n", mfps / 1000, mfps % 1000);
            count = 0;
            begin = OS_GetTick();
        }
    }
}

void TwlMain(void)
{
    int     vram_slot = 0, count = 0;
    CAMERAResult result;

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, and initialize VRAM.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();

    // GXでDMAを使わない (旧DMAはカメラDMAの邪魔になる)
    (void)GX_SetDefaultDMA(GX_DMA_NOT_USE);

    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

    GXS_SetGraphicsMode(GX_BGMODE_3);

    GXS_SetVisiblePlane(GX_PLANEMASK_BG3);
    G2S_SetBG3Priority(0);
    G2S_BG3Mosaic(FALSE);

    G2S_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);

    MI_CpuClearFast(shoot_buffer, WIDTH*HEIGHT*sizeof(u16));
    GXS_LoadBG3Bmp(shoot_buffer, 0, WIDTH*HEIGHT*sizeof(u16));

    GXS_DispOn();

    // 初期化
    OS_InitThread();
    OS_InitTick();
    OS_InitAlarm();
    MI_InitNDmaConfig();
    FS_Init( FS_DMA_NOT_USE );

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableInterrupts();

    // VRAM表示モード
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_A, BYTES_PER_LINE * HEIGHT);
    wp = 0;
    rp = 1;
    wp_pending = TRUE;
    stabilizedCount = 0;
    switchFlag = FALSE;
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, GX_BGMODE_0, GX_BG0_AS_2D);
    OS_WaitVBlankIntr();
    GX_DispOn();

    movie_buffer_max_count = ((u32)OS_GetMainArenaHi() - (u32)OS_GetMainArenaLo()) / (BYTES_PER_LINE * HEIGHT) - 2;
    movie_buffer = (u16*)OS_AllocFromMainArenaLo( (u32)OS_GetMainArenaHi() - (u32)OS_GetMainArenaLo(), 32);

    // カメラ初期化
    (void)CameraInit();

    // DMA割り込み設定
    (void)OS_EnableIrqMask(OS_IE_NDMA1);

    // カメラVSYNC割り込みコールバック
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // カメラエラー割り込みコールバック
    CAMERA_SetBufferErrorCallback(CameraIntrError);

    // カメラの再起動完了コールバック
    CAMERA_SetRebootCallback(CameraIntrReboot);

    CAMERA_SetTrimmingParams(0, 0, WIDTH, HEIGHT);
    CAMERA_SetTrimming(TRUE);
    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // キャプチャスタート
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
    OS_TPrintf("Camera is shooting a movie...\n");

    DEMOStartDisplay();

    // DSPシャッター音の初期化
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
        FS_InitFile(file);
        DSPi_OpenStaticComponentG711Core(file);
        if (!DSP_LoadG711(file, slotB, slotC))
        {
            OS_TPanic("failed to load G.711 DSP-component! (lack of WRAM-B/C)");
        }
        (void)FS_CloseFile(file);

        // サウンド拡張機能初期化
        SNDEX_Init();

        // シャッター音のロード
        {
            FSFile file[1];
            SNDEXFrequency freq;

            if(SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
            {
                OS_Panic("failed SNDEX_GetI2SFrequency");
            }
            // I2S動作周波数の初期値は32.73kHzのはずである
            if(freq != SNDEX_FREQUENCY_32730)
            {
                OS_Panic("default value is SNDEX_FREQUENCY_32730");
            }

            FS_InitFile(file);
            // 32.73kHzのシャッター音をロードする
            if( !FS_OpenFileEx(file, filename_shutter, FS_FILEMODE_R) )
            {
                OS_Panic("failed FS_OpenFileEx");
            }
            len_shutter = FS_GetFileLength(file);
            if( FS_ReadFile(file, StrmBuf_shutter, (s32)len_shutter) == -1)
            {
                OS_Panic("failed FS_ReadFile");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(StrmBuf_shutter, len_shutter);
            
            FS_InitFile(file);
            // 32.73kHzの動画撮影開始音をロードする
            if( !FS_OpenFileEx(file, filename_begin, FS_FILEMODE_R) )
            {
                OS_Panic("failed FS_OpenFileEx");
            }
            len_begin = FS_GetFileLength(file);
            if( FS_ReadFile(file, StrmBuf_begin, (s32)len_begin) == -1)
            {
                OS_Panic("failed FS_ReadFile");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(StrmBuf_begin, len_begin);
            
            FS_InitFile(file);
            // 32.73kHzの動画撮影終了音をロードする
            if( !FS_OpenFileEx(file, filename_end, FS_FILEMODE_R) )
            {
                OS_Panic("failed FS_OpenFileEx");
            }
            len_end = FS_GetFileLength(file);
            if( FS_ReadFile(file, StrmBuf_end, (s32)len_end) == -1)
            {
                OS_Panic("failed FS_ReadFile");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(StrmBuf_end, len_end);
        }
    }

    /* サウンド初期化 */
    SND_Init();
    SND_AssignWaveArc((SNDBankData*)sound_bank_data, 0, (SNDWaveArc*)sound_wave_data);
    SND_StartSeq(0, sound_seq_data, 0, (SNDBankData*)sound_bank_data);

    while (1)
    {
        // パッド情報の読み込みと操作
        DEMOReadKey();

        if (PAD_DetectFold() == TRUE)
        {
            // シャッター音再生中ならば、完了まで待つ
            while(DSP_IsShutterSoundPlaying())
            {
                OS_Sleep(100);
            }
            
            // 動画撮影中の場合、撮影を終了してからスリープへ入る必要がある
            if((movie_flag == 1) || (movie_flag == 2))
            {
                while(DSP_PlayShutterSound(StrmBuf_end, len_end) == FALSE)
                {
                }
                movie_flag = 0;
                // 外側カメラで動画撮影時終了時には、外側カメラランプを点灯状態に戻します
                if(current_camera == CAMERA_SELECT_OUT)
                {
                    if(CAMERA_SetLED(FALSE) != CAMERA_RESULT_SUCCESS)
                        OS_TPanic("CAMERA_SetLED(FALSE) was failed.\n");
                }
                // 動画撮影終了音が鳴り終わったことを確認してからスリープにはいる
                while(DSP_IsShutterSoundPlaying())
                {
                    OS_Sleep(100);
                }
            }

            
            // スリープする前に DSP を停止させる
            DSP_UnloadG711();

            // カメラ停止処理
            CAMERA_StopCapture();
            MI_StopNDma(NDMA_NO);
            result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
            if(result == CAMERA_RESULT_FATAL_ERROR)
                OS_Panic("CAMERA FATAL ERROR\n");
            wp_pending = TRUE;      // 次回も同じフレームを使用する
            CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
            CAMERA_ClearBuffer();
            CAMERA_StartCapture();

            PM_GoSleepMode(PM_TRIGGER_COVER_OPEN | PM_TRIGGER_CARD, 0, 0);

            // スリープ復帰後に DSP を再開する
            {
                FSFile  file[1];
                int     slotB;
                int     slotC;

                slotB = 0xFF;
                slotC = 0xFF;
                // このサンプルでは簡易化のため静的メモリ上のファイルを使用する。
                FS_InitFile(file);
                DSPi_OpenStaticComponentG711Core(file);
                if (!DSP_LoadG711(file, slotB, slotC))
                {
                    OS_TPanic("failed to load G.711 DSP-component! (lack of WRAM-B/C)");
                }
                (void)FS_CloseFile(file);
            }
            // カメラもスタンバイ状態になっているので、アクティブにする
            result = CAMERA_I2CActivate(current_camera);
            if(result == CAMERA_RESULT_FATAL_ERROR)
                OS_Panic("CAMERA FATAL ERROR\n");
            stabilizedCount = 0;
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_A) && (shoot_flag == 0) && (movie_flag == 0))
        {
            while(DSP_PlayShutterSound(StrmBuf_shutter, len_shutter) == FALSE)
            {
            }
            // 外側カメラで静止画撮影時は、外側カメラランプを一時消灯します
            if(current_camera == CAMERA_SELECT_OUT)
            {
                if(CAMERA_SwitchOffLED() != CAMERA_RESULT_SUCCESS)
                    OS_TPanic("CAMERA_SwitchOffLED was failed.\n");
            }
            OS_Sleep(200);
            shoot_flag = 1;
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_Y) && (movie_flag == 0) && (shoot_flag == 0))
        {
            while(DSP_PlayShutterSound(StrmBuf_begin, len_begin) == FALSE)
            {
            }
            movie_buffer_count = 0;
            movie_flag = 1;
            // 外側カメラで動画撮影時は、外側カメラランプを点滅状態にします
            if(current_camera == CAMERA_SELECT_OUT)
            {
                if(CAMERA_SetLED(TRUE) != CAMERA_RESULT_SUCCESS)
                    OS_TPanic("CAMERA_SetLED(TRUE) was failed.\n");
            }
        }
        else if(((DEMO_IS_TRIG(PAD_BUTTON_Y)) && (movie_flag == 1)) || (movie_flag == 2))
        {
            while(DSP_PlayShutterSound(StrmBuf_end, len_end) == FALSE)
            {
            }
            movie_flag = 0;
            // 外側カメラで動画撮影時終了時には、外側カメラランプを点灯状態に戻します
            if(current_camera == CAMERA_SELECT_OUT)
            {
                if(CAMERA_SetLED(FALSE) != CAMERA_RESULT_SUCCESS)
                    OS_TPanic("CAMERA_SetLED(FALSE) was failed.\n");
            }
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_B) && (movie_flag == 0))
        {
            movie_flag = 3;
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_X) && (movie_flag == 0))
        {
            switchFlag = TRUE;
        }

        OS_WaitVBlankIntr();

        if(shoot_flag == 2)
        {
            GXS_LoadBG3Bmp((void*)((int)shoot_buffer), 0, BYTES_PER_LINE * HEIGHT);
            shoot_flag = 0;
        }

        if(movie_flag == 3)
        {
            static int frame = 0;
            static int vsync = 0;

            if(vsync == 0) // カメラは15fpsなので、VBlankの4回に1回だけ画面を更新
            {
                GXS_LoadBG3Bmp((void*)(movie_buffer+WIDTH*HEIGHT*frame), 0, BYTES_PER_LINE * HEIGHT);
                frame++;
                if(frame >= movie_buffer_count)
                {
                    frame = 0;
                    movie_flag = 0;
                }
            }
            vsync = ++vsync % 4;
        }

        // サウンドメイン処理
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
    }
}

/*---------------------------------------------------------------------------*
  Project:  TwlSDK - DSP - demos - scaling-1
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

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
 640x480 の画像データを、指定した倍率、領域について拡大（縮小）を行うデモです。
 
 画像の拡大（縮小）の際、補間方法として３つのモードが選択できます。
 
 ・操作方法
     - A : 補間モードの変更
           Nearest Neighbor, Bilinear, Bicubic, Nearest Neighbor... の順に遷移
           
     - UP, DOWN, RIGHT, LEFT : 領域の移動
           拡大（縮小）処理を行う領域を移動します。
           640x480 の画像からはみ出すような移動はできません。
           
     - START : デモの終了
     
 ・その他
     - 倍率を変えたいときは、この文章の下の定数 SCALING_FACTOR_X, SCALING_FACTOR_Y を
       変更して再ビルドして下さい。
     
     - DSP の処理領域のサイズを変えたいときは、AREA_WIDTH, AREA_HEIGHT を
       変更して再ビルドして下さい。
       
 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/dsp.h>

#include <DEMO.h>
#include <twl/dsp/common/graphics.h>

/* 設定 */
#define RUN_ASYNC     0            // 拡縮処理を非同期で実行するかどうか
#define DMA_NO_FOR_FS 1            // FS_Init 用

#define AREA_WIDTH   185            // 拡縮処理をかける領域の width
#define AREA_HEIGHT  140            // 拡縮処理をかける領域の height

#define SCALING_FACTOR_X 1.4f      // X 方向の倍率（小数点第3位まで有効, 上限は 31 まで）
#define SCALING_FACTOR_Y 1.4f      // Y 方向の倍率（小数点第3位まで有効, 上限は 31 まで）

#define OUTPUT_WIDTH  DSP_CALC_SCALING_SIZE(AREA_WIDTH, SCALING_FACTOR_X)  // 処理後の解像度, f32 型による誤差を考慮
#define OUTPUT_HEIGHT DSP_CALC_SCALING_SIZE(AREA_HEIGHT, SCALING_FACTOR_Y)

/*---------------------------------------------------------------------------*
 画像データ (640x480)
 *---------------------------------------------------------------------------*/
extern const u8 _binary_output_dat[];
extern const u8 _binary_output_dat_end[];

#define DATA_WIDTH    640
#define DATA_HEIGHT   480

/*---------------------------------------------------------------------------*
 プロトタイプ宣言
*---------------------------------------------------------------------------*/
void VBlankIntr(void);

static void ExecScaling(void);
static void InitializeGraphics(void);
static void WriteScreenBuffer(u16 *data, u32 width, u32 height, u16 *scr);
static void ScalingCallbackFunc(void);

/*---------------------------------------------------------------------------*
 内部変数定義
*---------------------------------------------------------------------------*/
// メイン・サブ画面のスクリーンバッファ
static u16 ScrBuf[HW_LCD_WIDTH * HW_LCD_HEIGHT] ATTRIBUTE_ALIGN(32);
// DSP による変換結果を格納するバッファ
static u16 TmpBuf[OUTPUT_WIDTH * OUTPUT_HEIGHT] ATTRIBUTE_ALIGN(32);

static OSTick StartTick;       // DSP による処理時間を計測するための変数
static BOOL IsDspProcessing;   // DSP が処理を実行中かどうか（非同期で処理を実行時に使用）

static u16 AreaX = 0;          // 拡縮処理をかける領域の左上の X 座標
static u16 AreaY = 0;          // 拡縮処理をかける領域の左上の Y 座標

// 補間モード
static u16 ModeNames[3] = {
    DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR,
    DSP_GRAPHICS_SCALING_MODE_BILINEAR,
    DSP_GRAPHICS_SCALING_MODE_BICUBIC
    };

static u8 ModeNameStrings[3][24] = {
        "Nearest Neighbor",
        "Bilinear",
        "Bicubic"
        };

static u32 DspMode = 0;

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
    OS_InitAlarm();             // DSP_Scaling* の同期版関数を使用する場合は必須（内部で OS_Sleep を使用しているため）

    DEMOInitVRAM();
    InitializeGraphics();

    DEMOStartDisplay();

    // 最初はROMヘッダに従って WRAM が何かに割り当てられている可能性があるので、クリアする
    (void)MI_FreeWram_B( MI_WRAM_ARM9 );
    (void)MI_CancelWram_B( MI_WRAM_ARM9 );
    (void)MI_FreeWram_C( MI_WRAM_ARM9 );
    (void)MI_CancelWram_C( MI_WRAM_ARM9 );
    
    FS_Init(DMA_NO_FOR_FS);

    (void)OS_EnableInterrupts();

    IsDspProcessing = FALSE;
    
    // スクリーンバッファをクリア
    MI_CpuClear8(ScrBuf, sizeof(ScrBuf));

    // グラフィックスコンポーネントのロード
    DSP_OpenStaticComponentGraphics(&file);
    if(!DSP_LoadGraphics(&file, 0xFF, 0xFF))
    {
        OS_TPanic("failed to load graphics DSP-component! (lack of WRAM-B/C)");
    }
    
    // 初回実行
    ExecScaling();
    
    while (1)
    {
        DEMOReadKey();
        
        if (DEMO_IS_TRIG( PAD_BUTTON_START ))
        {
            break;    // 終了
        }
        
        // 処理の対象領域の移動
        if (DEMO_IS_PRESS( PAD_KEY_RIGHT ))
        {
            AreaX += 5;
            
            if (AreaX >= DATA_WIDTH - AREA_WIDTH - 1)
            {
                AreaX = DATA_WIDTH - AREA_WIDTH - 1;
            }
            ExecScaling();
        }
        else if (DEMO_IS_PRESS( PAD_KEY_LEFT ))
        {
            if (AreaX != 0)
            {
                if (AreaX <= 5)
                {
                    AreaX = 0;
                }
                else
                {
                    AreaX -= 5;
                }
            
                ExecScaling();
            }
        }
        
        if (DEMO_IS_PRESS( PAD_KEY_UP ))
        {
            if (AreaY != 0)
            {
                if (AreaY <= 5)
                {
                    AreaY = 0;
                }
                else
                {
                    AreaY -= 5;
                }
            
                ExecScaling();
            }
        }
        else if (DEMO_IS_PRESS( PAD_KEY_DOWN ))
        {
            AreaY += 5;
            if (AreaY >= DATA_HEIGHT - AREA_HEIGHT - 1)
            {
                AreaY = DATA_HEIGHT - AREA_HEIGHT - 1;
            }
            ExecScaling();
        }
        
        // 補間モードの変更
        if (DEMO_IS_TRIG( PAD_BUTTON_A ))
        {
            DspMode++;
            if (DspMode >= 3)
            {
                DspMode = 0;
            }
            
            ExecScaling();
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

/*--------------------------------------------------------------------------------
    DSP を用いて、設定したオプションにより画像データの拡縮を行う
 ---------------------------------------------------------------------------------*/
static void ExecScaling()
{
    // 実行
#if RUN_ASYNC

	if ( !IsDspProcessing )
	{
		StartTick = OS_GetTick();

	    DSP_ScalingAsyncEx(_binary_output_dat, TmpBuf, DATA_WIDTH, DATA_HEIGHT,
	                       SCALING_FACTOR_X, SCALING_FACTOR_Y, ModeNames[DspMode], AreaX, AreaY, AREA_WIDTH, AREA_HEIGHT, ScalingCallbackFunc);

	    IsDspProcessing = TRUE;
	}

#else
	StartTick = OS_GetTick();
    (void)DSP_ScalingEx(_binary_output_dat, TmpBuf, DATA_WIDTH, DATA_HEIGHT,
                  SCALING_FACTOR_X, SCALING_FACTOR_Y, ModeNames[DspMode], AreaX, AreaY, AREA_WIDTH, AREA_HEIGHT);
    OS_TPrintf("mode: %s, time: %d microsec.\n", ModeNameStrings[DspMode], OS_TicksToMicroSeconds(OS_GetTick() - StartTick));

    // スクリーンに表示するようにデータを調整
    WriteScreenBuffer(TmpBuf, OUTPUT_WIDTH, OUTPUT_HEIGHT, ScrBuf);
    
    // スクリーンバッファのキャッシュの破棄
    DC_FlushAll();

    // 処理結果を VRAM へロード
    GX_LoadBG3Bmp(ScrBuf, 0, HW_LCD_WIDTH * HW_LCD_HEIGHT * sizeof(u16));
#endif

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

/* data の内容を 256x192 のスクリーンバッファへ書き込む */
static void WriteScreenBuffer(u16 *data, u32 width, u32 height, u16 *scr)
{
    int i;
    u32 lp_count;
    u32 tmp_linesize;
    
    // scrbuf の作成
    if( height > HW_LCD_HEIGHT )
    {
        lp_count = HW_LCD_HEIGHT;
    }
    else
    {
        lp_count = height;
    }
    
    // 256x256 の BMP として下画面に表示する都合からサイズを考慮する必要がある
    // 1ラインずつコピー
    if( width > HW_LCD_WIDTH)
    {
        tmp_linesize = HW_LCD_WIDTH * sizeof(u16);
    }
    else
    {
        tmp_linesize = width * sizeof(u16);
    }
    
    for ( i=0; i < lp_count; i++ )
    {
        MI_CpuCopy( data + width * i, scr + HW_LCD_WIDTH * i, tmp_linesize );
    }
}

/* 拡縮処理の終了時に呼ばれるコールバック関数 */
static void ScalingCallbackFunc(void)
{
    OS_TPrintf("[Async]mode: %s, time: %d microsec.\n", ModeNameStrings[DspMode], OS_TicksToMicroSeconds(OS_GetTick() - StartTick));

	    // スクリーンに表示するようにデータを調整
    WriteScreenBuffer(TmpBuf, OUTPUT_WIDTH, OUTPUT_HEIGHT, ScrBuf);
	    
    // スクリーンバッファのキャッシュの破棄
    DC_FlushAll();

    // 処理結果を VRAM へロード
    GX_LoadBG3Bmp(ScrBuf, 0, HW_LCD_WIDTH * HW_LCD_HEIGHT * sizeof(u16));

    IsDspProcessing = FALSE;
}


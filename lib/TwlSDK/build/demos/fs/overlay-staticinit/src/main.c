/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - overlay-staticinit
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-14 #$
  $Rev: 9314 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "DEMO.h"

/* 各モードのインタフェース */
#include "mode.h"


/*---------------------------------------------------------------------------*/
/* constants */

// 自動サンプリング用定数
#define     SAMPLING_FREQUENCE      4
#define     SAMPLING_BUFSIZE        (SAMPLING_FREQUENCE + 1)
#define     SAMPLING_START_VCOUNT   0


/*---------------------------------------------------------------------------*/
/* variables */

// 各モードのインタフェース
BOOL    (*UpdateFrame) (int frame_count, const InputData * input, int player_count,
                        int own_player_id);
void    (*DrawFrame) (int frame);
void    (*EndFrame) (FSOverlayID *p_next_mode);

// タッチパネル用自動サンプリングバッファ
static TPData tp_auto_buf[SAMPLING_BUFSIZE];


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         InitApp

  Description:  アプリケーション基本部分の初期化

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitApp(void)
{
    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    FS_Init(FS_DMA_NOT_USE);

    // タッチパネルの初期化
    {
        TPCalibrateParam calibrate;
        TP_Init();
        if (TP_GetUserInfo(&calibrate))
        {
            TP_SetCalibrateParam(&calibrate);
        }
        (void)TP_RequestAutoSamplingStart(SAMPLING_START_VCOUNT, SAMPLING_FREQUENCE, tp_auto_buf,
                                          SAMPLING_BUFSIZE);
    }

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    // メインディスプレイでタッチパネルを使用するためにGX設定を切り替える。
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         GetInput

  Description:  現在の入力状態を取得

  Arguments:    input             取得した情報の格納先

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GetInput(InputData * input)
{
    if (input)
    {
        const u16 hold_bak = input->hold_bits;
        u16     hold_bits;
        // タッチパネル情報を更新
        const TPData *const cur_tp = tp_auto_buf + TP_GetLatestIndexInAuto();
        if (cur_tp->touch == TP_TOUCH_OFF)
        {
            input->tp.touch = TP_TOUCH_OFF;
        }
        else if (cur_tp->validity == TP_VALIDITY_VALID)
        {
            TP_GetCalibratedPoint(&input->tp, cur_tp);
        }
        // パッド情報(およびタッチペン接触ビット)を更新
        hold_bits = (u16)(PAD_Read() | (input->tp.touch ? PAD_BUTTON_TP : 0));
        input->push_bits = (u16)(~hold_bak & hold_bits);
        input->hold_bits = hold_bits;
        input->release_bits = (u16)(hold_bak & ~hold_bits);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  アプリケーションのメインエントリポイント

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // アプリケーションフレームワークと画面遷移状態を初期化します
    int         player_count = 1;
    int         own_player_id;
    InputData   input[1];
    FSOverlayID cur_mode;
    FSOverlayID prev_mode;
    FS_EXTERN_OVERLAY(top_menu);

    InitApp();
    cur_mode = FS_OVERLAY_ID(top_menu);
    prev_mode = FS_OVERLAY_ID(top_menu);
    UpdateFrame = NULL;
    DrawFrame = NULL;
    EndFrame = NULL;
    MI_CpuClear8(input, sizeof(input));
    player_count = 1;
    own_player_id = 0;

    // オーバーレイとして各モードを構成し, それらを繰り返し遷移します
    for (;;)
    {
        // 現在のモードを開始します。
        // ロード完了とともに NitroStaticInit() で自動初期化され
        // 必要なインタフェースが自己設定されます。
        // モードの終了条件は, 現在のモード自身が UpdateFrame で返します。
        {
            int     frame = 0;
            if (!FS_LoadOverlay(MI_PROCESSOR_ARM9, cur_mode))
            {
                OS_TPanic("failed to load specified mode(%08X)", cur_mode);
            }
            GetInput(&input[own_player_id]);
            for (;;)
            {
                GetInput(&input[own_player_id]);
                if (!UpdateFrame(frame, input, player_count, own_player_id))
                {
                    break;
                }
                DrawFrame(frame);
                DEMO_DrawFlip();
                OS_WaitVBlankIntr();
                ++frame;
            }
        }
        // 現在のモードを終了し, 次のモードへ移行します。
        // 現在のモードが移行先を明示的に設定しなかった場合
        // 前回のモードへ戻ることになります。
        {
            FSOverlayID next_mode = prev_mode;
            EndFrame(&next_mode);
            if (!FS_UnloadOverlay(MI_PROCESSOR_ARM9, cur_mode))
            {
                OS_TPanic("failed to unload specified mode(%08X)", cur_mode);
            }
            prev_mode = cur_mode;
            cur_mode = next_mode;
        }
    }

}

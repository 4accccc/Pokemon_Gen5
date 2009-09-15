/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - overlay-staticinit
  File:     mode_2.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-06 #$
  $Rev: 2135 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "mode.h"
#include "DEMO.h"

// NitroStaticInit() を static initializer に指定するには
// このヘッダをインクルードします
#include <nitro/sinit.h>


/*---------------------------------------------------------------------------*/
/* variables */

// タッチペン押下状態
static BOOL is_tp_on;
static struct Point { int x, y; }  bak_pos, cur_pos;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         MyUpdateFrame

  Description:  現在のモードで 1 フレーム分だけ内部状態を更新します

  Arguments:    frame_count      現在の動作フレームカウント
                input            入力情報の配列
                player_count     現在のプレーヤー総数 (input の有効要素数)
                own_player_id    自身のプレーヤー番号

  Returns:      現在のモードが今回のフレームで終了するならば FALSE を、
                そうでなければ TRUE を返します。
 *---------------------------------------------------------------------------*/
static BOOL MyUpdateFrame(int frame_count,
                          const InputData * input, int player_count, int own_player_id)
{
    (void)frame_count;
    (void)player_count;

    // タッチペンの入力状態を更新
    bak_pos = cur_pos;
    is_tp_on = IS_INPUT_(input[own_player_id], PAD_BUTTON_TP, push);
    if (is_tp_on)
    {
        cur_pos.x = input[own_player_id].tp.x;
        cur_pos.y = input[own_player_id].tp.y;
    }
    // タッチペン以外の何かを押したら終了
    return !IS_INPUT_(input[own_player_id], PAD_ALL_MASK, push);
}

/*---------------------------------------------------------------------------*
  Name:         MyDrawFrame

  Description:  現在のモードで内部状態をもとに描画更新します

  Arguments:    frame_count      現在の動作フレームカウント

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MyDrawFrame(int frame_count)
{
    // 初回フレームは画面をクリア
    if (frame_count == 0)
    {
        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
        DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
        DEMODrawText(0, 10, "%s", __FILE__);
        DEMODrawText(30, 40, "touch screen : draw line");
        DEMODrawText(30, 50, "press any key to return");
    }
    // 現在位置と軌跡を描画
    DEMOFillRect(cur_pos.x - 2, cur_pos.y - 2, 4, 4, GX_RGBA(0, 31, 0, 1));
    if (is_tp_on)
    {
        DEMODrawLine(bak_pos.x, bak_pos.y, cur_pos.x, cur_pos.y, GX_RGBA(16, 16, 31, 1));
    }
}

/*---------------------------------------------------------------------------*
  Name:         MyEndFrame

  Description:  現在のモードを終了します.

  Arguments:    p_next_mode      次のモードを明示的に指定する場合は.
                                 このポインタが指す先にIDを上書きします.
                                 特に指定しない場合, 現在のモードを呼び出した
                                 モードが選択されます.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MyEndFrame(FSOverlayID *p_next_mode)
{
    (void)p_next_mode;
}

/*---------------------------------------------------------------------------*
  Name:         NitroStaticInit

  Description:  static initializer としての自動初期化関数.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NitroStaticInit(void)
{
    UpdateFrame = MyUpdateFrame;
    DrawFrame = MyDrawFrame;
    EndFrame = MyEndFrame;

    /* モードごとに必要な初期化処理をここで行います. */

    is_tp_on = FALSE;
    cur_pos.x = HW_LCD_WIDTH / 2;
    cur_pos.y = HW_LCD_HEIGHT / 2;
    bak_pos = cur_pos;
}

//============================================================================
/**
 *  @file   manual_list.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_LIST
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "app/app_taskmenu.h"

#include "manual_graphic.h"
#include "manual_def.h"
#include "manual_common.h"
#include "manual_touchbar.h"
#include "manual_list.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_manual.h"


// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// APP_TASKMENU_WINの項目
typedef enum
{
          ATM_WIN_ITEM_UP_HALF,
  ATM_WIN_ITEM_ENABLE_START,     // ATM_WIN_ITEM_ENABLE_START<= <ATM_WIN_ITEM_ENABLE_END
          ATM_WIN_ITEM_0             = ATM_WIN_ITEM_ENABLE_START,
          ATM_WIN_ITEM_1,
          ATM_WIN_ITEM_2,
          ATM_WIN_ITEM_3,
          ATM_WIN_ITEM_4,
          ATM_WIN_ITEM_5,
  ATM_WIN_ITEM_ENABLE_END,
          ATM_WIN_ITEM_DOWN_HALF     = ATM_WIN_ITEM_ENABLE_END,
  ATM_WIN_ITEM_MAX,

  ATM_WIN_ITEM_ENABLE_NUM   = ATM_WIN_ITEM_ENABLE_END - ATM_WIN_ITEM_ENABLE_START,
}
ATM_WIN_ITEM;

#define ATM_PLATE_WIDTH (28)  // キャラ
#define ATM_PLATE_X     (2)   // キャラ
#define ATM_PLATE_Y     (0)   // キャラ

#define SCROLL_Y  (APP_TASKMENU_PLATE_HEIGHT*8/2)


// 文字の色を変更しておく
#define ATM_HALF_ITEM_PAL_LINE_POS0 (0x0)
#define ATM_HALF_ITEM_PAL_LINE_POS1 (0x1)
#define ATM_HALF_ITEM_PAL_COLOR_POS (0xd)
#define ATM_HALF_ITEM_TEXT_COLOR    (0x1ce7)

// 選択できないパネルの文字色カラー
#define ATM_HALF_ITEM_MSGCOLOR (PRINTSYS_MACRO_LSB(0xf,0xd,0x3))


// 入力状態
typedef enum
{
  INPUT_STATE_NONE,  // 何の入力アニメーションも行っていないので、入力可能。
  INPUT_STATE_END,   // もう何の入力も受け付けない。
  INPUT_STATE_SB,    // スクロールバーの入力を行っている最中。
  INPUT_STATE_TB,    // タッチバーの入力アニメーションを行っている最中なので、入力不可能。
  INPUT_STATE_ATM,   // APP_TASKMENU_WINの入力アニメーションを行っている最中なので、入力不可能。
}
INPUT_STATE;

// OBJ
enum
{
  OBJ_LIST_RES_NCG,
  OBJ_LIST_RES_NCL,
  OBJ_LIST_RES_NCE,
  OBJ_RES_MAX,
};

// スクロールバーのラインのタッチ領域、スクロールバーのカーソルの配置範囲
#define SB_LINE_TOUCH_X_MIN  (240)  // MIN<= <=MAX
#define SB_LINE_TOUCH_X_MAX  (247)
#define SB_LINE_TOUCH_Y_MIN  ( 17)  // MIN<= <=MAX
#define SB_LINE_TOUCH_Y_MAX  (150)
#define SB_CURSOR_X          (240)
#define SB_CURSOR_Y_MIN      ( 17)  // MIN<= <=MAX
#define SB_CURSOR_Y_MAX      (135)
#define SB_CURSOR_Y_OFFSET   (  8)  // カーソルの左上が(0,0)となっているので、カーソルの中心にするにはこれだけずらす必要がある。

// NEWアイコン
#define NEW_ICON_X       (0)
#define NEW_ICON_Y       (-SCROLL_Y+4)
#define NEW_ICON_ANMSEQ  (1)

// 終了リクエスト
#define END_REQ_COUNT_START_VAL (1)  // カウントダウン開始値
#define END_REQ_COUNT_NO        (-1)  // カウントダウンしない値


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct _MANUAL_LIST_WORK
{
  // パラメータ
  MANUAL_LIST_PARAM*          param;
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // ここで作成
  // APP_TASKMENU_WIN
  APP_TASKMENU_RES*           atm_res;
  APP_TASKMENU_WIN_WORK*      atm_win_wk[ATM_WIN_ITEM_MAX];  // NULLのときなし
  BOOL                        atm_win_is_show[ATM_WIN_ITEM_MAX];  // TRUEのとき表示している、FALSEのとき表示していない
                                                                  // atm_win_wk[i]がNULLでないときだけ有効な値が入っている

  u16                         atm_half_item_text_color_buf;

  // OBJ
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   sb_cursor_clwk;
  GFL_CLWK*                   new_icon_clwk[ATM_WIN_ITEM_MAX];

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // 入力状態
  INPUT_STATE                 input_state;
  BOOL                        b_sb_touch;  // TRUEのときスクロールバーをタッチしている
  BOOL                        b_sb_cursor_pos_change;  // TRUEのときスクロールバーをタッチしたためにparam->cursor_posの位置が変わった。

  // 終了リクエスト
  s8                          end_req_count;  // 負のときカウントダウンしない。正のときカウントダウンする。0になったら終了。

  // 初回フラグ
  BOOL                        first_create_done;  // 初回作成が完了していたらTRUE、完了していなかったらFALSE
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Manual_List_VBlankFunc( GFL_TCB* tcb, void* wk );

// 準備
static void Manual_List_Prepare( MANUAL_LIST_WORK* work );
static void Manual_List_PrepareAppTaskmenuWin( MANUAL_LIST_WORK* work );
// 転送(VBlank転送ではない)
static void Manual_List_Trans( MANUAL_LIST_WORK* work );
// 転送(VBlank転送)
static void Manual_List_TransVBlank( MANUAL_LIST_WORK* work );
// 後片付け
static void Manual_List_Finish( MANUAL_LIST_WORK* work );
static void Manual_List_FinishAppTaskmenuWin( MANUAL_LIST_WORK* work );

// 入力
static BOOL Manual_List_InputAppTaskmenuWin( MANUAL_LIST_WORK* work );
static BOOL Manual_List_InputSb( MANUAL_LIST_WORK* work );

// タッチの当たり判定テーブルの作成
static void Manual_List_MakeAtmTpHittbl( GFL_UI_TP_HITTBL* atm_tp_hittbl );  // atm_tp_hittbl[ATM_WIN_ITEM_ENABLE_NUM +1]

// MANUAL_LIST_PARAMのcursor_posをATM_WIN_ITEMに変換する
static ATM_WIN_ITEM Manual_List_ConvertCursorPosToAtmWinItem( MANUAL_LIST_WORK* work );
// 画面の1番最後にある全項目の中での番号を得る(画面の1番下(DOWN_HALFは除く)から更に続く場合もしくはちょうどそこに届いている場合は、
// 画面の1番下(DOWN_HALFは除く)にある項目の番号になる。そこに届いていない場合は、work->param->num-1になりその場所は画面の1番下より上である。)
static u16 Manual_List_GetTailPos( MANUAL_LIST_WORK* work );
// 画面に出ている選択できるパネルの個数
static u16 Manual_List_GetEnableNum( MANUAL_LIST_WORK* work );
// head_posの取ることができる最大値(含む、0からこの値まで変更してOK)を得る
static u16 Manual_List_GetHeadPosMax( MANUAL_LIST_WORK* work );

// リストが動いたのでつくり直す
static void Manual_List_UpdateAppTaskmenuWin( MANUAL_LIST_WORK* work );

// タッチしているy座標からhead_posを求める
static u16 Manual_List_GetHeadPosFromY( MANUAL_LIST_WORK* work, u16 num_for_head_pos, u16 y );
// head_posからスクロールバーのカーソルを置くy座標を求める(SB_CURSOR_Y_OFFSETも考慮してあるので、戻り値のyをそのままclwkに使ってやればよい)
static u16 Manual_List_GetSbCursorYFromHeadPos( MANUAL_LIST_WORK* work, u16 num_for_head_pos, u16 head_pos );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_LIST_WORK*  MANUAL_LIST_Init(
    MANUAL_LIST_PARAM*   param,
    MANUAL_COMMON_WORK*  cmn_wk
)
{
  // ワーク
  MANUAL_LIST_WORK*  work  = GFL_HEAP_AllocClearMemory( cmn_wk->heap_id, sizeof(MANUAL_LIST_WORK) );

  // パラメータ
  work->param  = param;
  {
    // out
    work->param->result = MANUAL_LIST_RESULT_RETURN;
  }
  // 共通
  work->cmn_wk = cmn_wk;

  // 0, NULLで初期化
  {
    u8 i;
    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      work->atm_win_wk[i] = NULL;
      work->atm_win_is_show[i] = FALSE;
    }
  }

  // 初回フラグ
  work->first_create_done = FALSE;

  // ここで作成
  Manual_List_Prepare( work );
  //Manual_List_Trans( work );
  Manual_List_TransVBlank( work );

  // スクロール
  GFL_BG_SetScrollReq( BG_FRAME_S_MAIN, GFL_BG_SCROLL_Y_SET, SCROLL_Y );

  // スクロールバーのカーソルを表示するか、非表示にするか
  {
    GFL_CLACT_WK_SetDrawEnable( work->sb_cursor_clwk, ( work->param->num > ATM_WIN_ITEM_ENABLE_NUM ) );
  }

  // スクロールバーのカーソルの位置
  {
    GFL_CLACTPOS pos;
    u16 y = Manual_List_GetSbCursorYFromHeadPos( work, Manual_List_GetHeadPosMax(work)+1, work->param->head_pos );
    pos.x = SB_CURSOR_X;
    pos.y = y; 
    GFL_CLACT_WK_SetPos( work->sb_cursor_clwk, &pos, CLSYS_DEFREND_SUB );
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_List_VBlankFunc, work, 1 );

  // 入力状態
  work->input_state = INPUT_STATE_NONE;
  work->b_sb_touch = FALSE;
  work->b_sb_cursor_pos_change = FALSE;
 
  // 終了リクエスト
  work->end_req_count = END_REQ_COUNT_NO;

  // マニュアルタッチバー
  {
    MANUAL_TOUCHBAR_TYPE t = MANUAL_TOUCHBAR_TYPE_TITLE;
    if( work->param->type == MANUAL_LIST_TYPE_CATEGORY )   t = MANUAL_TOUCHBAR_TYPE_CATEGORY;
    else if( work->param->type == MANUAL_LIST_TYPE_TITLE ) t = MANUAL_TOUCHBAR_TYPE_TITLE;
    MANUAL_TOUCHBAR_SetType( work->cmn_wk->mtb_wk, t );
  }

  return work;
}

// 終了処理
void  MANUAL_LIST_Exit(
    MANUAL_LIST_WORK*     work
)
{
  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // スクロール
  GFL_BG_SetScrollReq( BG_FRAME_S_MAIN, GFL_BG_SCROLL_Y_SET, 0 );

  // ここで作成
  //Manual_List_Finish( work );

  // ワーク
  GFL_HEAP_FreeMemory( work );
}

// 主処理
BOOL  MANUAL_LIST_Main(
    MANUAL_LIST_WORK*     work
)
{
  BOOL b_end = FALSE;

  // まだ書き終わっていないテキストがあるうちは、何の処理もさせずに戻る
  if( !PRINTSYS_QUE_IsFinished( work->cmn_wk->print_que_main ) )
  {
    return FALSE;
  }

  if( work->end_req_count == 0 )
  {
    b_end = TRUE;
  }
  else if( work->end_req_count > 0 )
  {
    if( work->end_req_count == END_REQ_COUNT_START_VAL )
    {
      Manual_List_Finish( work );
    }
    work->end_req_count--;
  }

  if( work->input_state != INPUT_STATE_END )
  {
    // APP_TASKMENU_WIN
    u8 i;
    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      if( work->atm_win_wk[i] && work->atm_win_is_show[i] ) APP_TASKMENU_WIN_Update( work->atm_win_wk[i] );
    }
  }

  // スクロールバーの入力を調べる
  if(    work->input_state == INPUT_STATE_NONE
      || work->input_state == INPUT_STATE_SB )
  {
    // スクロールバー
    BOOL b_sb = Manual_List_InputSb( work );
    if( b_sb ) work->input_state = INPUT_STATE_SB;
    else       work->input_state = INPUT_STATE_NONE;
  }

  // マニュアルタッチバーの入力を調べる
  if(    work->input_state == INPUT_STATE_NONE
      || work->input_state == INPUT_STATE_TB )
  {
    // マニュアルタッチバー
    MANUAL_TOUCHBAR_Main( work->cmn_wk->mtb_wk );

    if( work->input_state == INPUT_STATE_NONE )
    {
      TOUCHBAR_ICON icon = MANUAL_TOUCHBAR_GetTouch( work->cmn_wk->mtb_wk );
      if( icon != TOUCHBAR_SELECT_NONE )
      {
        work->input_state = INPUT_STATE_TB;
      }
    }
    else
    {
      TOUCHBAR_ICON icon = MANUAL_TOUCHBAR_GetTrg( work->cmn_wk->mtb_wk );
      if( icon != TOUCHBAR_SELECT_NONE )
      {
        switch( icon )
        {
        case TOUCHBAR_ICON_RETURN:
          {
            work->param->result = MANUAL_LIST_RESULT_RETURN;
          }
          break;
        }
        work->input_state = INPUT_STATE_END;
        //b_end = TRUE;
        work->end_req_count = END_REQ_COUNT_START_VAL;
      }
    }
  }

  // APP_TASKMENU_WINの入力を調べる
  if(    work->input_state == INPUT_STATE_NONE
      || work->input_state == INPUT_STATE_ATM )
  {
/*
描画を更新するだけなので、毎フレームAPP_TASKMENU_WIN_Updateを呼び出しても大丈夫なはずだから、外に出した。
    // APP_TASKMENU_WIN
    u8 i;
    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      if( work->atm_win_wk[i] && work->atm_win_is_show[i] ) APP_TASKMENU_WIN_Update( work->atm_win_wk[i] );
    }
 */

    if( work->input_state == INPUT_STATE_NONE )
    {
      BOOL b_decide = Manual_List_InputAppTaskmenuWin( work );
      if( b_decide ) work->input_state = INPUT_STATE_ATM;
    }
    else
    {
      BOOL b_finish = APP_TASKMENU_WIN_IsFinish( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)] );
      if( b_finish )
      {
        work->param->result = MANUAL_LIST_RESULT_ITEM;
        work->input_state = INPUT_STATE_END;
        //b_end = TRUE;
        work->end_req_count = END_REQ_COUNT_START_VAL;
      }
    }
  }
  
  return b_end;
}


//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in]       
 *  @param[in,out]       
 *
 *  @retval          
 */
//------------------------------------------------------------------

//-------------------------------------
/// 
//=====================================


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Manual_List_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_LIST_WORK* work = (MANUAL_LIST_WORK*)wk;

  // VBlank転送するならここで。
  //Manual_List_Trans( work );
}

// 準備
static void Manual_List_Prepare( MANUAL_LIST_WORK* work )
{
  // サブBG
  GFL_ARCHDL_UTIL_TransVramPalette(
      work->cmn_wk->handle_system,
      NARC_manual_bg_NCLR,
      PALTYPE_SUB_BG,
      BG_PAL_POS_S_REAR * 0x20,
      BG_PAL_NUM_S_REAR * 0x20,
      work->cmn_wk->heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      work->cmn_wk->handle_system,
      NARC_manual_bg_NCGR,
      BG_FRAME_S_REAR,
			0,
      0,  // 全転送
      FALSE,
      work->cmn_wk->heap_id );

/*
  GFL_ARCHDL_UTIL_TransVramScreen(
      work->cmn_wk->handle_system,
      NARC_manual_bg2_NSCR,
      BG_FRAME_S_REAR,
      0,
      0,  // 全転送
      FALSE,
      work->cmn_wk->heap_id );
*/

  GFL_ARCHDL_UTIL_TransVramScreenCharOfsVBlank(
      work->cmn_wk->handle_system,
      NARC_manual_bg2_NSCR,
      BG_FRAME_S_REAR,
      0,
      0,
      0,  // 全転送
      FALSE,
      work->cmn_wk->heap_id );

  // OBJ
  {
    // リソース
    work->obj_res[OBJ_LIST_RES_NCL] = GFL_CLGRP_PLTT_RegisterEx(
        work->cmn_wk->handle_system,
        NARC_manual_obj_NCLR,
        CLSYS_DRAW_SUB,
        OBJ_PAL_POS_S_LIST * 0x20,
        0,
        OBJ_PAL_NUM_S_LIST,
        work->cmn_wk->heap_id );
    work->obj_res[OBJ_LIST_RES_NCG] = GFL_CLGRP_CGR_Register(
        work->cmn_wk->handle_system,
        NARC_manual_obj_NCGR,
        FALSE,
        CLSYS_DRAW_SUB,
        work->cmn_wk->heap_id );
    work->obj_res[OBJ_LIST_RES_NCE] = GFL_CLGRP_CELLANIM_Register(
        work->cmn_wk->handle_system,
        NARC_manual_obj_NCER,
        NARC_manual_obj_NANR,
        work->cmn_wk->heap_id );

    // clwk
    // sb_cursor_clwk
    {
      GFL_CLWK_DATA data =
      {
        SB_CURSOR_X, SB_CURSOR_Y_MIN,
        0,
        0,
        BG_FRAME_PRI_S_REAR
      };
      work->sb_cursor_clwk = GFL_CLACT_WK_Create(
          MANUAL_GRAPHIC_GetClunit(work->cmn_wk->graphic),
          work->obj_res[OBJ_LIST_RES_NCG],
          work->obj_res[OBJ_LIST_RES_NCL],
          work->obj_res[OBJ_LIST_RES_NCE],
          &data,
          CLSYS_DEFREND_SUB,
          work->cmn_wk->heap_id );
      GFL_CLACT_WK_SetDrawEnable( work->sb_cursor_clwk, FALSE );
      GFL_CLACT_WK_SetAutoAnmFlag( work->sb_cursor_clwk, TRUE );
    }

    // new_icon_clwk
    {
      GFL_CLWK_DATA data =
      {
        NEW_ICON_X, NEW_ICON_Y,
        NEW_ICON_ANMSEQ,
        0,
        BG_FRAME_PRI_S_MAIN
      };
      u8 i;
      for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
      {
        data.pos_y = NEW_ICON_Y + APP_TASKMENU_PLATE_HEIGHT*i *8;
        work->new_icon_clwk[i] = GFL_CLACT_WK_Create(
            MANUAL_GRAPHIC_GetClunit(work->cmn_wk->graphic),
            work->obj_res[OBJ_LIST_RES_NCG],
            work->obj_res[OBJ_LIST_RES_NCL],
            work->obj_res[OBJ_LIST_RES_NCE],
            &data,
            CLSYS_DEFREND_SUB,
            work->cmn_wk->heap_id );
        GFL_CLACT_WK_SetDrawEnable( work->new_icon_clwk[i], FALSE );
        GFL_CLACT_WK_SetAutoAnmFlag( work->new_icon_clwk[i], TRUE );
      }
    }
  }

  // APP_TASKMENU_WIN
  Manual_List_PrepareAppTaskmenuWin( work );
}

static void Manual_List_PrepareAppTaskmenuWin( MANUAL_LIST_WORK* work )
{
  // APP_TASKMENU_WIN
  // リソース
  {
    work->atm_res = APP_TASKMENU_RES_Create(
        BG_FRAME_S_MAIN,
        BG_PAL_POS_S_ATM,
        work->cmn_wk->font,
        work->cmn_wk->print_que_main,
        work->cmn_wk->heap_id );
  }

  // 文字の色を変更しておく
  {
    work->atm_half_item_text_color_buf = ATM_HALF_ITEM_TEXT_COLOR;
    NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_BG_PLTT_SUB,
        ( BG_PAL_POS_S_ATM + ATM_HALF_ITEM_PAL_LINE_POS0 ) * 0x20 + ATM_HALF_ITEM_PAL_COLOR_POS * 0x2,
        &(work->atm_half_item_text_color_buf),
        0x2 );
    NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_BG_PLTT_SUB,
        ( BG_PAL_POS_S_ATM + ATM_HALF_ITEM_PAL_LINE_POS1 ) * 0x20 + ATM_HALF_ITEM_PAL_COLOR_POS * 0x2,
        &(work->atm_half_item_text_color_buf),
        0x2 );
  }

  // リストを動かした際に呼び出す関数を利用して、最初の表示を作成する
  Manual_List_UpdateAppTaskmenuWin(work);

  // カーソルの表示
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
    work->b_sb_cursor_pos_change = FALSE;
  }
}

// 転送(VBlank転送ではない)
static void Manual_List_Trans( MANUAL_LIST_WORK* work )
{
  GFL_BG_LoadScreenReq( BG_FRAME_S_REAR );
}

// 転送(VBlank転送)
static void Manual_List_TransVBlank( MANUAL_LIST_WORK* work )
{
  GFL_BG_LoadScreenV_Req( BG_FRAME_S_REAR );
}

// 後片付け
static void Manual_List_Finish( MANUAL_LIST_WORK* work )
{
  // APP_TASKMENU_WIN
  Manual_List_FinishAppTaskmenuWin( work );

  // OBJ
  {
    // clwk
    // new_icon_clwk
    {
      u8 i;
      for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
      {
        GFL_CLACT_WK_Remove( work->new_icon_clwk[i] );
      }
    }
    // sb_cursor_clwk
    {
      GFL_CLACT_WK_Remove( work->sb_cursor_clwk );
    }
    // リソース
    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_LIST_RES_NCE] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_LIST_RES_NCG] );
    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_LIST_RES_NCL] );
  }
}
static void Manual_List_FinishAppTaskmenuWin( MANUAL_LIST_WORK* work )
{
  // APP_TASKMENU_WIN

  // まだ書き終わっていないテキストがあるといけないので、クリアしておく
  PRINTSYS_QUE_Clear( work->cmn_wk->print_que_main );
  {
    u8 i;
    // ワーク
    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      if( work->atm_win_wk[i] )
      {
        APP_TASKMENU_WIN_Delete( work->atm_win_wk[i] );
      }
    }
    // リソース
    APP_TASKMENU_RES_Delete( work->atm_res );
  }

  GFL_BG_LoadScreenV_Req( BG_FRAME_S_MAIN );
}

// 入力
static BOOL Manual_List_InputAppTaskmenuWin( MANUAL_LIST_WORK* work )
{
  // 決定したときTRUEを返す

  int tp_hit;
  int enable_num;
  u16 tail_pos;

  GFL_UI_TP_HITTBL  atm_tp_hittbl[ATM_WIN_ITEM_ENABLE_NUM +1];
  Manual_List_MakeAtmTpHittbl(atm_tp_hittbl);

  tp_hit = GFL_UI_TP_HitTrg(atm_tp_hittbl);

  enable_num = Manual_List_GetEnableNum(work);
  tail_pos   = Manual_List_GetTailPos(work);

  // タッチorキーの切り替えチェック
  // キー入力のとき
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    if( 0<=tp_hit && tp_hit<enable_num )
    {
      // タッチに変更
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      // タッチに変更した場合は、このまま処理を続ける
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], FALSE );
    }
  }
  // タッチ入力のとき
  else
  {
    if( GFL_UI_KEY_GetTrg() )
    {
      if( !( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ) )  // タッチバーへの入力以外なら
      {
        // キーに変更
        GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
        // キーに変更した場合は、カーソルを表示して、処理を終える
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
        work->b_sb_cursor_pos_change = FALSE;
        PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
        return FALSE;
      }
    }
  }

  // キー入力受け付け
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    // キーのまま変更なしなので、GFL_UI_SetTouchOrKeyは不要。
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      // 決定
      APP_TASKMENU_WIN_SetDecide( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
      PMSND_PlaySE(MANUAL_SND_ATM_DECIDE);
      return TRUE;
    }
    if( work->param->num > 1 )
    {
      if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
      {
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], FALSE );
        
        if( work->param->cursor_pos > work->param->head_pos )  // Activeの移動だけで済む場合
        {
          work->param->cursor_pos--;
        }
        else
        {
          if( work->param->num <= ATM_WIN_ITEM_ENABLE_NUM )  // Activeの移動だけで済む場合  // 最後尾へループ
          {
            work->param->cursor_pos = work->param->num -1;
          }
          else  // リストを動かす必要があるとき
          {
            if( work->param->cursor_pos > 0 )
            {
              work->param->head_pos--;
              work->param->cursor_pos--;
            }
            else  // 最後尾へループ
            {
              work->param->head_pos = Manual_List_GetHeadPosMax(work);
              work->param->cursor_pos = work->param->num -1;
            }
            Manual_List_UpdateAppTaskmenuWin(work);

            // スクロールバーのカーソルの位置
            {
              GFL_CLACTPOS pos;
              u16 y = Manual_List_GetSbCursorYFromHeadPos( work, Manual_List_GetHeadPosMax(work)+1, work->param->head_pos );
              pos.x = SB_CURSOR_X;
              pos.y = y; 
              GFL_CLACT_WK_SetPos( work->sb_cursor_clwk, &pos, CLSYS_DEFREND_SUB );
            }
          }
        }

        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
        work->b_sb_cursor_pos_change = FALSE;
        PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
      }
      else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
      {
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], FALSE );

        if( work->param->cursor_pos < tail_pos )  // Activeの移動だけで済む場合
        {
          work->param->cursor_pos++;
        }
        else
        {
          if( work->param->num <= ATM_WIN_ITEM_ENABLE_NUM )  // Activeの移動だけで済む場合  // 先頭へループ
          {
            work->param->cursor_pos = 0;
          }
          else  // リストを動かす必要があるとき
          {
            if( work->param->cursor_pos < work->param->num -1 )
            {
              work->param->head_pos++;
              work->param->cursor_pos++;
            }
            else  // 先頭へループ
            {
              work->param->head_pos = 0;
              work->param->cursor_pos = 0;
            }
            Manual_List_UpdateAppTaskmenuWin(work);

            // スクロールバーのカーソルの位置
            {
              GFL_CLACTPOS pos;
              u16 y = Manual_List_GetSbCursorYFromHeadPos( work, Manual_List_GetHeadPosMax(work)+1, work->param->head_pos );
              pos.x = SB_CURSOR_X;
              pos.y = y; 
              GFL_CLACT_WK_SetPos( work->sb_cursor_clwk, &pos, CLSYS_DEFREND_SUB );
            }
          }
        }

        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
        work->b_sb_cursor_pos_change = FALSE;
        PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
      }
      else if( GFL_UI_KEY_GetRepeat() & ( PAD_KEY_LEFT | PAD_BUTTON_L ) )
      {
        if( work->param->cursor_pos > 0 )
        {
          APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], FALSE );
        
          if( work->param->num <= ATM_WIN_ITEM_ENABLE_NUM )  // Activeの移動だけで済む場合  // 先頭へ
          {
            work->param->cursor_pos = 0;
          }
          else  // リストを動かす必要があるとき
          {
            if( work->param->head_pos >= ATM_WIN_ITEM_ENABLE_NUM )
            {
              work->param->head_pos -= ATM_WIN_ITEM_ENABLE_NUM;
              work->param->cursor_pos -= ATM_WIN_ITEM_ENABLE_NUM;
            }
            else
            {
              work->param->head_pos = 0;
              if( work->param->cursor_pos >= ATM_WIN_ITEM_ENABLE_NUM )
              {
                work->param->cursor_pos -= ATM_WIN_ITEM_ENABLE_NUM;
              }
              else
              {
                work->param->cursor_pos = 0;
              }
            }
            Manual_List_UpdateAppTaskmenuWin(work);

            // スクロールバーのカーソルの位置
            {
              GFL_CLACTPOS pos;
              u16 y = Manual_List_GetSbCursorYFromHeadPos( work, Manual_List_GetHeadPosMax(work)+1, work->param->head_pos );
              pos.x = SB_CURSOR_X;
              pos.y = y; 
              GFL_CLACT_WK_SetPos( work->sb_cursor_clwk, &pos, CLSYS_DEFREND_SUB );
            }
          }

          APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
          work->b_sb_cursor_pos_change = FALSE;
          PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
        }
      }
      else if( GFL_UI_KEY_GetRepeat() & ( PAD_KEY_RIGHT | PAD_BUTTON_R ) )
      {
        if( work->param->cursor_pos < work->param->num -1 )
        {
          APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], FALSE );
        
          if( work->param->num <= ATM_WIN_ITEM_ENABLE_NUM )  // Activeの移動だけで済む場合  // 最後尾へ
          {
            work->param->cursor_pos = work->param->num -1;
          }
          else  // リストを動かす必要があるとき
          {
            work->param->head_pos += ATM_WIN_ITEM_ENABLE_NUM;
            if( work->param->head_pos > Manual_List_GetHeadPosMax(work) )
            {
              work->param->head_pos = Manual_List_GetHeadPosMax(work);
            }
            work->param->cursor_pos += ATM_WIN_ITEM_ENABLE_NUM;
            if( work->param->cursor_pos >= work->param->num )
            {
              work->param->cursor_pos = work->param->num -1;
            }
            Manual_List_UpdateAppTaskmenuWin(work);

            // スクロールバーのカーソルの位置
            {
              GFL_CLACTPOS pos;
              u16 y = Manual_List_GetSbCursorYFromHeadPos( work, Manual_List_GetHeadPosMax(work)+1, work->param->head_pos );
              pos.x = SB_CURSOR_X;
              pos.y = y; 
              GFL_CLACT_WK_SetPos( work->sb_cursor_clwk, &pos, CLSYS_DEFREND_SUB );
            }
          }

          APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
          work->b_sb_cursor_pos_change = FALSE;
          PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
        }
      }
    }
  }
  // タッチ入力受け付け
  else
  {
    // タッチのまま変更なしなので、GFL_UI_SetTouchOrKeyは不要。
    if( 0<=tp_hit && tp_hit<enable_num )
    {
      // 決定
      work->param->cursor_pos = work->param->head_pos + tp_hit;
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
      work->b_sb_cursor_pos_change = FALSE;
      APP_TASKMENU_WIN_SetDecide( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], TRUE );
      PMSND_PlaySE(MANUAL_SND_ATM_DECIDE);
      return TRUE;
    }
  }

  return FALSE;
}

static BOOL Manual_List_InputSb( MANUAL_LIST_WORK* work )
{
  // 戻り値(他の入力を受け付けてはならないときTRUEを返す)
  // タッチ中→タッチ中                          TRUE
  // タッチ中→タッチしていないとき              TRUE
  // タッチしていないとき→タッチ中              TRUE
  // タッチしていないとき→タッチしていないとき  FALSE
  // スクロールバーのカーソルが非表示のとき      FALSE


  // スクロールバーのカーソルが非表示のとき
  if( !GFL_CLACT_WK_GetDrawEnable( work->sb_cursor_clwk ) )
  {
    return FALSE;
  }

  if( work->b_sb_touch )
  {
    // タッチ中
    u32 x, y;  
    if( GFL_UI_TP_GetPointCont( &x, &y ) )
    {
      GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], FALSE );
      
      // スクロールバーのカーソルの位置
      {
        GFL_CLACTPOS pos;
        pos.x = SB_CURSOR_X;
        pos.y = (y >= SB_CURSOR_Y_OFFSET)?(y - SB_CURSOR_Y_OFFSET):(0);
        if( pos.y < SB_CURSOR_Y_MIN )      pos.y = SB_CURSOR_Y_MIN;
        else if( pos.y > SB_CURSOR_Y_MAX ) pos.y = SB_CURSOR_Y_MAX;
        GFL_CLACT_WK_SetPos( work->sb_cursor_clwk, &pos, CLSYS_DEFREND_SUB );
      }

      {
        u16 head_pos_new = Manual_List_GetHeadPosFromY( work, Manual_List_GetHeadPosMax(work)+1, (u16)y );
        if( head_pos_new != work->param->head_pos )  // リストを動かす必要があるとき
        {
          u16 tail_pos;
          work->param->head_pos = head_pos_new;
          tail_pos = Manual_List_GetTailPos( work );
          if( work->b_sb_cursor_pos_change || work->param->cursor_pos<work->param->head_pos || tail_pos<work->param->cursor_pos )
          {  // スクロールバーをタッチしてparam->cursor_posを変更しているなら、継続して変更させて先頭を選ばせるようにする。
            work->param->cursor_pos = work->param->head_pos;
            work->b_sb_cursor_pos_change = TRUE;
          }

          Manual_List_UpdateAppTaskmenuWin(work);

          PMSND_PlaySE( MANUAL_SND_SB_MOVE );
        }
      }
    }
    else
    {
      work->b_sb_touch = FALSE;
    }

    return TRUE; 
  }
  else
  {
    // タッチしていないとき
    u32 x, y;  
    if( GFL_UI_TP_GetPointTrg( &x, &y ) )
    {
      if(    SB_LINE_TOUCH_X_MIN<=x && x<=SB_LINE_TOUCH_X_MAX
          && SB_LINE_TOUCH_Y_MIN<=y && y<=SB_LINE_TOUCH_Y_MAX )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[Manual_List_ConvertCursorPosToAtmWinItem(work)], FALSE );
        
        PMSND_PlaySE( MANUAL_SND_SB_MOVE );
        work->b_sb_touch = TRUE;

        // スクロールバーのカーソルの位置
        {
          GFL_CLACTPOS pos;
          pos.x = SB_CURSOR_X;
          pos.y = (y >= SB_CURSOR_Y_OFFSET)?(y - SB_CURSOR_Y_OFFSET):(0);
          if( pos.y < SB_CURSOR_Y_MIN )      pos.y = SB_CURSOR_Y_MIN;
          else if( pos.y > SB_CURSOR_Y_MAX ) pos.y = SB_CURSOR_Y_MAX;
          GFL_CLACT_WK_SetPos( work->sb_cursor_clwk, &pos, CLSYS_DEFREND_SUB );
        }

        {
          u16 head_pos_new = Manual_List_GetHeadPosFromY( work, Manual_List_GetHeadPosMax(work)+1, (u16)y );
          if( head_pos_new != work->param->head_pos )  // リストを動かす必要があるとき
          {
            u16 tail_pos;
            work->param->head_pos = head_pos_new;
            tail_pos = Manual_List_GetTailPos( work );
            if( work->b_sb_cursor_pos_change || work->param->cursor_pos<work->param->head_pos || tail_pos<work->param->cursor_pos )
            {  // スクロールバーをタッチしてparam->cursor_posを変更しているなら、継続して変更させて先頭を選ばせるようにする。
              work->param->cursor_pos = work->param->head_pos;
              work->b_sb_cursor_pos_change = TRUE;
            }
            Manual_List_UpdateAppTaskmenuWin(work);
          }
        }
      }
    }
    if( work->b_sb_touch ) return TRUE;
    else                   return FALSE;
  }
}

// タッチの当たり判定テーブルの作成
static void Manual_List_MakeAtmTpHittbl( GFL_UI_TP_HITTBL* atm_tp_hittbl )  // atm_tp_hittbl[ATM_WIN_ITEM_ENABLE_NUM +1]
{
  u8 i;
  for( i=0; i<ATM_WIN_ITEM_ENABLE_NUM; i++ )
  {
    atm_tp_hittbl[i].rect.top    = SCROLL_Y + ( ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*(i+0) ) *8; 
    atm_tp_hittbl[i].rect.bottom = SCROLL_Y + ( ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*(i+1) ) *8 -1;
    atm_tp_hittbl[i].rect.left   = ( ATM_PLATE_X ) *8;
    atm_tp_hittbl[i].rect.right  = ( ATM_PLATE_X + ATM_PLATE_WIDTH ) *8 -1;
  }
  atm_tp_hittbl[ATM_WIN_ITEM_ENABLE_NUM].rect.top    = GFL_UI_TP_HIT_END;
  atm_tp_hittbl[ATM_WIN_ITEM_ENABLE_NUM].rect.bottom = 0;
  atm_tp_hittbl[ATM_WIN_ITEM_ENABLE_NUM].rect.left   = 0;
  atm_tp_hittbl[ATM_WIN_ITEM_ENABLE_NUM].rect.right  = 0;
}

// MANUAL_LIST_PARAMのcursor_posをATM_WIN_ITEMに変換する
static ATM_WIN_ITEM Manual_List_ConvertCursorPosToAtmWinItem( MANUAL_LIST_WORK* work )
{
  u16 item = work->param->cursor_pos - work->param->head_pos + ATM_WIN_ITEM_ENABLE_START;
  return item;
}
// 画面の1番最後にある全項目の中での番号を得る(画面の1番下(DOWN_HALFは除く)から更に続く場合もしくはちょうどそこに届いている場合は、
// 画面の1番下(DOWN_HALFは除く)にある項目の番号になる。そこに届いていない場合は、work->param->num-1になりその場所は画面の1番下より上である。)
static u16 Manual_List_GetTailPos( MANUAL_LIST_WORK* work )
{
  u16 tail_pos = work->param->head_pos + ATM_WIN_ITEM_ENABLE_NUM -1;
  if( tail_pos >= work->param->num ) tail_pos = work->param->num -1;
  return tail_pos;
}
// 画面に出ている選択できるパネルの個数
static u16 Manual_List_GetEnableNum( MANUAL_LIST_WORK* work )
{
  return Manual_List_GetTailPos(work) - work->param->head_pos +1;
}
// head_posの取ることができる全項目の中での番号の最大値(含む、0からこの値まで変更してOK)を得る
static u16 Manual_List_GetHeadPosMax( MANUAL_LIST_WORK* work )
{
  if( work->param->num <= ATM_WIN_ITEM_ENABLE_NUM )
  {
    return 0;
  }
  else
  {
    return (work->param->num - ATM_WIN_ITEM_ENABLE_NUM);
  }
}

// リストが動いたのでつくり直す
static void Manual_List_UpdateAppTaskmenuWin( MANUAL_LIST_WORK* work )
{
  u8  atm_win_start_idx = ATM_WIN_ITEM_UP_HALF;
  u16 item_start_idx    = work->param->head_pos -1;

  u8  atm_win_end_idx;  // 含む
  u16 item_end_idx;     // 含む

//  // まだ書き終わっていないテキストがあるといけないので、クリアしておく
//  PRINTSYS_QUE_Clear( work->cmn_wk->print_que_main );  // 書き終わるまでリストをつくり直さないことにしたので、クリアの必要はない

  // 先頭の調整
  if( work->param->head_pos == 0 )
  {
    if( work->atm_win_wk[ATM_WIN_ITEM_UP_HALF] )
    {
      // UP_HALFを非表示にする
      APP_TASKMENU_WIN_Hide(work->atm_win_wk[ATM_WIN_ITEM_UP_HALF]);
      work->atm_win_is_show[ATM_WIN_ITEM_UP_HALF] = FALSE;
    }
    atm_win_start_idx = ATM_WIN_ITEM_ENABLE_START;
    item_start_idx    = work->param->head_pos;
      
    // NEWアイコン
    GFL_CLACT_WK_SetDrawEnable( work->new_icon_clwk[ATM_WIN_ITEM_UP_HALF], FALSE );
  }

  // 最後尾の調整
  {
    u8 i;

    item_end_idx = Manual_List_GetTailPos(work) +1;
    if( item_end_idx >= work->param->num ) item_end_idx--;

    atm_win_end_idx = atm_win_start_idx + item_end_idx - item_start_idx;

    for( i=atm_win_end_idx +1; i<=ATM_WIN_ITEM_DOWN_HALF; i++ )
    {
      if( work->atm_win_wk[i] )
      {
        // 最後尾のデータより後ろは非表示にする
        APP_TASKMENU_WIN_Hide(work->atm_win_wk[i]);
        work->atm_win_is_show[i] = FALSE;
      }

      // NEWアイコン
      GFL_CLACT_WK_SetDrawEnable( work->new_icon_clwk[i], FALSE );
    }
  }

  // ワーク
  {
    u8  i;
    u16 item_idx = item_start_idx;
    
    APP_TASKMENU_ITEMWORK atm_item_wk;

    for( i=atm_win_start_idx; i<=atm_win_end_idx; i++ )
    {
      if( work->atm_win_wk[i] )
      {
        APP_TASKMENU_WIN_DeleteNotClearScreen( work->atm_win_wk[i] );
      }

      // 窓の設定
      atm_item_wk.str       = GFL_MSG_CreateString( work->cmn_wk->msgdata_main, work->param->item[item_idx].str_id );
      atm_item_wk.msgColor  = (i==ATM_WIN_ITEM_UP_HALF||i==ATM_WIN_ITEM_DOWN_HALF)?(ATM_HALF_ITEM_MSGCOLOR):(APP_TASKMENU_ITEM_MSGCOLOR);
      atm_item_wk.type      = APP_TASKMENU_WIN_TYPE_NORMAL;

      if( work->first_create_done )  // 2回目以降の作成
      {
        work->atm_win_wk[i] = APP_TASKMENU_WIN_CreateExNotTransCharacter(
            work->atm_res,
            &atm_item_wk,
            ATM_PLATE_X, ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*i,
            ATM_PLATE_WIDTH, APP_TASKMENU_PLATE_HEIGHT,
            0, FALSE,
            work->cmn_wk->heap_id );
      }
      else                           // 初回作成
      {
        work->atm_win_wk[i] = APP_TASKMENU_WIN_Create(
            work->atm_res,
            &atm_item_wk,
            ATM_PLATE_X, ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*i,
            ATM_PLATE_WIDTH,
            work->cmn_wk->heap_id );
      }

      APP_TASKMENU_WIN_Update( work->atm_win_wk[i] );  // print_que描画をするためにAPP_TASKMENU_WIN_Create直後に1回APP_TASKMENU_WIN_Updateを呼んでおく
      work->atm_win_is_show[i] = TRUE;

      // 文字列解放
      GFL_STR_DeleteBuffer( atm_item_wk.str );

      // NEWアイコン
      {
        if( work->param->item[item_idx].icon == MANUAL_LIST_ICON_NONE )
        {
          GFL_CLACT_WK_SetDrawEnable( work->new_icon_clwk[i], FALSE );
        }
        else if( work->param->item[item_idx].icon == MANUAL_LIST_ICON_NEW )
        {
          //GFL_CLACT_WK_SetAnmSeq( work->new_icon_clwk[i], NEW_ICON_ANMSEQ );  // 0フレーム目から再生されるように、アニメを設定し直す。
                                                                                // スクロールするだけなので、続きのフレームからでよいからコメントアウト。
          GFL_CLACT_WK_SetDrawEnable( work->new_icon_clwk[i], TRUE );
        }
      }

      item_idx++; 
    }

    work->first_create_done = TRUE;  // 次から2回目以降
  }
}

// タッチしているy座標からhead_posを求める
static u16 Manual_List_GetHeadPosFromY( MANUAL_LIST_WORK* work, u16 num_for_head_pos, u16 y )
{
  // 画面の1番上(UP_HALFは除く)にある項目の番号head_posを返す。
  // num_for_head_posは、画面の1番上(UP_HALFは除く)に来ることができる項目の個数。
  // yはタッチしている座標。

  u16 head_pos;

  if( num_for_head_pos <= 1 )
  {
    head_pos = 0;
  }
  else
  {
    u16 range_min;  // range_min<= <range_max
    u16 range_max;
    u16 one_range;  // 1つのパネル分に相当する区画の幅

    range_min = SB_CURSOR_Y_MIN + SB_CURSOR_Y_OFFSET;
    range_max = SB_CURSOR_Y_MAX + SB_CURSOR_Y_OFFSET +1;

    one_range = ( range_max - range_min ) / num_for_head_pos;

    // yが範囲外にあるとき
    if( y<range_min )
    {
      head_pos = 0;
    }
    else if( range_max<=y )
    {
      head_pos = num_for_head_pos -1;
    }
    // yが範囲内にあるとき
    else
    {
      // yは何番目の区画に入っているか
      for( head_pos=0; head_pos<num_for_head_pos; head_pos++ )
      {
        //u16 curr_min = range_min + one_range * (head_pos +0);  // curr_min<= <curr_max 
        //u16 curr_max = range_min + one_range * (head_pos +1);
        u16 curr_min = range_min + ( range_max - range_min ) * (head_pos +0) / num_for_head_pos;  // 計算の順番を入れ替えただけなので、
        u16 curr_max = range_min + ( range_max - range_min ) * (head_pos +1) / num_for_head_pos;  // 上のコメントアウトと同じ意味です。
      
        if( curr_min<=y && y<curr_max )
        {
          break;
        }
      }
    }
  }

  if( head_pos >= num_for_head_pos ) head_pos = 0;
  return head_pos;
}

// head_posからスクロールバーのカーソルを置くy座標を求める(SB_CURSOR_Y_OFFSETも考慮してあるので、戻り値のyをそのままclwkに使ってやればよい)
static u16 Manual_List_GetSbCursorYFromHeadPos( MANUAL_LIST_WORK* work, u16 num_for_head_pos, u16 head_pos )
{
  u16 y;

  {
    u16 range_min;  // range_min<= <range_max
    u16 range_max;
    u16 one_range;  // 1つのパネル分に相当する区画の幅

    range_min = SB_CURSOR_Y_MIN + SB_CURSOR_Y_OFFSET;
    range_max = SB_CURSOR_Y_MAX + SB_CURSOR_Y_OFFSET +1;

    one_range = ( range_max - range_min ) / num_for_head_pos;

    // 端にあるとき
    if( head_pos == 0 )
    {
      y = range_min - SB_CURSOR_Y_OFFSET;  // 端に置く
    }
    else if( head_pos == num_for_head_pos -1 )
    {
      y = range_max - SB_CURSOR_Y_OFFSET -1;  // 端に置く
    }
    // 内側にあるとき
    else
    {
      //u16 curr_min = range_min + one_range * (head_pos +0);  // curr_min<= <curr_max 
      //u16 curr_max = range_min + one_range * (head_pos +1);
      u16 curr_min = range_min + ( range_max - range_min ) * (head_pos +0) / num_for_head_pos;  // 計算の順番を入れ替えただけなので、
      u16 curr_max = range_min + ( range_max - range_min ) * (head_pos +1) / num_for_head_pos;  // 上のコメントアウトと同じ意味です。

      u16 curr_center = (curr_min + curr_max) /2;
      y = curr_center - SB_CURSOR_Y_OFFSET;  // 中央に置く
    }
  }

  if( y<SB_CURSOR_Y_MIN )      y = SB_CURSOR_Y_MIN;
  else if( y>SB_CURSOR_Y_MAX ) y = SB_CURSOR_Y_MAX;
  return y;
}


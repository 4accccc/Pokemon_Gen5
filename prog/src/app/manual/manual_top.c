//============================================================================
/**
 *  @file   manual_top.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_TOP
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
#include "manual_top.h"

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
  ATM_WIN_ITEM_CATEGORY,
  ATM_WIN_ITEM_ALL,
  ATM_WIN_ITEM_MAX,
}
ATM_WIN_ITEM;

#define ATM_PLATE_WIDTH (30)  // キャラ
#define ATM_PLATE_X     (1)   // キャラ
#define ATM_PLATE_Y     (3)   // キャラ

// タッチの当たり判定
static const GFL_UI_TP_HITTBL atm_tp_hittbl[ATM_WIN_ITEM_MAX +1] =
{
  {
    ( ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*(ATM_WIN_ITEM_CATEGORY+0) ) *8  ,
    ( ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*(ATM_WIN_ITEM_CATEGORY+1) ) *8 -1,
    ( ATM_PLATE_X ) *8,
    ( ATM_PLATE_X + ATM_PLATE_WIDTH ) *8 -1
  },  // rect
  {
    ( ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*(ATM_WIN_ITEM_ALL+0) ) *8  ,
    ( ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*(ATM_WIN_ITEM_ALL+1) ) *8 -1,
    ( ATM_PLATE_X ) *8,
    ( ATM_PLATE_X + ATM_PLATE_WIDTH ) *8 -1
  },  // rect
  { GFL_UI_TP_HIT_END, 0, 0, 0 },  // テーブル終了
};

// 入力状態
typedef enum
{
  INPUT_STATE_NONE,  // 何の入力アニメーションも行っていないので、入力可能。
  INPUT_STATE_END,   // もう何の入力も受け付けない。
  INPUT_STATE_TB,    // タッチバーの入力アニメーションを行っている最中なので、入力不可能。
  INPUT_STATE_ATM,   // APP_TASKMENU_WINの入力アニメーションを行っている最中なので、入力不可能。
}
INPUT_STATE;

// 終了リクエスト
#define END_REQ_COUNT_START_VAL (1)   // カウントダウン開始値
#define END_REQ_COUNT_NO        (-1)  // カウントダウンしない値


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct _MANUAL_TOP_WORK
{
  // パラメータ
  MANUAL_TOP_PARAM*           param;
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // ここで作成
  // APP_TASKMENU_WIN
  APP_TASKMENU_RES*           atm_res;
  APP_TASKMENU_WIN_WORK*      atm_win_wk[ATM_WIN_ITEM_MAX];

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // 入力状態
  INPUT_STATE                 input_state;

  // 終了リクエスト
  s8                          end_req_count;  // 負のときカウントダウンしない。正のときカウントダウンする。0になったら終了。
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Manual_Top_VBlankFunc( GFL_TCB* tcb, void* wk );

// 準備
static void Manual_Top_Prepare( MANUAL_TOP_WORK* work );
static void Manual_Top_PrepareAppTaskmenuWin( MANUAL_TOP_WORK* work );
// 転送(VBlank転送ではない)
static void Manual_Top_Trans( MANUAL_TOP_WORK* work );
// 転送(VBlank転送)
static void Manual_Top_TransVBlank( MANUAL_TOP_WORK* work );
// 後片付け
static void Manual_Top_Finish( MANUAL_TOP_WORK* work );
static void Manual_Top_FinishAppTaskmenuWin( MANUAL_TOP_WORK* work );

// 入力
static BOOL Manual_Top_InputAppTaskmenuWin( MANUAL_TOP_WORK* work );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_TOP_WORK*  MANUAL_TOP_Init(
    MANUAL_TOP_PARAM*    param,
    MANUAL_COMMON_WORK*  cmn_wk
)
{
  // ワーク
  MANUAL_TOP_WORK*  work  = GFL_HEAP_AllocClearMemory( cmn_wk->heap_id, sizeof(MANUAL_TOP_WORK) );

  // パラメータ
  work->param  = param;
  {
    // out
    work->param->result = MANUAL_TOP_RESULT_RETURN;
  }
  // 共通
  work->cmn_wk = cmn_wk;

  // ここで作成
  Manual_Top_Prepare( work );
  //Manual_Top_Trans( work );
  Manual_Top_TransVBlank( work );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_Top_VBlankFunc, work, 1 );

  // 入力状態
  work->input_state = INPUT_STATE_NONE;

  // 終了リクエスト
  work->end_req_count = END_REQ_COUNT_NO;
  
  // マニュアルタッチバー
  MANUAL_TOUCHBAR_SetType( work->cmn_wk->mtb_wk, MANUAL_TOUCHBAR_TYPE_TOP );

  return work;
}

// 終了処理
void  MANUAL_TOP_Exit(
    MANUAL_TOP_WORK*     work
)
{
  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ここで作成
  //Manual_Top_Finish( work );

  // ワーク
  GFL_HEAP_FreeMemory( work );
}

// 主処理
BOOL  MANUAL_TOP_Main(
    MANUAL_TOP_WORK*     work
)
{
  BOOL b_end = FALSE;

  if( work->end_req_count == 0 )
  {
    b_end = TRUE;
  }
  else if( work->end_req_count > 0 )
  {
    if( work->end_req_count == END_REQ_COUNT_START_VAL )
    {
      Manual_Top_Finish( work );
    }
    work->end_req_count--;
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
            work->param->result = MANUAL_TOP_RESULT_RETURN;
          }
          break;
        case TOUCHBAR_ICON_CLOSE:
          {
            work->param->result = MANUAL_TOP_RESULT_CLOSE;
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
    // APP_TASKMENU_WIN
    u8 i;
    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      APP_TASKMENU_WIN_Update( work->atm_win_wk[i] );
    }
      
    if( work->input_state == INPUT_STATE_NONE )
    {
      BOOL b_decide = Manual_Top_InputAppTaskmenuWin( work );
      if( b_decide ) work->input_state = INPUT_STATE_ATM;
    }
    else
    {
      BOOL b_finish = APP_TASKMENU_WIN_IsFinish( work->atm_win_wk[work->param->cursor_pos] );
      if( b_finish )
      {
        if( work->param->cursor_pos == ATM_WIN_ITEM_CATEGORY )
        {
          work->param->result = MANUAL_TOP_RESULT_CATEGORY;
        }
        else if( work->param->cursor_pos == ATM_WIN_ITEM_ALL )
        {
          work->param->result = MANUAL_TOP_RESULT_ALL;
        }
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
static void Manual_Top_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_TOP_WORK* work = (MANUAL_TOP_WORK*)wk;

  // VBlank転送するならここで。
  //Manual_Top_Trans( work );
}

// 準備
static void Manual_Top_Prepare( MANUAL_TOP_WORK* work )
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
      NARC_manual_bg1_NSCR,
      BG_FRAME_S_REAR,
      0,
      0,  // 全転送
      FALSE,
      work->cmn_wk->heap_id );
*/

  GFL_ARCHDL_UTIL_TransVramScreenCharOfsVBlank(
      work->cmn_wk->handle_system,
      NARC_manual_bg1_NSCR,
      BG_FRAME_S_REAR,
      0,
      0,
      0,  // 全転送
      FALSE,
      work->cmn_wk->heap_id );

  // APP_TASKMENU_WIN
  Manual_Top_PrepareAppTaskmenuWin( work );
}

static void Manual_Top_PrepareAppTaskmenuWin( MANUAL_TOP_WORK* work )
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

  // ワーク
  {
    u8 i;

    // 窓の設定
    APP_TASKMENU_ITEMWORK item[ATM_WIN_ITEM_MAX];

    item[ATM_WIN_ITEM_CATEGORY].str       = GFL_MSG_CreateString( work->cmn_wk->msgdata_system, manual_01_01 );
    item[ATM_WIN_ITEM_CATEGORY].msgColor  = APP_TASKMENU_ITEM_MSGCOLOR;
    item[ATM_WIN_ITEM_CATEGORY].type      = APP_TASKMENU_WIN_TYPE_NORMAL;

    item[ATM_WIN_ITEM_ALL].str            = GFL_MSG_CreateString( work->cmn_wk->msgdata_system, manual_01_02 );
    item[ATM_WIN_ITEM_ALL].msgColor       = APP_TASKMENU_ITEM_MSGCOLOR;
    item[ATM_WIN_ITEM_ALL].type           = APP_TASKMENU_WIN_TYPE_NORMAL;

    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      work->atm_win_wk[i] = APP_TASKMENU_WIN_Create(
          work->atm_res,
          &item[i],
          ATM_PLATE_X, ATM_PLATE_Y + APP_TASKMENU_PLATE_HEIGHT*i,
          ATM_PLATE_WIDTH,
          work->cmn_wk->heap_id );

      // 文字列解放
      GFL_STR_DeleteBuffer( item[i].str );
    }
  }

  // カーソルの表示
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], TRUE );
  }
}

// 転送(VBlank転送ではない)
static void Manual_Top_Trans( MANUAL_TOP_WORK* work )
{
  GFL_BG_LoadScreenReq( BG_FRAME_S_REAR );
}

// 転送(VBlank転送)
static void Manual_Top_TransVBlank( MANUAL_TOP_WORK* work )
{
  GFL_BG_LoadScreenV_Req( BG_FRAME_S_REAR );
}

// 後片付け
static void Manual_Top_Finish( MANUAL_TOP_WORK* work )
{
  Manual_Top_FinishAppTaskmenuWin( work );
}

static void Manual_Top_FinishAppTaskmenuWin( MANUAL_TOP_WORK* work )
{
  // APP_TASKMENU_WIN

  // まだ書き終わっていないテキストがあるといけないので、クリアしておく
  PRINTSYS_QUE_Clear( work->cmn_wk->print_que_main );
  {
    u8 i;
    // ワーク
    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      APP_TASKMENU_WIN_Delete( work->atm_win_wk[i] );
    }
    // リソース
    APP_TASKMENU_RES_Delete( work->atm_res );
  }

  GFL_BG_LoadScreenV_Req( BG_FRAME_S_MAIN );
}

// 入力
static BOOL Manual_Top_InputAppTaskmenuWin( MANUAL_TOP_WORK* work )
{
  // 決定したときTRUEを返す
  
  int tp_hit = GFL_UI_TP_HitTrg(atm_tp_hittbl);

  // タッチorキーの切り替えチェック
  // キー入力のとき
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    if( ATM_WIN_ITEM_CATEGORY<=tp_hit && tp_hit<=ATM_WIN_ITEM_ALL )
    {
      // タッチに変更
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      // タッチに変更した場合は、このまま処理を続ける
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], FALSE );
    }
  }
  // タッチ入力のとき
  else
  {
    if( GFL_UI_KEY_GetTrg() )
    {
      if( !( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_B | PAD_BUTTON_X ) ) )  // タッチバーへの入力以外なら
      {
        // キーに変更
        GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
        // キーに変更した場合は、カーソルを表示して、処理を終える
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], TRUE );
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
      APP_TASKMENU_WIN_SetDecide( work->atm_win_wk[work->param->cursor_pos], TRUE );
      PMSND_PlaySE(MANUAL_SND_ATM_DECIDE);
      return TRUE;
    }
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )  // 選択項目の多い画面と操作を合わせておきたいのでRepeatにした
    {
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], FALSE );
      if( work->param->cursor_pos == ATM_WIN_ITEM_CATEGORY )
      {
        work->param->cursor_pos = ATM_WIN_ITEM_ALL;
      }
      else
      {
        work->param->cursor_pos = ATM_WIN_ITEM_CATEGORY;
      }
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], TRUE );
      PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
    }
    else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )  // 選択項目の多い画面と操作を合わせておきたいのでRepeatにした
    {
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], FALSE );
      if( work->param->cursor_pos == ATM_WIN_ITEM_ALL )
      {
        work->param->cursor_pos = ATM_WIN_ITEM_CATEGORY;
      }
      else
      {
        work->param->cursor_pos = ATM_WIN_ITEM_ALL;
      }
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], TRUE );
      PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
    }
    else if( GFL_UI_KEY_GetRepeat() & ( PAD_KEY_LEFT | PAD_BUTTON_L ) )
    {
      if( work->param->cursor_pos == ATM_WIN_ITEM_ALL )
      {
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], FALSE );
        work->param->cursor_pos = ATM_WIN_ITEM_CATEGORY;
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], TRUE );
        PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
      }
    }
    else if( GFL_UI_KEY_GetRepeat() & ( PAD_KEY_RIGHT | PAD_BUTTON_R ) )
    {
      if( work->param->cursor_pos == ATM_WIN_ITEM_CATEGORY )
      {
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], FALSE );
        work->param->cursor_pos = ATM_WIN_ITEM_ALL;
        APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], TRUE );
        PMSND_PlaySE(MANUAL_SND_ATM_MOVE);
      }
    }
  }
  // タッチ入力受け付け
  else
  {
    // タッチのまま変更なしなので、GFL_UI_SetTouchOrKeyは不要。
    if( ATM_WIN_ITEM_CATEGORY<=tp_hit && tp_hit<=ATM_WIN_ITEM_ALL )
    {
      // 決定
      work->param->cursor_pos = tp_hit;
      APP_TASKMENU_WIN_SetActive( work->atm_win_wk[work->param->cursor_pos], TRUE );
      APP_TASKMENU_WIN_SetDecide( work->atm_win_wk[work->param->cursor_pos], TRUE );
      PMSND_PlaySE(MANUAL_SND_ATM_DECIDE);
      return TRUE;
    }
  }

  return FALSE;
}


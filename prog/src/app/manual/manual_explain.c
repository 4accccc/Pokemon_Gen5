//============================================================================
/**
 *  @file   manual_explain.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_EXPLAIN
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

#include "manual_graphic.h"
#include "manual_def.h"
#include "manual_common.h"
#include "manual_touchbar.h"
#include "manual_explain.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_manual.h"
#include "manual.naix"

// ダミー
#include "msg/msg_zkn.h"
#include "zukan_gra.naix"


// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// 入力状態
typedef enum
{
  INPUT_STATE_NONE,  // 何の入力アニメーションも行っていないので、入力可能。
  INPUT_STATE_END,   // もう何の入力も受け付けない。
  INPUT_STATE_TB,    // タッチバーの入力アニメーションを行っている最中なので、入力不可能。
}
INPUT_STATE;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct _MANUAL_EXPLAIN_WORK
{
  // パラメータ
  MANUAL_EXPLAIN_PARAM*       param;
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // ここで作成
  u8                          page_no;  // 0<= <work->param->page_num

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // 入力の状態
  INPUT_STATE                 input_state;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Manual_Explain_VBlankFunc( GFL_TCB* tcb, void* wk );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_EXPLAIN_WORK*  MANUAL_EXPLAIN_Init(
    MANUAL_EXPLAIN_PARAM*   param,
    MANUAL_COMMON_WORK*     cmn_wk
)
{
  // ワーク
  MANUAL_EXPLAIN_WORK*  work  = GFL_HEAP_AllocClearMemory( cmn_wk->heap_id, sizeof(MANUAL_EXPLAIN_WORK) );

  // パラメータ
  work->param  = param;
  {
    // out
    work->param->result   = MANUAL_EXPLAIN_RESULT_RETURN;
  }
  // 共通
  work->cmn_wk = cmn_wk;

  // ここで作成
  {
    work->page_no = 3;//0;
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_Explain_VBlankFunc, work, 1 );

  // 入力状態
  work->input_state = INPUT_STATE_NONE;

  // マニュアルタッチバー
  MANUAL_TOUCHBAR_SetType( work->cmn_wk->mtb_wk, MANUAL_TOUCHBAR_TYPE_EXPLAIN );
  MANUAL_TOUCHBAR_SetPage( work->cmn_wk->mtb_wk, work->param->page_num, work->page_no );

  return work;
}

// 終了処理
void  MANUAL_EXPLAIN_Exit(
    MANUAL_EXPLAIN_WORK*     work
)
{
  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ここで作成
  {
  }

  // ワーク
  GFL_HEAP_FreeMemory( work );
}

// 主処理
BOOL  MANUAL_EXPLAIN_Main(
    MANUAL_EXPLAIN_WORK*     work
)
{
  // マニュアルタッチバー
  MANUAL_TOUCHBAR_Main( work->cmn_wk->mtb_wk );

  work->param->result = MANUAL_EXPLAIN_RESULT_RETURN;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ) return TRUE;
  else return FALSE;
  return TRUE;
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
static void Manual_Explain_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_EXPLAIN_WORK* work = (MANUAL_EXPLAIN_WORK*)wk;
}


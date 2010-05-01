//============================================================================
/**
 *  @file   manual.c
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
#include "manual_top.h"

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
// APP_TASKMENU_WINの項目
typedef enum
{
  ATM_WIN_ITEM_CATEGORY,
  ATM_WIN_ITEM_INITIAL,
  ATM_WIN_ITEM_MAX,
}
ATM_WIN_ITEM;


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
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // ここで作成
  // APP_TASKMENU_WIN
  APP_TASKMENU_RES*           atm_res;
  APP_TASKMENU_WIN_WORK*      atm_win_wk[ATM_WIN_ITEM_MAX];

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;
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


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_TOP_WORK*  MANUAL_TOP_Init(
    MANUAL_COMMON_WORK*  cmn_wk
)
{
  // ワーク
  MANUAL_TOP_WORK*  work  = GFL_HEAP_AllocClearMemory( cmn_wk->heap_id, sizeof(MANUAL_TOP_WORK) );

  // 共通
  work->cmn_wk = cmn_wk;

  // ここで作成
  Manual_Top_Prepare( work );
  Manual_Top_Trans( work );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_Top_VBlankFunc, work, 1 );

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
  // APP_TASKMENU_WIN
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

  // ワーク
  GFL_HEAP_FreeMemory( work );
}

// 主処理
void  MANUAL_TOP_Main(
    MANUAL_TOP_WORK*     work
)
{
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

  GFL_ARCHDL_UTIL_TransVramScreen(
      work->cmn_wk->handle_system,
      NARC_manual_bg1_NSCR,
      BG_FRAME_S_REAR,
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

    item[ATM_WIN_ITEM_INITIAL].str        = GFL_MSG_CreateString( work->cmn_wk->msgdata_system, manual_01_02 );
    item[ATM_WIN_ITEM_INITIAL].msgColor   = APP_TASKMENU_ITEM_MSGCOLOR;
    item[ATM_WIN_ITEM_INITIAL].type       = APP_TASKMENU_WIN_TYPE_NORMAL;

    for( i=0; i<ATM_WIN_ITEM_MAX; i++ )
    {
      work->atm_win_wk[i] = APP_TASKMENU_WIN_Create(
          work->atm_res,
          &item[i],
          1, 1 + 3*i,
          30,
          work->cmn_wk->heap_id );

      // 文字列解放
      GFL_STR_DeleteBuffer( item[i].str );
    }
  }
}

// 転送(VBlank転送ではない)
static void Manual_Top_Trans( MANUAL_TOP_WORK* work )
{
  GFL_BG_LoadScreenReq( BG_FRAME_S_REAR );
}


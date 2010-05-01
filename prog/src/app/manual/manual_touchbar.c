//============================================================================
/**
 *  @file   manual.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_TOUCHBAR
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
FS_EXTERN_OVERLAY(ui_common);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct _MANUAL_TOUCHBAR_WORK
{
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // ここで作成
  TOUCHBAR_WORK*              tb;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Manual_Touchbar_VBlankFunc( GFL_TCB* tcb, void* wk );

// 準備
static void Manual_Touchbar_Prepare( MANUAL_TOUCHBAR_WORK* work );
// 転送(VBlank転送ではない)
static void Manual_Touchbar_Trans( MANUAL_TOUCHBAR_WORK* work );
static void Manual_Touchbar_TransVBlank( MANUAL_TOUCHBAR_WORK* work );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
// 初期化処理
MANUAL_TOUCHBAR_WORK*  MANUAL_TOUCHBAR_Init(
    MANUAL_COMMON_WORK*  cmn_wk
)
{
  // ワーク
  MANUAL_TOUCHBAR_WORK*  work;

  // オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

  // ワーク
  work  = GFL_HEAP_AllocClearMemory( cmn_wk->heap_id, sizeof(MANUAL_TOUCHBAR_WORK) );

  // 共通
  work->cmn_wk = cmn_wk;

  // ここで作成
  Manual_Touchbar_Prepare( work );
  Manual_Touchbar_Trans( work );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_Touchbar_VBlankFunc, work, 1 );

  return work;
}

// 終了処理
void  MANUAL_TOUCHBAR_Exit(
    MANUAL_TOUCHBAR_WORK*     work
)
{
  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ここで作成
  // タッチバー破棄
  TOUCHBAR_Exit( work->tb );

  // ワーク
  GFL_HEAP_FreeMemory( work );

  // オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );
}

// 主処理
void  MANUAL_TOUCHBAR_Main(
    MANUAL_TOUCHBAR_WORK*     work
)
{
  // タッチバー
  TOUCHBAR_Main( work->tb );
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
static void Manual_Touchbar_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_TOUCHBAR_WORK* work = (MANUAL_TOUCHBAR_WORK*)wk;

  // VBlank転送するならここで。
  //Manual_Touchbar_Trans( work );
}

// 準備
static void Manual_Touchbar_Prepare( MANUAL_TOUCHBAR_WORK* work )
{
  // タッチバーの設定
  {
    // アイコンの設定
    // 数分作る
    TOUCHBAR_SETUP      tb_setup;

    TOUCHBAR_ITEM_ICON  tb_item_icon_tbl[] =
    {
      {
        TOUCHBAR_ICON_RETURN,
        { TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
      },
      {
        TOUCHBAR_ICON_CLOSE,
        { TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y },
      },
      {
        TOUCHBAR_ICON_CUR_R,
        { TOUCHBAR_ICON_X_02, TOUCHBAR_ICON_Y },
      },
      {
        TOUCHBAR_ICON_CUR_L,
        { TOUCHBAR_ICON_X_00, TOUCHBAR_ICON_Y },
      },
    };

    // 設定構造体
    // さきほどの窓情報＋リソース情報をいれる
    GFL_STD_MemClear( &tb_setup, sizeof(TOUCHBAR_SETUP) );
    tb_setup.p_item     = tb_item_icon_tbl;            //上の窓情報
    tb_setup.item_num   = NELEMS(tb_item_icon_tbl);    //いくつ窓があるか
    tb_setup.p_unit     = MANUAL_GRAPHIC_GetClunit( work->cmn_wk->graphic );  //OBJ読み込みのためのCLUNIT
    tb_setup.bar_frm    = BG_FRAME_S_TB_BAR;           //BG読み込みのためのBG面
    tb_setup.bg_plt     = BG_PAL_POS_S_TB_BAR;         //BGﾊﾟﾚｯﾄ
    tb_setup.obj_plt    = OBJ_PAL_POS_S_TB_ICON;       //OBJﾊﾟﾚｯﾄ
    tb_setup.mapping    = APP_COMMON_MAPPING_32K;      //マッピングモード

    work->tb = TOUCHBAR_Init( &tb_setup, work->cmn_wk->heap_id );
  }
}

// 転送(VBlank転送ではない)
static void Manual_Touchbar_Trans( MANUAL_TOUCHBAR_WORK* work )
{
}


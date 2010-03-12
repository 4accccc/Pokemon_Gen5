/*
 *  @file   demo3d_exp_c_cruiser.c
 *  @brief  クルーザーデモ　例外処理
 *  @author hosaka,iwasawa
 *  @date   10.02.25
 */
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//アプリ共通素材
#include "app/app_menu_common.h"

// 文字列関連
#include "font/font.naix"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" //PRINT_QUE

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//タッチバー
#include "ui/touchbar.h"

//INFOWIN
#include "infowin/infowin.h"

#include "field/field_light_status.h"

//描画設定
#include "demo3d_graphic.h"

//アーカイブ
#include "arc_def.h"

#include "message.naix"
#include "townmap_gra.naix"

#include "demo3d_local.h"
#include "demo3d_graphic.h"
#include "demo3d_engine.h"
#include "demo3d_exp.h"

//-----------------------------------------------------------
// 遊覧船下画面
//-----------------------------------------------------------
#define C_CRUISER_POS_FLASH_SYNC (100) ///< 更新間隔

//--------------------------------------------------------------
///	座標テーブル
//==============================================================
static const GFL_CLACTPOS g_c_cruiser_postbl[] = 
{
  { 13*8,  17*8, },		//0F
  { 13*8,  17*8-4, },
  { 13*8,  16*8, },
  { 13*8,  16*8-4, },
  { 13*8,  15*8, },
  { 13*8,  15*8-4, },
  { 13*8,  14*8, },
  { 13*8,  14*8-4, },
  { 13*8,  13*8, },
  { 13*8,  13*8-4, },
  { 13*8,  12*8, },		//1000F	スカイアローブリッジ
  { 13*8,  12*8-4, },
  { 13*8,  11*8, },
  { 13*8,  11*8-4, },
  { 13*8,  10*8, },
  { 13*8,  10*8-4, },
  { 13*8,  9*8, },
  { 13*8,  9*8-4, },
  { 13*8,  8*8, },
  { 13*8,  8*8-4, },
  { 13*8,  7*8, },		//2000F
  { 13*8,  7*8-4, },
  { 13*8,  6*8, },
  { 13*8,  6*8-4, },
  { 13*8,  5*8, },		//2400F	スターライン
  { 14*8,  5*8-4, },
  { 15*8,  5*8-4, },
  { 16*8,  5*8, },		//2700F	折り返し地点
  { 16*8,  6*8-4, },
  { 16*8,  6*8, },
  { 16*8,  7*8-4, },		//3000F
  { 16*8,  7*8, },
  { 16*8,  8*8-4, },
  { 16*8,  8*8, },
  { 16*8,  9*8-4, },
  { 16*8,  9*8, },
  { 16*8,  10*8-4, },
  { 16*8,  10*8, },
  { 16*8,  11*8-4, },
  { 16*8,  11*8, },
  { 16*8,  12*8-4, },		//4000F	スカイアローブリッジ
  { 16*8,  12*8, },
  { 16*8,  13*8-4, },
  { 16*8,  13*8, },
  { 16*8,  14*8-4, },
  { 16*8-4,  14*8, },
  { 15*8,  15*8-4, },
  { 15*8-4,  15*8, },
  { 14*8,  16*8-4, },
  { 14*8-4,  16*8, },
  { 13*8,  17*8-4, },		//5000F
  { 13*8-4,  17*8, },
  { 12*8,  17*8, },
  { 12*8-4,  17*8, },
};

//--------------------------------------------------------------
///	遊覧船用ワーク(例外処理のユーザーワーク)
//==============================================================
typedef struct {
  u8 pos_id;
  u8 padding[3];
  UI_EASY_CLWK_RES clwk_res;
  GFL_CLWK* clwk_marker;
} EXP_C_CRUISER_WORK;

static void EXP_C_CRUISER_Init( APP_EXCEPTION_WORK* wk );
static void EXP_C_CRUISER_Main( APP_EXCEPTION_WORK* wk );
static void EXP_C_CRUISER_End( APP_EXCEPTION_WORK* wk );

const APP_EXCEPTION_FUNC_SET DATA_c_exp_funcset_c_cruiser = 
{
  EXP_C_CRUISER_Init,
  EXP_C_CRUISER_Main,
  EXP_C_CRUISER_End,
};

//-----------------------------------------------------------------------------
// 遊覧船 下画面
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
 *  @brief  BG管理モジュール リソース読み込み
 *
 *  @param  DEMO3D_BG_WORK* wk BG管理ワーク
 *  @param  heapID  ヒープID 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void _C_CRUISER_LoadBG( HEAPID heapID )
{
  ARCHANDLE *handle;

  handle  = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );

  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_townmap_gra_tmap_ship_NCLR,
      PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0, heapID );
  
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_townmap_gra_tmap_ship_NCGR,
            BG_FRAME_BACK_S, 0, 0, 0, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, NARC_townmap_gra_tmap_ship_NSCR,
            BG_FRAME_BACK_S, 0, 0, 0, heapID ); 

  GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  例外処理 遊覧船 初期化
 *
 *  @param  APP_EXCEPTION_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void EXP_C_CRUISER_Init( APP_EXCEPTION_WORK* wk )
{
  EXP_C_CRUISER_WORK* uwk;

  wk->userwork = GFL_HEAP_AllocClearMemory( wk->heapID, sizeof(EXP_C_CRUISER_WORK) );
  
  uwk = wk->userwork;

  _C_CRUISER_LoadBG( wk->heapID );

  {
    GFL_CLUNIT* clunit;
    UI_EASY_CLWK_RES_PARAM prm;

    clunit  = DEMO3D_GRAPHIC_GetClunit( wk->graphic );

    prm.draw_type = CLSYS_DRAW_SUB;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = ARCID_TOWNMAP_GRAPHIC;
    prm.pltt_id   = NARC_townmap_gra_tmap_ship_obj_NCLR;
    prm.ncg_id    = NARC_townmap_gra_tmap_ship_obj_NCGR;
    prm.cell_id   = NARC_townmap_gra_tmap_ship_obj_NCER;
    prm.anm_id    = NARC_townmap_gra_tmap_ship_obj_NANR;
    prm.pltt_line = PLTID_OBJ_COMMON_S;
    prm.pltt_src_ofs = 0;
    prm.pltt_src_num = 1;

    UI_EASY_CLWK_LoadResource( &uwk->clwk_res, &prm, clunit, wk->heapID );

    uwk->clwk_marker = UI_EASY_CLWK_CreateCLWK( &uwk->clwk_res, clunit, 40, 40, 0, wk->heapID );
      
    GFL_CLACT_WK_SetDrawEnable( uwk->clwk_marker, TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( uwk->clwk_marker, TRUE );

  }
  
  GFL_BG_SetVisible( BG_FRAME_BACK_S, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  例外処理 遊覧船 主処理
 *
 *  @param  APP_EXCEPTION_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void EXP_C_CRUISER_Main( APP_EXCEPTION_WORK* wk )
{
  int frame;
  EXP_C_CRUISER_WORK* uwk = wk->userwork;
  
  frame = DEMO3D_ENGINE_GetNowFrame( wk->engine ) >> FX32_SHIFT;
  if( uwk->pos_id < NELEMS(g_c_cruiser_postbl) )

  {
    if( uwk->pos_id * C_CRUISER_POS_FLASH_SYNC <= frame )
    {
      // 座標更新
      GFL_CLACT_WK_SetPos( uwk->clwk_marker, &g_c_cruiser_postbl[ uwk->pos_id ], CLSYS_DRAW_SUB );
      GFL_CLACT_WK_SetAnmFrame( uwk->clwk_marker, 0 );

      OS_TPrintf("frame=%d marker pos_id=%d x=%d, y=%d\n", 
          frame, uwk->pos_id, 
          g_c_cruiser_postbl[uwk->pos_id].x,
          g_c_cruiser_postbl[uwk->pos_id].y );

      uwk->pos_id++;

      // 終了判定
      if( uwk->pos_id == NELEMS(g_c_cruiser_postbl) )
      {
        // 非表示に
        GFL_CLACT_WK_SetDrawEnable( uwk->clwk_marker, FALSE );
        OS_TPrintf("marker visible off \n");
      }
    }
  }

}

//-----------------------------------------------------------------------------
/**
 *  @brief  例外処理 遊覧船 解放処理
 *
 *  @param  APP_EXCEPTION_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void EXP_C_CRUISER_End( APP_EXCEPTION_WORK* wk )
{
  EXP_C_CRUISER_WORK* uwk = wk->userwork;

  // OBJリソース開放
  UI_EASY_CLWK_UnLoadResource( &uwk->clwk_res );

  // ユーザーワーク解放
  GFL_HEAP_FreeMemory( wk->userwork );
}


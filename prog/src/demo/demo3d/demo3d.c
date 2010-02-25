//=============================================================================
/**
 *
 *  @file   demo3d.c
 *  @brief  3Dデモ再生アプリ
 *  @author genya hosaka
 *  @data   2009.11.27
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//タスクメニュー
#include "app/app_taskmenu.h"

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

//描画設定
#include "demo3d_graphic.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "demo/demo3d.h"

#include "message.naix"
#include "townmap_gra.naix"

#include "demo3d_engine.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
FS_EXTERN_OVERLAY(ui_common);

#define CHECK_KEY_TRG( key ) ( ( GFL_UI_KEY_GetTrg() & (key) ) == (key) )

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================
enum
{ 
  DEMO3D_HEAP_SIZE = 0x110000,  ///< ヒープサイズ
};

//-------------------------------------
/// フレーム
//=====================================
enum
{ 
  BG_FRAME_BAR_M  = GFL_BG_FRAME1_M,
  BG_FRAME_POKE_M = GFL_BG_FRAME2_M,
  BG_FRAME_BACK_M = GFL_BG_FRAME3_M,
  BG_FRAME_BACK_S = GFL_BG_FRAME2_S,
  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 
};
//-------------------------------------
/// パレット
//=====================================
enum
{ 
  //メインBG
  PLTID_BG_BACK_M       = 0,
  PLTID_BG_POKE_M       = 1,
  PLTID_BG_TASKMENU_M   = 11,
  PLTID_BG_TOUCHBAR_M   = 13,
  PLTID_BG_INFOWIN_M    = 15,
  //サブBG
  PLTID_BG_BACK_S       = 0,

  //メインOBJ
  PLTID_OBJ_TOUCHBAR_M  = 0, // 3本使用
  PLTID_OBJ_TOWNMAP_M = 14,   

  //サブOBJ
  PLTID_OBJ_COMMON_S = 0,
};

//=============================================================================
/**
 *                構造体定義
 */
//=============================================================================

// 不完全型
typedef struct _APP_EXCEPTION_WORK APP_EXCEPTION_WORK;

//--------------------------------------------------------------
/// 例外処理 関数定義
//==============================================================
typedef void (*APP_EXCEPTION_FUNC)( APP_EXCEPTION_WORK* wk );

//--------------------------------------------------------------
/// 例外処理 関数テーブル
//==============================================================
typedef struct {
  APP_EXCEPTION_FUNC Init;
  APP_EXCEPTION_FUNC Main;
  APP_EXCEPTION_FUNC End;
} APP_EXCEPTION_FUNC_SET;

//--------------------------------------------------------------
/// アプリ例外処理
//==============================================================
struct _APP_EXCEPTION_WORK {
  //[IN]
  HEAPID heapID;
  const DEMO3D_GRAPHIC_WORK* graphic;
  const DEMO3D_ENGINE_WORK* engine;

  //[PRIVATE]
  const APP_EXCEPTION_FUNC_SET* p_funcset;
  void *userwork;
  int  key_skip;  // キースキップ

  
};

//--------------------------------------------------------------
/// BG管理ワーク
//==============================================================
typedef struct 
{
  int dummy;
} DEMO3D_BG_WORK;

//--------------------------------------------------------------
/// メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

  DEMO3D_PARAM*         param;

  DEMO3D_BG_WORK        wk_bg;

  //描画設定
  DEMO3D_GRAPHIC_WORK *graphic;

  //フォント
  GFL_FONT                  *font;

  //プリントキュー
  PRINT_QUE                 *print_que;
  GFL_MSGDATA               *msg;

  // デモエンジン
  DEMO3D_ENGINE_WORK*   engine;

  // アプリ例外処理エンジン
  APP_EXCEPTION_WORK*   expection;
  
} DEMO3D_MAIN_WORK;


//=============================================================================
/**
 *              データ定義
 */
//=============================================================================


//=============================================================================
/**
 *              プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
/// PROC
//=====================================
static GFL_PROC_RESULT Demo3DProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT Demo3DProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT Demo3DProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-----------------------------------------------------------
// アプリ例外処理
//-----------------------------------------------------------
static APP_EXCEPTION_WORK* APP_EXCEPTION_Create( DEMO3D_ID demo_id, DEMO3D_GRAPHIC_WORK* graphic, DEMO3D_ENGINE_WORK* engine, HEAPID heapID );
static void APP_EXCEPTION_Delete( APP_EXCEPTION_WORK* wk );
static void APP_EXCEPTION_Main( APP_EXCEPTION_WORK* wk );
static int _key_check( DEMO3D_MAIN_WORK *wk );

//-----------------------------------------------------------
// 遊覧船下画面
//-----------------------------------------------------------
#define C_CRUISER_POS_FLASH_SYNC (100) ///< 更新間隔

//--------------------------------------------------------------
/// 座標テーブル
//==============================================================
static const GFL_CLACTPOS g_c_cruiser_postbl[] = 
{
  { 13*8,  17*8, },   //0F
  { 13*8,  17*8-4, },
  { 13*8,  16*8, },
  { 13*8,  16*8-4, },
  { 13*8,  15*8, },
  { 13*8,  15*8-4, },
  { 13*8,  14*8, },
  { 13*8,  14*8-4, },
  { 13*8,  13*8, },
  { 13*8,  13*8-4, },
  { 13*8,  12*8, },   //1000F スカイアローブリッジ
  { 13*8,  12*8-4, },
  { 13*8,  11*8, },
  { 13*8,  11*8-4, },
  { 13*8,  10*8, },
  { 13*8,  10*8-4, },
  { 13*8,  9*8, },
  { 13*8,  9*8-4, },
  { 13*8,  8*8, },
  { 13*8,  8*8-4, },
  { 13*8,  7*8, },    //2000F
  { 13*8,  7*8-4, },
  { 13*8,  6*8, },
  { 13*8,  6*8-4, },
  { 13*8,  5*8, },    //2400F スターライン
  { 14*8,  5*8-4, },
  { 15*8,  5*8-4, },
  { 16*8,  5*8, },    //2700F 折り返し地点
  { 16*8,  6*8-4, },
  { 16*8,  6*8, },
  { 16*8,  7*8-4, },    //3000F
  { 16*8,  7*8, },
  { 16*8,  8*8-4, },
  { 16*8,  8*8, },
  { 16*8,  9*8-4, },
  { 16*8,  9*8, },
  { 16*8,  10*8-4, },
  { 16*8,  10*8, },
  { 16*8,  11*8-4, },
  { 16*8,  11*8, },
  { 16*8,  12*8-4, },   //4000F スカイアローブリッジ
  { 16*8,  12*8, },
  { 16*8,  13*8-4, },
  { 16*8,  13*8, },
  { 16*8,  14*8-4, },
  { 16*8-4,  14*8, },
  { 15*8,  15*8-4, },
  { 15*8-4,  15*8, },
  { 14*8,  16*8-4, },
  { 14*8-4,  16*8, },
  { 13*8,  17*8-4, },   //5000F
  { 13*8-4,  17*8, },
  { 12*8,  17*8, },
  { 12*8-4,  17*8, },
};

//--------------------------------------------------------------
/// 遊覧船用ワーク(例外処理のユーザーワーク)
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

static const APP_EXCEPTION_FUNC_SET c_exp_funcset_c_cruiser = 
{
  EXP_C_CRUISER_Init,
  EXP_C_CRUISER_Main,
  EXP_C_CRUISER_End,
};

//=============================================================================
/**
 *                外部公開
 */
//=============================================================================
const GFL_PROC_DATA Demo3DProcData = 
{
  Demo3DProc_Init,
  Demo3DProc_Main,
  Demo3DProc_Exit,
};
//=============================================================================
/**
 *                PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 初期化処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          DEMO3D_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Demo3DProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  DEMO3D_MAIN_WORK *wk;
  DEMO3D_PARAM *param;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );
  
  //引数取得
  param = pwk;

  GF_ASSERT( param->demo_id != DEMO3D_ID_NULL && param->demo_id < DEMO3D_ID_MAX );

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DEMO3D, DEMO3D_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(DEMO3D_MAIN_WORK), HEAPID_DEMO3D );
  GFL_STD_MemClear( wk, sizeof(DEMO3D_MAIN_WORK) );

  // 初期化
  wk->heapID      = HEAPID_DEMO3D;
  wk->param       = param;
  
  //描画設定初期化
  wk->graphic = DEMO3D_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, param->demo_id, wk->heapID );

  //フォント作成
  wk->font      = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                        GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  //メッセージ
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
      NARC_message_mictest_dat, wk->heapID );

  //PRINT_QUE作成
  wk->print_que   = PRINTSYS_QUE_Create( wk->heapID );

  //3D 初期化
  wk->engine = Demo3D_ENGINE_Init( wk->graphic, param->demo_id, param->start_frame, wk->heapID );

  // BG/OBJを非表示にしておく
  GFL_BG_SetVisible( BG_FRAME_BACK_S, VISIBLE_OFF );
  GFL_BG_SetVisible( BG_FRAME_TEXT_S, VISIBLE_OFF );
  GFL_BG_SetVisible( BG_FRAME_BACK_M, VISIBLE_OFF );
  GFL_BG_SetVisible( BG_FRAME_POKE_M, VISIBLE_OFF );
  GFL_BG_SetVisible( BG_FRAME_BAR_M,  VISIBLE_OFF );
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF );
  
  // フェードイン リクエスト
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );

  // デモ毎の例外処理エンジン初期化
  wk->expection = APP_EXCEPTION_Create( param->demo_id, wk->graphic, wk->engine, wk->heapID );


  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 終了処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          DEMO3D_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Demo3DProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  DEMO3D_MAIN_WORK* wk = mywk;
  
  if( *seq == 0 )
  {
    // フェードアウト リクエスト
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 2 );
    (*seq)++;
    return GFL_PROC_RES_CONTINUE;
  }
  else if( GFL_FADE_CheckFade() == TRUE )
  {
    // フェード中は処理に入らない
    return GFL_PROC_RES_CONTINUE;
  }

  // 例外処理エンジン 終了処理
  APP_EXCEPTION_Delete( wk->expection );

  //メッセージ破棄
  GFL_MSG_Delete( wk->msg );

  //PRINT_QUE
  PRINTSYS_QUE_Delete( wk->print_que );

  //FONT
  GFL_FONT_Delete( wk->font );
  
  //3D 破棄
  Demo3D_ENGINE_Exit( wk->engine );

  //描画設定破棄
  DEMO3D_GRAPHIC_Exit( wk->graphic );

  //PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heapID );

  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 主処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          DEMO3D_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Demo3DProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  DEMO3D_MAIN_WORK* wk = mywk;
  BOOL is_end;

  // フェード中は処理しない
  if( GFL_FADE_CheckFade() == TRUE )
  {
    return GFL_PROC_RES_CONTINUE;
  }

  // 例外処理エンジン 主処理
  APP_EXCEPTION_Main( wk->expection );

  //PRINT_QUE
  PRINTSYS_QUE_Main( wk->print_que );

  //2D描画
  DEMO3D_GRAPHIC_2D_Draw( wk->graphic );

  //3D描画
  is_end = Demo3D_ENGINE_Main( wk->engine );

  // ループ検出かキー終了有効の時にはキーでも終了
  if( is_end || _key_check(wk))
  {
    // [OUT] フレーム値を設定
    wk->param->end_frame  = DEMO3D_ENGINE_GetNowFrame( wk->engine ) >> FX32_SHIFT; 
    wk->param->result     = DEMO3D_RESULT_FINISH;
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}


//=============================================================================
/**
 *                static関数
 */
//=============================================================================


//----------------------------------------------------------------------------------
/**
 * @brief キーチェック（key_skipがTRUE時だけデモ終了できる)
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _key_check( DEMO3D_MAIN_WORK *wk )
{
  if( wk->expection->key_skip )
  {
    if( GFL_UI_KEY_GetTrg() & 
          PAD_BUTTON_DECIDE 
        | PAD_BUTTON_CANCEL 
        | PAD_BUTTON_X 
        | PAD_BUTTON_Y
        | PAD_KEY_UP 
        | PAD_KEY_DOWN 
        | PAD_KEY_LEFT 
        | PAD_KEY_RIGHT 
    )
    {
      return TRUE;
    }
        
  }
  
  return FALSE;
}

//-----------------------------------------------------------------------------
// アプリごとの例外表示
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *  @brief  アプリ例外処理エンジン 生成
 *
 *  @param  DEMO3D_ID demo_id
 *  @param  heapID 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static APP_EXCEPTION_WORK* APP_EXCEPTION_Create( DEMO3D_ID demo_id, DEMO3D_GRAPHIC_WORK* graphic, DEMO3D_ENGINE_WORK* engine, HEAPID heapID )
{
  APP_EXCEPTION_WORK* wk;

  GF_ASSERT( graphic );
  GF_ASSERT( engine );
  
  // メインワーク アロケート
  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(APP_EXCEPTION_WORK) );

  // メンバ初期化
  wk->graphic = graphic;
  wk->engine = engine;
  wk->heapID = heapID;

  wk->p_funcset = NULL;
  wk->key_skip  = FALSE;

  // 遊覧船
  switch( demo_id )
  {
  case DEMO3D_ID_C_CRUISER :
    wk->p_funcset = &c_exp_funcset_c_cruiser; // 下画面登録
    wk->key_skip  = TRUE;                     // キースキップ有効
    break;
  default : 
    ;
  }

  // 初期化処理
  if( wk->p_funcset )
  {
    wk->p_funcset->Init( wk );
  }

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アプリ例外処理エンジン 削除
 *
 *  @param  APP_EXCEPTION_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void APP_EXCEPTION_Delete( APP_EXCEPTION_WORK* wk )
{
  GF_ASSERT(wk);

  // 終了処理
  if( wk->p_funcset )
  {
    wk->p_funcset->End( wk );
  }

  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アプリ例外処理エンジン 主処理
 *
 *  @param  APP_EXCEPTION_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void APP_EXCEPTION_Main( APP_EXCEPTION_WORK* wk )
{
  GF_ASSERT(wk);

  // 主処理
  if( wk->p_funcset )
  {
    wk->p_funcset->Main( wk );
  }
}




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


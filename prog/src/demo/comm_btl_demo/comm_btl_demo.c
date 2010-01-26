//=============================================================================
/**
 *
 *	@file		comm_btl_demo.c
 *	@brief  通信バトルデモ
 *	@author	hosaka genya
 *	@data		2009.09.29
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "poke_tool/pokeparty.h" // for POKEPARTY

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//シーンコントローラー
#include "ui/ui_scene.h"

//描画設定
#include "comm_btl_demo_graphic.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "demo/comm_btl_demo.h"

#include "message.naix"
#include "comm_btl_demo.naix"	// アーカイブ

#include "comm_btl_demo_res.cdat"

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  //@TODO デカめにとってある
  COMM_BTL_DEMO_HEAP_SIZE = 0x70000,  ///< ヒープサイズ

  TRAINER_CNT_NORMAL = 2,
  TRAINER_CNT_MULTI = 4,
};

enum
{ 
  OBJ_ANM_ID_BALL_NORMAL = 10,  ///< ボール：通常
  OBJ_ANM_ID_BALL_SICK = 11,    ///< ボール：状態異常
  OBJ_ANM_ID_BALL_DOWN = 12,    ///< ボール：瀕死
  OBJ_ANM_ID_BALL_NULL = 13,    ///< ボール：空
};

//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BAR_M	= GFL_BG_FRAME1_M,
	BG_FRAME_POKE_M	= GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M	= GFL_BG_FRAME3_M,
	BG_FRAME_BACK_S	= GFL_BG_FRAME2_S,
  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 
};

//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_BACK_M				= 0,
	PLTID_BG_POKE_M				= 1,
	PLTID_BG_TASKMENU_M		= 11,
	PLTID_BG_TOUCHBAR_M		= 13,
	PLTID_BG_INFOWIN_M		= 15,
	//サブBG
	PLTID_BG_BACK_S				=	0,

  // Main OBJ
  PLTID_OBJ_COMMON_M = 0,
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct 
{
	int dummy;
} COMM_BTL_DEMO_BG_WORK;

//--------------------------------------------------------------
///	OBJワーク
//==============================================================
typedef struct {
  //[IN]
  COMM_BTL_DEMO_GRAPHIC_WORK* graphic;
  HEAPID  heapID;
  //[PRIVATE]
  UI_EASY_CLWK_RES clres_common;
} COMM_BTL_DEMO_OBJ_WORK;

//--------------------------------------------------------------
///	G3D管理ワーク
//==============================================================
typedef struct {
  //[IN]
  COMM_BTL_DEMO_GRAPHIC_WORK* graphic;
  HEAPID  heapID;
  //[PRIVATE]
  GFL_G3D_UTIL*   g3d_util;
  u16 anm_unit_idx;   ///< アニメーションさせるUNITのIDX
  u8 padding[2];
  
  GFL_PTC_PTR     ptc;
  u8              spa_work[ PARTICLE_LIB_HEAP_SIZE ];
  u8  spa_num;

} COMM_BTL_DEMO_G3D_WORK;

//--------------------------------------------------------------
///	ボールUNITワーク
//==============================================================
typedef struct {
  // [IN]
  u8 num;
  u8 posid;
  u8 type;
  u8 padding[1];
  //[PRIVATE]
  GFL_CLWK* clwk[6];
  u32 timer;
} BALL_UNIT;

//--------------------------------------------------------------
///	トレーナーユニット
//==============================================================
typedef struct {
  // [IN]
  const COMM_BTL_DEMO_TRAINER_DATA* data;
  COMM_BTL_DEMO_OBJ_WORK* obj;
  // [PRIVATE]
  // @TODO BG名前表示
  BALL_UNIT ball; // ボール管理ワーク
} TRAINER_UNIT;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;
  COMM_BTL_DEMO_PARAM * pwk;

	COMM_BTL_DEMO_BG_WORK		wk_bg;
  COMM_BTL_DEMO_OBJ_WORK  wk_obj;
  COMM_BTL_DEMO_G3D_WORK  wk_g3d;

  // トレーナーユニット
  TRAINER_UNIT  trainer_unit[ COMM_BTL_DEMO_TRDATA_MAX ];

	//描画設定
	COMM_BTL_DEMO_GRAPHIC_WORK	*graphic;

	//フォント
	GFL_FONT						*font;

	//プリントキュー
	PRINT_QUE						*print_que;
	GFL_MSGDATA					*msg;
  
  // シーンコントローラ
  UI_SCENE_CNT_PTR  cntScene;

  int timer;  ///< デモ起動時間カウンタ

} COMM_BTL_DEMO_MAIN_WORK;


//=============================================================================
/**
 *							シーン定義
 */
//=============================================================================

// ノーマル開始
static BOOL SceneNormalStart_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneNormalStart_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneNormalStart_End( UI_SCENE_CNT_PTR cnt, void* work );

// ノーマル終了
static BOOL SceneNormalEnd_Main( UI_SCENE_CNT_PTR cnt, void* work );

// マルチ開始
static BOOL SceneMultiStart_Main( UI_SCENE_CNT_PTR cnt, void* work );

// マルチ終了
static BOOL SceneMultiEnd_Main( UI_SCENE_CNT_PTR cnt, void* work );


//--------------------------------------------------------------
///	SceneID
//==============================================================
typedef enum
{ 
  CBD_SCENE_ID_NORMAL_START = 0,  ///< ノーマル開始
  CBD_SCENE_ID_NORMAL_END,        ///< ノーマル終了
  CBD_SCENE_ID_MULTI_START,       ///< マルチ開始
  CBD_SCENE_ID_MULTI_END,         ///< マルチ終了

  CBD_SCENE_ID_MAX,
} CBD_SCENE_ID;

//--------------------------------------------------------------
///	SceneFunc
//==============================================================
static const UI_SCENE_FUNC_SET c_scene_func_tbl[ CBD_SCENE_ID_MAX ] = 
{
  // CBD_SCENE_ID_NORMAL_START
  {
    SceneNormalStart_Init,
    NULL,
    SceneNormalStart_Main,
    NULL,
    SceneNormalStart_End,
  },
  // CBD_SCENE_ID_NORMAL_END
  {
    NULL,
    NULL,
    SceneNormalEnd_Main,
    NULL,
    NULL,
  },
  // CBD_SCENE_ID_MULTI_START
  {
    NULL,
    NULL,
    SceneMultiStart_Main,
    NULL,
    NULL,
  },
  // CBD_SCENE_ID_MULTI_END
  {
    NULL,
    NULL,
    SceneMultiEnd_Main,
    NULL,
    NULL,
  },
};


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static CBD_SCENE_ID calc_first_scene( COMM_BTL_DEMO_PARAM* pwk );
BOOL type_is_normal( u8 type );
BOOL type_is_start( u8 type );

static void BALL_UNIT_Init( BALL_UNIT* unit, const POKEPARTY* party, u8 type, u8 posid, COMM_BTL_DEMO_OBJ_WORK* obj );
static void BALL_UNIT_Exit( BALL_UNIT* unit );
static void BALL_UNIT_SetStart( BALL_UNIT* unit );
static void BALL_UNIT_Main( BALL_UNIT* unit );

static void TRAINER_UNIT_Init( TRAINER_UNIT* unit, u8 type, u8 posid, const COMM_BTL_DEMO_TRAINER_DATA* data, COMM_BTL_DEMO_OBJ_WORK* obj );
static void TRAINER_UNIT_Main( TRAINER_UNIT* unit );
static void TRAINER_UNIT_Exit( TRAINER_UNIT* unit );

static void OBJ_Init( COMM_BTL_DEMO_OBJ_WORK* wk, COMM_BTL_DEMO_GRAPHIC_WORK* graphic, HEAPID heapID );
static void OBJ_End( COMM_BTL_DEMO_OBJ_WORK* wk );
static GFL_CLWK* OBJ_CreateCLWK( COMM_BTL_DEMO_OBJ_WORK* wk, s16 px, s16 py, u16 anim );

static void G3D_Init( COMM_BTL_DEMO_G3D_WORK* g3d, COMM_BTL_DEMO_GRAPHIC_WORK* graphic, HEAPID heapID );
static void G3D_End( COMM_BTL_DEMO_G3D_WORK* g3d );
static void G3D_Main( COMM_BTL_DEMO_G3D_WORK * g3d );
static void G3D_PTC_Setup( COMM_BTL_DEMO_G3D_WORK* g3d, int spa_idx );
static void G3D_PTC_Delete( COMM_BTL_DEMO_G3D_WORK* g3d );
static void G3D_PTC_CreateEmitter( COMM_BTL_DEMO_G3D_WORK * g3d, u16 idx, const VecFx32* pos );
static void G3D_AnimeSet( COMM_BTL_DEMO_G3D_WORK* g3d, u16 demo_id );
static void G3D_AnimeDel( COMM_BTL_DEMO_G3D_WORK* g3d );
static BOOL G3D_AnimeMain( COMM_BTL_DEMO_G3D_WORK* g3d );

  //-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT CommBtlDemoProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT CommBtlDemoProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT CommBtlDemoProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void CommBtlDemo_BG_LoadResource( COMM_BTL_DEMO_BG_WORK* wk, HEAPID heapID );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA CommBtlDemoProcData = 
{
	CommBtlDemoProc_Init,
	CommBtlDemoProc_Main,
	CommBtlDemoProc_Exit,
};

//@TODO hosakaのみ
#ifdef DEBUG_ONLY_FOR_genya_hosaka
// ワーク生成
static void debug_param( COMM_BTL_DEMO_PARAM* prm )
{ 
  int i;
  
  prm->type = COMM_BTL_DEMO_TYPE_NORMAL_START;
  prm->result = GFUser_GetPublicRand( COMM_BTL_DEMO_RESULT_MAX );

  HOSAKA_Printf( "result = %d \n", prm->result );

  for( i=0; i<COMM_BTL_DEMO_TRDATA_MAX; i++ )
  {
    prm->trainer_data[i].server_version = GFUser_GetPublicRand(2);
    prm->trainer_data[i].trsex = GFUser_GetPublicRand(2) ? PM_MALE : PM_FEMALE;
//  prm->trainer_data[i].str_trname = 1;
    
    // デバッグポケパーティー
    {
      POKEPARTY *party;
      int poke_cnt;
      int p;

      party = PokeParty_AllocPartyWork( HEAPID_COMM_BTL_DEMO );
      Debug_PokeParty_MakeParty( party );
      
      prm->trainer_data[i].party = party;
    
      poke_cnt = PokeParty_GetPokeCount( prm->trainer_data[i].party );

      for( p=0; p<poke_cnt; p++ )
      {
        POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, p );
        HOSAKA_Printf("poke [%d] condition=%d \n",p, PP_Get( pp, ID_PARA_condition, NULL ) );
      }

      HOSAKA_Printf("[%d] server_version=%d trsex=%d poke_cnt=%d \n",i, 
          prm->trainer_data[i].server_version,
          prm->trainer_data[i].trsex,
          poke_cnt );
    }
  }
}
// ワーク開放
static void debug_param_del( COMM_BTL_DEMO_PARAM* prm )
{
  int i;

  for( i=0; i<COMM_BTL_DEMO_TRDATA_MAX; i++ )
  {
    GFL_HEAP_FreeMemory( (POKEPARTY*)prm->trainer_data[i].party );
  }
}
#endif

//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					COMM_BTL_DEMO_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CommBtlDemoProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	COMM_BTL_DEMO_MAIN_WORK *wk;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_COMM_BTL_DEMO, COMM_BTL_DEMO_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(COMM_BTL_DEMO_MAIN_WORK), HEAPID_COMM_BTL_DEMO );
  GFL_STD_MemClear( wk, sizeof(COMM_BTL_DEMO_MAIN_WORK) );
	
#ifdef DEBUG_ONLY_FOR_genya_hosaka
  debug_param( pwk );
#endif

  // 初期化
  wk->heapID = HEAPID_COMM_BTL_DEMO;
  wk->pwk = pwk;
	
	//描画設定初期化
	wk->graphic	= COMM_BTL_DEMO_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_mictest_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );
  
  // シーンコントローラ作成
  wk->cntScene = UI_SCENE_CNT_Create( 
      wk->heapID, c_scene_func_tbl, CBD_SCENE_ID_MAX, 
      calc_first_scene(pwk), wk );

	//BGリソース読み込み
	CommBtlDemo_BG_LoadResource( &wk->wk_bg, wk->heapID );

  // G3D関連初期化
  G3D_Init( &wk->wk_g3d, wk->graphic, wk->heapID );

  // OBJ初期化
  OBJ_Init( &wk->wk_obj, wk->graphic, wk->heapID );

  // フェードイン リクエスト
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procpロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					COMM_BTL_DEMO_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CommBtlDemoProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	COMM_BTL_DEMO_MAIN_WORK* wk = mywk;

#ifdef DEBUG_ONLY_FOR_genya_hosaka
  debug_param_del( pwk );
#endif
  
  // フェード中は処理しない
  if( GFL_FADE_CheckFade() == TRUE )
  {
    return GFL_PROC_RES_CONTINUE;
  }
  
  // シーンコントーラ削除
  UI_SCENE_CNT_Delete( wk->cntScene );

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );
  
  // OBJ開放
  OBJ_End( &wk->wk_obj );

  // 3Dシステム開放
  G3D_End( &wk->wk_g3d );

	//描画設定破棄
	COMM_BTL_DEMO_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heapID );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					COMM_BTL_DEMO_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
#include "ui/debug_code/g3d/camera_test.c"
static GFL_PROC_RESULT CommBtlDemoProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	COMM_BTL_DEMO_MAIN_WORK* wk = mywk;

  //@TODO カメラテスト
#if 0
#ifdef PM_DEBUG
  {
    GFL_G3D_CAMERA* p_camera = COMM_BTL_DEMO_GRAPHIC_GetCamera( wk->graphic );
    debug_camera_test( p_camera );
  }
#endif
#endif

  G3D_Main( &wk->wk_g3d );

  // @TODO 汎用化
#if 0
  // デバッグfovy
  {
    static fx32 fovy = 40;
    
    GFL_G3D_CAMERA* p_camera = COMM_BTL_DEMO_GRAPHIC_GetCamera( wk->graphic );

    debug_camera_test( p_camera );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      fovy += 1;
      GFL_G3D_CAMERA_SetfovyCos( p_camera, FX_SinIdx( fovy/2 * PERSPWAY_COEFFICIENT ) );
      GFL_G3D_CAMERA_SetfovySin( p_camera, FX_CosIdx( fovy/2 * PERSPWAY_COEFFICIENT ) );
      HOSAKA_Printf("fovy = %d \n", fovy);
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      fovy -= 1;
      GFL_G3D_CAMERA_SetfovyCos( p_camera, FX_SinIdx( fovy/2 * PERSPWAY_COEFFICIENT ) );
      GFL_G3D_CAMERA_SetfovySin( p_camera, FX_CosIdx( fovy/2 * PERSPWAY_COEFFICIENT ) );
      HOSAKA_Printf("fovy = %d \n", fovy);
    }
  }
#endif
  
  // フェード中は処理しない
  if( GFL_FADE_CheckFade() == TRUE )
  {
    return GFL_PROC_RES_CONTINUE;
  }
	
  // SCENE
  if( UI_SCENE_CNT_Main( wk->cntScene ) )
  {
    return GFL_PROC_RES_FINISH;
  }

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

	//2D描画
	COMM_BTL_DEMO_GRAPHIC_2D_Draw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	COMM_BTL_DEMO_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void CommBtlDemo_BG_LoadResource( COMM_BTL_DEMO_BG_WORK* wk, HEAPID heapID )
{
	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( ARCID_COMM_BTL_DEMO_GRA, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_comm_btl_demo_bg_base_u_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0x0, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_comm_btl_demo_bg_base_d_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x0, heapID );
	
  //	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_comm_btl_demo_bg_base_d_NCGR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_comm_btl_demo_bg_base_d_NSCR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );	

	//	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_comm_btl_demo_bg_base_u_NCGR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_comm_btl_demo_bg_base_u_NSCR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );		

	GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  通常バトル前デモ 初期化処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneNormalStart_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  COMM_BTL_DEMO_MAIN_WORK* wk = work;
  
  // トレーナー初期化
  {
    int i;
    for( i=0; i<TRAINER_CNT_NORMAL; i++ )
    {
      TRAINER_UNIT_Init( &wk->trainer_unit[i], wk->pwk->type, i, &wk->pwk->trainer_data[i], &wk->wk_obj );
    }
  }

#ifdef DEBUG_ONLY_FOR_genya_hosaka
    // フェードアウト リクエスト
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, 1 );
#endif

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  通常バトル前デモ 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneNormalStart_Main( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  COMM_BTL_DEMO_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );
  
  // トレーナー主処理
  {
    int i;
    for( i=0; i<TRAINER_CNT_NORMAL; i++ )
    {
      TRAINER_UNIT_Main( &wk->trainer_unit[i] );
    }
  }

  switch( seq )
  {
  case 0:
    G3D_AnimeSet( &wk->wk_g3d, DEMO_ID_01_A );

    UI_SCENE_CNT_IncSubSeq( cnt );
    break;
  case 1:
    if( G3D_AnimeMain( &wk->wk_g3d ) == FALSE )
    {
      G3D_AnimeDel( &wk->wk_g3d );
      G3D_AnimeSet( &wk->wk_g3d, DEMO_ID_DRAW );

      // ptcワーク生成( vs )
      G3D_PTC_Setup( &wk->wk_g3d, NARC_comm_btl_demo_vs_demo02_spa );

      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;
  case 2:
    {
      // パーティクル表示
      {
        int i;
        VecFx32 pos = {0,0,-100};

        for( i=0; i<wk->wk_g3d.spa_num; i++ )
        {
          G3D_PTC_CreateEmitter( &wk->wk_g3d, i, &pos );
        }
      }

      wk->timer = 0;
      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;
  case 3:
    if( wk->timer++ == 40 )
    {
      // フェードアウト リクエスト
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 2 );
      wk->timer = 0;
      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;
  case 4:
    if( G3D_AnimeMain( &wk->wk_g3d ) == FALSE )
    {
      G3D_AnimeDel( &wk->wk_g3d );
      return TRUE;
    }
    break;
  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  通常バトル前デモ 終了処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneNormalStart_End( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  COMM_BTL_DEMO_MAIN_WORK* wk = work;
  
  // トレーナー開放
  {
    int i;
    for( i=0; i<TRAINER_CNT_NORMAL; i++ )
    {
      TRAINER_UNIT_Exit( &wk->trainer_unit[i] );
    }
  }

  // PTCワーク開放
  G3D_PTC_Delete( &wk->wk_g3d );

// @TODO 保坂のみループ
#ifdef DEBUG_ONLY_FOR_genya_hosaka
  if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_START) == FALSE )
  {
    UI_SCENE_CNT_SetNextScene( cnt, CBD_SCENE_ID_NORMAL_START );
  }
#else
  // 終了
  UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
#endif

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  通常バトル後デモ 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneNormalEnd_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  // 終了
  UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  マルチバトル前デモ 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneMultiStart_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  // 終了
  UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  マルチバトル後デモ 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneMultiEnd_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  // 終了
  UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  最初のシーンを判定
 *
 *	@param	COMM_BTL_DEMO_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static CBD_SCENE_ID calc_first_scene( COMM_BTL_DEMO_PARAM* pwk )
{
  switch( pwk->type )
  {
  case COMM_BTL_DEMO_TYPE_NORMAL_START:
    return CBD_SCENE_ID_NORMAL_START;

  case COMM_BTL_DEMO_TYPE_NORMAL_END:
    return CBD_SCENE_ID_NORMAL_END;

  case COMM_BTL_DEMO_TYPE_MULTI_START:
    return CBD_SCENE_ID_MULTI_START;

  case COMM_BTL_DEMO_TYPE_MULTI_END:
    return CBD_SCENE_ID_MULTI_END;

  default : GF_ASSERT(0);
  }

  return CBD_SCENE_ID_NORMAL_START;
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	u8 type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL type_is_normal( u8 type )
{
  switch( type )
  {
  case COMM_BTL_DEMO_TYPE_NORMAL_START:
    return TRUE;

  case COMM_BTL_DEMO_TYPE_NORMAL_END:
    return TRUE;

  case COMM_BTL_DEMO_TYPE_MULTI_START:
    return FALSE;

  case COMM_BTL_DEMO_TYPE_MULTI_END:
    return FALSE;

  default : GF_ASSERT(0);
  }

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイプがスタートかエンドか取得
 *
 *	@param	u8 type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL type_is_start( u8 type )
{
  switch( type )
  {
  case COMM_BTL_DEMO_TYPE_NORMAL_START:
    return TRUE;

  case COMM_BTL_DEMO_TYPE_NORMAL_END:
    return FALSE;

  case COMM_BTL_DEMO_TYPE_MULTI_START:
    return TRUE;

  case COMM_BTL_DEMO_TYPE_MULTI_END:
    return FALSE;

  default : GF_ASSERT(0);
  }
  
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ポケパラからボールOBJのアニメIDを取得
 *
 *	@param	POKEMON_PARAM* pp 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 PokeParaToBallAnim( POKEMON_PARAM* pp )
{
  if( pp == NULL )
  {
    return OBJ_ANM_ID_BALL_NULL; // ボールなし
  }

  if( PP_Get( pp, ID_PARA_hp, NULL ) == 0 )
  {
    return OBJ_ANM_ID_BALL_DOWN; // 瀕死
  }
  else
  {
    // 常態異常
    u32 prm = PP_Get( pp, ID_PARA_condition, NULL );

    if( prm == POKESICK_NULL )
    {
      return OBJ_ANM_ID_BALL_NORMAL; // 通常
    }
    else
    {
      return OBJ_ANM_ID_BALL_SICK; // 状態異常
    }
  }

  GF_ASSERT(0);

  return 10; // 不具合対策
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ボールユニット初期化
 *
 *	@param	BALL_UNIT* unit
 *	@param	POKEPARTY* party
 *	@param	type
 *	@param	posid
 *	@param	obj 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BALL_UNIT_Init( BALL_UNIT* unit, const POKEPARTY* party, u8 type, u8 posid, COMM_BTL_DEMO_OBJ_WORK* obj )
{
  int i;
  int is_normal = type_is_normal(type);
  int max;

  GF_ASSERT(unit);
  GF_ASSERT(party);
  GF_ASSERT(obj);

  unit->num = PokeParty_GetPokeCount( party );
  unit->type = type;
  unit->posid = posid;
  unit->timer = 0;

  max = PokeParty_GetPokeCountMax( party );

  HOSAKA_Printf("max=%d pokenum=%d\n",max, unit->num);

  for( i=0; i<max; i++ )
  {
    s16 px;
    s16 py;
  
    if( type_is_normal(type) )
    {
      //@TODO 定数化
      // ノーマル
      px = 128 + (i+1) * 16;
      py= (posid==0) ? 21*8 : 7*8 ;

      if( posid == 1 ){ px *= -1; }
    }
    else
    {
      // マルチ
      //@TODO posid による座標調整
      px = 128 + (i+1) * 16;
      py= (posid==0) ? 21*8 : 7*8 ;

      if( posid == 1 ){ px *= -1; }
    }

    // 開始デモ
    if( type_is_start(type) )
    {
      // 状態取得
      int anm;
      POKEMON_PARAM* pp = NULL;
      
      // ポケモンが存在する間はPOKEMON_PARAMを取得
      if( i < unit->num )
      {
        pp  = PokeParty_GetMemberPointer( party, i );
      }

      anm = PokeParaToBallAnim( pp );

      unit->clwk[i] = OBJ_CreateCLWK( obj, px, py, anm );
    }
    else
    // 終了デモ
    {
      // 全て通常状態で初期化
      unit->clwk[i] = OBJ_CreateCLWK( obj, px, py, OBJ_ANM_ID_BALL_NORMAL );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ボールユニット開放
 *
 *	@param	BALL_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BALL_UNIT_Exit( BALL_UNIT* unit )
{
  int i;

  GF_ASSERT(unit);

  for( i=0; i<unit->num; i++ )
  {
    GFL_CLACT_WK_Remove( unit->clwk[i] );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメ開始
 *
 *	@param	BALL_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BALL_UNIT_SetStart( BALL_UNIT* unit )
{
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	BALL_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BALL_UNIT_Main( BALL_UNIT* unit )
{
  // マルチ
  // シングル
  // 開始
  // 終了
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナーユニット 初期化
 *
 *	@param	TRAINER_UNIT* unit
 *	@param	posid
 *	@param	COMM_BTL_DEMO_TRAINER_DATA* data
 *	@param	obj 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_Init( TRAINER_UNIT* unit, u8 type, u8 posid, const COMM_BTL_DEMO_TRAINER_DATA* data, COMM_BTL_DEMO_OBJ_WORK* obj )
{
  u8 num;

  GF_ASSERT( obj );

  // メンバ初期化
  unit->data = data;
  unit->obj = obj;

  // ボール初期化
  BALL_UNIT_Init( &unit->ball, data->party, type, posid, unit->obj );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナーユニット 主処理
 *
 *	@param	TRAINER_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_Main( TRAINER_UNIT* unit )
{
  BALL_UNIT_Main( &unit->ball );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレイナーユニット 後処理
 *
 *	@param	TRAINER_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_Exit( TRAINER_UNIT* unit )
{
  BALL_UNIT_Exit( &unit->ball );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化
 *
 *	@param	COMM_BTL_DEMO_OBJ_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void OBJ_Init( COMM_BTL_DEMO_OBJ_WORK* wk, COMM_BTL_DEMO_GRAPHIC_WORK* graphic, HEAPID heapID )
{
  UI_EASY_CLWK_RES_PARAM prm;
  GFL_CLUNIT* unit;

  
  GF_ASSERT( wk );
  GF_ASSERT( graphic );

  // メンバ初期化
  wk->graphic = graphic;
  wk->heapID = heapID;

  // リソース展開
  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = ARCID_COMM_BTL_DEMO_GRA;
  prm.pltt_id   = NARC_comm_btl_demo_line_num_full_NCLR;
  prm.ncg_id    = NARC_comm_btl_demo_line_num_full_NCGR;
  prm.cell_id   = NARC_comm_btl_demo_line_num_full_NCER;
  prm.anm_id    = NARC_comm_btl_demo_line_num_full_NANR;
  prm.pltt_line = PLTID_OBJ_COMMON_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 4;
  
  unit = COMM_BTL_DEMO_GRAPHIC_GetClunit( wk->graphic );
  UI_EASY_CLWK_LoadResource( &wk->clres_common, &prm, unit, heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ開放
 *
 *	@param	COMM_BTL_DEMO_OBJ_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void OBJ_End( COMM_BTL_DEMO_OBJ_WORK* wk )
{
  GF_ASSERT( wk );
  GF_ASSERT( wk->graphic );

  UI_EASY_CLWK_UnLoadResource( &wk->clres_common );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  CLWK生成
 *
 *	@param	COMM_BTL_DEMO_OBJ_WORK* wk
 *	@param	px
 *	@param	py
 *	@param	anim 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* OBJ_CreateCLWK( COMM_BTL_DEMO_OBJ_WORK* wk, s16 px, s16 py, u16 anim )
{
  GFL_CLUNIT* unit;

  GF_ASSERT( wk );
  GF_ASSERT( wk->graphic );
  
  unit = COMM_BTL_DEMO_GRAPHIC_GetClunit( wk->graphic );

  return UI_EASY_CLWK_CreateCLWK( &wk->clres_common, unit, px, py, anim, wk->heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK* g3d 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_Init( COMM_BTL_DEMO_G3D_WORK* g3d, COMM_BTL_DEMO_GRAPHIC_WORK* graphic, HEAPID heapID )
{
  int i;
  
  GF_ASSERT( graphic );

  // ブレンド指定
  G2_SetBlendAlpha( GX_PLANEMASK_BG0, GX_PLANEMASK_BG3|GX_PLANEMASK_BG0, 16, 0 );
//  G2_SetBlendAlpha( GX_PLANEMASK_BG0, GX_PLANEMASK_BG3|GX_PLANEMASK_BG2|GX_PLANEMASK_BG1|GX_PLANEMASK_BG0, 0, 0 );
  
  // メンバ初期化
  g3d->graphic = graphic;
  g3d->heapID = heapID;

  // 3D管理ユーティリティーの生成
  g3d->g3d_util = GFL_G3D_UTIL_Create( 10, 16, heapID );

  // PTC SYSTEM
  GFL_PTC_Init( heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  終了処理
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK* g3d 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_End( COMM_BTL_DEMO_G3D_WORK* g3d )
{ 
  int i;

  GF_ASSERT( g3d );
  GF_ASSERT( g3d->g3d_util );
  
  // PTC開放
  GFL_PTC_Exit();

  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( g3d->g3d_util );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  3D関連 主処理
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK * g3d 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_Main( COMM_BTL_DEMO_G3D_WORK * g3d )
{
  GF_ASSERT( g3d );

  if( g3d->ptc )
  {
    GFL_PTC_Main();
  }
}

static VecFx32 sc_camera_eye = { 0, 0, FX32_CONST(70), };
static VecFx32 sc_camera_up = { 0, FX32_ONE, 0 };
static VecFx32 sc_camera_at = { 0, 0, -FX32_ONE };

//-----------------------------------------------------------------------------
/**
 *	@brief  PTCリソースロード
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK* g3d
 *	@param	spa_idx 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_PTC_Setup( COMM_BTL_DEMO_G3D_WORK* g3d, int spa_idx )
{
  GF_ASSERT( g3d->ptc == NULL );

  // PTCワーク生成
  g3d->ptc = GFL_PTC_Create( g3d->spa_work, sizeof(g3d->spa_work), TRUE, g3d->heapID );
  
  /* パーティクルのカメラを正射影に設定 */	
  {
    GFL_G3D_PROJECTION projection; 
    projection.type = GFL_G3D_PRJORTH;
    projection.param1 = FX32_CONST(4); 
    projection.param2 = -FX32_CONST(4); 
    projection.param3 = -FX32_CONST(3); 
    projection.param4 = FX32_CONST(3);  
    projection.near = FX32_ONE * 1;
    projection.far  = FX32_CONST( 1024 );	// 正射影なので関係ないが、念のためクリップのfarを設定
    projection.scaleW = FX32_ONE;//0
    GFL_PTC_PersonalCameraDelete( g3d->ptc );
    GFL_PTC_PersonalCameraCreate( g3d->ptc, &projection, DEFAULT_PERSP_WAY, &sc_camera_eye, &sc_camera_up, &sc_camera_at, g3d->heapID );
  }

  // リソース展開
  {
    void *res;

    res = GFL_PTC_LoadArcResource( ARCID_COMM_BTL_DEMO_GRA, spa_idx, g3d->heapID );
    g3d->spa_num = GFL_PTC_GetResNum( res );
    HOSAKA_Printf("load spa_idx=%d num=%d \n", spa_idx, g3d->spa_num );
    GFL_PTC_SetResourceEx( g3d->ptc, res, TRUE, NULL );
  }

}

//-----------------------------------------------------------------------------
/**
 *	@brief  PTCワーク削除
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK* g3d 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_PTC_Delete( COMM_BTL_DEMO_G3D_WORK* g3d )
{
  GF_ASSERT( g3d->ptc );

  GFL_PTC_Delete( g3d->ptc );
  g3d->ptc = NULL;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  エミッタ生成
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK * g3d
 *	@param	idx
 *	@param	VecFx32* pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_PTC_CreateEmitter( COMM_BTL_DEMO_G3D_WORK * g3d, u16 idx, const VecFx32* pos )
{
  GFL_EMIT_PTR emit;

  GF_ASSERT(g3d);
  GF_ASSERT(g3d->ptc);

  emit = GFL_PTC_CreateEmitter( g3d->ptc, idx, pos );

  HOSAKA_Printf("emit=%d \n", emit);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメーション初期化
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK* g3d
 *	@param	demo_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_AnimeSet( COMM_BTL_DEMO_G3D_WORK* g3d, u16 demo_id )
{
  int i;
  
  // ユニット生成
  g3d->anm_unit_idx = GFL_G3D_UTIL_AddUnit( g3d->g3d_util, &sc_setup[ demo_id ] );
  HOSAKA_Printf("demo_id=%d add unit idx=%d ",demo_id, g3d->anm_unit_idx );
  
  {
    GFL_G3D_OBJ* obj;
    int anime_count;
    
    obj         = GFL_G3D_UTIL_GetObjHandle( g3d->g3d_util, g3d->anm_unit_idx );
    anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );

    HOSAKA_Printf("anime_count=%d \n", anime_count);

    // アニメーション有効化
    for( i=0; i<anime_count; i++ )
    {
      GFL_G3D_OBJECT_EnableAnime( obj, i );
    }

    //@TODO 男女からテクスチャのパレットを設定
    {
      GFL_G3D_RND* rnd = GFL_G3D_OBJECT_GetG3Drnd(obj);
      GFL_G3D_RES* tex = GFL_G3D_RENDER_GetG3DresTex(rnd);

//      GFL_G3D_TransVramTexturePlttOnly(

  //    GFL_G3D_RENDER_SetTexture( rnd, 2, tex );

//      GFL_G3D_RENDER_GetRenderObj();
//BOOL GFL_G3D_RENDER_SetTexture( GFL_G3D_RND* g3Drnd, const int mdlidx, const GFL_G3D_RES* tex )
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメーション削除
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK* g3d 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void G3D_AnimeDel( COMM_BTL_DEMO_G3D_WORK* g3d )
{
  GF_ASSERT( g3d );

  HOSAKA_Printf("del unit_idx=%d\n", g3d->anm_unit_idx );

  // ユニット削除
  GFL_G3D_UTIL_DelUnit( g3d->g3d_util, g3d->anm_unit_idx );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  描画処理
 *
 *	@param	COMM_BTL_DEMO_G3D_WORK* g3d 
 *
 *	@retval TRUE:ループ再生中, FALSE: 再生終了
 */
//-----------------------------------------------------------------------------
static BOOL G3D_AnimeMain( COMM_BTL_DEMO_G3D_WORK* g3d )
{ 
  GFL_G3D_OBJSTATUS status;
  BOOL is_loop;

  GF_ASSERT( g3d );
  GF_ASSERT( g3d->g3d_util );
  
  // ステータス初期化
  VEC_Set( &status.trans, 0, 0, 0 );
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status.rotate );

#if 0
    {
      static int frame = 0;

      // アニメーションフレーム操作
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
      {
        frame+= FX32_ONE;
        HOSAKA_Printf("frame=%d\n",frame);
      }
      else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
      {
        frame-= FX32_ONE;
        HOSAKA_Printf("frame=%d\n",frame);
      }
      
      GFL_G3D_OBJECT_GetAnimeFrame( obj, 0, &frame );
      HOSAKA_Printf("frame=%d \n", frame>>FX32_SHIFT );
    }
#endif
  
  {
    int i;
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( g3d->g3d_util, g3d->anm_unit_idx );
    int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
    
    // アニメーション更新
    for( i=0; i<anime_count; i++ )
    {
//    GFL_G3D_OBJECT_SetAnimeFrame( obj, i, &frame );
      is_loop = GFL_G3D_OBJECT_LoopAnimeFrame( obj, i, FX32_ONE );
    }

    //3D描画
    COMM_BTL_DEMO_GRAPHIC_3D_StartDraw( g3d->graphic );
    
    // 描画
    GFL_G3D_DRAW_DrawObject( obj, &status );
    
    COMM_BTL_DEMO_GRAPHIC_3D_EndDraw( g3d->graphic );
  }

  return is_loop;
}


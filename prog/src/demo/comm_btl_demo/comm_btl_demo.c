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

enum
{ 
  // ボール
  NORMAL_POSID0_BALL_PX_BASE = 128+8,
  NORMAL_POSID0_BALL_PX_OFS = 16,
  NORMAL_POSID0_BALL_PY = 7*8,
  NORMAL_POSID1_BALL_PY = 21*8,
  // トレーナー名
  TRNAME_CSX = 10,
  TRNAME_CSY = 2,
  NORMAL_POSID0_TRNAME_CPX = 16,
  NORMAL_POSID0_TRNAME_CPY = 3,
  NORMAL_POSID1_TRNAME_CPX = 4,
  NORMAL_POSID1_TRNAME_CPY = 17,
  NORMAL_POSID0_TRNAME_PTC_PX = 0x800,
  NORMAL_POSID0_TRNAME_PTC_PY = 0x2800,
  NORMAL_POSID1_TRNAME_PTC_PX = 0xffffe000, 
  NORMAL_POSID1_TRNAME_PTC_PY = 0xffffe000,
};

//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_TEXT_M	= GFL_BG_FRAME1_M,
	BG_FRAME_BAR_M	= GFL_BG_FRAME2_M,
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
	PLTID_BG_TEXT_M				= 10,
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

#define BALL_CLWK_MAX ( 6 )

//--------------------------------------------------------------
///	ボールUNITワーク
//==============================================================
typedef struct {
  // [IN]
  u8 type;
  u8 posid;
  u8 num;
  u8 max;
  //[PRIVATE]
  GFL_CLWK* clwk[ BALL_CLWK_MAX ];
  u32 timer;
  u8 is_start;
  u8 padding[3];
} BALL_UNIT;

//--------------------------------------------------------------
///	トレーナーユニット
//==============================================================
typedef struct {
  // [IN]
  const COMM_BTL_DEMO_TRAINER_DATA* data;
  COMM_BTL_DEMO_OBJ_WORK* obj;
  COMM_BTL_DEMO_G3D_WORK* g3d;
  GFL_FONT* font;
  u8 type;
  u8 posid;
  u8 padding[2];
  // [PRIVATE]
  GFL_BMPWIN* win_name;
  BALL_UNIT ball; // ボール管理ワーク
  u32 timer;
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

  u8 type;
  int timer;  ///< デモ起動時間カウンタ

} COMM_BTL_DEMO_MAIN_WORK;


//=============================================================================
/**
 *							シーン定義
 */
//=============================================================================

// ノーマル開始
static BOOL SceneStartDemo_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneStartDemo_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneStartDemo_End( UI_SCENE_CNT_PTR cnt, void* work );

// ノーマル終了
static BOOL SceneEndDemo_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneEndDemo_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneEndDemo_End( UI_SCENE_CNT_PTR cnt, void* work );


//--------------------------------------------------------------
///	SceneID
//==============================================================
typedef enum
{ 
  CBD_SCENE_ID_NORMAL_START = 0,  ///< バトル開始デモ
  CBD_SCENE_ID_NORMAL_END,        ///< バトル終了デモ

  CBD_SCENE_ID_MAX,
} CBD_SCENE_ID;

//--------------------------------------------------------------
///	SceneFunc
//==============================================================
static const UI_SCENE_FUNC_SET c_scene_func_tbl[ CBD_SCENE_ID_MAX ] = 
{
  // CBD_SCENE_ID_NORMAL_START
  {
    SceneStartDemo_Init,
    NULL,
    SceneStartDemo_Main,
    NULL,
    SceneStartDemo_End,
  },
  // CBD_SCENE_ID_NORMAL_END
  {
    SceneEndDemo_Init,
    NULL,
    SceneEndDemo_Main,
    NULL,
    SceneEndDemo_End,
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

static void TRAINER_UNIT_Init( TRAINER_UNIT* unit, u8 type, u8 posid, const COMM_BTL_DEMO_TRAINER_DATA* data, COMM_BTL_DEMO_OBJ_WORK* obj, COMM_BTL_DEMO_G3D_WORK* g3d, GFL_FONT* font );
static void TRAINER_UNIT_Main( TRAINER_UNIT* unit );
static void TRAINER_UNIT_Exit( TRAINER_UNIT* unit );
static void TRAINER_UNIT_DrawTrainerName( TRAINER_UNIT* unit, GFL_FONT *font );

static void TRAINER_UNIT_CNT_Init( COMM_BTL_DEMO_MAIN_WORK* wk );
static void TRAINER_UNIT_CNT_Exit( COMM_BTL_DEMO_MAIN_WORK* wk );
static void TRAINER_UNIT_CNT_Main( COMM_BTL_DEMO_MAIN_WORK* wk );
static void TRAINER_UNIT_CNT_BallSetStart( COMM_BTL_DEMO_MAIN_WORK* wk );

static void OBJ_Init( COMM_BTL_DEMO_OBJ_WORK* wk, COMM_BTL_DEMO_GRAPHIC_WORK* graphic, HEAPID heapID );
static void OBJ_Exit( COMM_BTL_DEMO_OBJ_WORK* wk );
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
#ifdef PM_DEBUG
// ワーク生成
static void debug_param( COMM_BTL_DEMO_PARAM* prm )
{ 
  int i;

  HOSAKA_Printf("in param type = %d \n", prm->type);
  
  prm->result = GFUser_GetPublicRand( COMM_BTL_DEMO_RESULT_MAX );

  HOSAKA_Printf( "result = %d \n", prm->result );

  for( i=0; i<COMM_BTL_DEMO_TRDATA_MAX; i++ )
  {
    prm->trainer_data[i].server_version = GFUser_GetPublicRand(2);
    prm->trainer_data[i].trsex = (i!=0) ? PM_MALE : PM_FEMALE; //@TODO

    // トレーナー名
    {
      //終端コードを追加してからSTRBUFに変換
      STRCODE debugname[32] = L"とうふ";
      
      debugname[3] = GFL_STR_GetEOMCode();

      prm->trainer_data[i].str_trname = GFL_STR_CreateBuffer( sizeof(STRCODE)*10, HEAPID_COMM_BTL_DEMO );
      GFL_STR_SetStringCode( prm->trainer_data[i].str_trname, debugname );
    }
    
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
        if( GFUser_GetPublicRand(2) == 0 )
        {
          PP_SetSick( pp, POKESICK_DOKU );
        }
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
    GFL_STR_DeleteBuffer( prm->trainer_data[i].str_trname );
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
	
#ifdef PM_DEBUG
  debug_param( pwk );
#endif

  // 初期化
  wk->heapID = HEAPID_COMM_BTL_DEMO;
  wk->pwk = pwk;
  wk->type = wk->pwk->type;
	
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
  OBJ_Exit( &wk->wk_obj );

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
#ifdef PM_DEBUG
#if 0
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
  
  // 会話フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT,
      NARC_font_default_nclr,
      PALTYPE_MAIN_BG,
      0x20*PLTID_BG_TEXT_M, 0x20, heapID );
	
	
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
 *	@brief  バトル前デモ 初期化処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneStartDemo_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  COMM_BTL_DEMO_MAIN_WORK* wk = work;
  
  // トレイナーユニット初期化
  TRAINER_UNIT_CNT_Init( wk );

#ifdef DEBUG_ONLY_FOR_genya_hosaka
  GFL_BG_ClearScreen( BG_FRAME_TEXT_M ); // テキスト面を消去しておく
    // フェードアウト リクエスト
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, 1 );
#endif

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  バトル前デモ 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneStartDemo_Main( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  COMM_BTL_DEMO_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  TRAINER_UNIT_CNT_Main( wk );

  switch( seq )
  {
  case 0:
    G3D_PTC_Setup( &wk->wk_g3d, NARC_comm_btl_demo_vs_demo01_spa );

    if( type_is_normal(wk->type) )
    {
      G3D_AnimeSet( &wk->wk_g3d, DEMO_ID_01_A );
    }
    else
    {
      G3D_AnimeSet( &wk->wk_g3d, DEMO_ID_02_A );
    }
    
    // ボールアニメ開始
    TRAINER_UNIT_CNT_BallSetStart( wk );

    UI_SCENE_CNT_IncSubSeq( cnt );
    break;
  case 1:
    if( G3D_AnimeMain( &wk->wk_g3d ) == FALSE )
    {
      G3D_AnimeDel( &wk->wk_g3d );
      G3D_AnimeSet( &wk->wk_g3d, DEMO_ID_DRAW );

      // ptcワーク生成「VS」
      G3D_PTC_Delete( &wk->wk_g3d );
      G3D_PTC_Setup( &wk->wk_g3d, NARC_comm_btl_demo_vs_demo02_spa );

      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;
  case 2:
    {
      // 「VS」パーティクル表示
      {
        int i;
        for( i=0; i<wk->wk_g3d.spa_num; i++ )
        {
          G3D_PTC_CreateEmitter( &wk->wk_g3d, i, &(VecFx32){0,0,-100} );
        }
      }

      wk->timer = 0;
      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;
  case 3:
    //@TODO タイミング
    if( wk->timer++ == 120+15 )
    {
      // フェードアウト リクエスト
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 2 );
    }
    
    // アニメ再生
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
 *	@brief  バトル前デモ 終了処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneStartDemo_End( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  COMM_BTL_DEMO_MAIN_WORK* wk = work;
  
  // トレーナー開放
  TRAINER_UNIT_CNT_Exit( wk );

  // PTCワーク開放
  G3D_PTC_Delete( &wk->wk_g3d );
  
  // 終了
  UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );

// @TODO 保坂のみループ
#ifdef DEBUG_ONLY_FOR_genya_hosaka
  if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_START) == FALSE )
  {
    UI_SCENE_CNT_SetNextScene( cnt, CBD_SCENE_ID_NORMAL_START );
  }
#endif

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  バトル後デモ 初期化処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	*work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneEndDemo_Init( UI_SCENE_CNT_PTR cnt, void *work )
{
  COMM_BTL_DEMO_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  バトル後デモ 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneEndDemo_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  // 終了
  UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  バトル後デモ 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneEndDemo_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  COMM_BTL_DEMO_MAIN_WORK* wk = work;

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
  case COMM_BTL_DEMO_TYPE_MULTI_START:
    return CBD_SCENE_ID_NORMAL_START;

  case COMM_BTL_DEMO_TYPE_NORMAL_END:
  case COMM_BTL_DEMO_TYPE_MULTI_END:
    return CBD_SCENE_ID_NORMAL_END;

  default : GF_ASSERT_MSG( 0 , "demo type=%d ", pwk->type);
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

  return OBJ_ANM_ID_BALL_NORMAL; // 絶対ここには来ない
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

  GF_ASSERT(unit);
  GF_ASSERT(party);
  GF_ASSERT(obj);

  unit->num = PokeParty_GetPokeCount( party );
  unit->max = PokeParty_GetPokeCountMax( party );
  unit->type = type;
  unit->posid = posid;
  unit->timer = 0;

  HOSAKA_Printf("max=%d pokenum=%d\n", unit->max, unit->num);

  for( i=0; i<unit->max; i++ )
  {
    s16 px;
    s16 py;
    int anm;
  
    if( type_is_normal(type) )
    {
      // ノーマル
      px = NORMAL_POSID0_BALL_PX_BASE + (i) * NORMAL_POSID0_BALL_PX_OFS;
      py= (posid==0) ? NORMAL_POSID0_BALL_PY : NORMAL_POSID1_BALL_PY;

      if( posid == 1 ){ px *= -1; }
    }
    else
    {
      //@TODO posid による座標調整
      // マルチ
      px = NORMAL_POSID0_BALL_PX_BASE + (i) * NORMAL_POSID0_BALL_PX_OFS;
      py= (posid==0) ? NORMAL_POSID0_BALL_PY : NORMAL_POSID1_BALL_PY;

      if( posid == 1 ){ px *= -1; }
    }

    // 開始デモ
    if( type_is_start(type) )
    {
      // 状態取得
      POKEMON_PARAM* pp = NULL;
      
      // ポケモンが存在する間はPOKEMON_PARAMを取得
      if( i < unit->num )
      {
        pp  = PokeParty_GetMemberPointer( party, i );
      }

      anm = PokeParaToBallAnim( pp );

    }
    else
    // 終了デモ
    {
      // 全て通常状態で初期化
      anm = OBJ_ANM_ID_BALL_NORMAL;
    }
      
    // CLWK生成
    unit->clwk[i] = OBJ_CreateCLWK( obj, px, py, anm );
    GFL_CLACT_WK_SetDrawEnable( unit->clwk[i] , FALSE );
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

  for( i=0; i<unit->max; i++ )
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
  unit->is_start = TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ボール主処理
 *
 *	@param	BALL_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BALL_UNIT_Main( BALL_UNIT* unit )
{
  const int OPEN_SYNC = 12;

  if( unit->is_start == FALSE )
  {
    return;
  }

  switch( unit->type )
  {
  case COMM_BTL_DEMO_TYPE_NORMAL_START :
    if( unit->timer >= OPEN_SYNC )
    {
      int id = unit->timer - OPEN_SYNC;

      HOSAKA_Printf("unit->timer=%d ",unit->timer);
      HOSAKA_Printf("ball open id=%d\n", id);

      if( id < BALL_CLWK_MAX )
      {
        // 開く
        GFL_CLACT_WK_SetDrawEnable( unit->clwk[id] , TRUE );
      }
      else
      {
        int i;

        // プライオリティを下げておく
        for( i=0; i<unit->max; i++ )
        {
          GFL_CLACT_WK_SetBgPri( unit->clwk[i], 1 );
        }

        unit->is_start = FALSE; // 終了
      }
    }
    break;
  case COMM_BTL_DEMO_TYPE_NORMAL_END :
    break;
  case COMM_BTL_DEMO_TYPE_MULTI_START :
    //@TODO ノーマルと一緒でいける可能性がある
    break;
  case COMM_BTL_DEMO_TYPE_MULTI_END :
    break;
  default : GF_ASSERT(0);
  }

  unit->timer++;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナー名用BMPWIN生成
 *
 *	@param	u8 type
 *	@param	posid 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN* TRAINERNAME_WIN_Create( u8 type, u8 posid )
{
  GFL_BMPWIN* win;
  s16 px, py;

  if( type_is_normal(type) )
  {
    if( posid == 0 )
    {
      px = NORMAL_POSID0_TRNAME_CPX;
      py = NORMAL_POSID0_TRNAME_CPY;
    }
    else
    {
      px = NORMAL_POSID1_TRNAME_CPX;
      py = NORMAL_POSID1_TRNAME_CPY;
    }
  }
  else
  {
    //@TODO マルチ
  }

  win = GFL_BMPWIN_Create( 
      BG_FRAME_TEXT_M,        // BGフレーム
      px, py,                 // 表示座標 
      TRNAME_CSX, TRNAME_CSY, // 表示サイズ
      PLTID_BG_TEXT_M,        // パレット
      GFL_BMP_CHRAREA_GET_B   // キャラ取得方向
      );

  return win;
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
static void TRAINER_UNIT_Init( TRAINER_UNIT* unit, u8 type, u8 posid, const COMM_BTL_DEMO_TRAINER_DATA* data, COMM_BTL_DEMO_OBJ_WORK* obj, COMM_BTL_DEMO_G3D_WORK* g3d, GFL_FONT* font )
{
  u8 num;

  GF_ASSERT( obj );

  // メンバ初期化
  unit->data = data;
  unit->obj = obj;
  unit->g3d = g3d;
  unit->type = type;
  unit->posid = posid;
  unit->font = font;
  unit->timer = 0;

  // ボール初期化
  BALL_UNIT_Init( &unit->ball, data->party, type, posid, unit->obj );

  // トレーナー名初期化
  unit->win_name = TRAINERNAME_WIN_Create( type, posid );
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
  GF_ASSERT( unit->g3d );

  // ボール主処理
  BALL_UNIT_Main( &unit->ball );

  //@TODO タイミング：マジックナンバー
  // トレーナー名表示
  if( unit->timer == 45 ) 
  { 
    // トレーナー名表示
    TRAINER_UNIT_DrawTrainerName( unit, unit->font );

    // パーティクル表示
    {
      int p;
      fx32 fx, fy;

      if( type_is_normal(unit->type) )
      {
        if( unit->posid == 0 )
        {
          fx = NORMAL_POSID0_TRNAME_PTC_PX;
          fy = NORMAL_POSID0_TRNAME_PTC_PY;
        }
        else
        {
          fx = NORMAL_POSID1_TRNAME_PTC_PX;
          fy = NORMAL_POSID1_TRNAME_PTC_PY;
        }
      }
      else
      {
        //@TODO マルチ
      }

      for( p=0; p<unit->g3d->spa_num; p++ )
      {
        G3D_PTC_CreateEmitter( unit->g3d, p, &(VecFx32){fx,fy,-100} );
      }
    }
  
  }  

//  HOSAKA_Printf("unit timer=%d \n", unit->timer);

  unit->timer++;
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
  // ボール開放
  BALL_UNIT_Exit( &unit->ball );
  // トレーナー名BMPWIN解放
  GFL_BMPWIN_Delete( unit->win_name );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナーユニット トレーナー名生成、表示
 *
 *	@param	TRAINER_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_DrawTrainerName( TRAINER_UNIT* unit, GFL_FONT *font )
{
  // 生成＆転送

  GFL_FONTSYS_SetColor( 0xf, 0x1, 0x2 );

#if 0
  if( unit->data->trsex == PM_MALE )
  {
    // 男処理
    GFL_FONTSYS_SetColor( 0xf, 0x5, 0x6 );
  }
  else
  {
    // 女処理
    GFL_FONTSYS_SetColor( 0xf, 0x3, 0x4 );
  }
#endif

  PRINTSYS_Print( GFL_BMPWIN_GetBmp(unit->win_name), 0, 0, unit->data->str_trname, font );
  GFL_BMPWIN_MakeTransWindow_VBlank( unit->win_name );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナーユニット 初期化コントローラー
 *
 *	@param	COMM_BTL_DEMO_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_CNT_Init( COMM_BTL_DEMO_MAIN_WORK* wk )
{
  int i;
  int max = (type_is_normal(wk->type) ? TRAINER_CNT_NORMAL : TRAINER_CNT_MULTI ); 

  for( i=0; i<max; i++ )
  {
    TRAINER_UNIT_Init( &wk->trainer_unit[i],
          wk->type, i,
          &wk->pwk->trainer_data[i], 
          &wk->wk_obj,
          &wk->wk_g3d,
          wk->font
          );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナーユニット 後処理コントローラー
 *
 *	@param	COMM_BTL_DEMO_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_CNT_Exit( COMM_BTL_DEMO_MAIN_WORK* wk )
{
  int i;
  int max = (type_is_normal(wk->type) ? TRAINER_CNT_NORMAL : TRAINER_CNT_MULTI ); 

  for( i=0; i<max; i++ )
  {
    TRAINER_UNIT_Exit( &wk->trainer_unit[i] );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナーユニット 主処理コントローラー
 *
 *	@param	COMM_BTL_DEMO_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_CNT_Main( COMM_BTL_DEMO_MAIN_WORK* wk )
{
  int i;
  int max = (type_is_normal(wk->type) ? TRAINER_CNT_NORMAL : TRAINER_CNT_MULTI ); 

#if 0
  // パーティクル座標調整
  {
    static fx32 fx=0;
    static fx32 fy=0;
    
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )  { fy+=FX32_ONE/2; }else
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){ fy-=FX32_ONE/2; }else
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){ fx-=FX32_ONE/2; }else
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){ fx+=FX32_ONE/2; }else
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      if( wk->wk_g3d.ptc != NULL )
      {
        int p;
        HOSAKA_Printf("{0x%x,0x%x}\n", fx, fy);
    
        for( p=0; p<wk->wk_g3d.spa_num; p++ )
        {
          G3D_PTC_CreateEmitter( &wk->wk_g3d, p, &(VecFx32){fx,fy,-100} );
        }
      }
    }
  }
#endif

  for( i=0; i<max; i++ )
  {
    TRAINER_UNIT_Main( &wk->trainer_unit[i] );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  トレーナーユニット ボール開始コントローラー
 *
 *	@param	COMM_BTL_DEMO_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TRAINER_UNIT_CNT_BallSetStart( COMM_BTL_DEMO_MAIN_WORK* wk )
{
  int i;
  int max = (type_is_normal(wk->type) ? TRAINER_CNT_NORMAL : TRAINER_CNT_MULTI ); 

  for( i=0; i<max; i++ )
  {
    BALL_UNIT_SetStart( &wk->trainer_unit[i].ball );
  }
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
  prm.pltt_src_ofs = 0;
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
static void OBJ_Exit( COMM_BTL_DEMO_OBJ_WORK* wk )
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
  G2_SetBlendAlpha( GX_PLANEMASK_BG0, GX_PLANEMASK_OBJ|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_BG0, 16, 0 );
//  G2_SetBlendAlpha( GX_PLANEMASK_BG0, GX_PLANEMASK_BG3|GX_PLANEMASK_BG2|GX_PLANEMASK_BG1|GX_PLANEMASK_BG0, 0, 0 );
  
  // メンバ初期化
  g3d->graphic = graphic;
  g3d->heapID = heapID;

  // 3D管理ユーティリティーの生成
  g3d->g3d_util = GFL_G3D_UTIL_Create( 16, 20, heapID );

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
    GFL_PTC_SetResource( g3d->ptc, res, TRUE, NULL );
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

    HOSAKA_Printf("obj_count=%d resCount=%d resIdx=%d \n",
      GFL_G3D_UTIL_GetObjCount(g3d->g3d_util),
      GFL_G3D_UTIL_GetUnitResCount(g3d->g3d_util,0),
      GFL_G3D_UTIL_GetUnitResIdx(g3d->g3d_util, 0)
    );

    //@TODO 男女からテクスチャのパレットを設定
    {
      GFL_G3D_RND* rnd = GFL_G3D_OBJECT_GetG3Drnd(obj);
//    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );
//    GFL_G3D_RES* tex = GFL_G3D_RENDER_GetG3DresTex(rnd);
//    GFL_G3D_RES* mdl = GFL_G3D_RENDER_GetG3DresMdl(rnd);

//    GFL_G3D_UTIL_SetObjAnotherUnitAnime

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
      static int speed = 1;
#ifdef DEBUG_ONLY_FOR_genya_hosaka
      if( PAD_BUTTON_SELECT & GFL_UI_KEY_GetTrg() )
      {
        speed ^= 1;
      }
#endif
//    GFL_G3D_OBJECT_SetAnimeFrame( obj, i, &frame );
      is_loop = GFL_G3D_OBJECT_LoopAnimeFrame( obj, i, speed * FX32_ONE );
    }

    //3D描画
    COMM_BTL_DEMO_GRAPHIC_3D_StartDraw( g3d->graphic );
    
    // 描画
    GFL_G3D_DRAW_DrawObject( obj, &status );
    
    COMM_BTL_DEMO_GRAPHIC_3D_EndDraw( g3d->graphic );
  }

  return is_loop;
}


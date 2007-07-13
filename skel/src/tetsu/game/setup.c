//============================================================================================
/**
 * @file	setup.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "textprint.h"
#include "gfl_use.h"

#include "main.h"
#include "testmode.h"

#include "double3Ddisp.h"

#include "setup.h"
#include "src/sample_graphic/haruka.naix"

//============================================================================================
//
//
//	メインコントロール
//
//
//============================================================================================
#define BACKGROUND_COL	(GX_RGB(20,20,31))		//背景色
#define DTCM_SIZE		(0x1000)				//DTCMエリアのサイズ
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _GAME_SYSTEM {
	GFL_G3D_UTIL*			g3Dutil;		//g3Dutil Lib ハンドル
	GFL_G3D_SCENE*			g3Dscene;		//g3Dscene Lib ハンドル
	GFL_G3D_CAMERA*			g3Dcamera[2];	//g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET*		g3Dlightset[2];	//g3Dlight Lib ハンドル
	GFL_BMPWIN*				bitmapWin[32];	//bitmapWin Lib ハンドル
	GFL_PTC_PTR				ptc;			//パーティクルLibハンドル
	u8						spa_work[PARTICLE_LIB_HEAP_SIZE];	//パーティクル使用ワーク
	GFL_TCB*				dbl3DdispVintr;	//vIntrTaskハンドル

	SCENE_MAP*				sceneMap;		//３Ｄマップ設定ハンドル
	SCENE_ACT*				sceneAct;		//３Ｄオブジェクト設定ハンドル

	HEAPID					heapID;
};

//------------------------------------------------------------------
/**
 * @brief	３Ｄグラフィック環境データ
 */
//------------------------------------------------------------------
//カメラ初期設定データ
static const VecFx32 camera0Target	= { 0, 0, 0 };
static const VecFx32 camera0Pos	= { 0, (FX32_ONE * 100), (FX32_ONE * 180) };
static const VecFx32 camera1Pos	= { (FX32_ONE * 100 ), (FX32_ONE * 100), (FX32_ONE * 180) };

//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
	{ 0, {{ 0, 0, -(FX16_ONE-1) }, 0x7fff } },
};
static const GFL_G3D_LIGHT_DATA light1Tbl[] = {
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, 0x001f } },
};
static const GFL_G3D_LIGHTSET_SETUP light0Setup = { light0Tbl, NELEMS(light0Tbl) };
static const GFL_G3D_LIGHTSET_SETUP light1Setup = { light1Tbl, NELEMS(light1Tbl) };

//------------------------------------------------------------------
/**
 * @brief	３Ｄリソースデータ
 */
//------------------------------------------------------------------
#define G3D_FRM_PRI		(1)			//３Ｄ面の描画プライオリティー
#define G3D_OBJ_COUNT	(256)		//g3Dsceneで使用するsceneObjの最大設定可能数
#define G3D_OBJWORK_SZ	(64)		//g3Dsceneで使用するsceneObjのワークサイズ
#define G3D_ACC_COUNT	(32)		//g3Dsceneで使用するsceneObjAccesaryの最大設定可能数

static const char g3DarcPath2[] = {"src/sample_graphic/haruka.narc"};

//３Ｄグラフィックリソーステーブル
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ (u32)g3DarcPath2, NARC_haruka_test_floor2_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_stay_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_walk_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_run_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_attack_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_shoot_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_spell_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_sit_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_human2_hit_nsbca,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_sword_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_shield_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_bow_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_staff_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_test_wall_nsbmd,  GFL_G3D_UTIL_RESPATH },
};

//---------------------
//３Ｄオブジェクトアニメーション定義テーブル
static const GFL_G3D_UTIL_ANM g3Dutil_Human2AnmTbl[] = {
	{ G3DRES_HUMAN2_STAY_BCA, 0 },
	{ G3DRES_HUMAN2_WALK_BCA, 0 },
	{ G3DRES_HUMAN2_RUN_BCA, 0 },
	{ G3DRES_HUMAN2_ATTACK_BCA, 0 },
	{ G3DRES_HUMAN2_SHOOT_BCA, 0 },
	{ G3DRES_HUMAN2_SPELL_BCA, 0 },
	{ G3DRES_HUMAN2_SIT_BCA, 0 },
	{ G3DRES_HUMAN2_HIT_BCA, 0 },
};

//---------------------
//３Ｄオブジェクト定義テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_MAP_FLOOR, 0, G3DRES_MAP_FLOOR, NULL, 0 },
	// プレーヤー数 (=PLAYER_SETUP_NUM)
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	{ G3DRES_HUMAN2_BMD, 0, G3DRES_HUMAN2_BMD, g3Dutil_Human2AnmTbl, NELEMS(g3Dutil_Human2AnmTbl)},
	// 
	{ G3DRES_ACCE_SWORD, 0, G3DRES_ACCE_SWORD, NULL, 0 },
	{ G3DRES_ACCE_SHIELD, 0, G3DRES_ACCE_SHIELD, NULL, 0 },
	{ G3DRES_ACCE_BOW, 0, G3DRES_ACCE_BOW, NULL, 0 },
	{ G3DRES_ACCE_STAFF, 0, G3DRES_ACCE_STAFF, NULL, 0 },
	{ G3DRES_EFFECT_WALL, 0, G3DRES_EFFECT_WALL, NULL, 0 },
};

//---------------------
//g3Dscene 初期設定テーブルデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl, NELEMS(g3Dutil_resTbl),
	g3Dutil_objTbl, NELEMS(g3Dutil_objTbl),
};

//パーティクル用アーカイブテーブル（sample）
static	const	char	*GraphicFileTable[]={
	"src/sample_graphic/spa.narc",
};

//------------------------------------------------------------------
/**
 * @brief		２Ｄリソースデータ
 */
//------------------------------------------------------------------
#define TEXT_FRM			(GFL_BG_FRAME3_M)
#define TEXT_FRM_PRI		(0)
#define TEXT_PLTT			(15)
#define G2D_FONT_COL		(0x7fff)
#define G2D_FONTSELECT_COL	(0x001f)

static const char font_path[] = {"src/gfl_graphic/gfl_font.dat"};

static const GFL_BG_BGCNT_HEADER textBGcont = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

static const u16 defaultFontPalette[] = {
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0),
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0),
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0),
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB(31,31,31),
};

#define PLAYER_STATUS_WIN_SX	(14)
#define PLAYER_STATUS_WIN_SY	(3)

//bmp, writex, writey, spacex, spacey, colorF, colorB, mode
static const GFL_TEXT_PRINTPARAM playerStatusWinPrintParam = {
	NULL, 1, 1, 1, 1, 15, 1, GFL_TEXT_WRITE_16
};
static const GFL_TEXT_PRINTPARAM msgWinPrintParam = {
	NULL, 1, 1, 1, 1, 1, 15, GFL_TEXT_WRITE_16
};

static const u16 bitmapWinList[][4] = {
	//ステータスウインドウ
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 0 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 0 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 1 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 0 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 0 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 1 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 1 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 1 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 0 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 2 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 1 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 2 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 0 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 3 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
	{ ( PLAYER_STATUS_WIN_SX + 2 ) * 1 + 1, ( PLAYER_STATUS_WIN_SY + 1 ) * 3 + 1 ,
		PLAYER_STATUS_WIN_SX, PLAYER_STATUS_WIN_SY },
};

//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
//ＢＧ設定関数
static void	bg_init( GAME_SYSTEM* gs );
static void	bg_exit( GAME_SYSTEM* gs );
//３Ｄ関数
static void g3d_load( GAME_SYSTEM* gs );
static void g3d_control( GAME_SYSTEM* gs );
static void g3d_draw( GAME_SYSTEM* gs );
static void	g3d_unload( GAME_SYSTEM* gs );
//２Ｄ関数
static void g2d_load( GAME_SYSTEM* gs );
static void g2d_control( GAME_SYSTEM* gs );
static void g2d_draw( GAME_SYSTEM* gs );
static void	g2d_unload( GAME_SYSTEM* gs );

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
GAME_SYSTEM*	SetupGameSystem( HEAPID heapID )
{
	GAME_SYSTEM*	gs = GFL_HEAP_AllocMemory( heapID, sizeof(GAME_SYSTEM) );
	gs->heapID = heapID;

	//乱数初期化
	GFL_STD_MtRandInit(0);

	//ARCシステム初期化
	GFL_ARC_Init( &GraphicFileTable[0], 1 );
	//BG初期化
	bg_init( gs );
	//３Ｄデータのロード
	g3d_load( gs );
	//２Ｄデータのロード
	g2d_load( gs );
	//両面３Ｄ用vIntrTask設定
	gs->dbl3DdispVintr = GFUser_VIntr_CreateTCB( GFL_G3D_DOUBLE3D_VblankIntrTCB, NULL, 0 );

	return gs;
}

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
void	RemoveGameSystem( GAME_SYSTEM* gs )
{
	g3d_unload( gs );	//３Ｄデータ破棄
	g2d_unload( gs );	//２Ｄデータ破棄

	GFL_TCB_DeleteTask( gs->dbl3DdispVintr );
	bg_exit( gs );

	GFL_ARC_Exit();

	GFL_HEAP_FreeMemory( gs );
}

//------------------------------------------------------------------
/**
 * @brief	システムメイン関数（前）
 */
//------------------------------------------------------------------
void	MainGameSystemPref( GAME_SYSTEM* gs )
{
	g2d_control( gs );
	g3d_control( gs );
}

//------------------------------------------------------------------
/**
 * @brief	システムメイン関数（後）
 */
//------------------------------------------------------------------
void	MainGameSystemAfter( GAME_SYSTEM* gs )
{
	g2d_draw( gs );
	g3d_draw( gs );
}

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void G3DsysSetup( void );
static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};	

static void	bg_init( GAME_SYSTEM* gs )
{
	//ＢＧシステム起動
	GFL_BG_Init( gs->heapID );

	//ＶＲＡＭ設定
	GX_SetBankForTex(GX_VRAM_TEX_01_AB);
	GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G); 

	//背景色パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( gs->heapID, 16*2 );
		plt[0] = BACKGROUND_COL;
		GFL_BG_LoadPalette( GFL_BG_FRAME0_M, plt, 16*2, 0 );	//メイン画面の背景色転送
		GFL_BG_LoadPalette( GFL_BG_FRAME0_S, plt, 16*2, 0 );	//サブ画面の背景色転送
		GFL_HEAP_FreeMemory( plt );
	}
	//ＢＧモード設定
	GFL_BG_InitBG( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( TEXT_FRM, &textBGcont, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( TEXT_FRM, TEXT_FRM_PRI );
	GFL_BG_SetVisible( TEXT_FRM, VISIBLE_ON );

	//ビットマップウインドウシステムの起動
	GFL_BMPWIN_Init( gs->heapID );

	//３Ｄシステム起動
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, gs->heapID, G3DsysSetup );
	GFL_G3D_DOUBLE3D_Init( gs->heapID );	//両面３Ｄ用の初期化
	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( GAME_SYSTEM* gs )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_G3D_DOUBLE3D_Exit();
	GFL_G3D_Exit();
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl( TEXT_FRM );
	GFL_BG_Exit();
}

// ３Ｄセットアップコールバック
static void G3DsysSetup( void )
{
	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_TOON );
	G3X_AntiAlias( TRUE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend(TRUE);
	G3X_EdgeMarking( FALSE );
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );
}


//------------------------------------------------------------------
/**
 * @brief		３Ｄデータロード
 */
//------------------------------------------------------------------
//作成
static void g3d_load( GAME_SYSTEM* gs )
{
	//配置物設定

	//g3Dutilを使用し配列管理をする
	gs->g3Dutil = GFL_G3D_UTIL_Create( &g3Dutil_setup, gs->heapID );
	{
		//アニメーションの初期有効設定(0番のアニメーションを有効にする)
		int i;
		for( i=0; i<PLAYER_SETUP_NUM; i++ ){
			GFL_G3D_OBJECT_EnableAnime
			( GFL_G3D_UTIL_GetObjHandle( gs->g3Dutil, G3DOBJ_HUMAN1+i), HUMAN2_ANM_STAY );
		}
	}

	//g3Dsceneを使用し管理をする
	gs->g3Dscene = GFL_G3D_SCENE_Create( gs->g3Dutil, 
						G3D_OBJ_COUNT, G3D_OBJWORK_SZ, G3D_ACC_COUNT, TRUE, gs->heapID );

	gs->sceneMap = Create3Dmap( gs->g3Dscene, gs->heapID );
	gs->sceneAct = Create3Dact( gs->g3Dscene, gs->heapID );

	//カメラ作成
	{
		fx32 far = 4096 << FX32_SHIFT;	//メインカメラはdefault設定よりfarを伸ばしたいので再設定

		gs->g3Dcamera[MAINCAMERA_ID] = GFL_G3D_CAMERA_CreateDefault
									( &camera0Pos, &camera0Target, gs->heapID );
		gs->g3Dcamera[SUBCAMERA_ID] = GFL_G3D_CAMERA_CreateDefault
									( &camera1Pos, &camera0Target, gs->heapID );
		GFL_G3D_CAMERA_SetFar( gs->g3Dcamera[MAINCAMERA_ID], &far );
	}

	//ライト作成
	gs->g3Dlightset[MAINLIGHT_ID] = GFL_G3D_LIGHT_Create( &light0Setup, gs->heapID );
	gs->g3Dlightset[SUBLIGHT_ID] = GFL_G3D_LIGHT_Create( &light1Setup, gs->heapID );//未使用

	//カメラライト0反映
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera[MAINCAMERA_ID] );
	GFL_G3D_LIGHT_Switching( gs->g3Dlightset[MAINLIGHT_ID] );

	//パーティクルリソース読み込み
	gs->ptc=GFL_PTC_Create( gs->spa_work, PARTICLE_LIB_HEAP_SIZE,TRUE, gs->heapID );
	GFL_PTC_SetResource( gs->ptc, GFL_PTC_LoadArcResource( 0, 0, gs->heapID ), TRUE, NULL );
}
	
//動作
static void g3d_control( GAME_SYSTEM* gs )
{
	GFL_G3D_SCENE_Main( gs->g3Dscene ); 
}

//描画
static void g3d_draw( GAME_SYSTEM* gs )
{
	//フラグによって描画を切り替える
	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		GFL_G3D_CAMERA_Switching( gs->g3Dcamera[MAINCAMERA_ID] );
		GFL_G3D_SCENE_SetDrawParticleSW( gs->g3Dscene, TRUE );
	} else {
		GFL_G3D_CAMERA_Switching( gs->g3Dcamera[SUBCAMERA_ID] );
		GFL_G3D_SCENE_SetDrawParticleSW( gs->g3Dscene, FALSE );
	}
	GFL_G3D_SCENE_Draw( gs->g3Dscene );  
	GFL_G3D_DOUBLE3D_SetSwapFlag();
}

//破棄
static void g3d_unload( GAME_SYSTEM* gs )
{
	GFL_PTC_Delete(gs->ptc);

	GFL_G3D_LIGHT_Delete( gs->g3Dlightset[SUBLIGHT_ID] );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera[SUBCAMERA_ID] );
	GFL_G3D_LIGHT_Delete( gs->g3Dlightset[MAINLIGHT_ID] );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera[MAINCAMERA_ID] );

	Delete3Dact( gs->sceneAct );
	Delete3Dmap( gs->sceneMap );

	GFL_G3D_SCENE_Delete( gs->g3Dscene );  

	GFL_G3D_UTIL_Delete( gs->g3Dutil );
}
	
//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void	g2d_load( GAME_SYSTEM* gs )
{
	int i;

	//フォント読み込み
	GFL_TEXT_CreateSystem( font_path );
	//パレット作成＆転送
	GFL_BG_LoadPalette( TEXT_FRM, (void*)defaultFontPalette, 16*2, TEXT_PLTT * 16*2 );

	//作成
	for( i=0; i<NELEMS(bitmapWinList); i++ ){
		//ビットマップウインドウ作成

		gs->bitmapWin[i] = GFL_BMPWIN_Create(	TEXT_FRM, 
												bitmapWinList[i][0], bitmapWinList[i][1], 
												bitmapWinList[i][2], bitmapWinList[i][3], 
												TEXT_PLTT, GFL_BG_CHRAREA_GET_B );
		//テキスト背景塗りつぶし
		GFL_BMP_Fill(	GFL_BMPWIN_GetBmp( gs->bitmapWin[i] ), 0, 0, 
						bitmapWinList[i][2] * 8, bitmapWinList[i][3] * 8, 1 );

		//ビットマップキャラクターをアップデート
		GFL_BMPWIN_TransVramCharacter( gs->bitmapWin[i] );
	}
}

static void g2d_control( GAME_SYSTEM* gs )
{
}

static void g2d_draw( GAME_SYSTEM* gs )
{
	int	i;

	GFL_BG_ClearScreen( TEXT_FRM );

	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		for( i=0; i<NELEMS(bitmapWinList); i++ ){
			GFL_BMPWIN_MakeScreen( gs->bitmapWin[i] );
		}
	} else {
	}
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void	g2d_unload( GAME_SYSTEM* gs )
{
	int i;

	//破棄
	for( i=0; i<NELEMS(bitmapWinList); i++ ){
		GFL_BMPWIN_Delete( gs->bitmapWin[i] );
	}
	GFL_TEXT_DeleteSystem();
}

//------------------------------------------------------------------
/**
 * @brief		ハンドル取得
 */
//------------------------------------------------------------------
GFL_G3D_UTIL*		Get_GS_G3Dutil( GAME_SYSTEM* gs )
{
	return gs->g3Dutil;
}

GFL_G3D_SCENE*		Get_GS_G3Dscene( GAME_SYSTEM* gs )
{
	return gs->g3Dscene;
}

GFL_G3D_CAMERA*		Get_GS_G3Dcamera( GAME_SYSTEM* gs, int cameraID )
{
	return gs->g3Dcamera[ cameraID ];
}

GFL_G3D_LIGHTSET*	Get_GS_G3Dlight( GAME_SYSTEM* gs, int lightID )
{
	return gs->g3Dlightset[ lightID ];
}

GFL_BMPWIN*			Get_GS_BmpWin( GAME_SYSTEM* gs, int bmpwinID )
{
	return gs->bitmapWin[ bmpwinID ];
}

GFL_PTC_PTR			Get_GS_Perticle( GAME_SYSTEM* gs )
{
	return gs->ptc;
}

SCENE_MAP*			Get_GS_SceneMap( GAME_SYSTEM* gs )
{
	return gs->sceneMap;
}

SCENE_ACT*			Get_GS_SceneAct( GAME_SYSTEM* gs )
{
	return gs->sceneAct;
}

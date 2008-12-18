//============================================================================================
/**
 * @file	setup.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include <gflib.h>
#include <textprint.h>

#include "system\gfl_use.h"
#include "system/main.h"
//#include "testmode.h"

#include "double3Ddisp.h"

#include "setup.h"

#include "arc_def.h"

//============================================================================================
//
//
//	メインコントロール
//
//
//============================================================================================
#define BACKGROUND_COL	(GX_RGB(31,31,31))		//背景色
#define DTCM_SIZE		(0x1000)				//DTCMエリアのサイズ
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
#define BITMAPWIN_COUNT		(32)

struct _GAME_SYSTEM {
	GFL_G3D_UTIL*			g3Dutil;		//g3Dutil Lib ハンドル
	u16						g3DutilUnitIdx;	//g3Dutil Unitインデックス
	GFL_G3D_SCENE*			g3Dscene;		//g3Dscene Lib ハンドル
	GFL_G3D_CAMERA*			g3Dcamera[2];	//g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET*		g3Dlightset[2];	//g3Dlight Lib ハンドル
	GFL_TCB*				dbl3DdispVintr;	//vIntrTaskハンドル

	GFL_PTC_PTR				ptc;			//パーティクルLibハンドル
	u8						spa_work[PARTICLE_LIB_HEAP_SIZE];	//パーティクル使用ワーク

	GFL_BMPWIN*				bitmapWin[BITMAPWIN_COUNT];	//bitmapWin Lib ハンドル

	u32						clactRes[64][2];//clact リソースINDEX
	u32						clactResCount;	//clact リソース数
	GFL_CLUNIT*				clactUnit[8];	//clact ユニット
	GFL_TCB*				g2dVintr;		//2D用vIntrTaskハンドル
	GFL_TCB*				g3dVintr;		//3D用vIntrTaskハンドル

	SCENE_ACTSYS*			sceneActSys;	//３Ｄアクターシステム設定ハンドル
	SCENE_MAP*				sceneMap;		//３Ｄマップ設定ハンドル
	GFL_BBDACT_SYS*			bbdActSys;		//ビルボードアクトシステム設定ハンドル

	HEAPID					heapID;
};

//------------------------------------------------------------------
/**
 * @brief	ディスプレイ環境データ
 */
//------------------------------------------------------------------
///ＶＲＡＭバンク設定構造体
static const GFL_DISP_VRAM dispVram = {
	GX_VRAM_BG_16_F,				//メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE,			//メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_128_C,			//サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_16_I,			//サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_01_AB,				//テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_0_G,			//テクスチャパレットスロットに割り当て
	GX_OBJVRAMMODE_CHAR_1D_64K,		// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

//------------------------------------------------------------------
/**
 * @brief	アーカイブテーブル
 */
//------------------------------------------------------------------
#include "test_graphic/test3d.naix"
#include "test_graphic/mapobj.naix"

#if 0
static	const	char	*GraphicFileTable[]={
	"test_graphic/test3d.narc",
	"test_graphic/mapobj.narc",
	"test_graphic/spaEffect.narc",
	"test_graphic/fld_act.narc",
	"test_graphic/fld_map.narc",
	"test_graphic/build_model.narc",
};
#endif

//------------------------------------------------------------------
/**
 * @brief	３Ｄグラフィック環境データ
 */
//------------------------------------------------------------------
//カメラ初期設定データ
static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 camera0Pos	= { 0, (FX32_ONE * 100), (FX32_ONE * 180) };
static const VecFx32 camera1Pos	= { (FX32_ONE * 100 ), (FX32_ONE * 100), (FX32_ONE * 180) };

//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
//	{ 1, {{ (FX16_ONE-1)/2, (FX16_ONE-1)/2, (FX16_ONE-1)/2 }, GX_RGB(31,0,0) } },
//	{ 2, {{ (FX16_ONE-1)/2, (FX16_ONE-1)/2, (FX16_ONE-1)/2 }, GX_RGB(0,31,0) } },
//	{ 3, {{ (FX16_ONE-1)/2, (FX16_ONE-1)/2, (FX16_ONE-1)/2 }, GX_RGB(0,0,31) } },
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
#define G3D_FRM_PRI			(1)			//３Ｄ面の描画プライオリティー
#define G3D_UTIL_RESCOUNT	(512)		//g3Dutilで使用するリソースの最大設定可能数
#define G3D_UTIL_OBJCOUNT	(128)		//g3Dutilで使用するオブジェクトの最大設定可能数
#define G3D_SCENE_OBJCOUNT	(256)		//g3Dsceneで使用するsceneObjの最大設定可能数
#define G3D_OBJWORK_SZ		(64)		//g3Dsceneで使用するsceneObjのワークサイズ
#define G3D_ACC_COUNT		(32)		//g3Dsceneで使用するsceneObjAccesaryの最大設定可能数
#define G3D_BBDACT_RESMAX	(64)		//billboardActで使用するリソースの最大設定可能数
#define G3D_BBDACT_ACTMAX	(256)		//billboardActで使用するオブジェクトの最大設定可能数

//------------------------------------------------------------------
/**
 * @brief		２Ｄリソースデータ
 */
//------------------------------------------------------------------
static const GFL_BG_BGCNT_HEADER playiconBGcont = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER textBGcont = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER maskBGcont = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

static const u16 defaultFontPalette[] = {
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0),
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0),
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0),
	GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB( 0, 0, 0), GX_RGB(31,31,31),
};

#define GAME_STATUS_WIN_PX		(18)
#define GAME_STATUS_WIN_PY		(0)
#define GAME_STATUS_WIN_SX		(14)
#define GAME_STATUS_WIN_SY		(18)
#define GAME_MSG_WIN_PX			(0)
#define GAME_MSG_WIN_PY			(18)
#define GAME_MSG_WIN_SX			(32)
#define GAME_MSG_WIN_SY			(6)
#define GAME_MAP_WIN_PX			(0)
#define GAME_MAP_WIN_PY			(0)
#define GAME_MAP_WIN_SX			(18)
#define GAME_MAP_WIN_SY			(18)

//bmp, writex, writey, spacex, spacey, colorF, colorB, mode
static const GFL_TEXT_PRINTPARAM playerStatusWinPrintParam = {
	NULL, 1, 1, 1, 1, 15, 1, GFL_TEXT_WRITE_16
};
static const GFL_TEXT_PRINTPARAM msgWinPrintParam = {
	NULL, 1, 1, 1, 1, 1, 15, GFL_TEXT_WRITE_16
};

static const u16 textBitmapWinList[][5] = {
	//ステータスウインドウ
	{ GAME_STATUS_WIN_PX, GAME_STATUS_WIN_PY, GAME_STATUS_WIN_SX, GAME_STATUS_WIN_SY, TEXT_PLTT },
	//メッセージウインドウ
	{ GAME_MSG_WIN_PX, GAME_MSG_WIN_PY, GAME_MSG_WIN_SX, GAME_MSG_WIN_SY, TEXT_PLTT },
	//マップウインドウ
	{ GAME_MAP_WIN_PX, GAME_MAP_WIN_PY, GAME_MAP_WIN_SX, GAME_MAP_WIN_SY, MAP_PLTT },
};

static const u16 maskBitmapWinList[][5] = {
	//マップウインドウ
	{ GAME_MAP_WIN_PX+1, GAME_MAP_WIN_PY+1, GAME_MAP_WIN_SX-2, GAME_MAP_WIN_SY-2, MASK_PLTT },
};

static const GFL_CLSYS_INIT clactIni = {
	0,0,	// メイン　サーフェースの左上座標
	0,1000,	// サブ　サーフェースの左上座標
	0,128,	// メイン画面OAM管理開始位置、管理数
	0,128,	// サブ画面OAM管理開始位置、管理数
	64,		// セルVram転送管理数
};

static const GFL_OBJGRP_INIT_PARAM objgrpIni = {
	64,			///< 登録できるキャラデータ数
	64,			///< 登録できるパレットデータ数
	64,			///< 登録できるセルアニメパターン数
	16,			///< 登録できるマルチセルアニメパターン数（※現状未対応）
};

enum {
	CLACT_RESLOAD_END = 0xffffffff,
	CLACT_RESLOAD_CGX = 0xfffffffe,
	CLACT_RESLOAD_PLT = 0xfffffffd,
	CLACT_RESLOAD_CEL = 0xfffffffc,
	CLACT_RESLOAD_CGX_TRANS = 0xfffffffb,
	CLACT_RESLOAD_CGX_TRANS_LOOP = 0xfffffffa,
};

static const u32 clactResList[] = {
	//マップアイコン
	CLACT_RESLOAD_CGX, NARC_mapobj_icon_NCGR, (u32)GFL_VRAM_2D_SUB,
	CLACT_RESLOAD_PLT, NARC_mapobj_icon_NCLR, (u32)GFL_VRAM_2D_SUB, PLTT_DATSIZ * 0,
	CLACT_RESLOAD_PLT, NARC_mapobj_icon2_NCLR, (u32)GFL_VRAM_2D_SUB, PLTT_DATSIZ * 1,
	CLACT_RESLOAD_CEL, NARC_mapobj_icon_NCER, NARC_mapobj_icon_NANR,
	//ステータスアイコン
	CLACT_RESLOAD_CEL, NARC_mapobj_status_NCER, NARC_mapobj_status_NANR,
	CLACT_RESLOAD_CGX_TRANS_LOOP, NARC_mapobj_status_NCGR, (u32)GFL_VRAM_2D_MAIN, STATUS_SETUP_NUM,
	CLACT_RESLOAD_PLT, NARC_mapobj_status_NCLR, (u32)GFL_VRAM_2D_MAIN, PLTT_DATSIZ * 0,
	//終了コマンド
	CLACT_RESLOAD_END,
};

static const u32 clactUnitList[] = {
	CLACT_WKSIZ_MAPOBJ,	//マップアイコン
	CLACT_WKSIZ_STATUS,	//ステータスアイコン
};

static const GXRgb edgeColorTable[8] = {
	GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),
	GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),
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
static void	g2d_vblank( GFL_TCB* tcb, void* work );
static void	g3d_vblank( GFL_TCB* tcb, void* work );
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );

static void* GetScrnData_for_NitroScrnData( void* scrnFile );
static void* GetCharData_for_NitroCharData( void* charFile );
static void* GetPlttData_for_NitroPlttData( void* plttFile );
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
//	GFL_ARC_Init( &GraphicFileTable[0], NELEMS(GraphicFileTable) );	gfl_use.cで1回だけ初期化に変更

	//VRAMクリア
	GFL_DISP_ClearVRAM( GX_VRAM_D );
	//VRAM設定
	GFL_DISP_SetBank( &dispVram );

	//BG初期化
	bg_init( gs );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

	//３Ｄデータのロード
	g3d_load( gs );
	gs->g3dVintr = GFUser_VIntr_CreateTCB( g3d_vblank, (void*)gs, 0 );
	//２Ｄデータのロード
	g2d_load( gs );
	gs->g2dVintr = GFUser_VIntr_CreateTCB( g2d_vblank, (void*)gs, 0 );

	return gs;
}

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
void	RemoveGameSystem( GAME_SYSTEM* gs )
{
	GFL_TCB_DeleteTask( gs->g2dVintr );
	g2d_unload( gs );	//２Ｄデータ破棄
	GFL_TCB_DeleteTask( gs->g3dVintr );
	g3d_unload( gs );	//３Ｄデータ破棄

	bg_exit( gs );
//	GFL_ARC_Exit();

	GFL_HEAP_FreeMemory( gs );
}

//------------------------------------------------------------------
/**
 * @brief	システムメイン関数
 */
//------------------------------------------------------------------
void	MainGameSystem( GAME_SYSTEM* gs )
{
	g2d_control( gs );
	g2d_draw( gs );
	g3d_control( gs );
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

	//背景色パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( gs->heapID, 16*2 );
		plt[0] = BACKGROUND_COL;
		GFL_BG_LoadPalette( GFL_BG_FRAME0_M, plt, 16*2, 0 );	//メイン画面の背景色転送
		GFL_BG_LoadPalette( GFL_BG_FRAME0_S, plt, 16*2, 0 );	//サブ画面の背景色転送
		GFL_HEAP_FreeMemory( plt );
	}
	//ＢＧモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( PLAYICON_FRM, &playiconBGcont, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( PLAYICON_FRM, PLAYICON_FRM_PRI );
	GFL_BG_SetVisible( PLAYICON_FRM, VISIBLE_ON );
	GFL_BG_SetBGControl( TEXT_FRM, &textBGcont, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( TEXT_FRM, TEXT_FRM_PRI );
	GFL_BG_SetVisible( TEXT_FRM, VISIBLE_ON );
	GFL_BG_SetBGControl( MASK_FRM, &maskBGcont, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( MASK_FRM, MASK_FRM_PRI );
	GFL_BG_SetVisible( MASK_FRM, VISIBLE_ON );

	G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 16, 8 );

	//ビットマップウインドウシステムの起動
	GFL_BMPWIN_Init( gs->heapID );

	//３Ｄシステム起動
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, gs->heapID, G3DsysSetup );
	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( GAME_SYSTEM* gs )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );

	GFL_G3D_Exit();
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl( MASK_FRM );
	GFL_BG_FreeBGControl( TEXT_FRM );
	GFL_BG_FreeBGControl( PLAYICON_FRM );
	GFL_BG_Exit();
}

// ３Ｄセットアップコールバック
static void G3DsysSetup( void )
{
	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_TOON );
	G3X_AntiAlias( TRUE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );

	// フォグセットアップ
    {
        u32     fog_table[8];
        int     i;

        G3X_SetFog(TRUE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x0800, 0x0e00 );

		G3X_SetFogColor(BACKGROUND_COL, 0);

        for ( i=0; i<8; i++ ){
            fog_table[i] = (u32)(((i*16)<<0) | ((i*16+4)<<8) | ((i*16+8)<<16) | ((i*16+12)<<24));
        }
        G3X_SetFogTable(&fog_table[0]);
	}

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

	G3X_SetEdgeColorTable( edgeColorTable ); 
	G3X_EdgeMarking( TRUE );

	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );
}


static SCENE_ACT* testSceneAct[8];
#define	NPC_SETNUM	(4)
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
	gs->g3Dutil = GFL_G3D_UTIL_Create( G3D_UTIL_RESCOUNT, G3D_UTIL_OBJCOUNT, gs->heapID );
	//g3Dsceneを使用し管理をする
	gs->g3Dscene = GFL_G3D_SCENE_Create( gs->g3Dutil, 
						G3D_SCENE_OBJCOUNT, G3D_OBJWORK_SZ, G3D_ACC_COUNT, TRUE, gs->heapID );

	//カメラ作成
	{
		//fx32 far = 4096 << FX32_SHIFT;	//メインカメラはdefault設定よりfarを伸ばしたいので再設定
		fx32 far = 768 << FX32_SHIFT;	//メインカメラはdefault設定よりfarを伸ばしたいので再設定

		gs->g3Dcamera[MAINCAMERA_ID] = GFL_G3D_CAMERA_CreateDefault
									( &camera0Pos, &cameraTarget, gs->heapID );
		gs->g3Dcamera[SUBCAMERA_ID] = GFL_G3D_CAMERA_CreateDefault
									( &camera1Pos, &cameraTarget, gs->heapID );
		GFL_G3D_CAMERA_SetFar( gs->g3Dcamera[MAINCAMERA_ID], &far );
	}

	//ライト作成
	gs->g3Dlightset[MAINLIGHT_ID] = GFL_G3D_LIGHT_Create( &light0Setup, gs->heapID );
	gs->g3Dlightset[SUBLIGHT_ID] = GFL_G3D_LIGHT_Create( &light1Setup, gs->heapID );//未使用

	gs->sceneActSys = Create3DactSys( gs->g3Dscene, gs->heapID );
	gs->sceneMap = Create3Dmap( gs->g3Dscene, gs->heapID );
	gs->bbdActSys = GFL_BBDACT_CreateSys
					( G3D_BBDACT_RESMAX, G3D_BBDACT_ACTMAX, g3d_trans_BBD, gs->heapID );
	//-------------------
	//ＮＰＣテスト
	{
		int i;
		VecFx32 trans;

		for( i=0; i<NPC_SETNUM; i++ ){
			testSceneAct[i] = Create3DactNPC( gs->g3Dscene, gs->heapID );

			do{
				trans.x = (GFUser_GetPublicRand( 32 ) - ( 32/2 )) * FX32_ONE*16;
				trans.y = 0;
				trans.z = (GFUser_GetPublicRand( 32 ) - ( 32/2 )) * FX32_ONE*16;
			}while( CheckGroundOutRange( gs->sceneMap, &trans ) == FALSE );
			Set3DactTrans( testSceneAct[i], &trans );
		}
	}
	//-------------------
	//カメラライト0反映
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera[MAINCAMERA_ID] );
	GFL_G3D_LIGHT_Switching( gs->g3Dlightset[MAINLIGHT_ID] );

	//パーティクルリソース読み込み
	gs->ptc=GFL_PTC_Create( gs->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, gs->heapID );
	GFL_PTC_SetResource(	gs->ptc, GFL_PTC_LoadArcResource( ARCID_EFFECT, 0, gs->heapID ), 
							TRUE, NULL );
}
	
//動作
static void g3d_control( GAME_SYSTEM* gs )
{
	GFL_G3D_SCENE_Main( gs->g3Dscene ); 
	GFL_BBDACT_Main( gs->bbdActSys );
}

//描画
static void g3d_draw( GAME_SYSTEM* gs )
{
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera[MAINCAMERA_ID] );
	GFL_G3D_SCENE_SetDrawParticleSW( gs->g3Dscene, TRUE );

	GFL_BBDACT_Draw( gs->bbdActSys, gs->g3Dcamera[MAINCAMERA_ID], gs->g3Dlightset[MAINLIGHT_ID] );
	GFL_G3D_SCENE_Draw( gs->g3Dscene );  
}

//破棄
static void g3d_unload( GAME_SYSTEM* gs )
{
	GFL_PTC_Delete(gs->ptc);

	//-------------------
	//ビルボードテスト
	//ＮＰＣテスト
	{
		int i;
		for( i=0; i<NPC_SETNUM; i++ ){
			Delete3DactNPC( testSceneAct[i] );
		}
	}
	//-------------------
	GFL_BBDACT_DeleteSys( gs->bbdActSys );
	Delete3Dmap( gs->sceneMap );
	Delete3DactSys( gs->sceneActSys );

	GFL_G3D_LIGHT_Delete( gs->g3Dlightset[SUBLIGHT_ID] );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera[SUBCAMERA_ID] );
	GFL_G3D_LIGHT_Delete( gs->g3Dlightset[MAINLIGHT_ID] );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera[MAINCAMERA_ID] );

	GFL_G3D_SCENE_Delete( gs->g3Dscene );  
	GFL_G3D_UTIL_Delete( gs->g3Dutil );
}
	
static void	g3d_vblank( GFL_TCB* tcb, void* work )
{
	GAME_SYSTEM* gs =  (GAME_SYSTEM*)work;
}

//BBD用VRAM転送関数
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz )
{
	NNS_GFD_DST_TYPE transType;

	if( type == GFL_BBDACT_TRANSTYPE_DATA ){
		transType = NNS_GFD_DST_3D_TEX_VRAM;
	} else {
		transType = NNS_GFD_DST_3D_TEX_PLTT;
	}
	NNS_GfdRegisterNewVramTransferTask( transType, dst, (void*)src, siz );
}

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void	g2d_load( GAME_SYSTEM* gs )
{
	int i,p;
	//ＢＧ関連
	{
		//パレット作成＆転送
		GFL_BG_LoadPalette( TEXT_FRM, (void*)defaultFontPalette, 
							PLTT_DATSIZ, TEXT_PLTT * PLTT_DATSIZ );

		for( i=0; i<BITMAPWIN_COUNT; i++ ){
			gs->bitmapWin[i] = NULL;
		}

		p = 0;
		//作成
		for( i=0; i<NELEMS(textBitmapWinList); i++ ){
			//ビットマップウインドウ作成

			gs->bitmapWin[p] = GFL_BMPWIN_Create(	TEXT_FRM, 
												textBitmapWinList[i][0], textBitmapWinList[i][1], 
												textBitmapWinList[i][2], textBitmapWinList[i][3], 
												textBitmapWinList[i][4], GFL_BG_CHRAREA_GET_B );
			//テキスト背景塗りつぶし
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gs->bitmapWin[p] ), 0 );
	
			//ビットマップキャラクターをアップデート
			GFL_BMPWIN_TransVramCharacter( gs->bitmapWin[p] );
			GFL_BMPWIN_MakeScreen( gs->bitmapWin[p] );
			p++;
		}
		for( i=0; i<NELEMS(maskBitmapWinList); i++ ){
			//ビットマップウインドウ作成

			gs->bitmapWin[p] = GFL_BMPWIN_Create(	MASK_FRM, 
												maskBitmapWinList[i][0], maskBitmapWinList[i][1], 
												maskBitmapWinList[i][2], maskBitmapWinList[i][3], 
												maskBitmapWinList[i][4], GFL_BG_CHRAREA_GET_B );
			//マスク面背景塗りつぶし
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gs->bitmapWin[p] ), 0 ); 
	
			//ビットマップキャラクターをアップデート
			GFL_BMPWIN_TransVramCharacter( gs->bitmapWin[p] );
			GFL_BMPWIN_MakeScreen( gs->bitmapWin[p] );
			p++;
		}
		{
			//マップ用
			void* map_pltt = GFL_ARC_LoadDataAlloc
						( ARCID_SAMPLE, NARC_haruka_gamemap_NCLR, gs->heapID );
			void* map_char = GFL_ARC_LoadDataAlloc
						( ARCID_SAMPLE, NARC_haruka_gamemap_NCGR, gs->heapID );
			void* bmp_adrs = (void*)(GFL_BMP_GetCharacterAdrs
										( GFL_BMPWIN_GetBmp( gs->bitmapWin[G2DBMPWIN_MAP] )));
	
			GFL_BG_LoadPalette( TEXT_FRM, GetPlttData_for_NitroPlttData( map_pltt ), 
								PLTT_DATSIZ, MAP_PLTT * PLTT_DATSIZ );
			GFL_STD_MemCopy32(	GetCharData_for_NitroCharData( map_char ), bmp_adrs, 
								GAME_MAP_WIN_SX * GAME_MAP_WIN_SY * 0x20 );
	
			GFL_HEAP_FreeMemory( map_char );
			GFL_HEAP_FreeMemory( map_pltt );
		}
		{
			//アイコン用
			void* playicon_pltt = GFL_ARC_LoadDataAlloc
						( ARCID_SAMPLE, NARC_haruka_playicon_NCLR, gs->heapID );
			void* playicon_char = GFL_ARC_LoadDataAlloc
						( ARCID_SAMPLE, NARC_haruka_playicon_NCGR, gs->heapID );
			void* playicon_scrn = GFL_ARC_LoadDataAlloc
						( ARCID_SAMPLE, NARC_haruka_playicon_NSCR, gs->heapID );
	
			GFL_BG_LoadPalette( PLAYICON_FRM, GetPlttData_for_NitroPlttData( playicon_pltt ), 
								PLTT_DATSIZ*2, PLAYICON_PLTT * PLTT_DATSIZ );
			GFL_BG_LoadCharacter( PLAYICON_FRM, GetCharData_for_NitroCharData( playicon_char ),
								8*16*0x20, 0 );
#if 0
			GFL_BG_LoadScreenBuffer( PLAYICON_FRM, GetScrnData_for_NitroScrnData( playicon_scrn ), 
								0x800 );
#endif
			GFL_HEAP_FreeMemory( playicon_scrn );
			GFL_HEAP_FreeMemory( playicon_char );
			GFL_HEAP_FreeMemory( playicon_pltt );
		}
	}
	GFL_BG_LoadScreenReq( PLAYICON_FRM );
	GFL_BG_LoadScreenReq( TEXT_FRM );
	GFL_BG_LoadScreenReq( MASK_FRM );

	//ＯＢＪ関連
	{
		ARCHANDLE* clactArc = GFL_ARC_OpenDataHandle( ARCID_MAPOBJ, gs->heapID );
		u32	comm;
		u32	celIDtmp;

		//システム初期化
		GFL_CLACT_Init( &clactIni, gs->heapID );
		GFL_OBJGRP_sysStart( gs->heapID, &dispVram, &objgrpIni );

		i = 0;
		gs->clactResCount = 0;	//clact リソース数

		while( clactResList[i] != CLACT_RESLOAD_END ){
			comm = clactResList[i];
			switch( comm ){ 
			case CLACT_RESLOAD_CGX:
				{
					u32 cgxID = clactResList[++i];
					GFL_VRAM_TYPE vtype = clactResList[++i];
					gs->clactRes[gs->clactResCount][0] = comm;
					gs->clactRes[gs->clactResCount][1] = GFL_OBJGRP_RegisterCGR
										( clactArc, cgxID, FALSE, vtype, gs->heapID );
					gs->clactResCount++;
				}
				break;
			case CLACT_RESLOAD_CEL:
				{
					u32 celID = clactResList[++i];
					u32 anmID = clactResList[++i];
					gs->clactRes[gs->clactResCount][0] = comm;
					gs->clactRes[gs->clactResCount][1] = GFL_OBJGRP_RegisterCellAnim
										( clactArc, celID, anmID, gs->heapID );
					celIDtmp = gs->clactRes[gs->clactResCount][1];
					gs->clactResCount++;
				}
				break;
			case CLACT_RESLOAD_PLT:
				{
					u32	palID = clactResList[++i];
					GFL_VRAM_TYPE vtype = clactResList[++i];
					u16	poffs  = clactResList[++i];
					gs->clactRes[gs->clactResCount][0] = comm;
					gs->clactRes[gs->clactResCount][1] = GFL_OBJGRP_RegisterPltt
										( clactArc, palID, vtype, poffs, gs->heapID );
					gs->clactResCount++;
				}
				break;
			case CLACT_RESLOAD_CGX_TRANS:
				{	//転送ＣＧＸ用。直前に対応セルを登録すること
					u32 cgxID = clactResList[++i];
					GFL_VRAM_TYPE vtype = clactResList[++i];
					gs->clactRes[gs->clactResCount][0] = comm;
					gs->clactRes[gs->clactResCount][1] = GFL_OBJGRP_RegisterCGR_VramTransfer
										( clactArc, cgxID, FALSE, vtype, celIDtmp, gs->heapID );
					gs->clactResCount++;
				}
				break;
			case CLACT_RESLOAD_CGX_TRANS_LOOP:
				{	//転送ＣＧＸ用。直前に対応セルを登録すること
					u32 cgxID = clactResList[++i];
					GFL_VRAM_TYPE vtype = clactResList[++i];
					int loopCount = clactResList[++i];
					int j;

					for( j=0; j<loopCount; j++ ){
						gs->clactRes[gs->clactResCount][0] = comm;
						gs->clactRes[gs->clactResCount][1] = GFL_OBJGRP_RegisterCGR_VramTransfer
											( clactArc, cgxID, FALSE, vtype, celIDtmp, gs->heapID );
						gs->clactResCount++;
					}
				}
				break;
			}
			i++;
		}
		//ＣＬＡＣＴユニット作成
		for( i=0; i<NELEMS(clactUnitList); i++ ){
			gs->clactUnit[i] = GFL_CLACT_UNIT_Create( clactUnitList[i], gs->heapID );
		}

		GFL_ARC_CloseDataHandle( clactArc );
	}
}

static void g2d_control( GAME_SYSTEM* gs )
{
	int i;

	GFL_CLACT_ClearOamBuff();
	for( i=0; i<NELEMS(clactUnitList); i++ ){
		GFL_CLACT_UNIT_Draw( gs->clactUnit[i] );
	}
}

static void g2d_draw( GAME_SYSTEM* gs )
{
	GFL_CLACT_Main();
}

static void	g2d_unload( GAME_SYSTEM* gs )
{
	int i;
	{
		for( i=0; i<NELEMS(clactUnitList); i++ ){
 			GFL_CLACT_UNIT_Delete( gs->clactUnit[i] );
		}

		for( i=0; i<gs->clactResCount; i++ ){
			switch( gs->clactRes[i][0] ){
			case CLACT_RESLOAD_CGX:
				GFL_OBJGRP_ReleaseCGR( gs->clactRes[i][1] );
				break;
			case CLACT_RESLOAD_CEL:
				GFL_OBJGRP_ReleaseCellAnim( gs->clactRes[i][1] );
				break;
			case CLACT_RESLOAD_PLT:
				GFL_OBJGRP_ReleasePltt( gs->clactRes[i][1] );
				break;
			}
		}
		GFL_OBJGRP_sysEnd();
		GFL_OBJGRP_Exit();
		GFL_CLACT_Exit();
	}
	{
		//破棄
		for( i=0; i<BITMAPWIN_COUNT; i++ ){
			if( gs->bitmapWin[i] ){
				GFL_BMPWIN_Delete( gs->bitmapWin[i] );
			}
		}
	}
}

static void	g2d_vblank( GFL_TCB* tcb, void* work )
{
	GAME_SYSTEM* gs =  (GAME_SYSTEM*)work;

	GFL_CLACT_VBlankFuncTransOnly();
}

//------------------------------------------------------------------
/**
 * @brief		NNSポインタ取得関連
 */
//------------------------------------------------------------------
static void* GetScrnData_for_NitroScrnData( void* scrnFile )
{
	NNSG2dScreenData* data;

	NNS_G2dGetUnpackedScreenData( scrnFile, &data );
	return &data->rawData[0];
}

static void* GetCharData_for_NitroCharData( void* charFile )
{
	NNSG2dCharacterData* data;

	NNS_G2dGetUnpackedBGCharacterData( charFile, &data );
	return data->pRawData;
}

static void* GetPlttData_for_NitroPlttData( void* plttFile )
{
	NNSG2dPaletteData* data;

	NNS_G2dGetUnpackedPaletteData( plttFile, &data );
	return data->pRawData;
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
	GF_ASSERT( bmpwinID < BITMAPWIN_COUNT );
	GF_ASSERT( gs->bitmapWin[ bmpwinID ] );
		
	return gs->bitmapWin[ bmpwinID ];
}

GFL_PTC_PTR			Get_GS_Perticle( GAME_SYSTEM* gs )
{
	return gs->ptc;
}

SCENE_ACTSYS*		Get_GS_SceneActSys( GAME_SYSTEM* gs )
{
	return gs->sceneActSys;
}

SCENE_MAP*			Get_GS_SceneMap( GAME_SYSTEM* gs )
{
	return gs->sceneMap;
}

GFL_BBDACT_SYS*		Get_GS_BillboardActSys( GAME_SYSTEM* gs )	
{
	return gs->bbdActSys;
}

GFL_CLUNIT*			Get_GS_ClactUnit( GAME_SYSTEM* gs, u32 unitID )
{
	return gs->clactUnit[unitID];
}

u32					Get_GS_ClactResIdx( GAME_SYSTEM* gs, u32 resID )
{
	GF_ASSERT( resID < gs->clactResCount );
	return gs->clactRes[resID][1];
}


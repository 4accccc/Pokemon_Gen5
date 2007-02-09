//============================================================================================
/**
 * @file	debug_watanabe.c
 * @brief	デバッグ＆テスト用アプリ（渡辺さん用）
 * @author	tamada
 * @date	2007.02.01
 */
//============================================================================================
#include "gflib.h"
#include "textprint.h"

#include "main.h"
#include "testmode.h"

static void	TestModeWorkSet( HEAPID heapID );
static void	TestModeWorkRelease( void );
static BOOL	TestModeControl( void );
//============================================================================================
//
//
//		プロセスの定義
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プロセスの初期化
 *
 * ここでヒープの生成や各種初期化処理を行う。
 * 初期段階ではmywkはNULLだが、GFL_PROC_AllocWorkを使用すると
 * 以降は確保したワークのアドレスとなる。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeMainProcInit
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TestModeWorkSet( GFL_HEAPID_APP );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeMainProcMain
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( TestModeControl() == TRUE ){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスの終了処理
 *
 * 単に終了した場合、親プロセスに処理が返る。
 * GFL_PROC_SysSetNextProcを呼んでおくと、終了後そのプロセスに
 * 処理が遷移する。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeMainProcEnd
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TestModeWorkRelease();

	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &TestMainProcData, NULL);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeMainProcData = {
	DebugWatanabeMainProcInit,
	DebugWatanabeMainProcMain,
	DebugWatanabeMainProcEnd,
};





//============================================================================================
//
//
//	メインコントロール
//
//
//============================================================================================
typedef struct {
	int						seq;

	u16						g3DresTblIdx;
	u16						g3DobjTblIdx;
	u16						g3DanmTblIdx;

	u16						work[16];
}TESTMODE_WORK;

TESTMODE_WORK* tetsuWork;

#include "sample_graphic/titledemo.naix"

//ＢＧ設定関数
static void	bg_init( void );
static void	bg_exit( void );
//３ＤＢＧ作成関数
static void	g3d_load( void );
static void g3d_draw( void );
static void	g3d_unload( void );
//エフェクト
static void g3d_control_effect( void );

//------------------------------------------------------------------
/**
 * @brief	データ
 */
//------------------------------------------------------------------
static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};	

#define DTCM_SIZE		(0x1000)
#define G3D_FRM_PRI		(1)

#define G3D_UTIL_RESSIZ	(64)
#define G3D_UTIL_OBJSIZ	(64)
#define G3D_UTIL_ANMSIZ	(64)

//リソース設定テーブル
enum {
	G3RES_AIR,
	G3RES_AIRANM,
	G3RES_IAR,
	G3RES_IARANM,
};

static const GFL_G3D_UTIL_RES g3DresouceTable[] = 
{
{(u32)"src/sample_graphic/titledemo.narc",NARC_titledemo_title_air_nsbmd,GFL_G3D_UTIL_RESPATH,TRUE},
{(u32)"src/sample_graphic/titledemo.narc",NARC_titledemo_title_air_nsbta,GFL_G3D_UTIL_RESPATH,0},
{(u32)"src/sample_graphic/titledemo.narc",NARC_titledemo_title_iar_nsbmd,GFL_G3D_UTIL_RESPATH,TRUE},
{(u32)"src/sample_graphic/titledemo.narc",NARC_titledemo_title_iar_nsbta,GFL_G3D_UTIL_RESPATH,0},
};

//オブジェクト設定テーブル
enum {
	G3OBJ_AIR,
	G3OBJ_IAR,
};

static const GFL_G3D_UTIL_OBJ g3DobjectTable[] = 
{
	{
		G3RES_AIR,0,G3RES_AIR,
		{ -FX32_ONE*64, 0, 0 },								//座標
		{ FX32_ONE*4/5, FX32_ONE*4/5, FX32_ONE*4/5 },		//スケール
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
		0,TRUE,
	},
	{
		G3RES_IAR,0,G3RES_IAR,
		{ FX32_ONE*64, -FX32_ONE*48, 0 },					//座標
		{ FX32_ONE*3/5, FX32_ONE*3/5, FX32_ONE*3/5 },		//スケール
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
		0,TRUE,
	},
};

//アニメーション設定テーブル
enum {
	G3ANM_AIR,
	G3ANM_IAR,
};

static const GFL_G3D_UTIL_ANM g3DanimetionTable[] = 
{
	{ G3RES_AIRANM, 0, G3OBJ_AIR, TRUE },
	{ G3RES_IARANM, 0, G3OBJ_IAR, TRUE },
};

static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 cameraPos		= { 0, (FX32_ONE * 58), (FX32_ONE * 256) };
static const VecFx32 cameraUp		= { 0, FX32_ONE, 0 };

#define cameraPerspway	( 0x0b60 )
#define cameraAspect	( FX32_ONE * 4 / 3 )
#define cameraNear		( 1 << FX32_SHIFT )
#define cameraFar		( 1024 << FX32_SHIFT )

#define g3DanmRotateSpeed	( 0x100 )
#define g3DanmFrameSpeed	( FX32_ONE )
	

//------------------------------------------------------------------
/**
 * @brief	ワークの確保と破棄
 */
//------------------------------------------------------------------
static void	TestModeWorkSet( HEAPID heapID )
{
	tetsuWork = GFL_HEAP_AllocMemoryClear( heapID, sizeof(TESTMODE_WORK) );
}

static void	TestModeWorkRelease( void )
{
	GFL_HEAP_FreeMemory( tetsuWork );
}

//------------------------------------------------------------------
/**
 * @brief	リスト選択
 */
//------------------------------------------------------------------
static BOOL	TestModeControl( void )
{
	BOOL return_flag = FALSE;
	int i;

	switch( tetsuWork->seq ){

	case 0:
		//初期化
		bg_init();
		tetsuWork->seq++;
		break;

	case 1:
		//画面作成
		g3d_load();	//３Ｄデータ作成
		tetsuWork->seq++;
		break;

	case 2:
		if( GFL_UI_KeyGetTrg() & PAD_BUTTON_R ){
			tetsuWork->seq++;
		}
		g3d_control_effect();
		g3d_draw();		//３Ｄデータ描画
		break;

	case 3:
		//終了
		g3d_unload();	//３Ｄデータ破棄
		bg_exit();
		return_flag = TRUE;
		break;
	}
	return return_flag;
}

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bg_init( void )
{
	u16 heapID = GFL_HEAPID_APP;

	//ＢＧシステム起動
	GFL_BG_sysInit( heapID );

	//ＶＲＡＭ設定
	GX_SetBankForTex(GX_VRAM_TEX_01_AB);
	GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G); 

	//ＢＧモード設定
	GFL_BG_InitBG( &bgsysHeader );

	//３Ｄシステム起動
	GFL_G3D_sysInit( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, heapID, NULL );
	GFL_BG_BGControlSet3D( G3D_FRM_PRI );
	GFL_G3D_UtilsysInit( G3D_UTIL_RESSIZ, G3D_UTIL_OBJSIZ, G3D_UTIL_ANMSIZ, heapID );  
}

static void	bg_exit( void )
{
	GFL_G3D_UtilsysExit();  
	GFL_G3D_sysExit();
	GFL_BG_sysExit();
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄデータコントロール
 */
//------------------------------------------------------------------
//作成
static void g3d_load( void )
{
	//リソース＆オブジェクト＆アニメーションを一括設定
	GFL_G3D_UtilAllLoad( g3DresouceTable, NELEMS(g3DresouceTable), &tetsuWork->g3DresTblIdx,
						 g3DobjectTable, NELEMS(g3DobjectTable), &tetsuWork->g3DobjTblIdx,
						 g3DanimetionTable, NELEMS(g3DanimetionTable), &tetsuWork->g3DanmTblIdx );

	//カメラセット
	GFL_G3D_sysProjectionSet(	GFL_G3D_PRJPERS, 
								FX_SinIdx( cameraPerspway ), FX_CosIdx( cameraPerspway ), 
								cameraAspect, 0, cameraNear, cameraFar, 0 );
	GFL_G3D_sysLookAtSet( (VecFx32*)&cameraPos, (VecFx32*)&cameraUp, (VecFx32*)&cameraTarget );

	tetsuWork->work[0] = 0;
}
	
//描画
static void g3d_draw( void )
{
	GFL_G3D_UtilDraw();
}
	
//破棄
static void g3d_unload( void )
{
	//リソース＆オブジェクト＆アニメーションを一括破棄
	GFL_G3D_UtilAllUnload(	NELEMS(g3DresouceTable), &tetsuWork->g3DresTblIdx,
							NELEMS(g3DobjectTable), &tetsuWork->g3DobjTblIdx,
							NELEMS(g3DanimetionTable), &tetsuWork->g3DanmTblIdx );
}
	
//------------------------------------------------------------------
/**
 * @brief	３Ｄ動作
 */
//------------------------------------------------------------------
static void g3d_control_effect( void )
{
	MtxFx33 rotate;
	VecFx32 rotate_tmp = { 0, 0, 0 };
	GFL_G3D_OBJ* g3Dobj;
	GFL_G3D_ANM* g3Danm;

	//回転計算
	{
		//AIRのオブジェクトハンドルを取得
		g3Dobj = GFL_G3D_UtilObjGet( tetsuWork->g3DobjTblIdx + G3OBJ_AIR );
		//AIRのアニメーションハンドルを取得
		g3Danm = GFL_G3D_UtilAnmGet( tetsuWork->g3DanmTblIdx + G3ANM_AIR );

		rotate_tmp.y = g3DanmRotateSpeed * tetsuWork->work[0];	//Ｙ軸回転
		GFL_G3D_UtilObjDrawRotateCalcYX( &rotate_tmp, &rotate );

		//AIRの回転ステータスをセット
		GFL_G3D_ObjContSetRotate( g3Dobj, &rotate );
		//アニメーションコントロール
		GFL_G3D_ObjContAnmFrameAutoLoop( g3Danm, g3DanmFrameSpeed );
	}
	{
		//IARのオブジェクトハンドルを取得
		g3Dobj = GFL_G3D_UtilObjGet( tetsuWork->g3DobjTblIdx + G3OBJ_IAR );
		//IARのアニメーションハンドルを取得
		g3Danm = GFL_G3D_UtilAnmGet( tetsuWork->g3DanmTblIdx + G3ANM_IAR );

		rotate_tmp.y = -g3DanmRotateSpeed * tetsuWork->work[0];	//Ｙ軸回転
		GFL_G3D_UtilObjDrawRotateCalcYX( &rotate_tmp, &rotate );

		//IARの回転ステータスをセット
		GFL_G3D_ObjContSetRotate( g3Dobj, &rotate );
		//アニメーションコントロール
		GFL_G3D_ObjContAnmFrameAutoLoop( g3Danm, g3DanmFrameSpeed );
	}
	tetsuWork->work[0]++;
}






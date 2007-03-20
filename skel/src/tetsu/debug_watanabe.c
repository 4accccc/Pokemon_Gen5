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
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WATANABE_DEBUG, 0x20000 );
	TestModeWorkSet( HEAPID_WATANABE_DEBUG );
//	TestModeWorkSet( GFL_HEAPID_APP );

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
	GFL_HEAP_DeleteHeap( HEAPID_WATANABE_DEBUG );

//	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &TestMainProcData, NULL);
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
	HEAPID					heapID;
	int						seq;

	GFL_G3D_UTIL*			g3Dutil;
	GFL_G3D_OBJSTATUS		g3DobjStatus[16];
	GFL_G3D_SCENE*			g3Dscene;
	u32						g3DsceneObjID;
	GFL_G3D_CAMERA*			g3Dcamera[4];
	GFL_G3D_LIGHTSET*		g3Dlightset[4];

	u16						nowCameraNum;
	u16						nowLightNum;

	u16						work[16];
	u16						mode;
}TETSU_WORK;

TETSU_WORK* tetsuWork;

typedef struct {
	int		seq;

	VecFx32	rotateTmp;
}BALL_WORK;

#include "sample_graphic/test9ball.naix"

//ＢＧ設定関数
static void	bg_init( HEAPID heapID );
static void	bg_exit( void );
//３ＤＢＧ作成関数
static void G3DsysSetup( void );
static void g3d_load( HEAPID heapID );
static void g3d_move( void );
static void g3d_draw( void );
static void	g3d_unload( void );

static inline void rotateCalc( VecFx32* rotSrc, MtxFx33* rotDst );
static void ball_rotateX( GFL_G3D_SCENEOBJ* sceneObj, void* work );
static void ball_rotateY( GFL_G3D_SCENEOBJ* sceneObj, void* work );
static void ball_rotateZ( GFL_G3D_SCENEOBJ* sceneObj, void* work );

static void SceneObjTransAddAll( GFL_G3D_SCENE* g3Dscene, VecFx32* trans );
static void CameraMove( GFL_G3D_CAMERA* g3Dcamera, VecFx32* trans );
//------------------------------------------------------------------
/**
 * @brief	データ
 */
//------------------------------------------------------------------
#include "debug_watanabe.dat"

//------------------------------------------------------------------
/**
 * @brief	ワークの確保と破棄
 */
//------------------------------------------------------------------
static void	TestModeWorkSet( HEAPID heapID )
{
	tetsuWork = GFL_HEAP_AllocMemoryClear( heapID, sizeof(TETSU_WORK) );
	tetsuWork->heapID = heapID;
}

static void	TestModeWorkRelease( void )
{
	GFL_HEAP_FreeMemory( tetsuWork );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
#define MOVE_SPEED ( FX32_ONE*2)

static BOOL	TestModeControl( void )
{
	BOOL return_flag = FALSE;
	int i;

	switch( tetsuWork->seq ){

	case 0:
		//初期化
		GFL_STD_MTRandInit(0);
		bg_init( tetsuWork->heapID );
		tetsuWork->seq++;
		break;

	case 1:
		//画面作成
		g3d_load( tetsuWork->heapID );	//３Ｄデータ作成
		tetsuWork->work[0] = 0;
		tetsuWork->work[1] = 0;
		tetsuWork->mode = 3;
		tetsuWork->seq++;
		break;

	case 2:
		if( GFL_UI_KeyGetTrg() & PAD_BUTTON_R ){
			tetsuWork->seq++;
		}
#if 0
		if( GFL_UI_KeyGetCont() & PAD_KEY_LEFT ){
			VecFx32 trans = {  MOVE_SPEED, 0, 0 };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		} else if( GFL_UI_KeyGetCont() & PAD_KEY_RIGHT ){
			VecFx32 trans = { -MOVE_SPEED, 0, 0 };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		} else if( GFL_UI_KeyGetCont() & PAD_KEY_UP ){
			VecFx32 trans = { 0, 0,  MOVE_SPEED };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		} else if( GFL_UI_KeyGetCont() & PAD_KEY_DOWN ){
			VecFx32 trans = { 0, 0, -MOVE_SPEED };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		}
#else
		if( GFL_UI_KeyGetCont() & PAD_KEY_LEFT ){
			VecFx32 trans = { -MOVE_SPEED, 0, 0 };
			CameraMove( tetsuWork->g3Dcamera[ tetsuWork->nowCameraNum ], &trans );
		} else if( GFL_UI_KeyGetCont() & PAD_KEY_RIGHT ){
			VecFx32 trans = {  MOVE_SPEED, 0, 0 };
			CameraMove( tetsuWork->g3Dcamera[ tetsuWork->nowCameraNum ], &trans );
		} else if( GFL_UI_KeyGetCont() & PAD_KEY_UP ){
			VecFx32 trans = { 0, 0, -MOVE_SPEED };
			CameraMove( tetsuWork->g3Dcamera[ tetsuWork->nowCameraNum ], &trans );
		} else if( GFL_UI_KeyGetCont() & PAD_KEY_DOWN ){
			VecFx32 trans = { 0, 0,  MOVE_SPEED };
			CameraMove( tetsuWork->g3Dcamera[ tetsuWork->nowCameraNum ], &trans );
		}
#endif
		if( GFL_UI_KeyGetTrg() & PAD_BUTTON_B ){
			if( tetsuWork->mode > 1 ){
				tetsuWork->mode--;
			} else {
				tetsuWork->mode = 3;
			}
		}

		if( GFL_UI_KeyGetTrg() & PAD_BUTTON_A ){
			if( tetsuWork->mode & 1 ){
				GFL_G3D_CameraSwitching( tetsuWork->g3Dcamera[1] );
				tetsuWork->nowCameraNum = 1;
			}
			if( tetsuWork->mode & 2 ){
				GFL_G3D_LightSwitching( tetsuWork->g3Dlightset[1] );
				tetsuWork->nowLightNum = 1;
			}
		}
		if( GFL_UI_KeyGetTrg() & PAD_BUTTON_X ){
			if( tetsuWork->mode & 1 ){
				GFL_G3D_CameraSwitching( tetsuWork->g3Dcamera[0] );
				tetsuWork->nowCameraNum = 0;
			}
			if( tetsuWork->mode & 2 ){
				GFL_G3D_LightSwitching( tetsuWork->g3Dlightset[0] );
				tetsuWork->nowLightNum = 0;
			}
		}
		if( GFL_UI_KeyGetTrg() & PAD_BUTTON_Y ){
			if( tetsuWork->mode & 1 ){
				GFL_G3D_CameraSwitching( tetsuWork->g3Dcamera[2] );
				tetsuWork->nowCameraNum = 2;
			}
			if( tetsuWork->mode & 2 ){
				GFL_G3D_LightSwitching( tetsuWork->g3Dlightset[2] );
				tetsuWork->nowLightNum = 2;
			}
		}
		g3d_move();
		tetsuWork->work[0]++;
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
static void	bg_init( HEAPID heapID )
{
	//ＢＧシステム起動
	GFL_BG_sysInit( heapID );

	//ＶＲＡＭ設定
	GX_SetBankForTex(GX_VRAM_TEX_01_AB);
	GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G); 

	//パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocMemoryLowClear( heapID, 16*2 );
		plt[0] = GX_RGB( 8, 15, 8);
		GFL_BG_PaletteSet( GFL_BG_FRAME0_M, plt, 16*2, 0 );
		GFL_BG_PaletteSet( GFL_BG_FRAME1_M, plt, 16*2, 0 );

		GFL_HEAP_FreeMemory( plt );
	}

	//ＢＧモード設定
	GFL_BG_InitBG( &bgsysHeader );

	//３Ｄシステム起動
	GFL_G3D_sysInit( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, heapID, G3DsysSetup );
	GFL_BG_BGControlSet3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	GFL_DISP_DispSelect( GFL_DISP_3D_TO_MAIN );
}

static void	bg_exit( void )
{
	GFL_G3D_sysExit();
	GFL_BG_sysExit();
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄデータコントロール
 */
//------------------------------------------------------------------
//作成
static void g3d_load( HEAPID heapID )
{
	int	i;

	//配置物設定
	tetsuWork->g3Dutil = GFL_G3D_UtilsysCreate( &g3Dutil_setup, heapID );
	tetsuWork->g3Dscene = GFL_G3D_SceneCreate( tetsuWork->g3Dutil, NELEMS(g3DsceneObjData)+1,
												sizeof(BALL_WORK), heapID );
	tetsuWork->g3DsceneObjID = GFL_G3D_SceneObjAdd
								( tetsuWork->g3Dscene, g3DsceneObjData, NELEMS(g3DsceneObjData) );

	//カメラライト0作成
	tetsuWork->g3Dcamera[0] = GFL_G3D_CameraDefaultCreate( &camera0Pos, &cameraTarget, heapID );
	tetsuWork->g3Dlightset[0] = GFL_G3D_LightCreate( &light0Setup, heapID );

	//カメラライト1作成
	tetsuWork->g3Dcamera[1] = GFL_G3D_CameraDefaultCreate( &camera1Pos, &cameraTarget, heapID );
	tetsuWork->g3Dlightset[1] = GFL_G3D_LightCreate( &light1Setup, heapID );

	//カメラライト2作成
	tetsuWork->g3Dcamera[2] = GFL_G3D_CameraDefaultCreate( &camera2Pos, &cameraTarget, heapID );
	tetsuWork->g3Dlightset[2] = GFL_G3D_LightCreate( &light2Setup, heapID );

	//カメラライト0反映
	GFL_G3D_CameraSwitching( tetsuWork->g3Dcamera[0] );
	GFL_G3D_LightSwitching( tetsuWork->g3Dlightset[0] );
	tetsuWork->nowCameraNum = 0;
	tetsuWork->nowLightNum = 0;
}
	
//動作
static void g3d_move( void )
{
	GFL_G3D_SceneMain( tetsuWork->g3Dscene );  
}

//描画
static void g3d_draw( void )
{
	GFL_G3D_SceneDraw( tetsuWork->g3Dscene );  
}
	
//破棄
static void g3d_unload( void )
{
	GFL_G3D_LightDelete( tetsuWork->g3Dlightset[2] );
	GFL_G3D_CameraDelete( tetsuWork->g3Dcamera[2] );
	GFL_G3D_LightDelete( tetsuWork->g3Dlightset[1] );
	GFL_G3D_CameraDelete( tetsuWork->g3Dcamera[1] );
	GFL_G3D_LightDelete( tetsuWork->g3Dlightset[0] );
	GFL_G3D_CameraDelete( tetsuWork->g3Dcamera[0] );

	GFL_G3D_SceneObjDel( tetsuWork->g3Dscene, tetsuWork->g3DsceneObjID, NELEMS(g3DsceneObjData) );
	GFL_G3D_SceneDelete( tetsuWork->g3Dscene );  

	GFL_G3D_UtilsysDelete( tetsuWork->g3Dutil );
}
	
//------------------------------------------------------------------
/**
 * @brief	３Ｄ動作
 */
//------------------------------------------------------------------
static inline void rotateCalc( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotX33(	rotDst, FX_SinIdx((u16)rotSrc->x), FX_CosIdx((u16)rotSrc->x) );

	MTX_RotY33(	&tmp, FX_SinIdx((u16)rotSrc->y), FX_CosIdx((u16)rotSrc->y) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)rotSrc->z), FX_CosIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst, &tmp, rotDst );
}

static inline void drawSWset( GFL_G3D_SCENEOBJ* sceneObj, BOOL sw )
{
	BOOL swBuf = sw;
	GFL_G3D_SceneObjDrawSWSet( sceneObj, &swBuf );
}

static void ball_rotateX( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
#if 0
	MtxFx33 rotate;
	BALL_WORK* ballWk = (BALL_WORK*)work;

	switch( ballWk->seq ){
		case 0:
			{
				u16 val;
				GFL_G3D_SceneObjIDGet( sceneObj, &val );
				ballWk->rotateTmp.x = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.y = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.z = g3DanmRotateSpeed * val;
			}
			drawSWset( sceneObj, TRUE );
			ballWk->seq++;
			break;
		case 1:
			ballWk->rotateTmp.x += g3DanmRotateSpeed;
			rotateCalc( &ballWk->rotateTmp, &rotate );
			GFL_G3D_SceneObjStatusRotateSet( sceneObj, &rotate );
			break;
	}
#endif
}

static void ball_rotateY( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
#if 0
	MtxFx33 rotate;
	BALL_WORK* ballWk = (BALL_WORK*)work;

	switch( ballWk->seq ){
		case 0:
			{
				u16 val;
				GFL_G3D_SceneObjIDGet( sceneObj, &val );
				ballWk->rotateTmp.x = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.y = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.z = g3DanmRotateSpeed * val;
			}
			drawSWset( sceneObj, TRUE );
			ballWk->seq++;
			break;
		case 1:
			ballWk->rotateTmp.y += g3DanmRotateSpeed;
			rotateCalc( &ballWk->rotateTmp, &rotate );
			GFL_G3D_SceneObjStatusRotateSet( sceneObj, &rotate );
			break;
	}
#endif
}

static void ball_rotateZ( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
#if 0
	MtxFx33 rotate;
	BALL_WORK* ballWk = (BALL_WORK*)work;

	switch( ballWk->seq ){
		case 0:
			{
				u16 val;
				GFL_G3D_SceneObjIDGet( sceneObj, &val );
				ballWk->rotateTmp.x = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.y = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.z = g3DanmRotateSpeed * val;
			}
			drawSWset( sceneObj, TRUE );
			ballWk->seq++;
			break;
		case 1:
			ballWk->rotateTmp.z += g3DanmRotateSpeed;
			rotateCalc( &ballWk->rotateTmp, &rotate );
			GFL_G3D_SceneObjStatusRotateSet( sceneObj, &rotate );
			break;
	}
#endif
}

static void SceneObjTransAddAll( GFL_G3D_SCENE* g3Dscene, VecFx32* trans )
{
	GFL_G3D_SCENEOBJ*	g3DsceneObj;
	VecFx32				tmp;
	int					i;
	u16					idx = tetsuWork->g3DsceneObjID;

	for( i=0; i<NELEMS(g3DsceneObjData); i++ ){
		g3DsceneObj = GFL_G3D_SceneObjGet( g3Dscene, idx );
		GFL_G3D_SceneObjStatusTransGet( g3DsceneObj, &tmp );
		tmp.x += trans->x;
		tmp.y += trans->y;
		tmp.z += trans->z;
		GFL_G3D_SceneObjStatusTransSet( g3DsceneObj, &tmp );
		idx++;
	}
}

static void CameraMove( GFL_G3D_CAMERA* g3Dcamera, VecFx32* trans )
{
	VecFx32	tmpVec;
	
	GFL_G3D_CameraPosGet( g3Dcamera, &tmpVec );
	tmpVec.x += trans->x;
	tmpVec.y += trans->y;
	tmpVec.z += trans->z;
	GFL_G3D_CameraPosSet( g3Dcamera, &tmpVec );

	GFL_G3D_CameraTargetGet( g3Dcamera, &tmpVec );
	tmpVec.x += trans->x;
	tmpVec.y += trans->y;
	tmpVec.z += trans->z;
	GFL_G3D_CameraTargetSet( g3Dcamera, &tmpVec );

	GFL_G3D_CameraSwitching( g3Dcamera );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄセットアップ
 */
//------------------------------------------------------------------
static void G3DsysSetup( void )
{
	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_TOON );
	G3X_AntiAlias( FALSE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( FALSE );		// アルファブレンド　オン
	G3X_EdgeMarking( FALSE );
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);
}





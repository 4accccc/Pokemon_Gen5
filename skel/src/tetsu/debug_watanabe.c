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
#include "g3d_scene.h"

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

	u16						work[16];
}TETSU_WORK;

TETSU_WORK* tetsuWork;

#include "sample_graphic/test9ball.naix"

//ＢＧ設定関数
static void	bg_init( HEAPID heapID );
static void	bg_exit( void );
//３ＤＢＧ作成関数
static void g3d_load( HEAPID heapID );
static void g3d_move( void );
static void g3d_draw( void );
static void	g3d_unload( void );

static inline void rotateCalc( VecFx32* rotSrc, MtxFx33* rotDst );
static void ball_rotateX( GFL_G3D_SCENEOBJ* sceneObj, void* work );
static void ball_rotateY( GFL_G3D_SCENEOBJ* sceneObj, void* work );
static void ball_rotateZ( GFL_G3D_SCENEOBJ* sceneObj, void* work );

static void SceneObjTransAddAll( GFL_G3D_SCENE* g3Dscene, VecFx32* trans );
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

//---------------------
static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 cameraPos		= { 0, (FX32_ONE * 100), (FX32_ONE * 180) };
static const VecFx32 cameraUp		= { 0, FX32_ONE, 0 };

#define cameraPerspway	( 0x0b60 )
#define cameraAspect	( FX32_ONE * 4 / 3 )
#define cameraNear		( 1 << FX32_SHIFT )
#define cameraFar		( 1024 << FX32_SHIFT )

#define g3DanmRotateSpeed	( 0x200 )
#define g3DanmFrameSpeed	( FX32_ONE )
	
static const VecFx32 light0Vec		= { -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) };
static const VecFx32 light1Vec		= {  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) };

//---------------------
static const char g3DarcPath[] = {"src/sample_graphic/test9ball.narc"};

enum {
	G3DRES_BALL_BMD = 0,
	G3DRES_BALL1_TEX,
	G3DRES_BALL2_TEX,
	G3DRES_BALL3_TEX,
	G3DRES_BALL4_TEX,
	G3DRES_BALL5_TEX,
	G3DRES_BALL6_TEX,
	G3DRES_BALL7_TEX,
	G3DRES_BALL8_TEX,
	G3DRES_BALL9_TEX,
};

static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ (u32)g3DarcPath, NARC_test9ball_test1ball_nsbmd, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test1ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test2ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test3ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test4ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test5ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test6ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test7ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test8ball_nsbtx, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_test9ball_test9ball_nsbtx, GFL_G3D_UTIL_RESPATH },
};

//---------------------
enum {
	G3DOBJ_BALL1 = 0,
	G3DOBJ_BALL2,
	G3DOBJ_BALL3,
	G3DOBJ_BALL4,
	G3DOBJ_BALL5,
	G3DOBJ_BALL6,
	G3DOBJ_BALL7,
	G3DOBJ_BALL8,
	G3DOBJ_BALL9,
};

static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL1_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL2_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL3_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL4_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL5_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL6_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL7_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL8_TEX, NULL, 0 },
	{ G3DRES_BALL_BMD, 0, G3DRES_BALL9_TEX, NULL, 0 },
};

//---------------------
static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl, NELEMS(g3Dutil_resTbl),
	g3Dutil_objTbl, NELEMS(g3Dutil_objTbl),
};

//---------------------
static const GFL_G3D_SCENEOBJ_DATA g3DsceneObjData[] = {
	{	G3DOBJ_BALL1, 0, 1, TRUE, 
		{	{ -FX32_ONE*48, 0, -FX32_ONE*48 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateX,
	},
	{	G3DOBJ_BALL2, 0, 1, TRUE, 
		{	{ -FX32_ONE*48, 0,  FX32_ONE* 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateY,
	},
	{	G3DOBJ_BALL3, 0, 1, TRUE, 
		{	{ -FX32_ONE*48, 0,  FX32_ONE*48 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateZ,
	},
	{	G3DOBJ_BALL4, 0, 1, TRUE, 
		{	{ -FX32_ONE* 0, 0, -FX32_ONE*48 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateZ,
	},
	{	G3DOBJ_BALL5, 0, 1, TRUE, 
		{	{  FX32_ONE* 0, 0,  FX32_ONE* 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateX,
	},
	{	G3DOBJ_BALL6, 0, 1, TRUE, 
		{	{  FX32_ONE* 0, 0,  FX32_ONE*48 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateY,
	},
	{	G3DOBJ_BALL7, 0, 1, TRUE, 
		{	{  FX32_ONE*48, 0, -FX32_ONE*48 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateY,
	},
	{	G3DOBJ_BALL8, 0, 1, TRUE, 
		{	{  FX32_ONE*48, 0,  FX32_ONE* 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateZ,
	},
	{	G3DOBJ_BALL9, 0, 1, TRUE, 
		{	{  FX32_ONE*48, 0,  FX32_ONE*48 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},ball_rotateX,
	},
};

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
static BOOL	TestModeControl( void )
{
	BOOL return_flag = FALSE;
	int i;

	switch( tetsuWork->seq ){

	case 0:
		//初期化
		bg_init( tetsuWork->heapID );
		tetsuWork->seq++;
		break;

	case 1:
		//画面作成
		g3d_load( tetsuWork->heapID );	//３Ｄデータ作成
		tetsuWork->seq++;
		break;

	case 2:
		if( GFL_UI_KeyGetTrg() & PAD_BUTTON_R ){
			tetsuWork->seq++;
		} else if( GFL_UI_KeyGetTrg() & PAD_KEY_LEFT ){
			VecFx32 trans = { -FX32_ONE, 0, 0 };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		} else if( GFL_UI_KeyGetTrg() & PAD_KEY_RIGHT ){
			VecFx32 trans = { FX32_ONE, 0, 0 };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		} else if( GFL_UI_KeyGetTrg() & PAD_KEY_UP ){
			VecFx32 trans = { 0, 0, -FX32_ONE };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		} else if( GFL_UI_KeyGetTrg() & PAD_KEY_DOWN ){
			VecFx32 trans = { 0, 0, FX32_ONE };
			SceneObjTransAddAll( tetsuWork->g3Dscene, &trans );
		}
		g3d_move();
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
						DTCM_SIZE, heapID, NULL );
	GFL_BG_BGControlSet3D( G3D_FRM_PRI );
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

	tetsuWork->g3Dutil = GFL_G3D_UtilsysCreate( &g3Dutil_setup, heapID );
	tetsuWork->g3Dscene = GFL_G3D_SceneCreate( tetsuWork->g3Dutil, 32, 16, heapID );
	tetsuWork->g3DsceneObjID = GFL_G3D_SceneObjAdd
								( tetsuWork->g3Dscene, g3DsceneObjData, NELEMS(g3DsceneObjData) );

	//カメラセット
	GFL_G3D_sysProjectionSet(	GFL_G3D_PRJPERS, 
								FX_SinIdx( cameraPerspway ), FX_CosIdx( cameraPerspway ), 
								cameraAspect, 0, cameraNear, cameraFar, 0 );
	GFL_G3D_sysLookAtSet( (VecFx32*)&cameraPos, (VecFx32*)&cameraUp, (VecFx32*)&cameraTarget );
	GFL_G3D_sysLightSet( 0, (VecFx16*)&light0Vec, 0x7fff );

	tetsuWork->work[0] = 0;
}
	
//動作
static void g3d_move( void )
{
	GFL_G3D_SceneMain( tetsuWork->g3Dscene );  
	tetsuWork->work[0]++;
}

//描画
static void g3d_draw( void )
{
	GFL_G3D_SceneDraw( tetsuWork->g3Dscene );  
}
	
//破棄
static void g3d_unload( void )
{
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

static void ball_rotateX( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	MtxFx33 rotate;
	VecFx32 rotate_tmp = { 0, 0, 0 };

	rotate_tmp.x = g3DanmRotateSpeed * tetsuWork->work[0];	//Ｙ軸回転
	rotateCalc( &rotate_tmp, &rotate );
	GFL_G3D_SceneObjStatusRotateSet( sceneObj, &rotate );
}

static void ball_rotateY( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	MtxFx33 rotate;
	VecFx32 rotate_tmp = { 0, 0, 0 };

	rotate_tmp.y = g3DanmRotateSpeed * tetsuWork->work[0];	//Ｙ軸回転
	rotateCalc( &rotate_tmp, &rotate );
	GFL_G3D_SceneObjStatusRotateSet( sceneObj, &rotate );
}

static void ball_rotateZ( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	MtxFx33 rotate;
	VecFx32 rotate_tmp = { 0, 0, 0 };

	rotate_tmp.z = g3DanmRotateSpeed * tetsuWork->work[0];	//Ｙ軸回転
	rotateCalc( &rotate_tmp, &rotate );
	GFL_G3D_SceneObjStatusRotateSet( sceneObj, &rotate );
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
		tmp.x += trans->z;
		GFL_G3D_SceneObjStatusTransSet( g3DsceneObj, &tmp );
		idx++;
	}
}


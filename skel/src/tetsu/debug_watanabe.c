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
#include "gfl_use.h"

#include "main.h"
#include "testmode.h"

#include "double3Ddisp.h"

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
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WATANABE_DEBUG, 0x80000 );
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
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
	HEAPID					heapID;
	int						seq;

	GFL_G3D_UTIL*			g3Dutil;
	GFL_G3D_OBJSTATUS		g3DobjStatus[16];
	GFL_G3D_SCENE*			g3Dscene;
	u32						g3DsceneObjID;
	u32						g3DsceneMapObjID;
	u32						g3DsceneMapObjCount;
	GFL_G3D_CAMERA*			g3Dcamera[4];
	GFL_G3D_LIGHTSET*		g3Dlightset[4];

	u16						nowCameraNum;
	u16						nowLightNum;

	u16						work[16];
	u16						mode;

	int						contSeq;
	u16						contRotate;
	u16						nowRotate;
	u16						updateRotate;
	s16						updateRotateOffs;
	VecFx32					contPos;
	BOOL					updateRotateFlag;
	BOOL					SpeedUpFlag;

	u16						mainCameraRotate;
	u16						autoCameraRotate1;
	u16						autoCameraRotate2;

	u16*					mapAttr;

	TCB*					dbl3DdispVintr;
}TETSU_WORK;

typedef struct {
	const GFL_G3D_SCENEOBJ_DATA*	data;
	int								count;
	u16*							mapAttr;
}GFL_G3D_SCENEOBJ_DATA_SETUP;

typedef struct {
	int		seq;

	VecFx32	rotateTmp;
}OBJ_WORK;

typedef struct {
	const char*				data;
	GFL_G3D_SCENEOBJ_DATA	floor;
}MAPDATA;

u32 DebugCullingCount = 0;
//------------------------------------------------------------------
/**
 * @brief	マップデータ
 */
//------------------------------------------------------------------
static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 camera0Pos	= { 0, (FX32_ONE * 100), (FX32_ONE * 180) };
static const VecFx32 camera1Pos	= { (FX32_ONE * 100 ), (FX32_ONE * 100), (FX32_ONE * 180) };
static const VecFx32 camera2Pos	= { -(FX32_ONE * 100 ), (FX32_ONE * 100), (FX32_ONE * 180) };

#define g3DanmRotateSpeed	( 0x200 )
#define g3DanmFrameSpeed	( FX32_ONE )
	
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
	{ 0, {{ 0, 0, -(FX16_ONE-1) }, 0x7fff } },
};
static const GFL_G3D_LIGHT_DATA light1Tbl[] = {
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, 0x001f } },
};
static const GFL_G3D_LIGHT_DATA light2Tbl[] = {
	{ 0, {{ (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, 0x7e00 } },
};

static const GFL_G3D_LIGHTSET_SETUP light0Setup = { light0Tbl, NELEMS(light0Tbl) };
static const GFL_G3D_LIGHTSET_SETUP light1Setup = { light1Tbl, NELEMS(light1Tbl) };
static const GFL_G3D_LIGHTSET_SETUP light2Setup = { light2Tbl, NELEMS(light2Tbl) };

static void simpleRotateY( GFL_G3D_SCENEOBJ* sceneObj, void* work );
static void moveHaruka( GFL_G3D_SCENEOBJ* sceneObj, void* work );

#include "debug_watanabe.dat"

static const MAPDATA mapDataTbl;
//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
//ＢＧ設定関数
static void	bg_init( HEAPID heapID );
static void	bg_exit( void );
//３ＤＢＧ作成関数
static void G3DsysSetup( void );
static void g3d_load( HEAPID heapID );
static void g3d_move( void );
static void g3d_draw( void );
static void	g3d_unload( void );

static void KeyControlInit( void );
static BOOL KeyControlEndCheck( void );
static void KeyControlCameraMove1( void );

static GFL_G3D_SCENEOBJ_DATA_SETUP* MapDataCreate( const MAPDATA* map, HEAPID heapID );
static void MapDataDelete( GFL_G3D_SCENEOBJ_DATA_SETUP* setup );
static void MapAttrDelete( u16* mapAttr );
static u16	MapAttrGet( u16* mapAttr, VecFx32* pos );

TETSU_WORK* tetsuWork;

//------------------------------------------------------------------
/**
 * @brief	ワークの確保と破棄
 */
//------------------------------------------------------------------
static void	TestModeWorkSet( HEAPID heapID )
{
	tetsuWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(TETSU_WORK) );
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
		GFL_STD_MtRandInit(0);
		bg_init( tetsuWork->heapID );
		tetsuWork->dbl3DdispVintr = GFUser_VIntr_CreateTCB
								( GFL_G3D_DOUBLE3D_VblankIntrTCB, NULL, 0 );

		tetsuWork->seq++;
		break;

	case 1:
		//画面作成
		KeyControlInit();
		g3d_load( tetsuWork->heapID );	//３Ｄデータ作成
		tetsuWork->seq++;
		break;

	case 2:
		if( KeyControlEndCheck() == TRUE ){
			tetsuWork->seq++;
		}
		KeyControlCameraMove1();

		g3d_move();
		g3d_draw();		//３Ｄデータ描画
		break;

	case 3:
		//終了
		g3d_unload();	//３Ｄデータ破棄

		GFL_TCB_DeleteTask( tetsuWork->dbl3DdispVintr );
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
static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};	

#define DTCM_SIZE		(0x1000)
#define G3D_FRM_PRI		(1)

static void	bg_init( HEAPID heapID )
{
	//ＢＧシステム起動
	GFL_BG_Init( heapID );

	//ＶＲＡＭ設定
	GX_SetBankForTex(GX_VRAM_TEX_01_AB);
	GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G); 

	//パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( heapID, 16*2 );
		plt[0] = GX_RGB( 0, 0, 0 );
		GFL_BG_LoadPalette( GFL_BG_FRAME0_M, plt, 16*2, 0 );
		GFL_BG_LoadPalette( GFL_BG_FRAME1_M, plt, 16*2, 0 );

		GFL_HEAP_FreeMemory( plt );
	}

	//ＢＧモード設定
	GFL_BG_InitBG( &bgsysHeader );

	//３Ｄシステム起動
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, heapID, G3DsysSetup );
	GFL_G3D_DOUBLE3D_Init( heapID );

	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( void )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_G3D_DOUBLE3D_Exit();
	GFL_G3D_Exit();
	GFL_BG_Exit();
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
	tetsuWork->g3Dutil = GFL_G3D_UTIL_Create( &g3Dutil_setup, heapID );
	tetsuWork->g3Dscene = GFL_G3D_SCENE_Create( tetsuWork->g3Dutil, 1000,
												sizeof(OBJ_WORK), heapID );
	tetsuWork->g3DsceneObjID = GFL_G3D_SCENEOBJ_Add
								( tetsuWork->g3Dscene, g3DsceneObjData, NELEMS(g3DsceneObjData) );
	{
		GFL_G3D_SCENEOBJ_DATA_SETUP* setup = MapDataCreate( &mapDataTbl, heapID );
		tetsuWork->g3DsceneMapObjID = GFL_G3D_SCENEOBJ_Add
								( tetsuWork->g3Dscene, setup->data, setup->count );
		tetsuWork->g3DsceneMapObjCount = setup->count;
		tetsuWork->mapAttr = setup->mapAttr;
		MapDataDelete( setup );
	}

	//カメラ作成
	tetsuWork->g3Dcamera[0] = GFL_G3D_CAMERA_CreateDefault( &camera0Pos, &cameraTarget, heapID );
	tetsuWork->g3Dcamera[1] = GFL_G3D_CAMERA_CreateDefault( &camera1Pos, &cameraTarget, heapID );

	//ライト作成
	tetsuWork->g3Dlightset[0] = GFL_G3D_LIGHT_Create( &light0Setup, heapID );
	tetsuWork->g3Dlightset[1] = GFL_G3D_LIGHT_Create( &light1Setup, heapID );

	//カメラライト0反映
	GFL_G3D_CAMERA_Switching( tetsuWork->g3Dcamera[0] );
	GFL_G3D_LIGHT_Switching( tetsuWork->g3Dlightset[0] );
	tetsuWork->nowCameraNum = 0;
	tetsuWork->nowLightNum = 0;

	tetsuWork->work[0] = 0;
}
	
//動作
static void g3d_move( void )
{
	DebugCullingCount = 0;
	GFL_G3D_SCENE_Main( tetsuWork->g3Dscene );  
	//OS_Printf("CullingCount = %d\n",DebugCullingCount);
	tetsuWork->work[0]++;
}

//描画
static void g3d_draw( void )
{
	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		GFL_G3D_CAMERA_Switching( tetsuWork->g3Dcamera[0] );
	} else {
		GFL_G3D_CAMERA_Switching( tetsuWork->g3Dcamera[1] );
	}
	GFL_G3D_SCENE_Draw( tetsuWork->g3Dscene );  
	GFL_G3D_DOUBLE3D_SetSwapFlag();
}
	
//破棄
static void g3d_unload( void )
{
	GFL_G3D_LIGHT_Delete( tetsuWork->g3Dlightset[1] );
	GFL_G3D_CAMERA_Delete( tetsuWork->g3Dcamera[1] );
	GFL_G3D_LIGHT_Delete( tetsuWork->g3Dlightset[0] );
	GFL_G3D_CAMERA_Delete( tetsuWork->g3Dcamera[0] );

	MapAttrDelete( tetsuWork->mapAttr );
	GFL_G3D_SCENEOBJ_Remove
		(tetsuWork->g3Dscene, tetsuWork->g3DsceneMapObjID, tetsuWork->g3DsceneMapObjCount ); 
	GFL_G3D_SCENEOBJ_Remove(tetsuWork->g3Dscene, tetsuWork->g3DsceneObjID, NELEMS(g3DsceneObjData));
	GFL_G3D_SCENE_Delete( tetsuWork->g3Dscene );  

	GFL_G3D_UTIL_Delete( tetsuWork->g3Dutil );
}
	
//------------------------------------------------------------------
/**
 * @brief	３Ｄ動作
 */
//------------------------------------------------------------------
#define VIEW_LENGTH		(64*6)
#define VIEW_SCALAR_MAX	(0x20000)

//回転マトリクス変換
static inline void rotateCalc( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotX33(	rotDst, FX_SinIdx((u16)rotSrc->x), FX_CosIdx((u16)rotSrc->x) );

	MTX_RotY33(	&tmp, FX_SinIdx((u16)rotSrc->y), FX_CosIdx((u16)rotSrc->y) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)rotSrc->z), FX_CosIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst, &tmp, rotDst );
}

static inline void SetDrawSW( GFL_G3D_SCENEOBJ* sceneObj, BOOL sw )
{
	BOOL swBuf = sw;
	GFL_G3D_SCENEOBJ_SetDrawSW( sceneObj, &swBuf );
}

static inline void SetAlpha( GFL_G3D_SCENEOBJ* sceneObj, u8 alpha, int scalar )
{
	//半透明処理の関係上、奥に見えるものから優先( 距離を200分割 )
	u8	drawPri = 249 - ( scalar * 200 / (VIEW_LENGTH * VIEW_LENGTH) );
	u8	alphaBuf = alpha;

	GFL_G3D_SCENEOBJ_SetDrawPri( sceneObj, &drawPri );
	GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &alphaBuf );
}

static inline void ResetAlpha( GFL_G3D_SCENEOBJ* sceneObj )
{
	u8	alphaBuf = 31;
	u8	drawPri = 3;

	GFL_G3D_SCENEOBJ_SetDrawPri( sceneObj, &drawPri );
	GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &alphaBuf );
}

//２Ｄカリング
static BOOL culling2DView( GFL_G3D_SCENEOBJ* sceneObj, VecFx32* objPos, int* scalar )
{
	//カメラポインタ取得
	GFL_G3D_CAMERA*	camera;
	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		camera = tetsuWork->g3Dcamera[0];	//下カメラ
	} else {
		camera = tetsuWork->g3Dcamera[1];	//上カメラ
	}

	//カメラ位置によるカリング
	*scalar = GFL_G3D_CAMERA_GetDotProductXZfast( camera, objPos );
	//カメラとのスカラーによる位置判定(0は水平、正は前方、負は後方)
	if( *scalar < -0x800 ){
		DebugCullingCount++;
		SetDrawSW( sceneObj, FALSE );
		return FALSE;
	}

	{	//ターゲット周囲判定によるカリング
		VecFx32 cameraTarget;
		fx32	diffX, diffZ, diffLen;
			
		GFL_G3D_CAMERA_GetTarget( camera, &cameraTarget );

		diffX = ( cameraTarget.x - objPos->x ) >> FX32_SHIFT;
		diffZ = ( cameraTarget.z - objPos->z ) >> FX32_SHIFT;
		diffLen = ( diffX * diffX + diffZ * diffZ );
		if( diffLen > ( VIEW_LENGTH * VIEW_LENGTH )){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
	}
	SetDrawSW( sceneObj, TRUE );
	return TRUE;
}

//注視点との位置関係取得（ＸＺのみ）
static int checkPos_toTarget( VecFx32* objPos )
{
	VecFx32 cameraPos, cameraTarget;
	int viewVecX, viewVecZ, objVecX, objVecZ;

	//カメラ情報取得
	GFL_G3D_CAMERA_GetPos( tetsuWork->g3Dcamera[ tetsuWork->nowCameraNum ], &cameraPos );
	GFL_G3D_CAMERA_GetTarget( tetsuWork->g3Dcamera[ tetsuWork->nowCameraNum ], &cameraTarget );

	//視界ベクトル計算（整数部のみ）
	viewVecX = ( cameraTarget.x - cameraPos.x ) >> FX32_SHIFT;
	viewVecZ = ( cameraTarget.z - cameraPos.z ) >> FX32_SHIFT;

	//対象物体ベクトル計算（整数部のみ）
	objVecX = ( objPos->x - cameraTarget.x ) >> FX32_SHIFT;
	objVecZ = ( objPos->z - cameraTarget.z ) >> FX32_SHIFT;

	//視界ベクトルと対象物体ベクトルの内積計算（ＸＺ）
	return viewVecX * objVecX + viewVecZ * objVecZ;
}


//------------------------------------------------------------------
static void simpleRotateY( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	MtxFx33 rotate;
	OBJ_WORK* ballWk = (OBJ_WORK*)work;

	switch( ballWk->seq ){
		case 0:
			{
				u16 val;
				GFL_G3D_SCENEOBJ_GetObjID( sceneObj, &val );
				ballWk->rotateTmp.x = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.y = g3DanmRotateSpeed * val;
				ballWk->rotateTmp.z = g3DanmRotateSpeed * val;
			}
			ballWk->seq++;
			break;
		case 1:
			ballWk->rotateTmp.y += g3DanmRotateSpeed;
			rotateCalc( &ballWk->rotateTmp, &rotate );
			GFL_G3D_SCENEOBJ_SetRotate( sceneObj, &rotate );
			break;
	}
}

static void moveHaruka( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	MtxFx33			rotate;
	VecFx32			rotVec;
	VecFx32			trans;
	GFL_G3D_OBJ*	g3Dobj = GFL_G3D_SCENEOBJ_GetG3DobjHandle( sceneObj );

	trans.x = tetsuWork->contPos.x;
	trans.y = 0;
	trans.z = tetsuWork->contPos.z;
	rotVec.x = 0;
	rotVec.y = tetsuWork->nowRotate + 0x8000;
	rotVec.z = 0;
	rotateCalc( &rotVec, &rotate );

	GFL_G3D_SCENEOBJ_SetPos( sceneObj, &trans );
	GFL_G3D_SCENEOBJ_SetRotate( sceneObj, &rotate );

	if( GFL_UI_KEY_GetCont() & (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)){
		int speed;

		if( tetsuWork->SpeedUpFlag == TRUE ){
			speed = FX32_ONE * 2; 
		} else {
			speed = FX32_ONE; 
		}
		GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj, 0, speed ); 
	} else {
		GFL_G3D_OBJECT_ResetAnimeFrame( g3Dobj, 0 );
	}
}

static void moveStopHaruka( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	minePos;
	int		scalar;
			
	GFL_G3D_SCENEOBJ_GetPos( sceneObj, &minePos );
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
	SetAlpha( sceneObj, 8, scalar );
}

static void moveWall( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	minePos;
	int		scalar;
			
	GFL_G3D_SCENEOBJ_GetPos( sceneObj, &minePos );
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
	//スカラーによる位置判定により半透明処理をする※ターゲット位置に相当するスカラー値
	if(( scalar <= 0x1800 )&& GFL_G3D_DOUBLE3D_GetFlip()){ 
		SetAlpha( sceneObj, 16, scalar );
	} else {
		ResetAlpha( sceneObj );
	}
}

static void moveSkelWall( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	minePos;
	int		scalar;
			
	GFL_G3D_SCENEOBJ_GetPos( sceneObj, &minePos );
#if 0
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
#else
	culling2DView( sceneObj, &minePos, &scalar );
	if( GFL_G3D_DOUBLE3D_GetFlip() ){ 
		OS_Printf("scalar = %x\n", scalar );
	}
#endif
	SetAlpha( sceneObj, 8, scalar );
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
 * @brief	コントロール初期化
 */
//------------------------------------------------------------------
static void KeyControlInit( void )
{
	tetsuWork->work[1] = 0;
	tetsuWork->mode = 3;
	tetsuWork->contSeq = 0;
	tetsuWork->contRotate = 0;
	tetsuWork->nowRotate = 0;
	tetsuWork->contPos.x = cameraTarget.x;
	tetsuWork->contPos.y = cameraTarget.y;
	tetsuWork->contPos.z = cameraTarget.z;
	tetsuWork->updateRotateFlag = FALSE;
	tetsuWork->updateRotate = 0;
	tetsuWork->updateRotateOffs = 0;
	tetsuWork->mainCameraRotate = 0;
	tetsuWork->autoCameraRotate1 = 0;
	tetsuWork->autoCameraRotate2 = 0;
}

//------------------------------------------------------------------
/**
 * @brief	コントロール終了チェック
 */
//------------------------------------------------------------------
static BOOL KeyControlEndCheck( void )
{
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		return TRUE;
	} else {
		return FALSE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	メインカメラ移動コントロール１
 */
//------------------------------------------------------------------
#define MOVE_SPEED ( 2 )
#define ROTATE_SPEED ( 0x200 )

static void KeyControlCameraMove1( void )
{
	switch( tetsuWork->contSeq ){

	case 0:
		{
			int cameraNum = tetsuWork->nowCameraNum;
			GFL_G3D_CAMERA* g3Dcamera;
			VecFx32 move = { 0, 0, 0 };
			VecFx32 cameraPos = { 0, 0, 0 };
			VecFx32 cameraTarget = { 0, 0, 0 };
			VecFx32 cameraOffs = { 0, 0, 0 };
			BOOL	moveFlag = FALSE;
			int		key = GFL_UI_KEY_GetCont();

			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
				if( tetsuWork->updateRotateFlag == FALSE ){
					tetsuWork->updateRotateFlag = TRUE;
					tetsuWork->updateRotate = tetsuWork->nowRotate;
					{
						s16 diff = tetsuWork->nowRotate - tetsuWork->mainCameraRotate;
						if( diff > 0 ){
							if( diff >= 0x6000 ){
								tetsuWork->updateRotateOffs = 0x1000;
							} else if( diff >= 0x4000 ){
								tetsuWork->updateRotateOffs = 0x1000;
							} else {
								tetsuWork->updateRotateOffs = 0x800;
							}
						} else {
							if( diff <= -0x6000 ){
								tetsuWork->updateRotateOffs = -0x1000;
							} else if( diff <= -0x4000 ){
								tetsuWork->updateRotateOffs = -0x1000;
							} else {
								tetsuWork->updateRotateOffs = -0x800;
							}
						}
					}
				}
			}
			if( GFL_UI_KEY_GetCont() & PAD_BUTTON_B ){
				tetsuWork->SpeedUpFlag = TRUE;
			} else {
				tetsuWork->SpeedUpFlag = FALSE;
			}

			if( (key & ( PAD_KEY_UP|PAD_KEY_LEFT )) == ( PAD_KEY_UP|PAD_KEY_LEFT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x2000;
				moveFlag = TRUE;

			} else if( (key & ( PAD_KEY_DOWN|PAD_KEY_LEFT )) == ( PAD_KEY_DOWN|PAD_KEY_LEFT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x6000;
				moveFlag = TRUE;

			} else if( (key & ( PAD_KEY_DOWN|PAD_KEY_RIGHT )) == ( PAD_KEY_DOWN|PAD_KEY_RIGHT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0xa000;
				moveFlag = TRUE;

			} else if( (key & ( PAD_KEY_UP|PAD_KEY_RIGHT )) == ( PAD_KEY_UP|PAD_KEY_RIGHT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0xe000;
				moveFlag = TRUE;

			} else if( (key & ( PAD_KEY_UP )) == ( PAD_KEY_UP ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x0000;
				moveFlag = TRUE;

			} else if( (key & ( PAD_KEY_LEFT )) == ( PAD_KEY_LEFT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x4000;
				moveFlag = TRUE;

			} else if( (key & ( PAD_KEY_DOWN )) == ( PAD_KEY_DOWN ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x8000;
				moveFlag = TRUE;

			} else if( (key & ( PAD_KEY_RIGHT )) == ( PAD_KEY_RIGHT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0xc000;
				moveFlag = TRUE;
			}

			if( moveFlag == TRUE ){
				int speed;

				if( tetsuWork->SpeedUpFlag == TRUE ){
					speed = -MOVE_SPEED*2;
				} else {
					speed = -MOVE_SPEED;
				}
				move.x = speed * FX_SinIdx( tetsuWork->nowRotate );
				move.z = speed * FX_CosIdx( tetsuWork->nowRotate );

				{
					VecFx32 tmpPos;
					u16 attrData;

					tmpPos.x = tetsuWork->contPos.x + move.x;
					tmpPos.z = tetsuWork->contPos.z + move.z;

					attrData = MapAttrGet( tetsuWork->mapAttr, &tmpPos );
					if( attrData == 0 ){
						tetsuWork->contPos.x = tmpPos.x;
						tetsuWork->contPos.z = tmpPos.z;
					}
				}
			}
			tetsuWork->contPos.y = 0;

			tetsuWork->autoCameraRotate1 += 0x0080;

			//下カメラ制御
			if( tetsuWork->updateRotateFlag == TRUE ){
				if( tetsuWork->mainCameraRotate != tetsuWork->updateRotate ){
					tetsuWork->mainCameraRotate += tetsuWork->updateRotateOffs;
					if( (tetsuWork->updateRotateOffs > 0x100 )
						||( tetsuWork->updateRotateOffs < -0x100 )){
						tetsuWork->updateRotateOffs /= 2;
					}
				}else{
					tetsuWork->contRotate = tetsuWork->updateRotate;
					tetsuWork->updateRotateFlag = FALSE;
				}
			}
			cameraOffs.x = 80 * FX_SinIdx( tetsuWork->mainCameraRotate );
			cameraOffs.y = FX32_ONE * 80;
			cameraOffs.z = 80 * FX_CosIdx( tetsuWork->mainCameraRotate );

			cameraPos.x = tetsuWork->contPos.x + cameraOffs.x;
			cameraPos.y = tetsuWork->contPos.y + cameraOffs.y;
			cameraPos.z = tetsuWork->contPos.z + cameraOffs.z;

			cameraTarget.x = tetsuWork->contPos.x;
			cameraTarget.y = tetsuWork->contPos.y + FX32_ONE*32;
			cameraTarget.z = tetsuWork->contPos.z;

			g3Dcamera = tetsuWork->g3Dcamera[0];
			GFL_G3D_CAMERA_SetPos( g3Dcamera, &cameraPos );
			GFL_G3D_CAMERA_SetTarget( g3Dcamera, &cameraTarget );

			//上カメラ制御
			cameraOffs.x = 32 * FX_SinIdx( tetsuWork->autoCameraRotate1 );
			cameraOffs.y = FX32_ONE * 32;
			cameraOffs.z = 32 * FX_CosIdx( tetsuWork->autoCameraRotate1 );

			cameraPos.x = tetsuWork->contPos.x + cameraOffs.x;
			cameraPos.y = tetsuWork->contPos.y + cameraOffs.y;
			cameraPos.z = tetsuWork->contPos.z + cameraOffs.z;

			cameraTarget.x = tetsuWork->contPos.x;
			cameraTarget.y = tetsuWork->contPos.y + FX32_ONE*8;
			cameraTarget.z = tetsuWork->contPos.z;

			g3Dcamera = tetsuWork->g3Dcamera[1];
			GFL_G3D_CAMERA_SetPos( g3Dcamera, &cameraPos );
			GFL_G3D_CAMERA_SetTarget( g3Dcamera, &cameraTarget );
		}
		break;
	}
}


//------------------------------------------------------------------
/**
 * @brief		セットアップ用３Ｄマップデータ作成
 */
//------------------------------------------------------------------
static const  GFL_G3D_OBJSTATUS defaultStatus = {
	{ 0, 0, 0 },
	{ FX32_ONE, FX32_ONE, FX32_ONE },
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
};
#define mapSizeX	(32)
#define mapSizeZ	(32)
#define mapGrid		(FX32_ONE*64)
#define defaultMapX	(-mapGrid*16+mapGrid/2)
#define defaultMapZ	(-mapGrid*16+mapGrid/2)

static void GetWallData
			( GFL_G3D_SCENEOBJ_DATA* data, u16 chkCode, u16 up, u16 down, u16 left, u16 right ); 

static inline u16 GET_MAPCODE( u16* mapdata, int x, int z )
{
	u16	tmpdata;

	if(( x < 0 )||( x >= mapSizeX )||( z < 0 )||( z >= mapSizeZ )){
		return  '■';
	}
	tmpdata = mapdata[ z * mapSizeX + x ];
	return  (( tmpdata & 0x00ff ) << 8 ) + (( tmpdata & 0xff00 ) >> 8 );
}

static GFL_G3D_SCENEOBJ_DATA_SETUP* MapDataCreate( const MAPDATA* map, HEAPID heapID )
{
	GFL_G3D_SCENEOBJ_DATA_SETUP*	setup;
	GFL_G3D_SCENEOBJ_DATA*			pdata;
	GFL_G3D_SCENEOBJ_DATA			data;
	u16*							mapAttr;
	int count = 0;
	int		x,z;
	u16*	mapData = (u16*)map->data;	//めんどいので全角のみに制限
	int	sizeX = mapSizeX;
	int	sizeZ = mapSizeZ;
	u16 mapCode;

	mapAttr = GFL_HEAP_AllocMemory( heapID, 2*sizeX*sizeZ );
	setup = GFL_HEAP_AllocMemoryLo( heapID, sizeof(GFL_G3D_SCENEOBJ_DATA_SETUP) );
	pdata = GFL_HEAP_AllocMemoryLo( heapID, sizeof(GFL_G3D_SCENEOBJ_DATA)*sizeX*sizeZ+1 );

	pdata[ count ] = map->floor;
	count++;

	for( z=0; z<sizeZ; z++ ){
		for( x=0; x<sizeX; x++ ){
			mapCode = GET_MAPCODE( mapData, x, z );
			switch( mapCode ){
			default:
			case '　':
				mapAttr[z*sizeX+x] = 0;
				break;
			case '■':	//壁
				GetWallData( &data, mapCode, 
							GET_MAPCODE( mapData, x, z-1 ), GET_MAPCODE( mapData, x, z+1 ), 
							GET_MAPCODE( mapData, x-1, z ), GET_MAPCODE( mapData, x+1, z ) );
				data.status.trans.x = defaultMapX + (mapGrid * x);
				data.status.trans.z = defaultMapZ + (mapGrid * z);
				data.func = moveWall;
				pdata[ count ] = data;
				count++;
				mapAttr[z*sizeX+x] = 1;
				break;
			case '□':	//透過壁
				GetWallData( &data, mapCode, 
							GET_MAPCODE( mapData, x, z-1 ), GET_MAPCODE( mapData, x, z+1 ), 
							GET_MAPCODE( mapData, x-1, z ), GET_MAPCODE( mapData, x+1, z ) );
				data.status.trans.x = defaultMapX + (mapGrid * x);
				data.status.trans.z = defaultMapZ + (mapGrid * z);
				data.func = moveSkelWall;
				pdata[ count ] = data;
				count++;
				mapAttr[z*sizeX+x] = 1;
				break;
			case '○':	//配置人物１
				data.objID = G3DOBJ_HARUKA_STOP; 
				data.movePriority = 0;
				data.drawPriority = 250;
				data.cullingFlag = TRUE;
				data.drawSW = TRUE;
				data.status = defaultStatus;
				data.blendAlpha = 31;
				data.status.trans.x = defaultMapX + (mapGrid * x);
				data.status.trans.z = defaultMapZ + (mapGrid * z);
				data.status.scale.x = FX32_ONE*8;
				data.status.scale.y = FX32_ONE*8;
				data.status.scale.z = FX32_ONE*8;
				data.func = moveStopHaruka;
				pdata[ count ] = data;
				count++;
				mapAttr[z*sizeX+x] = 0;
				break;
			case '◎':	//プレーヤー
				tetsuWork->contPos.x = defaultMapX + (mapGrid * x);
				tetsuWork->contPos.y = 0;
				tetsuWork->contPos.z = defaultMapZ + (mapGrid * z);
				mapAttr[z*sizeX+x] = 0;
				break;
			case '●':	//配置人物２
				mapAttr[z*sizeX+x] = 0;
				break;
			}
		}
	}
	setup->data = pdata;
	setup->count = count;
	setup->mapAttr = mapAttr;

	return setup;
}

static void MapDataDelete( GFL_G3D_SCENEOBJ_DATA_SETUP* setup )
{
	GFL_HEAP_FreeMemory( (void*)setup->data );
	GFL_HEAP_FreeMemory( setup );
}

static void MapAttrDelete( u16* mapAttr )
{
	GFL_HEAP_FreeMemory( mapAttr );
}

static u16 MapAttrGet( u16* mapAttr, VecFx32* pos )
{
	int x = (pos->x + mapGrid*16)/mapGrid;
	int z = (pos->z + mapGrid*16)/mapGrid;
	return mapAttr[z*mapSizeX+x];
}

static const char mapData1[] = {
"■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■"	//0
"■　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　■"	//30
"■　　　○　　　　　　　　　　　　　　　　○　　　　　　　　　■"	//2
"■　　■■■■■■■■■■■■■■■■■■■■■■■■■■　　■"	//28
"■　　■　　　　　　　　　　　　○　　　　　　　　　　　■　　■"	//4
"■　　■　■■■■■■■■■■■■■■■■■■■■■■　■　　■"	//26
"■　　■　■　　　　○　　　　　　　　　　　　　　　■　■　　■"	//6
"■　　■　■　■■■■■■■■■■■■■■■■■■　■　■　　■"	//24
"■　　■　■　■　　　　　　　　　　　　○　　　■　■　■　　■"	//8
"■　　■　■　■　■■■■■■■■■■■■■■　■○■　■　　■"	//22
"■　　■　■　■　■　　　　　　　○　　　　■　■　■　■　　■"	//10
"■　　■　■　■　■○■■■■■■■■■■　■　■　■　■　　■"	//20
"■　　■　■　■　■　■　　　　　　　　■　■　■　■○■　　■"	//12
"■　　■　■　■　■　■　■■■■■■　■　■　■　■　■　　■"	//18//210
"■　　■　■　■　■　■　■　　　　■○■　■　■　■　■　　■"	//14
"■　　■○■　■　■　■　■　●　　■　■　■　■　■　■○　■"	//14
"■　　■　■　■　■　■○■　　　　■　■　■　■　■　■　　■"	//14
"■　　■　■　■　■　■　■　　　　■　■　■　■　■　■　　■"	//14//56
"■　○■　■　■　■　■　■　■■■■　■○■　■　■　■　　■"	//17
"■　　■　■　■　■　　　　　　　　　　■　■　■　■　■　　■"	//11
"■　　■　■　■　■　■■■■■■■■■■　■　■　■　■　　■"	//20
"■　　■　■○■　■　　　　　○　　　　　　■　■　■　■　　■"	//10
"■　　■　■　■　■　■■■■■■■■■■■■　■　■　■　　■"	//21
"■　○■　■　　　　　○　　　　　　　　　○　　■　■　■　　■"	//7
"■　　■　■　■■■■■■■■■■■■■■■■■■　■　■　　■"	//24
"■　　■　■　　　　　　　　　　○　　　　　　　　　■　■　　■"	//6
"■　　■　■　■■■■■■■■■■■■■■■■■■■■　■○　■"	//25
"■　　　　　　　　　　○　　　　　　　　　　○　　　　　■　　■"	//3
"■　　□■■■■■■■■■■■■■■■■■■■■■■■■■　　■"	//28
"■　　　　　○　　　　　　　　　　○　　　　　　　　　　　　　■"	//2
"■　　　　　　　　　　　　○　　　　　　　　　　　　　　　　　■"	//29
"■　◎　■■■■■■■■■■■■■■■■■■■■■■■■■■■■"	//0/210-7
};//210+56+203-19 = 450

static const char mapData2[] = {
"■　■　　　　　　　　　　　■　　■　　　■　　　　■　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　■　　　　■　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　■　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　■　　　　■　　　　　"	//0
"■　■■■■　■■■■■■■■　　■　　　■　　　　　　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　■■■■■■■■■■■"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　　　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　　　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　　　　　　　"	//0
"■　■■■■■■■■　■■■■　　■■■■■■■　　■■■■■■"	//0
"■　■　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//0
"■　■　　　　　■■■■■■■■■■■■■■■■■■■■　　■■"	//0
"■　■　　　　　■　　　　　　　　　　　　　■　　　　　　　　　"	//0
"■　■　　　　　■　　□□□■■■□□□　　■　　　　　　　　　"	//0
"■　■　　　　　■　　□　　　　　　　□　　■　　　　　　　　　"	//0
"■　■■■■　　■　　□　　　　　　　□　　■　　　　　　　　　"	//0
"　　　　　　　　■　　□　　　　　　　□　　■　　■■■■■■■"	//0
"　　　　　　　　■　　□　　　　　　　□　　■　　■　　　　　　"	//0
"　　　■　　　　■　　□□□　　　□□□　　■　　■　　　　　　"	//0
"　　　■　　　　■　　　　　　　　　　　　　■　　■■■■■■　"	//0
"　　　■　　　　■　　　　　　　　　　　　　■　　■　　　　　　"	//0
"　　　■　　　　■　　　　　　　　　　　　　■　　■　　　　　　"	//0
"　　　■■■■■■■■■■■■■■■■■　　■　　■■■■■■　"	//0
"　　　■　　　　　　　　　　　　　　　　　　　　　■　　　　　　"	//0
"　　　■　　　　　　　　　　　　　　　　　　　　　■　　　　　　"	//0
"　　　■■■■■■■■■■■■■■■■■■■■■■■■■■■■　"	//0
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//0
"　　　■■■■■■■■■■■■　　■■■■■■■■■■　■■■■"	//0
"　　　■■　　　　　　　　　　　　　　　　　　　　■■　　　　　"	//0
"　　　■■　　　　　■■■■■■■■■■■■■　　■■　　　　　"	//0
"　◎　■■　　　　　■■■■■■■■■■■■■　　■■　　　　　"	//0
"　　　■■　　　　　　　　　　　　　　　　　　　　■■　　　　　"	//0
};//210+56+203-19 = 450

static const char mapData3[] = {
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　○　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　◎　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
};

static const MAPDATA mapDataTbl = {
	mapData1,
	{	G3DOBJ_MAP_FLOOR, 0, 1, 31, FALSE, TRUE,
		{	{ 0, 0, 0 },
			{ FX32_ONE*8, FX32_ONE*8, FX32_ONE*8 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

typedef struct {
	u16		wallCode;
	fx32	rotateY;
}WALL_CODE_TBL;

static const WALL_CODE_TBL wallCodeTbl[] = {	//上下左右の壁あり状態
	{ G3DOBJ_MAP_WALL,		0x0000 },			//空空空空
	{ G3DOBJ_MAP_WALL3,		0x8000 },			//壁空空空
	{ G3DOBJ_MAP_WALL3,		0x0000 },			//空壁空空
	{ G3DOBJ_MAP_WALL2_2,	0x0000 },			//壁壁空空
	{ G3DOBJ_MAP_WALL3,		0xc000 },			//空空壁空
	{ G3DOBJ_MAP_WALL2_1,	0xc000 },			//壁空壁空
	{ G3DOBJ_MAP_WALL2_1,	0x0000 },			//空壁壁空
	{ G3DOBJ_MAP_WALL1,		0x4000 },			//壁壁壁空
	{ G3DOBJ_MAP_WALL3,		0x4000 },			//空空空壁
	{ G3DOBJ_MAP_WALL2_1,	0x8000 },			//壁空空壁
	{ G3DOBJ_MAP_WALL2_1,	0x4000 },			//空壁空壁
	{ G3DOBJ_MAP_WALL1,		0xc000 },			//壁壁空壁
	{ G3DOBJ_MAP_WALL2_2,	0x4000 },			//空空壁壁
	{ G3DOBJ_MAP_WALL1,		0x0000 },			//壁空壁壁
	{ G3DOBJ_MAP_WALL1,		0x8000 },			//空壁壁壁
	{ G3DOBJ_MAP_WALL0,		0x0000 },			//壁壁壁壁
};

static void GetWallData
			( GFL_G3D_SCENEOBJ_DATA* data, u16 chkCode, u16 up, u16 down, u16 left, u16 right ) 
{
	u16 pattern = 0;
	VecFx32 rotate = {0,0,0};
	MtxFx33 rotMtx = defaultStatus.rotate;

	if( up == chkCode ){
		pattern |= 0x0001;
	}
	if( down == chkCode ){
		pattern |= 0x0002;
	}
	if( left == chkCode ){
		pattern |= 0x0004;
	}
	if( right == chkCode ){
		pattern |= 0x0008;
	}
	data->objID = wallCodeTbl[ pattern ].wallCode;
	data->movePriority = 0;
	data->drawPriority = 2;
	data->cullingFlag = TRUE;
	data->drawSW = TRUE;
	data->blendAlpha = 31;
	data->status = defaultStatus;

	rotate.y = wallCodeTbl[ pattern ].rotateY;
	rotateCalc( &rotate, &rotMtx );
	data->status.rotate = rotMtx;
}
















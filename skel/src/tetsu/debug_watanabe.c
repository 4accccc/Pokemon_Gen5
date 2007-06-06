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
	u8						spa_work[PARTICLE_LIB_HEAP_SIZE];
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
	BOOL					MoveFlag;
	BOOL					SpeedUpFlag;

	u16						mainCameraRotate;
	u16						autoCameraRotate1;
	u16						autoCameraRotate2;

	u16*					mapAttr;
	u16						accesaryType;

	GFL_TCB*				dbl3DdispVintr;
	int						vBlankCounter;
	u16						nowAnmIdx;
	u16						nowAccesary;

	GFL_PTC_PTR				ptc;

}TETSU_WORK;

typedef struct {
	const GFL_G3D_SCENEOBJ_DATA*	data;
	int								count;
	u16*							mapAttr;
}GFL_G3D_SCENEOBJ_DATA_SETUP;

typedef struct {
	int		seq;
}PLAYER_WORK;

typedef struct {
	int		seq;

	VecFx32	rotateTmp;
}OBJ_WORK;

typedef struct {
	const char*				data;
	GFL_G3D_SCENEOBJ_DATA	floor;
}MAPDATA;

u32 DebugCullingCount = 0;
u32 DebugDrawCount = 0;
//------------------------------------------------------------------
/**
 * @brief	マップデータ
 */
//------------------------------------------------------------------
static const VecFx32 camera0Target	= { 0, 0, 0 };
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

//アーカイブテーブル
static	const	char	*GraphicFileTable[]={
	"src/sample_graphic/spa.narc",
};

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
		//ARCシステム初期化
		GFL_ARC_Init( &GraphicFileTable[0], 1 );

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
		tetsuWork->vBlankCounter = GFUser_VIntr_GetVblankCounter(); 
		tetsuWork->vBlankCounter = 1;
		GFUser_VIntr_ResetVblankCounter(); 

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

		GFL_ARC_Exit();

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
		plt[0] = GX_RGB( 20, 20, 31 );
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
	BOOL result;

	//配置物設定
	tetsuWork->g3Dutil = GFL_G3D_UTIL_Create( &g3Dutil_setup, heapID );
	tetsuWork->g3Dscene = GFL_G3D_SCENE_Create( tetsuWork->g3Dutil, 1000,
												sizeof(OBJ_WORK), 32, TRUE, heapID );
	{
		//自機作成
		tetsuWork->g3DsceneObjID = GFL_G3D_SCENEOBJ_Add
							( tetsuWork->g3Dscene, g3DsceneObjData, NELEMS(g3DsceneObjData) );
		//アニメーションの初期有効設定
		GFL_G3D_SCENEOBJ_EnableAnime
				( GFL_G3D_SCENEOBJ_Get( tetsuWork->g3Dscene, G3DSCOBJ_PLAYER), 0 );
		tetsuWork->nowAnmIdx = 0;
		tetsuWork->nowAccesary = 0;
	}
	{
		//マップ作成
		GFL_G3D_SCENEOBJ_DATA_SETUP* setup = MapDataCreate( &mapDataTbl, heapID );
		tetsuWork->g3DsceneMapObjID = GFL_G3D_SCENEOBJ_Add
								( tetsuWork->g3Dscene, setup->data, setup->count );
		tetsuWork->g3DsceneMapObjCount = setup->count;
		tetsuWork->mapAttr = setup->mapAttr;
		MapDataDelete( setup );
	}

	//カメラ作成
	tetsuWork->g3Dcamera[0] = GFL_G3D_CAMERA_CreateDefault( &camera0Pos, &camera0Target, heapID );
	{
		fx32 far = 4096 << FX32_SHIFT;
		GFL_G3D_CAMERA_SetFar( tetsuWork->g3Dcamera[0], &far );
	}
	tetsuWork->g3Dcamera[1] = GFL_G3D_CAMERA_CreateDefault( &camera1Pos, &camera0Target, heapID );

	//ライト作成
	tetsuWork->g3Dlightset[0] = GFL_G3D_LIGHT_Create( &light0Setup, heapID );
	tetsuWork->g3Dlightset[1] = GFL_G3D_LIGHT_Create( &light1Setup, heapID );

	//カメラライト0反映
	GFL_G3D_CAMERA_Switching( tetsuWork->g3Dcamera[0] );
	GFL_G3D_LIGHT_Switching( tetsuWork->g3Dlightset[0] );
	tetsuWork->nowCameraNum = 0;
	tetsuWork->nowLightNum = 0;

	tetsuWork->work[0] = 0;

	//パーティクルリソース読み込み
	tetsuWork->ptc=GFL_PTC_Create
			(tetsuWork->spa_work,PARTICLE_LIB_HEAP_SIZE,TRUE,tetsuWork->heapID);
	GFL_PTC_SetResource(tetsuWork->ptc,GFL_PTC_LoadArcResource(0,0,tetsuWork->heapID),TRUE,NULL);
}
	
//動作
static void g3d_move( void )
{
	DebugCullingCount = 0;
	DebugDrawCount = 0;
	GFL_G3D_SCENE_Main( tetsuWork->g3Dscene );  
	tetsuWork->work[0]++;

	if(GFL_PTC_GetEmitterNum(tetsuWork->ptc)<5){
		VecFx32	pos={0,0,0};
		GFL_PTC_CreateEmitter(tetsuWork->ptc,0,&pos);
	}
}

//描画
static void g3d_draw( void )
{
	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		GFL_G3D_CAMERA_Switching( tetsuWork->g3Dcamera[0] );
		//OS_Printf("CullingCount = %d\n",DebugCullingCount);
		//OS_Printf("DrawCount = %d\n",DebugDrawCount);
		GFL_G3D_SCENE_SetDrawParticleSW( tetsuWork->g3Dscene, TRUE );
	} else {
		GFL_G3D_CAMERA_Switching( tetsuWork->g3Dcamera[1] );
		GFL_G3D_SCENE_SetDrawParticleSW( tetsuWork->g3Dscene, FALSE );
	}
	GFL_G3D_SCENE_Draw( tetsuWork->g3Dscene );  
	GFL_G3D_DOUBLE3D_SetSwapFlag();
}
	
//破棄
static void g3d_unload( void )
{
	GFL_PTC_Delete(tetsuWork->ptc);

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
#define VIEW_LENGTH		(64*2)
#define VIEW_SCALAR_MAX	(0x20000)

#define BETWEEN_SCALAR	(-0x01000000)
#define BETWEEN_THETA	(0x01100)

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

//対象物とカメラとの視界位置によるカリング（ＸＺのみ）
static BOOL checkPos_bitweenCamera( GFL_G3D_CAMERA* camera, u16 theta, VecFx32* objPos )
{
	VecFx32 cameraPos, cameraTarget;
	VecFx32 vecView, vecObj;
	VecFx32 vecA_cross, vecB_cross;
	fx32	sin, cos;
	int		scalarA, scalarB;
	s16		tmpTheta;

	//カメラ情報取得
	GFL_G3D_CAMERA_GetPos( camera, &cameraPos );
	GFL_G3D_CAMERA_GetTarget( camera, &cameraTarget );

	//視界ベクトル計算（整数部のみ）
	vecView.x = ( cameraTarget.x - cameraPos.x ) >> FX32_SHIFT;
	vecView.z = ( cameraTarget.z - cameraPos.z ) >> FX32_SHIFT;

	//対象物体ベクトル計算（整数部のみ）
	vecObj.x = ( objPos->x - cameraPos.x ) >> FX32_SHIFT;
	vecObj.z = ( objPos->z - cameraPos.z ) >> FX32_SHIFT;

	//カメラの方向から左側境界ベクトルＡに直交するベクトルを作成
	tmpTheta = -theta + 0x4000;	//負方向θ+90°の角度値
	sin = FX_SinIdx((u16)tmpTheta);
	cos = FX_CosIdx((u16)tmpTheta);
	vecA_cross.x = (vecView.x * cos ) - (vecView.z * sin );
	vecA_cross.z = (vecView.x * sin ) + (vecView.z * cos );

	//カメラの方向から右側境界ベクトルＢに直交するベクトルを作成
	tmpTheta = theta - 0x4000;	//正方向θ-90°の角度値
	sin = FX_SinIdx((u16)tmpTheta);
	cos = FX_CosIdx((u16)tmpTheta);
	vecB_cross.x = (vecView.x * cos ) - (vecView.z * sin );
	vecB_cross.z = (vecView.x * sin ) + (vecView.z * cos );

	//ベクトルＡに直交するベクトルとベクトルＢに直交するベクトルに対しての内積が
	//ともに指定スカラー値範囲内であれば２方向間に存在すると決定する
	scalarA = vecA_cross.x * vecObj.x + vecA_cross.z * vecObj.z;
	scalarB = vecB_cross.x * vecObj.x + vecB_cross.z * vecObj.z;
	if(( scalarA < BETWEEN_SCALAR )||( scalarB < BETWEEN_SCALAR )){
		return FALSE;
	} else {
		return TRUE;
	}
}

//対象物の周囲位置によるカリング（ＸＺのみ）
static BOOL checkPos_aroundTarget( GFL_G3D_CAMERA* camera, fx32 len, VecFx32* objPos )
{
	VecFx32 cameraTarget;
	fx32	diffX, diffZ, diffLen;
				
	GFL_G3D_CAMERA_GetTarget( camera, &cameraTarget );

	diffX = ( cameraTarget.x - objPos->x ) >> FX32_SHIFT;
	diffZ = ( cameraTarget.z - objPos->z ) >> FX32_SHIFT;
	diffLen = ( diffX * diffX + diffZ * diffZ );
	if( diffLen > len ){
		return FALSE;
	} else {
		return TRUE;
	}
}

//２Ｄカリング
static BOOL culling2DView( GFL_G3D_SCENEOBJ* sceneObj, VecFx32* objPos, int* scalar )
{
	GFL_G3D_CAMERA*	camera;

	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		//カメラポインタ取得
		camera = tetsuWork->g3Dcamera[0];	//下カメラ
		//カメラ位置による事前カリング
		*scalar = GFL_G3D_CAMERA_GetDotProductXZfast( camera, objPos );
		//カメラとのスカラーによる位置判定(0は水平、正は前方、負は後方)
		if( *scalar < -0x800 ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
		//ターゲット視界判定によるカリング
		if( checkPos_bitweenCamera( camera, BETWEEN_THETA, objPos ) == FALSE ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
		DebugDrawCount++;
	} else {
		//カメラポインタ取得
		camera = tetsuWork->g3Dcamera[1];	//上カメラ
		//カメラ位置による事前カリング
		*scalar = GFL_G3D_CAMERA_GetDotProductXZfast( camera, objPos );
		//カメラとのスカラーによる位置判定(0は水平、正は前方、負は後方)
		if( *scalar < -0x800 ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
		//ターゲット周囲判定によるカリング
		if( checkPos_aroundTarget( camera, VIEW_LENGTH * VIEW_LENGTH, objPos ) == FALSE ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
	}
	SetDrawSW( sceneObj, TRUE );
	return TRUE;
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
	PLAYER_WORK*	player = (PLAYER_WORK*)work;

	trans.x = tetsuWork->contPos.x;
	trans.y = 0;
	trans.z = tetsuWork->contPos.z;
	rotVec.x = 0;
	rotVec.y = tetsuWork->nowRotate + 0x8000;
	rotVec.z = 0;
	rotateCalc( &rotVec, &rotate );

	GFL_G3D_SCENEOBJ_SetPos( sceneObj, &trans );
	GFL_G3D_SCENEOBJ_SetRotate( sceneObj, &rotate );

	switch( player->seq ){
	case 0:
		{
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
				if( tetsuWork->nowAccesary ){
					GFL_G3D_SCENEOBJ_DisableAnime( sceneObj, tetsuWork->nowAnmIdx );
					switch( tetsuWork->nowAccesary ){
					defasult:
						tetsuWork->nowAnmIdx = 0;
						break;
					case 1:
						tetsuWork->nowAnmIdx = 4;
						break;
					case 2:
						tetsuWork->nowAnmIdx = 3;
						break;
					case 3:
						tetsuWork->nowAnmIdx = 4;
						break;
					}
					GFL_G3D_SCENEOBJ_EnableAnime( sceneObj, tetsuWork->nowAnmIdx );
					player->seq = 1;
				}
				break;
			}
		}
		{
			u16		anmIdx;
			fx32	anmSpeed;
	
			if( tetsuWork->MoveFlag == TRUE ){
				if( tetsuWork->SpeedUpFlag == TRUE ){
					anmIdx = 2;
					anmSpeed = FX32_ONE*4;
				} else {
					anmIdx = 1;
					anmSpeed = FX32_ONE*3;
				}
			} else {
				anmIdx = 0;
				anmSpeed = FX32_ONE*2;
			}
			if( tetsuWork->nowAnmIdx != anmIdx ){
				GFL_G3D_SCENEOBJ_DisableAnime( sceneObj, tetsuWork->nowAnmIdx );
				tetsuWork->nowAnmIdx = anmIdx;
				GFL_G3D_SCENEOBJ_EnableAnime( sceneObj, tetsuWork->nowAnmIdx );
			}
			GFL_G3D_SCENEOBJ_LoopAnimeFrame( sceneObj, tetsuWork->nowAnmIdx, anmSpeed ); 
		}
		{
			//アクセサリーの変更
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
				BOOL result;
				MtxFx33	rotate;
				VecFx32	rotVec = { 0, 0, 0 };

				//アクセサリーを外す(設定されてない場合はなにもしないで帰ってくる)
				GFL_G3D_SCENEOBJ_ACCESORY_Remove( sceneObj );

				//ナンバー切り替え
				tetsuWork->nowAccesary++;
				tetsuWork->nowAccesary&= 3;

				switch( tetsuWork->nowAccesary ){
				case 0:	//手ぶら
					break;
				case 1:	//ハンマー
					result = GFL_G3D_SCENEOBJ_ACCESORY_Add( sceneObj,
									g3DsceneAccesoryData1, NELEMS(g3DsceneAccesoryData1) );
					GF_ASSERT( result == TRUE );
					rotVec.x = 0x8000;
					rotVec.z = 0x4000;
					rotateCalc( &rotVec, &rotate );
					GFL_G3D_SCENEOBJ_ACCESORY_SetRotate( sceneObj, 0, &rotate );
					break;
				case 2:	//弓
					result = GFL_G3D_SCENEOBJ_ACCESORY_Add( sceneObj,
									g3DsceneAccesoryData2, NELEMS(g3DsceneAccesoryData2) );
					GF_ASSERT( result == TRUE );
					rotVec.z = 0x4000;
					rotateCalc( &rotVec, &rotate );
					GFL_G3D_SCENEOBJ_ACCESORY_SetRotate( sceneObj, 0, &rotate );
					break;
				case 3:	//杖
					result = GFL_G3D_SCENEOBJ_ACCESORY_Add( sceneObj,
									g3DsceneAccesoryData3, NELEMS(g3DsceneAccesoryData3) );
					GF_ASSERT( result == TRUE );
					rotVec.z = 0x4000;
					rotateCalc( &rotVec, &rotate );
					GFL_G3D_SCENEOBJ_ACCESORY_SetRotate( sceneObj, 0, &rotate );
					break;
				}
			}
		}
		break;
	case 1:
		{
			BOOL result;

			result = GFL_G3D_SCENEOBJ_IncAnimeFrame( sceneObj, tetsuWork->nowAnmIdx, FX32_ONE*2 );
			if( result == FALSE ){
				GFL_G3D_SCENEOBJ_ResetAnimeFrame( sceneObj, tetsuWork->nowAnmIdx );
				GFL_G3D_SCENEOBJ_DisableAnime( sceneObj, tetsuWork->nowAnmIdx );
				tetsuWork->nowAnmIdx = 0;
				GFL_G3D_SCENEOBJ_EnableAnime( sceneObj, tetsuWork->nowAnmIdx );
				player->seq = 0;
			}
		}
		break;
	}
}

static void moveStopHaruka( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	minePos;
	int		scalar;
			
	GFL_G3D_SCENEOBJ_GetPos( sceneObj, &minePos );
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
	SetAlpha( sceneObj, 16, scalar );
}

static void moveWall( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	minePos;
	int		scalar;
			
	GFL_G3D_SCENEOBJ_GetPos( sceneObj, &minePos );
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
	//スカラーによる位置判定により半透明処理をする※ターゲット位置に相当するスカラー値
	if(( scalar < 0x800 )&& GFL_G3D_DOUBLE3D_GetFlip()){ 
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
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
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
	tetsuWork->contPos.x = camera0Target.x;
	tetsuWork->contPos.y = camera0Target.y;
	tetsuWork->contPos.z = camera0Target.z;
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
#define MOVE_SPEED ( 1 )
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
			int		key = GFL_UI_KEY_GetCont();
#if 0
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
				//if( tetsuWork->updateRotateFlag == FALSE ){
					tetsuWork->updateRotateFlag = TRUE;
					tetsuWork->updateRotate = tetsuWork->nowRotate;
				//}
			}
#endif
			if( GFL_UI_KEY_GetCont() & PAD_BUTTON_B ){
				tetsuWork->SpeedUpFlag = TRUE;
			} else {
				tetsuWork->SpeedUpFlag = FALSE;
			}
#if 0
			if( (key & ( PAD_KEY_UP|PAD_KEY_LEFT )) == ( PAD_KEY_UP|PAD_KEY_LEFT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x2000;
				tetsuWork->MoveFlag = TRUE;

			} else if( (key & ( PAD_KEY_DOWN|PAD_KEY_LEFT )) == ( PAD_KEY_DOWN|PAD_KEY_LEFT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x6000;
				tetsuWork->MoveFlag = TRUE;

			} else if( (key & ( PAD_KEY_DOWN|PAD_KEY_RIGHT )) == ( PAD_KEY_DOWN|PAD_KEY_RIGHT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0xa000;
				tetsuWork->MoveFlag = TRUE;

			} else if( (key & ( PAD_KEY_UP|PAD_KEY_RIGHT )) == ( PAD_KEY_UP|PAD_KEY_RIGHT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0xe000;
				tetsuWork->MoveFlag = TRUE;

			} else if( (key & ( PAD_KEY_UP )) == ( PAD_KEY_UP ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x0000;
				tetsuWork->MoveFlag = TRUE;

			} else if( (key & ( PAD_KEY_LEFT )) == ( PAD_KEY_LEFT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x4000;
				tetsuWork->MoveFlag = TRUE;

			} else if( (key & ( PAD_KEY_DOWN )) == ( PAD_KEY_DOWN ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x8000;
				tetsuWork->MoveFlag = TRUE;

			} else if( (key & ( PAD_KEY_RIGHT )) == ( PAD_KEY_RIGHT ) ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0xc000;
				tetsuWork->MoveFlag = TRUE;
			}
#else
			tetsuWork->MoveFlag = FALSE;

			if( key & PAD_KEY_UP ){
				tetsuWork->nowRotate = tetsuWork->contRotate;
				tetsuWork->MoveFlag = TRUE;
			}
			if( key & PAD_KEY_LEFT ){
				if( key & PAD_KEY_UP ){
					tetsuWork->contRotate += 0x100;
					tetsuWork->nowRotate = tetsuWork->contRotate;
				} else {
					//tetsuWork->contRotate += 0x200;
					//tetsuWork->nowRotate = tetsuWork->contRotate;
					tetsuWork->nowRotate = tetsuWork->contRotate + 0x4000;
					tetsuWork->MoveFlag = TRUE;
				}
			}
			if( key & PAD_KEY_DOWN ){
				tetsuWork->nowRotate = tetsuWork->contRotate + 0x8000;
				tetsuWork->MoveFlag = TRUE;
			}
			if( key & PAD_KEY_RIGHT ){
				if( key & PAD_KEY_UP ){
					tetsuWork->contRotate -= 0x100;
					tetsuWork->nowRotate = tetsuWork->contRotate;
				} else {
					//tetsuWork->contRotate -= 0x200;
					//tetsuWork->nowRotate = tetsuWork->contRotate;
					tetsuWork->nowRotate = tetsuWork->contRotate - 0x4000;
					tetsuWork->MoveFlag = TRUE;
				}
			}
#endif
			if( tetsuWork->MoveFlag == TRUE ){
				int speed;

				if( tetsuWork->SpeedUpFlag == TRUE ){
					speed = -MOVE_SPEED*2;
				} else {
					speed = -MOVE_SPEED;
				}
				speed *= tetsuWork->vBlankCounter; 
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

			//下カメラ制御
#if 0
			if( tetsuWork->updateRotateFlag == TRUE ){
				int i;
				for( i=0; i<tetsuWork->vBlankCounter; i++ ){
					if( tetsuWork->mainCameraRotate > tetsuWork->updateRotate ){
						tetsuWork->mainCameraRotate -= 0x200;
						if( tetsuWork->mainCameraRotate <= tetsuWork->updateRotate ){
							tetsuWork->contRotate = tetsuWork->updateRotate;
							tetsuWork->updateRotateFlag = FALSE;
						}
					}else if( tetsuWork->mainCameraRotate < tetsuWork->updateRotate ){
						tetsuWork->mainCameraRotate += 0x200;
						if( tetsuWork->mainCameraRotate >= tetsuWork->updateRotate ){
							tetsuWork->contRotate = tetsuWork->updateRotate;
							tetsuWork->updateRotateFlag = FALSE;
						}
					} else {
						tetsuWork->contRotate = tetsuWork->updateRotate;
						tetsuWork->updateRotateFlag = FALSE;
					}
				}
			} else {
				tetsuWork->mainCameraRotate = tetsuWork->contRotate;
			}
#else
			tetsuWork->mainCameraRotate = tetsuWork->contRotate;
#endif
			//cameraOffs.x = 80 * FX_SinIdx( tetsuWork->mainCameraRotate );
			//cameraOffs.y = FX32_ONE * 80;
			//cameraOffs.y = FX32_ONE * 48;
			//cameraOffs.z = 80 * FX_CosIdx( tetsuWork->mainCameraRotate );
			cameraOffs.x = 40 * FX_SinIdx( tetsuWork->mainCameraRotate );
			cameraOffs.y = FX32_ONE * 24;
			cameraOffs.z = 40 * FX_CosIdx( tetsuWork->mainCameraRotate );

			cameraPos.x = tetsuWork->contPos.x + cameraOffs.x;
			cameraPos.y = tetsuWork->contPos.y + cameraOffs.y;
			cameraPos.z = tetsuWork->contPos.z + cameraOffs.z;

			cameraTarget.x = tetsuWork->contPos.x;
			//cameraTarget.y = tetsuWork->contPos.y + FX32_ONE*32;
			cameraTarget.y = tetsuWork->contPos.y + FX32_ONE*16;
			cameraTarget.z = tetsuWork->contPos.z;

			g3Dcamera = tetsuWork->g3Dcamera[0];
			GFL_G3D_CAMERA_SetPos( g3Dcamera, &cameraPos );
			GFL_G3D_CAMERA_SetTarget( g3Dcamera, &cameraTarget );

			//上カメラ制御
			//cameraOffs.x = 32 * FX_SinIdx( tetsuWork->autoCameraRotate1 );
			//cameraOffs.y = FX32_ONE * 32;
			//cameraOffs.z = 32 * FX_CosIdx( tetsuWork->autoCameraRotate1 );
			cameraOffs.x = 24 * FX_SinIdx( tetsuWork->autoCameraRotate1 );
			cameraOffs.y = FX32_ONE * 24;
			cameraOffs.z = 24 * FX_CosIdx( tetsuWork->autoCameraRotate1 );

			cameraPos.x = tetsuWork->contPos.x + cameraOffs.x;
			cameraPos.y = tetsuWork->contPos.y + cameraOffs.y;
			cameraPos.z = tetsuWork->contPos.z + cameraOffs.z;

			cameraTarget.x = tetsuWork->contPos.x;
			//cameraTarget.y = tetsuWork->contPos.y + FX32_ONE*8;
			cameraTarget.y = tetsuWork->contPos.y + FX32_ONE*10;
			cameraTarget.z = tetsuWork->contPos.z;

			g3Dcamera = tetsuWork->g3Dcamera[1];
			GFL_G3D_CAMERA_SetPos( g3Dcamera, &cameraPos );
			GFL_G3D_CAMERA_SetTarget( g3Dcamera, &cameraTarget );

			tetsuWork->autoCameraRotate1 += 0x0080;
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
#define mapSizeX	16//(32)
#define mapSizeZ	16//(32)
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

static const char mapData0[] = {
"■■■■■■■■■■■■■■■■"
"■　　　　　　　　　　　　　　■"
"■　■■■■■■■■■■■■　■"
"■　■　　　　　　　　　　■　■"
"■　■　■■■　■■■■　■　■"
"■　■　■　■　　　　■　■　■"
"■　■　■　■○　　　■　■　■"
"■　　　■　■■■　　■　■　■"
"■　■　■　　　■　　　　■　■"
"■　■　■■■　■■■■　■　■"
"■　■　　　■　　　　■　■　■"
"■　■■■　■■■■■■　■　■"
"■　◎　■　　　　　　　　■　■"
"■　　　■■■■■　■■■■　■"
"■　　　　　　　　　　　　　　■"
"■■■■■■■■■■■■■■■■"
};

static const char mapData1[] = {
"■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■"	//32
"■　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　■"	//2///34
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
"■　　■　■　■　■　■　■■■■■■　■　■　■　■　■　　■"	//18///180
"■　　■　■　■　■　■　■　　　　■○■　■　■　■　■　　■"	//14
"■　　■○■　■　■　■　■　●　　■　■　■　■　■　■○　■"	//14
"■　　■　■　■　■　■○■　　　　■　■　■　■　■　■　　■"	//14
"■　　■　■　■　■　■　■　　　　■　■　■　■　■　■　　■"	//14///56////270
"■　○■　■　■　■　■　■　■■■■　■○■　■　■　■　　■"	//17
"■　　■　■　■　■　　　　　　　　　　■　■　■　■　■　　■"	//11
"■　　■　■　■　■　■■■■■■■■■■　■　■　■　■　　■"	//20
"■　　■　■○■　■　　　　　○　　　　　　■　■　■　■　　■"	//10///58
"■　　■　■　■　■　■■■■■■■■■■■■　■　■　■　　■"	//21
"■　○■　■　　　　　○　　　　　　　　　○　　■　■　■　　■"	//7
"■　　■　■　■■■■■■■■■■■■■■■■■■　■　■　　■"	//24
"■　　■　■　　　　　　　　　　○　　　　　　　　　■　■　　■"	//6///58
"■　　■　■　■■■■■■■■■■■■■■■■■■■■　■○　■"	//25
"■　　　　　　　　　　○　　　　　　　　　　○　　　　　■　　■"	//3
"■　　□■■■■■■■■■■■■■■■■■■■■■■■■■　　■"	//28
"■　　　　　○　　　　　　　　　　○　　　　　　　　　　　　　■"	//2///58
"■　　　　　　　　　　　　○　　　　　　　　　　　　　　　　　■"	//2
"■　◎　■■■■■■■■■■■■■■■■■■■■■■■■■■■■"	//32///34////208
};//478+27

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
"○　○　○　○　○　○　○　○　○　○　○　○　○　○　○　○　"	//16
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　■　■　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　■◎■　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
};

static const char mapData4[] = {
"　　○　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//16
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　◎　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
};

static const MAPDATA mapDataTbl = {
	mapData0,
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

#define SAVE_MTX_IDX 30
static void storeJntMtx(NNSG3dRS* rs)
{
    //
    // NODEDESCコマンドのコールバックCで呼ばれる
    //
    int jntID;
    NNS_G3D_GET_JNTID(NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs)),
                      &jntID,
                      "wrist_l_end");
    SDK_ASSERT(jntID >= 0);

    if (NNS_G3dRSGetCurrentNodeDescID(rs) == jntID)
    {
        // wrist_l_endを処理している
        fx32 posScale = NNS_G3dRSGetPosScale(rs);
        fx32 invPosScale = NNS_G3dRSGetInvPosScale(rs);
#if 0
        //
        // 退避先のスタックIdxがモデル描画の際に使われていないことを確認
        //
        SDK_ASSERTMSG( SAVE_MTX_IDX >= 
				NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs))->info.firstUnusedMtxStackID,
                      "SAVE_MTX_IDX will be destroyed");
#endif
        // 描画直前にしかposScaleでのスケーリングはかからないので、
        // 明示的にスケーリングしなくてはならない。
        NNS_G3dGeScale(posScale, posScale, posScale);

        // 描画中にストアされない場所にカレント行列を退避しておく
        NNS_G3dGeStoreMtx(SAVE_MTX_IDX);

        // 描画にカレント行列が使われるかもしれないのでスケーリングを元に戻しておく。
        NNS_G3dGeScale(invPosScale, invPosScale, invPosScale);

        // これ以上コールバックを使用する必要がない場合はリセットしておくと
        // パフォーマンスへの悪影響が若干小さくなる可能性がある。
        NNS_G3dRSResetCallBack(rs, NNS_G3D_SBC_NODEDESC);
    }
}
















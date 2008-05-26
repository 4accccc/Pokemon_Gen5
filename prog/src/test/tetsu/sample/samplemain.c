//============================================================================================
/**
 * @file	samplemain.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "include/system/gfl_use.h"
#include "textprint.h"

void	SampleBoot( HEAPID heapID );
void	SampleEnd( void );
BOOL	SampleMain( void );

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	メイン制御
 *
 *
 *
 *
 *
 */
//============================================================================================
typedef struct _SAMPLE_SETUP SAMPLE_SETUP;

static SAMPLE_SETUP*	SetupGameSystem( HEAPID heapID );
static void				RemoveGameSystem( SAMPLE_SETUP* gs );
static void				MainGameSystem( SAMPLE_SETUP* gs );

//アーカイブＩＮＤＥＸ
enum {
	ARCID_TEST3D = 0,
	ARCID_FLDACT,
};

typedef struct _CURSOR_CONT	CURSOR_CONT;
static CURSOR_CONT*		CreateCursor( SAMPLE_SETUP*	gs, HEAPID heapID );
static void				DeleteCursor( CURSOR_CONT* cursor );
static void				MainCursor( CURSOR_CONT* cursor);
static void				GetCursorTrans( CURSOR_CONT* cursor, VecFx32* trans );

typedef struct _MAP_CONT	MAP_CONT;
static MAP_CONT*		CreateMapper( SAMPLE_SETUP*	gs, HEAPID heapID );
static void				DeleteMapper( MAP_CONT* mapper );
static void				MainMapper( MAP_CONT* mapper, VecFx32* pos );

//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
	HEAPID			heapID;
	int				seq;
	int				timer;

	SAMPLE_SETUP*	gs;
	CURSOR_CONT*	cursor;
	MAP_CONT*		mapper;

}SAMPLE_WORK;

//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( int cont );

SAMPLE_WORK* sampleWork;

//------------------------------------------------------------------
/**
 * @brief	ワークの確保と破棄
 */
//------------------------------------------------------------------
void	SampleBoot( HEAPID heapID )
{
	sampleWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(SAMPLE_WORK) );
	sampleWork->heapID = heapID;

	GFL_UI_TP_Init( sampleWork->heapID );
}

void	SampleEnd( void )
{
	GFL_UI_TP_Exit();

	GFL_HEAP_FreeMemory( sampleWork );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
static void Debug_Regenerate( void );
#define MAPOFFS 0//((64-3) * 8 * FX32_ONE)
//------------------------------------------------------------------
BOOL	SampleMain( void )
{
	BOOL return_flag = FALSE;
	int i;

	sampleWork->timer++;
	GFL_UI_TP_Main();

	switch( sampleWork->seq ){

	case 0:
		//システムセットアップ
		sampleWork->gs = SetupGameSystem( sampleWork->heapID );

		sampleWork->seq++;
		break;

	case 1:
		sampleWork->cursor = CreateCursor( sampleWork->gs, sampleWork->heapID );
		sampleWork->mapper = CreateMapper( sampleWork->gs, sampleWork->heapID );

		sampleWork->seq++;
		break;

	case 2:
		if( GameEndCheck( GFL_UI_KEY_GetCont() ) == TRUE ){
			sampleWork->seq++;
		}
		{
			VecFx32 pos;

			MainCursor( sampleWork->cursor );
			GetCursorTrans( sampleWork->cursor, &pos );
			MainMapper( sampleWork->mapper, &pos );
		}
		MainGameSystem( sampleWork->gs );
		break;

	case 3:
		DeleteMapper( sampleWork->mapper );
		DeleteCursor( sampleWork->cursor );

		sampleWork->seq++;
		break;

	case 4:
		RemoveGameSystem( sampleWork->gs );
		return_flag = TRUE;
		break;
	}
	return return_flag;
}

//------------------------------------------------------------------
/**
 * @brief	終了チェック
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( int cont )
{
	int resetCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START;

	if( (cont & resetCont ) == resetCont ){
		return TRUE;
	} else {
		return FALSE;
	}
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	セットアップ
 *
 *
 *
 *
 *
 */
//============================================================================================
#define BACKGROUND_COL	(GX_RGB(20,20,31))		//背景色
#define DTCM_SIZE		(0x1000)				//DTCMエリアのサイズ

typedef struct _SAMPLE_MAP		SAMPLE_MAP;
static SAMPLE_MAP*	Create3Dmap( HEAPID heapID );
static void			Delete3Dmap( SAMPLE_MAP* sampleMap );
static void			Draw3Dmap( SAMPLE_MAP* sampleMap, GFL_G3D_CAMERA* g3Dcamera );
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _SAMPLE_SETUP {
	GFL_G3D_UTIL*			g3Dutil;		//g3Dutil Lib ハンドル
	u16						g3DutilUnitIdx;	//g3Dutil Unitインデックス
	GFL_G3D_SCENE*			g3Dscene;		//g3Dscene Lib ハンドル
	GFL_G3D_CAMERA*			g3Dcamera;		//g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET*		g3Dlightset;	//g3Dlight Lib ハンドル

	SAMPLE_MAP*				sampleMap;

	HEAPID					heapID;
};

//------------------------------------------------------------------
/**
 * @brief	ディスプレイ環境データ
 */
//------------------------------------------------------------------
///ＶＲＡＭバンク設定構造体
static const GFL_BG_DISPVRAM dispVram = {
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
};

//------------------------------------------------------------------
/**
 * @brief	アーカイブテーブル
 */
//------------------------------------------------------------------
static	const	char	*GraphicFileTable[]={
	"test_graphic/test3d.narc",
	"test_graphic/fld_act.narc",
};

static const char font_path[] = {"gfl_font.dat"};
//------------------------------------------------------------------
/**
 * @brief	３Ｄグラフィック環境データ
 */
//------------------------------------------------------------------
//カメラ初期設定データ
static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 cameraPos	= { 0, (FX32_ONE * 100), (FX32_ONE * 180) };

//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light0Setup = { light0Tbl, NELEMS(light0Tbl) };
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
static void	bg_init( SAMPLE_SETUP* gs );
static void	bg_exit( SAMPLE_SETUP* gs );
//３Ｄ関数
static void g3d_load( SAMPLE_SETUP* gs );
static void g3d_control( SAMPLE_SETUP* gs );
static void g3d_draw( SAMPLE_SETUP* gs );
static void	g3d_unload( SAMPLE_SETUP* gs );

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static SAMPLE_SETUP*	SetupGameSystem( HEAPID heapID )
{
	SAMPLE_SETUP*	gs = GFL_HEAP_AllocMemory( heapID, sizeof(SAMPLE_SETUP) );
	gs->heapID = heapID;

	//乱数初期化
	GFL_STD_MtRandInit(0);

	//ARCシステム初期化
	GFL_ARC_Init( &GraphicFileTable[0], NELEMS(GraphicFileTable) );

	//VRAMクリア
	GFL_DISP_ClearVRAM( GX_VRAM_D );
	//VRAM設定
	GFL_DISP_SetBank( &dispVram );

	//BG初期化
	bg_init( gs );

	//３Ｄデータのロード
	g3d_load( gs );

	return gs;
}

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static void	RemoveGameSystem( SAMPLE_SETUP* gs )
{
	g3d_unload( gs );	//３Ｄデータ破棄

	bg_exit( gs );
	GFL_ARC_Exit();

	GFL_HEAP_FreeMemory( gs );
}

//------------------------------------------------------------------
/**
 * @brief	システムメイン関数
 */
//------------------------------------------------------------------
static void	MainGameSystem( SAMPLE_SETUP* gs )
{
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

static void	bg_init( SAMPLE_SETUP* gs )
{
	//フォント読み込み
	GFL_TEXT_CreateSystem( font_path );

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
	G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 16, 8 );

	//３Ｄシステム起動
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, gs->heapID, G3DsysSetup );
	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( SAMPLE_SETUP* gs )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );

	GFL_G3D_Exit();
	GFL_BG_Exit();
	GFL_TEXT_DeleteSystem();
}

// ３Ｄセットアップコールバック
static void G3DsysSetup( void )
{
	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_TOON );
	G3X_AntiAlias( TRUE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

	G3X_SetEdgeColorTable( edgeColorTable ); 
	G3X_EdgeMarking( TRUE );

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
static void g3d_load( SAMPLE_SETUP* gs )
{
	//配置物設定

	//g3Dutilを使用し配列管理をする
	gs->g3Dutil = GFL_G3D_UTIL_Create( G3D_UTIL_RESCOUNT, G3D_UTIL_OBJCOUNT, gs->heapID );
	//g3Dsceneを使用し管理をする
	gs->g3Dscene = GFL_G3D_SCENE_Create( gs->g3Dutil, 
						G3D_SCENE_OBJCOUNT, G3D_OBJWORK_SZ, G3D_ACC_COUNT, TRUE, gs->heapID );

	//カメラ作成
	gs->g3Dcamera = GFL_G3D_CAMERA_CreateDefault( &cameraPos, &cameraTarget, gs->heapID );
	{
		fx32 far = 4096 << FX32_SHIFT;

		GFL_G3D_CAMERA_SetFar( gs->g3Dcamera, &far );
	}
	//ライト作成
	gs->g3Dlightset = GFL_G3D_LIGHT_Create( &light0Setup, gs->heapID );

	gs->sampleMap = Create3Dmap( gs->heapID );

	//カメラライト0反映
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera );
	GFL_G3D_LIGHT_Switching( gs->g3Dlightset );
}
	
//動作
static void g3d_control( SAMPLE_SETUP* gs )
{
	GFL_G3D_SCENE_Main( gs->g3Dscene ); 
}

//描画
static void g3d_draw( SAMPLE_SETUP* gs )
{
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera );
	GFL_G3D_LIGHT_Switching( gs->g3Dlightset );
	Draw3Dmap( gs->sampleMap, gs->g3Dcamera );
	GFL_G3D_SCENE_Draw( gs->g3Dscene );  
}

//破棄
static void g3d_unload( SAMPLE_SETUP* gs )
{
	GFL_G3D_LIGHT_Delete( gs->g3Dlightset );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera );

	Delete3Dmap( gs->sampleMap );

	GFL_G3D_SCENE_Delete( gs->g3Dscene );  
	GFL_G3D_UTIL_Delete( gs->g3Dutil );
}
	

//============================================================================================
/**
 * @brief	マップ描画システム
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
#define MAP_BLOCK_COUNT	(4)
#define MAPID_NULL	(0xffffffff)

typedef struct {
	u32					idx;
	VecFx32				trans;
	u32					texDataAdrs;
	u32					texPlttAdrs;
#if 0
	u8					g3DresTex[0x1000];
#else
	GFL_G3D_RES*		g3Dres;
#endif
}MAP_BLOCK_DATA;

struct _SAMPLE_MAP {
	GFL_G3D_SCENE*		g3Dscene;
	HEAPID				heapID;
	MAP_BLOCK_DATA		mapBlock[MAP_BLOCK_COUNT];
};

static void	CreateMapGraphicData
		( SAMPLE_MAP* sampleMap, const int blockID, const VecFx32* trans, const u32 mapID );
static void	DeleteMapGraphicData( SAMPLE_MAP* sampleMap, int blockID );

//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
#include "graphic_data/test_graphic/fld_act.naix"

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ生成
 */
//------------------------------------------------------------------
static SAMPLE_MAP*	Create3Dmap( HEAPID heapID )
{
	SAMPLE_MAP* sampleMap = GFL_HEAP_AllocClearMemory( heapID, sizeof(SAMPLE_MAP) );
	int i;

	sampleMap->heapID = heapID;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		sampleMap->mapBlock[i].idx = MAPID_NULL;
	}
	return sampleMap;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ破棄
 */
//------------------------------------------------------------------
static void	Delete3Dmap( SAMPLE_MAP* sampleMap )
{
	GFL_HEAP_FreeMemory( sampleMap );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄマップデータセットアップ
 */
//------------------------------------------------------------------
static void	CreateMapGraphicData
		( SAMPLE_MAP* sampleMap, const int blockID, const VecFx32* trans, const u32 mapID )
{
	GFL_G3D_SCENEOBJ_DATA*	pdata;
	GFL_G3D_RES*			g3DresTex;
	NNSG3dTexKey			texDataKey;
	NNSG3dPlttKey			texPlttKey;
	MAP_BLOCK_DATA*			mapBlock;

	GF_ASSERT( blockID < MAP_BLOCK_COUNT );
	if( mapID == MAPID_NULL ){
		return;
	}
	mapBlock = &sampleMap->mapBlock[blockID];

	GF_ASSERT( mapBlock->idx == MAPID_NULL );

	mapBlock->idx = mapID;
	VEC_Set( &mapBlock->trans, trans->x, trans->y, trans->z );
#if 0
	g3DresTex = (GFL_G3D_RES*)&mapBlock->g3DresTex[0];
	GFL_G3D_LoadResourceArc( ARCID_FLDACT, NARC_fld_act_tex32x32_nsbtx, g3DresTex ); 
#else
	g3DresTex = GFL_G3D_CreateResourceArc( ARCID_FLDACT, NARC_fld_act_tex32x32_nsbtx ); 
	mapBlock->g3Dres = g3DresTex;
#endif
	if( GFL_G3D_TransVramTexture( g3DresTex ) == FALSE ){
		GF_ASSERT(0);
	}
	texDataKey = GFL_G3D_GetTextureDataVramKey( g3DresTex );
	texPlttKey = GFL_G3D_GetTexturePlttVramKey( g3DresTex );

	mapBlock->texDataAdrs = NNS_GfdGetTexKeyAddr( texDataKey );
	mapBlock->texPlttAdrs = NNS_GfdGetPlttKeyAddr( texPlttKey );
	{
		//テクスチャデータ作成
		NNSG3dResFileHeader*	file = GFL_G3D_GetResourceFileHeader( g3DresTex ); 
		NNSG3dResTex*			texfile = GFL_G3D_GetResTex( g3DresTex );
		u32						texData = (u32)((u8*)texfile + texfile->texInfo.ofsTex);
		GFL_BMP_DATA*			bmpTex = GFL_BMP_Create( 4,4,GFL_BMP_16_COLOR,sampleMap->heapID );
		GFL_TEXT_PRINTPARAM		printParam = { NULL, 0, 0, 1, 0, 15, 1, GFL_TEXT_WRITE_16 };
		char					mapName[] = {"mapID"};
		char					texName[] = {"0000"};
		u32						texSize = GFL_BMP_GetBmpDataSize( bmpTex );
		u32						colorB = mapID%7+1;

		//マップＩＤを文字表示する
		GFL_BMP_Clear( bmpTex, colorB );
		printParam.bmp = bmpTex;
		printParam.colorB = colorB;
		texName[0] = '0'+ mapID/1000;
		texName[1] = '0'+ mapID%1000/100;
		texName[2] = '0'+ mapID%100/10;
		texName[3] = '0'+ mapID%10;

		printParam.writex = 4;
		printParam.writey = 8;
		printParam.colorF = 15;
		GFL_TEXT_PrintSjisCode( mapName, &printParam );
		printParam.writex = 4;
		printParam.writey = 16;
		printParam.colorF = 14;
		GFL_TEXT_PrintSjisCode( texName, &printParam );

		GFL_BMP_DataConv_to_BMPformat( bmpTex, FALSE, sampleMap->heapID );
		GFL_STD_MemCopy32( GFL_BMP_GetCharacterAdrs( bmpTex ), (void*)texData, texSize ); 

		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, 
											mapBlock->texDataAdrs, (void*)texData, texSize );
		GFL_BMP_Delete( bmpTex );
	}
	OS_Printf("mapGraphic block%x is Created id=%x, tKey=%x, pKey=%x, tAdrs=%x, pAdrs=%x\n", 
				blockID, mapBlock->idx, texDataKey, texPlttKey,
				mapBlock->texDataAdrs, mapBlock->texPlttAdrs );
}

//------------------------------------------------------------------
static void	DeleteMapGraphicData( SAMPLE_MAP* sampleMap, int blockID )
{
	GFL_G3D_RES*			g3DresTex;
	MAP_BLOCK_DATA*			mapBlock;

	GF_ASSERT( blockID < MAP_BLOCK_COUNT );

	mapBlock = &sampleMap->mapBlock[blockID];

	if( mapBlock->idx == MAPID_NULL ){
		return;
	}
#if 0
	g3DresTex = (GFL_G3D_RES*)&mapBlock->g3DresTex[0];
	if( GFL_G3D_FreeVramTexture( g3DresTex ) == FALSE ){
		GF_ASSERT(0);
	}
#else
	
	g3DresTex = mapBlock->g3Dres;
	if( GFL_G3D_FreeVramTexture( g3DresTex ) == FALSE ){
		GF_ASSERT(0);
	}
	GFL_G3D_DeleteResource( g3DresTex );
#endif
	mapBlock->idx = MAPID_NULL;
	OS_Printf("mapGraphic block%x is Deleted\n", blockID);
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップディスプレイ
 */
//------------------------------------------------------------------
static void	Draw3Dmap( SAMPLE_MAP* sampleMap, GFL_G3D_CAMERA* g3Dcamera )
{
	MAP_BLOCK_DATA* mapBlock;
	int				i;

	G3X_Reset();

	//カメラ設定
	{
		VecFx32		camPos, camUp, target;

		GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );
		G3_LookAt( &camPos, &camUp, &target, NULL );
	}
	//マテリアル設定
	G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), TRUE );
	G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE );
	G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 63, 31, 0);

	G3_Scale( FX32_ONE*256, FX32_ONE*256, FX32_ONE*256 );

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &sampleMap->mapBlock[i];
		if( mapBlock->idx != MAPID_NULL ){
			G3_PushMtx();
	
			G3_Translate( mapBlock->trans.x/256, mapBlock->trans.y/256, mapBlock->trans.z/256 );

			G3_TexImageParam(	GX_TEXFMT_PLTT16, GX_TEXGEN_TEXCOORD, 
								GX_TEXSIZE_S32, GX_TEXSIZE_T32,
								GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
								GX_TEXPLTTCOLOR0_TRNS, mapBlock->texDataAdrs );
			G3_TexPlttBase( mapBlock->texPlttAdrs, GX_TEXFMT_PLTT16 );

			G3_Begin( GX_BEGIN_QUADS );

			G3_TexCoord( 0, FX32_ONE*32 );
			G3_Vtx( -FX16_ONE, 0, FX16_ONE );

			G3_TexCoord( FX32_ONE*32, FX32_ONE*32 );
			G3_Vtx( FX16_ONE, 0, FX16_ONE );

			G3_TexCoord( FX32_ONE*32, 0 );
			G3_Vtx( FX16_ONE, 0, -FX16_ONE );

			G3_TexCoord( 0, 0 );
			G3_Vtx( -FX16_ONE, 0, -FX16_ONE );
	
			G3_End();
			G3_PopMtx(1);
		}
	}
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	コントローラ
 *
 *
 *
 *
 *
 */
//============================================================================================
//============================================================================================
/**
 * @brief	カーソル
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
struct _CURSOR_CONT {
	HEAPID				heapID;
	SAMPLE_SETUP*		gs;
	u16					unitIdx;
	u16					resIdx;
	u16					objIdx;

	u16					cursorIdx;
	VecFx32				trans;
};

//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
#include "graphic_data/test_graphic/test3d.naix"

enum {
	G3DRES_CURSOR = 0,
};
//３Ｄグラフィックリソーステーブル
static const GFL_G3D_UTIL_RES cursor_g3Dutil_resTbl[] = {
	{ ARCID_TEST3D, NARC_haruka_test_wall_nsbmd, GFL_G3D_UTIL_RESARC },
};

enum {
	G3DOBJ_CURSOR = 0,
};
//３Ｄオブジェクト定義テーブル
static const GFL_G3D_UTIL_OBJ cursor_g3Dutil_objTbl[] = {
	{ G3DRES_CURSOR, 0, G3DRES_CURSOR, NULL, 0 },
};

//---------------------
//g3Dscene 初期設定テーブルデータ
static const GFL_G3D_UTIL_SETUP cursor_g3Dsetup = {
	cursor_g3Dutil_resTbl, NELEMS(cursor_g3Dutil_resTbl),
	cursor_g3Dutil_objTbl, NELEMS(cursor_g3Dutil_objTbl),
};

static const GFL_G3D_SCENEOBJ_DATA cursorObject[] = {
	{	G3DOBJ_CURSOR, 0, 1, 16, TRUE, TRUE,
		{	{ 0, -FX32_ONE*64, 0 },
			{ FX32_ONE/4, FX32_ONE/4, FX32_ONE/4 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
static CURSOR_CONT*	CreateCursor( SAMPLE_SETUP*	gs, HEAPID heapID )
{
	CURSOR_CONT* cursor = GFL_HEAP_AllocClearMemory( heapID, sizeof(CURSOR_CONT) );

	cursor->heapID = heapID;
	cursor->gs = gs;
	cursor->trans.x = FX32_ONE*1024;//0;
	cursor->trans.y = FX32_ONE*1024;//0;
	cursor->trans.z = FX32_ONE*1024;//0;

	//３Ｄデータセットアップ
	cursor->unitIdx = GFL_G3D_SCENE_AddG3DutilUnit( cursor->gs->g3Dscene, &cursor_g3Dsetup );
	cursor->resIdx = GFL_G3D_SCENE_GetG3DutilUnitResIdx( cursor->gs->g3Dscene, cursor->unitIdx );
	cursor->objIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( cursor->gs->g3Dscene, cursor->unitIdx );
	cursor->cursorIdx = GFL_G3D_SCENEOBJ_Add(	cursor->gs->g3Dscene, 
												cursorObject, NELEMS(cursorObject),
												cursor->objIdx );
	
	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx),
								&cursor->trans );
	return cursor;
}

//------------------------------------------------------------------
/**
 * @brief	終了
 */
//------------------------------------------------------------------
static void	DeleteCursor( CURSOR_CONT* cursor )
{
	GFL_G3D_SCENEOBJ_Remove( cursor->gs->g3Dscene, cursor->cursorIdx, NELEMS(cursorObject) ); 
	GFL_G3D_SCENE_DelG3DutilUnit( cursor->gs->g3Dscene, cursor->unitIdx );

	GFL_HEAP_FreeMemory( cursor );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
#define MV_SPEED	(FX32_ONE*4)
static void	MainCursor( CURSOR_CONT* cursor )
{
	VecFx32	pos, target;
	VecFx32	vecMove = { 0, 0, 0 };
	int		key;

	key = GFL_UI_KEY_GetCont();

	if( key & PAD_BUTTON_A ){
		vecMove.y = -MV_SPEED;
	}
	if( key & PAD_BUTTON_B ){
		vecMove.y = MV_SPEED;
	}
	if( key & PAD_KEY_UP ){
		vecMove.z = -MV_SPEED;
	}
	if( key & PAD_KEY_DOWN ){
		vecMove.z = MV_SPEED;
	}
	if( key & PAD_KEY_LEFT ){
		vecMove.x = -MV_SPEED;
	}
	if( key & PAD_KEY_RIGHT ){
		vecMove.x = MV_SPEED;
	}
	VEC_Add( &cursor->trans, &vecMove, &cursor->trans );
	VEC_Add( &cursor->trans, &cameraPos, &pos );
	target.x = cursor->trans.x;
	target.y = 0;
	target.z = cursor->trans.z;

	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx), 
								&target );
	GFL_G3D_CAMERA_SetTarget( cursor->gs->g3Dcamera, &target );
	GFL_G3D_CAMERA_SetPos( cursor->gs->g3Dcamera, &pos );
}

//------------------------------------------------------------------
/**
 * @brief	
 */
//------------------------------------------------------------------
static void	GetCursorTrans( CURSOR_CONT* cursor, VecFx32* trans )
{
	*trans = cursor->trans;
}


//============================================================================================
/**
 * @brief	マップ
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
typedef struct {
	u32			mapID;
	VecFx32		trans;
}MAP_BLOCK_IDX;

struct _MAP_CONT {
	HEAPID			heapID;
	SAMPLE_SETUP*	gs;
	MAP_BLOCK_IDX	mapBlockIdx[MAP_BLOCK_COUNT];
};

#define MAP3D_SCALE			(FX32_ONE*1)
#define MAP3D_GRID_LEN		(MAP3D_SCALE*16)
#define MAP3D_GRID_SIZEX	(32)
#define MAP3D_GRID_SIZEZ	(32)
#define MAP_BLOCK_LENX		(MAP3D_GRID_LEN * MAP3D_GRID_SIZEX)
#define MAP_BLOCK_LENZ		(MAP3D_GRID_LEN * MAP3D_GRID_SIZEZ)
#define MAP_BLOCK_SIZX		(32)
#define MAP_BLOCK_SIZZ		(32)
#define MAP_BLOCK_IDXMAX	(MAP_BLOCK_SIZX * MAP_BLOCK_SIZZ)

static void  GetMapperBlockIndex( const VecFx32* pos, MAP_BLOCK_IDX* mapBlockIdx );
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
static const u32 mapIdxData[] = {
	  9, 10, 11, 12, 13, 14, 15,
	 31, 32, 33, 34, 35, 36, 37,
	 66, 67, 68, 69, 70, 71, 72,
	 22, 23, 24, 25, 26, 27, 28,
	 45, 46, 47, 48, 49, 50, 51,
	 87, 88, 89, 90, 91, 92, 93,
	 53, 54, 55, 56, 57, 58, 59,
};

static const VecFx32 mapTransOffs[] = {
//	{ -FX32_ONE*256, 0, -FX32_ONE*256 },
//	{  FX32_ONE*256, 0, -FX32_ONE*256 },
//	{ -FX32_ONE*256, 0,  FX32_ONE*256 },
//	{  FX32_ONE*256, 0,  FX32_ONE*256 },
	{  FX32_ONE*256, 0,  FX32_ONE*256 },
	{  FX32_ONE*(256+512), 0,  FX32_ONE*256 },
	{  FX32_ONE*256, 0,  FX32_ONE*(256+512) },
	{  FX32_ONE*(256+512), 0,  FX32_ONE*(256+512) },
};

//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
static MAP_CONT*	CreateMapper( SAMPLE_SETUP*	gs, HEAPID heapID )
{
	MAP_CONT* mapper = GFL_HEAP_AllocClearMemory( heapID, sizeof(MAP_CONT) );
	int i;

	mapper->heapID = heapID;
	mapper->gs = gs;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapper->mapBlockIdx[i].mapID = MAPID_NULL;
	}
	return mapper;
}

//------------------------------------------------------------------
/**
 * @brief	終了
 */
//------------------------------------------------------------------
static void	DeleteMapper( MAP_CONT* mapper )
{
	int i;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		DeleteMapGraphicData( mapper->gs->sampleMap, i );
	}
	GFL_HEAP_FreeMemory( mapper );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
static void	MainMapper( MAP_CONT* mapper, VecFx32* pos )
{
	MAP_BLOCK_IDX nowBlockIdx[MAP_BLOCK_COUNT];
	int	i;

 	GetMapperBlockIndex( pos, &nowBlockIdx[0] );

	if(   ( mapper->mapBlockIdx[0].mapID != nowBlockIdx[0].mapID )
		||( mapper->mapBlockIdx[1].mapID != nowBlockIdx[1].mapID )
		||( mapper->mapBlockIdx[2].mapID != nowBlockIdx[2].mapID )
		||( mapper->mapBlockIdx[3].mapID != nowBlockIdx[3].mapID )){
		//reload
		
		//情報更新
		for( i=0; i<MAP_BLOCK_COUNT; i++ ){
			DeleteMapGraphicData( mapper->gs->sampleMap, i );
		}
		for( i=0; i<MAP_BLOCK_COUNT; i++ ){
			CreateMapGraphicData( mapper->gs->sampleMap, i, 
								&nowBlockIdx[i].trans, nowBlockIdx[i].mapID );
			mapper->mapBlockIdx[i] = nowBlockIdx[i];
		}
		OS_Printf("\n");
	}
}


//------------------------------------------------------------------
/**
 * @brief	マップブロック取得
 */
//------------------------------------------------------------------
static const int blockIdxOffs[][MAP_BLOCK_COUNT][2] = {
	{ {-1,-1},{-1, 0},{ 0,-1},{ 0, 0} },	//自分のいるブロックから左上方向に４ブロックとる
	{ {-1, 0},{-1, 1},{ 0, 0},{ 0, 1} },	//自分のいるブロックから右上方向に４ブロックとる
	{ { 0,-1},{ 0, 0},{ 1,-1},{ 1, 0} },	//自分のいるブロックから左下方向に４ブロックとる
	{ { 0, 0},{ 0, 1},{ 1, 0},{ 1, 1} },	//自分のいるブロックから右下方向に４ブロックとる
};

static void GetMapperBlockIndex( const VecFx32* pos, MAP_BLOCK_IDX* mapBlockIdx )
{
	u32		blockIdx, blockx, blockz;
	fx32	posx, posz;
	int		i, offsx, offsz, blockPat = 0;
	
	blockx = FX_Whole( FX_Div( pos->x, MAP_BLOCK_LENX ) );
	blockz = FX_Whole( FX_Div( pos->z, MAP_BLOCK_LENZ ) );

	posx = FX_Mod( pos->x, MAP_BLOCK_LENX );
	posz = FX_Mod( pos->z, MAP_BLOCK_LENZ );

	if( posx > (MAP_BLOCK_LENX/2) ){
		blockPat += 1;
	}
	if( posz > (MAP_BLOCK_LENZ/2) ){
		blockPat += 2;
	}
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		offsx = blockIdxOffs[blockPat][i][1];
		offsz = blockIdxOffs[blockPat][i][0];

		blockIdx = (blockz + offsz) * MAP_BLOCK_SIZX + (blockx + offsx);

		if( blockIdx >= MAP_BLOCK_IDXMAX ){
			blockIdx = MAPID_NULL;
		}
		mapBlockIdx[i].mapID = blockIdx;
		mapBlockIdx[i].trans.x = FX32_ONE*(256 + (blockx + offsx)*512);
		mapBlockIdx[i].trans.y = 0;
		mapBlockIdx[i].trans.z = FX32_ONE*(256 + (blockz + offsz)*512);
	}
}



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
	ARCID_FLDMAP,
	ARCID_SAMPLEMAP,
};

typedef struct _MAP_CONT	MAP_CONT;
static MAP_CONT*		CreateMapper( SAMPLE_SETUP*	gs, HEAPID heapID );
static void				DeleteMapper( MAP_CONT* mapper );
static void				MainMapper( MAP_CONT* mapper, VecFx32* pos );
static void				GetWorldMapVecN( MAP_CONT* mapper, VecFx32* pos, VecFx32* vecN );
static void				GetWorldMapHeight( MAP_CONT* mapper, VecFx32* pos, fx32* height );

typedef struct _CURSOR_CONT	CURSOR_CONT;
static CURSOR_CONT*		CreateCursor( SAMPLE_SETUP*	gs, MAP_CONT* mapper, HEAPID heapID );
static void				DeleteCursor( CURSOR_CONT* cursor );
static void				MainCursor( CURSOR_CONT* cursor);
static void				GetCursorTrans( CURSOR_CONT* cursor, VecFx32* trans );

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
		sampleWork->mapper = CreateMapper( sampleWork->gs, sampleWork->heapID );
		sampleWork->cursor = CreateCursor( sampleWork->gs, sampleWork->mapper, sampleWork->heapID );

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
		DeleteCursor( sampleWork->cursor );
		DeleteMapper( sampleWork->mapper );

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
//#define BACKGROUND_COL	(GX_RGB(23,29,31))		//背景色
//#define FOG_COL			(GX_RGB(31,31,31))		//フォグ色
#define BACKGROUND_COL	(GX_RGB(30,31,31))		//背景色
#define FOG_COL			(GX_RGB(31,31,31))		//フォグ色
#define DTCM_SIZE		(0x1000)				//DTCMエリアのサイズ

typedef struct _SAMPLE_MAP		SAMPLE_MAP;
static SAMPLE_MAP*	Create3Dmap( HEAPID heapID );
static void			Delete3Dmap( SAMPLE_MAP* sampleMap );
static void			Draw3Dmap( SAMPLE_MAP* sampleMap, GFL_G3D_CAMERA* g3Dcamera );
static void			Load3Dmap( SAMPLE_MAP* sampleMap );
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
	GFL_TCB*				g3dVintr;		//3D用vIntrTaskハンドル

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
	"test_graphic/fld_map.narc",
	"test_graphic/sample_map.narc",
};

static const char font_path[] = {"gfl_font.dat"};
//------------------------------------------------------------------
/**
 * @brief	３Ｄグラフィック環境データ
 */
//------------------------------------------------------------------
//カメラ初期設定データ
static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 cameraPos	= { 0, (FX32_ONE * 64), (FX32_ONE * 128) };

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
static void	g3d_vblank( GFL_TCB* tcb, void* work );

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static SAMPLE_SETUP*	SetupGameSystem( HEAPID heapID )
{
	SAMPLE_SETUP*	gs = GFL_HEAP_AllocClearMemory( heapID, sizeof(SAMPLE_SETUP) );
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
	gs->g3dVintr = GFUser_VIntr_CreateTCB( g3d_vblank, (void*)gs, 0 );

	return gs;
}

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static void	RemoveGameSystem( SAMPLE_SETUP* gs )
{
	GFL_TCB_DeleteTask( gs->g3dVintr );
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
#if 1
	// フォグセットアップ
    {
        u32     fog_table[8];
        int     i;

        //G3X_SetFog(TRUE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x1000, 0x01000 );
        G3X_SetFog(TRUE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x0400, 0x00e00 );

        G3X_SetFogColor(FOG_COL, 0);

        for ( i=0; i<8; i++ ){
            fog_table[i] = (u32)(((i*16)<<0) | ((i*16+4)<<8) | ((i*16+8)<<16) | ((i*16+12)<<24));
        }
        G3X_SetFogTable(&fog_table[0]);
	}

#else
	G3X_SetFog( FALSE, 0, 0, 0 );
#endif
	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

	G3X_SetEdgeColorTable( edgeColorTable ); 
	//G3X_EdgeMarking( TRUE );
	G3X_EdgeMarking( FALSE );

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
		//fx32 far = 1500 << FX32_SHIFT;
		//fx32 far = 4096 << FX32_SHIFT;
		fx32 far = 1024 << FX32_SHIFT;

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
	Load3Dmap( gs->sampleMap );
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
	
static void	g3d_vblank( GFL_TCB* tcb, void* work )
{
	SAMPLE_SETUP* gs =  (SAMPLE_SETUP*)work;

	//Load3Dmap( gs->sampleMap );
}

//============================================================================================
/**
 * @brief	マップ描画システム
 */
//============================================================================================
#include "map_data.h"

//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
#define MAP_BLOCK_COUNT		(9)
#define MAPID_NULL			(0xffffffff)
#define	MAPDATA_SIZE		(0x10000) 
#define	MAPTEX_SIZE			(0x4800) 
#define	MAPATTR_SIZE		(0x7000) 
#define	MAPPLTT_SIZE		(0x200) 
#define	MAPTRANS_SIZE		(0x800) 
#define	MAPVRAMTRANS_SIZE	(0x2000) 

#define	MAPSCALE_WIDTH		(256) 
#define MAP_WIDTH			(MAPSCALE_WIDTH*2)

typedef struct {
	u32				idx;
	VecFx32			trans;
	ARCHANDLE*		arc;

	u32				loadSeq;
	u32				loadCount;

	BOOL			dataLoadReq;
	u32				dataID;
	GFL_G3D_RES*	g3DresMdl;	//地形モデルリソース
	u8				data[MAPDATA_SIZE];

	BOOL			texLoadReq;
	u32				texID;
	GFL_G3D_RES*	g3DresTex;	//地形テクスチャリソース
	u8				tex[MAPTEX_SIZE];

	BOOL			attrLoadReq;
	u32				attrID;
	u8				attr[MAPATTR_SIZE];

	GFL_G3D_RND*	g3Drnd;		//地形レンダー

	NNSGfdTexKey	texKey;
	NNSGfdPlttKey	plttKey;
}MAP_BLOCK_DATA;

struct _SAMPLE_MAP {
	GFL_G3D_SCENE*		g3Dscene;
	HEAPID				heapID;
	MAP_BLOCK_DATA		mapBlock[MAP_BLOCK_COUNT];
};

static void	CreateMapGraphicData( SAMPLE_MAP* sampleMap );
static void	DeleteMapGraphicData( SAMPLE_MAP* sampleMap );

//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
#include "graphic_data/test_graphic/fld_act.naix"
#include "graphic_data/test_graphic/fld_map.naix"
#include "graphic_data/test_graphic/sample_map.naix"

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ生成
 */
//------------------------------------------------------------------
static SAMPLE_MAP*	Create3Dmap( HEAPID heapID )
{
	SAMPLE_MAP* sampleMap = GFL_HEAP_AllocClearMemory( heapID, sizeof(SAMPLE_MAP) );

	sampleMap->heapID = heapID;

	CreateMapGraphicData( sampleMap );
	OS_Printf("高さデータサイズ = %x\n", sizeof(NormalVtxSt));
	return sampleMap;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ破棄
 */
//------------------------------------------------------------------
static void	Delete3Dmap( SAMPLE_MAP* sampleMap )
{
	DeleteMapGraphicData( sampleMap );

	GFL_HEAP_FreeMemory( sampleMap );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップデータセットアップ
 */
//------------------------------------------------------------------
static void	CreateMapGraphicData( SAMPLE_MAP* sampleMap )
{
	MAP_BLOCK_DATA*	mapBlock;
	int				i;
	u32				g3DresHeaderSize = GFL_G3D_GetResourceHeaderSize();

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &sampleMap->mapBlock[i];

		//初期化
		mapBlock->idx = MAPID_NULL;
		VEC_Set( &mapBlock->trans, 0, 0, 0 );
		mapBlock->loadSeq = 0;
		mapBlock->loadCount = 0;

		mapBlock->dataLoadReq = FALSE;
		mapBlock->dataID = 0;
		mapBlock->texLoadReq = FALSE;
		mapBlock->texID = 0;
		mapBlock->attrLoadReq = FALSE;
		mapBlock->attrID = 0;

		mapBlock->g3Drnd = NULL;
		//アーカイブハンドル作成
		//mapBlock->arc = GFL_ARC_OpenDataHandle( ARCID_FLDMAP, sampleMap->heapID );
		mapBlock->arc = GFL_ARC_OpenDataHandle( ARCID_SAMPLEMAP, sampleMap->heapID );
		GF_ASSERT( mapBlock->arc );
		//モデルリソースヘッダ作成
		mapBlock->g3DresMdl = GFL_HEAP_AllocClearMemory( sampleMap->heapID, g3DresHeaderSize );
		//テクスチャリソースヘッダ作成
		mapBlock->g3DresTex = GFL_HEAP_AllocClearMemory( sampleMap->heapID, g3DresHeaderSize );

		//モデルリソースハンドル内容設定
		GFL_G3D_CreateResource( mapBlock->g3DresMdl, 
								GFL_G3D_RES_CHKTYPE_MDL,(void*)&mapBlock->data[0] );
		//テクスチャリソースハンドル内容設定
		GFL_G3D_CreateResource( mapBlock->g3DresTex, 
								GFL_G3D_RES_CHKTYPE_TEX, (void*)&mapBlock->tex[0] );

		//テクスチャＶＲＡＭ確保
		mapBlock->texKey = NNS_GfdAllocTexVram( MAPTEX_SIZE, FALSE, 0 );
		//パレットＶＲＡＭ確保
		mapBlock->plttKey = NNS_GfdAllocPlttVram( MAPPLTT_SIZE, FALSE, 0 );

		GF_ASSERT( mapBlock->texKey != NNS_GFD_ALLOC_ERROR_TEXKEY );
		GF_ASSERT( mapBlock->plttKey != NNS_GFD_ALLOC_ERROR_PLTTKEY );
	}
}

//------------------------------------------------------------------
static void	DeleteMapGraphicData( SAMPLE_MAP* sampleMap )
{
	MAP_BLOCK_DATA*	mapBlock;
	int				i;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &sampleMap->mapBlock[i];

		if( mapBlock->g3Drnd != NULL ){
			GFL_G3D_RENDER_Delete( mapBlock->g3Drnd );
		}
		NNS_GfdFreePlttVram( mapBlock->plttKey );
		NNS_GfdFreeTexVram( mapBlock->texKey );

		GFL_HEAP_FreeMemory( mapBlock->g3DresTex );
		GFL_HEAP_FreeMemory( mapBlock->g3DresMdl );
		GFL_ARC_CloseDataHandle( mapBlock->arc );
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロード
 */
//------------------------------------------------------------------
enum {
	MDL_LOAD_START = 0,
	MDL_LOAD,
	TEX_LOAD_START,
	TEX_LOAD,
	RND_CREATE,
	TEX_TRANS,
	PLTT_TRANS,
	ATTR_LOAD_START,
	ATTR_LOAD,
};

static void	Load3Dmap( SAMPLE_MAP* sampleMap )
{
	MAP_BLOCK_DATA*	mapBlock;
	int				i;
	u32				dataAdrs;

	GF_ASSERT( sampleMap );

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &sampleMap->mapBlock[i];

		if( mapBlock->dataLoadReq == TRUE ){
			switch( mapBlock->loadSeq ){
			case MDL_LOAD_START:
				if( mapBlock->g3Drnd != NULL ){
					GFL_G3D_RENDER_Delete( mapBlock->g3Drnd );
					mapBlock->g3Drnd = NULL;
				}
				//モデルデータロード開始
				GFL_ARC_LoadDataOfsByHandle( mapBlock->arc, mapBlock->dataID, 
												0, MAPTRANS_SIZE, (void*)mapBlock->data );
				mapBlock->loadCount = 0;
				mapBlock->loadCount++;
				mapBlock->loadSeq = MDL_LOAD;
				break;
			case MDL_LOAD:
				dataAdrs = mapBlock->loadCount * MAPTRANS_SIZE;

				if( (dataAdrs + MAPTRANS_SIZE) < MAPDATA_SIZE){
					//ロード継続
					GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPTRANS_SIZE, 
														(void*)&mapBlock->data[dataAdrs] );
					mapBlock->loadCount++;
					break;
				}
				//最終ロード
				GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPDATA_SIZE - dataAdrs,
													(void*)&mapBlock->data[dataAdrs] );
				mapBlock->loadSeq = TEX_LOAD_START;
				break;
			case TEX_LOAD_START:
				if( mapBlock->texLoadReq == FALSE ){
					mapBlock->loadSeq = RND_CREATE;
					break;
				}
				//テクスチャロード開始
				GFL_ARC_LoadDataOfsByHandle( mapBlock->arc, mapBlock->texID, 
												0, MAPTRANS_SIZE, (void*)mapBlock->tex );
				mapBlock->loadCount = 0;
				mapBlock->loadCount++;
				mapBlock->loadSeq = TEX_LOAD;
				break;
			case TEX_LOAD:
				dataAdrs = mapBlock->loadCount * MAPTRANS_SIZE;

				if( (dataAdrs + MAPTRANS_SIZE) < MAPTEX_SIZE){
					//ロード継続
					GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPTRANS_SIZE, 
														(void*)&mapBlock->tex[dataAdrs] );
					mapBlock->loadCount++;
					break;
				}
				//最終ロード
				GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPTEX_SIZE - dataAdrs,
													(void*)&mapBlock->tex[dataAdrs] );
				//テクスチャリソースへのＶＲＡＭキーの設定
				NNS_G3dTexSetTexKey( GFL_G3D_GetResTex(mapBlock->g3DresTex), mapBlock->texKey, 0 );
				NNS_G3dPlttSetPlttKey( GFL_G3D_GetResTex(mapBlock->g3DresTex), mapBlock->plttKey );

				mapBlock->loadSeq = RND_CREATE;
				break;
			case RND_CREATE:
				//レンダー作成
				mapBlock->g3Drnd = GFL_G3D_RENDER_Create(	mapBlock->g3DresMdl, 0, 
															mapBlock->g3DresTex );
				mapBlock->loadCount = 0;
				mapBlock->loadSeq = TEX_TRANS;
				break;
			case TEX_TRANS:
				dataAdrs = mapBlock->loadCount * MAPVRAMTRANS_SIZE;

				if( (dataAdrs + MAPVRAMTRANS_SIZE) < MAPTEX_SIZE){
					NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, 
						NNS_GfdGetTexKeyAddr( mapBlock->texKey ) + dataAdrs,
						(void*)(GFL_G3D_GetAdrsTextureData( mapBlock->g3DresTex ) + dataAdrs),
						MAPVRAMTRANS_SIZE );
					//転送継続
					mapBlock->loadCount++;
					break;
				}
				//最終転送
				NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, 
						NNS_GfdGetTexKeyAddr( mapBlock->texKey ) + dataAdrs,
						(void*)(GFL_G3D_GetAdrsTextureData( mapBlock->g3DresTex ) + dataAdrs),
						MAPTEX_SIZE - dataAdrs );
				mapBlock->loadSeq = PLTT_TRANS;
				break;
			case PLTT_TRANS:
				NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, 
						NNS_GfdGetPlttKeyAddr( mapBlock->plttKey ),
						(void*)GFL_G3D_GetAdrsTexturePltt( mapBlock->g3DresTex ),
						MAPPLTT_SIZE );
				//mapBlock->dataLoadReq = FALSE;
				//mapBlock->texLoadReq = FALSE;
				//mapBlock->loadSeq = 0;
				mapBlock->loadSeq = ATTR_LOAD_START;
				break;
			case ATTR_LOAD_START:
				//アトリビュートデータロード開始
				GFL_ARC_LoadDataOfsByHandle( mapBlock->arc, mapBlock->attrID, 
												0, MAPTRANS_SIZE, (void*)mapBlock->attr );
				mapBlock->loadCount = 0;
				mapBlock->loadCount++;
				mapBlock->loadSeq = ATTR_LOAD;
				break;
			case ATTR_LOAD:
				dataAdrs = mapBlock->loadCount * MAPTRANS_SIZE;

				if( (dataAdrs + MAPTRANS_SIZE) < MAPATTR_SIZE){
					//ロード継続
					GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPTRANS_SIZE, 
														(void*)&mapBlock->attr[dataAdrs] );
					mapBlock->loadCount++;
					break;
				}
				//最終ロード
				GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPATTR_SIZE - dataAdrs,
													(void*)&mapBlock->attr[dataAdrs] );
				mapBlock->dataLoadReq = FALSE;
				mapBlock->texLoadReq = FALSE;
				mapBlock->attrLoadReq = FALSE;
				mapBlock->loadSeq = 0;
				break;
			}
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロードリクエスト設定
 */
//------------------------------------------------------------------
static void	Set3DmapLoadReq( SAMPLE_MAP* sampleMap, const int blockID, const VecFx32* trans, 
								const u32 datID, const u32 texID, const u32 attrID )
{
	MAP_BLOCK_DATA* mapBlock;

	GF_ASSERT( blockID < MAP_BLOCK_COUNT );

	mapBlock = &sampleMap->mapBlock[blockID];

	if( mapBlock->texID != texID ){
		mapBlock->texID = texID;
		mapBlock->texLoadReq = TRUE;
	}
	mapBlock->attrID = attrID;
	mapBlock->attrLoadReq = TRUE;
	mapBlock->dataID = datID;
	mapBlock->dataLoadReq = TRUE;

	mapBlock->loadCount = 0;
	mapBlock->loadSeq = 0;

	mapBlock->idx = datID;
	VEC_Set( &mapBlock->trans, trans->x, trans->y, trans->z );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップアトリビュートポインタ取得
 */
//------------------------------------------------------------------
static void*	Get3DmapAttrData( SAMPLE_MAP* sampleMap, const int blockID )
{
	MAP_BLOCK_DATA* mapBlock;

	GF_ASSERT( blockID < MAP_BLOCK_COUNT );

	//OS_Printf("attrAdrs = %x\n",&sampleMap->mapBlock[blockID].attr);
	return (void*)&sampleMap->mapBlock[blockID].attr;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップディスプレイ
 */
//------------------------------------------------------------------
static void	Draw3Dmap( SAMPLE_MAP* sampleMap, GFL_G3D_CAMERA* g3Dcamera )
{
	MAP_BLOCK_DATA* mapBlock;
	BOOL			cullResult; 
	VecFx32			scale = { FX32_ONE, FX32_ONE, FX32_ONE };
	MtxFx33			rotate = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };
	int				i;

	//描画開始
	GFL_G3D_DRAW_Start();
	//カメラグローバルステート設定		
 	GFL_G3D_DRAW_SetLookAt();

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &sampleMap->mapBlock[i];
		if( mapBlock->g3Drnd != NULL ){
			// 位置設定
			NNS_G3dGlbSetBaseTrans( &mapBlock->trans );
			// 角度設定
			NNS_G3dGlbSetBaseRot( &rotate );
			// スケール設定
			NNS_G3dGlbSetBaseScale( &scale );
			//グローバルステート反映
			NNS_G3dGlbFlush();

			//cullResult = GFL_G3D_DRAW_CheckCulling( g3Dobj );
			NNS_G3dDraw( GFL_G3D_RENDER_GetRenderObj(mapBlock->g3Drnd) );
		}
	}
	//描画終了（バッファスワップ）
	//GFL_G3D_DRAW_End();							
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
	MAP_CONT*			mapper;
	u16					unitIdx;
	u16					resIdx;
	u16					objIdx;

	u16					cursorIdx;
	VecFx32				trans;
	fx32				cameraHeight;
	u16					direction;
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
		{	{ 0, 0, 0 },
			//{ FX32_ONE/8, FX32_ONE/8, FX32_ONE/8 },
			{ FX32_ONE/64, FX32_ONE/64, FX32_ONE/64 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
static CURSOR_CONT*	CreateCursor( SAMPLE_SETUP*	gs, MAP_CONT* mapper, HEAPID heapID )
{
	CURSOR_CONT* cursor = GFL_HEAP_AllocClearMemory( heapID, sizeof(CURSOR_CONT) );

	cursor->heapID = heapID;
	cursor->gs = gs;
	cursor->mapper = mapper;

	cursor->trans.x = FX32_ONE* MAP_WIDTH*1;//FX32_ONE* MAP_WIDTH*16;
	cursor->trans.y = FX32_ONE*0;
	cursor->trans.z = FX32_ONE* MAP_WIDTH*1;//FX32_ONE* MAP_WIDTH*16;
	cursor->cameraHeight = 0;
	cursor->direction = 0;

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
#define MV_SPEED		(1)//(2)
#define RT_SPEED		(FX32_ONE/8)
#define	CAMERA_LENGTH	(16)//(128)
#define	CAMERA_TARGET_HEIGHT	(4)//(8)
static void	MainCursor( CURSOR_CONT* cursor )
{
	VecFx32	pos, target;
	VecFx32	vecMove = { 0, 0, 0 };
	VecFx32	vecUD = { 0, 0, 0 };
	int		key;

	key = GFL_UI_KEY_GetCont();

	if( key & PAD_BUTTON_B ){
		vecMove.y = -MV_SPEED * FX32_ONE;
	}
	if( key & PAD_BUTTON_A ){
		vecMove.y = MV_SPEED * FX32_ONE;
	}
	if( key & PAD_BUTTON_Y ){
		cursor->cameraHeight -= MV_SPEED * FX32_ONE;
	}
	if( key & PAD_BUTTON_X ){
		cursor->cameraHeight += MV_SPEED * FX32_ONE;
	}
	if( key & PAD_KEY_UP ){
		vecMove.x = MV_SPEED * FX_SinIdx( (u16)(cursor->direction + 0x8000) );
		vecMove.z = MV_SPEED * FX_CosIdx( (u16)(cursor->direction + 0x8000) );
	}
	if( key & PAD_KEY_DOWN ){
		vecMove.x = MV_SPEED * FX_SinIdx( (u16)(cursor->direction + 0x0000) );
		vecMove.z = MV_SPEED * FX_CosIdx( (u16)(cursor->direction + 0x0000) );
	}
	if( key & PAD_KEY_LEFT ){
		vecMove.x = MV_SPEED * FX_SinIdx( (u16)(cursor->direction + 0xc000) );
		vecMove.z = MV_SPEED * FX_CosIdx( (u16)(cursor->direction + 0xc000) );
	}
	if( key & PAD_KEY_RIGHT ){
		vecMove.x = MV_SPEED * FX_SinIdx( (u16)(cursor->direction + 0x4000) );
		vecMove.z = MV_SPEED * FX_CosIdx( (u16)(cursor->direction + 0x4000) );
	}
	//vecMove.x /= 2;
	//vecMove.z /= 2;
	if( key & PAD_BUTTON_R ){
		cursor->direction -= RT_SPEED;
	}
	if( key & PAD_BUTTON_L ){
		cursor->direction += RT_SPEED;
	}
	VEC_Add( &cursor->trans, &vecMove, &cursor->trans );
	GetWorldMapHeight( cursor->mapper, &cursor->trans, &cursor->trans.y );
	//OS_Printf(" height = %x\n", cursor->trans.y/4096 );

	cursor->cameraHeight += vecMove.y;

	VEC_Set(&target,cursor->trans.x,cursor->trans.y+CAMERA_TARGET_HEIGHT*FX32_ONE,cursor->trans.z);
	pos.x = cursor->trans.x + CAMERA_LENGTH * FX_SinIdx(cursor->direction);
	pos.y = cursor->trans.y + cursor->cameraHeight;
	pos.z = cursor->trans.z + CAMERA_LENGTH * FX_CosIdx(cursor->direction);

	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx), 
								&cursor->trans );
	GFL_G3D_CAMERA_SetTarget( cursor->gs->g3Dcamera, &target );
	GFL_G3D_CAMERA_SetPos( cursor->gs->g3Dcamera, &pos );

	//OS_Printf(" pos {%x,%x,%x}\n", cursor->trans.x, cursor->trans.y, cursor->trans.z );
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
static void GetWorldMapGrahicDataID( u32 idx, u32* datID, u32* texID, u32* attrID );
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
#define MAP_BLOCK_SET_COUNT	(9)//(4)

typedef struct {
	u32			mapID;
	VecFx32		trans;
}MAP_BLOCK_IDX;

struct _MAP_CONT {
	HEAPID			heapID;
	SAMPLE_SETUP*	gs;
	MAP_BLOCK_IDX	mapBlockIdx[MAP_BLOCK_COUNT];
};

#define MAP_BLOCK_LENX		(FX32_ONE * MAP_WIDTH)
#define MAP_BLOCK_LENZ		(FX32_ONE * MAP_WIDTH)
#define MAP_BLOCK_SIZX		(4)//(32)
#define MAP_BLOCK_SIZZ		(4)//(32)
#define MAP_BLOCK_IDXMAX	(MAP_BLOCK_SIZX * MAP_BLOCK_SIZZ)
#define MAP3D_GRID_SIZEX	(32)
#define MAP3D_GRID_SIZEZ	(32)
#define MAP3D_GRID_LENX		(MAP_BLOCK_LENX / MAP3D_GRID_SIZEX)
#define MAP3D_GRID_LENZ		(MAP_BLOCK_LENZ / MAP3D_GRID_SIZEZ)

static void GetMapperBlockIndex
		( const VecFx32* pos, MAP_BLOCK_IDX* mapBlockIdx, GFL_G3D_CAMERA* g3Dcamera );
static BOOL	ReloadMapperBlock( MAP_CONT* mapper, MAP_BLOCK_IDX* new );
static BOOL	CheckMapperBlockReload( MAP_BLOCK_IDX* resist, MAP_BLOCK_IDX* now, MAP_BLOCK_IDX* new );
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
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
	GFL_HEAP_FreeMemory( mapper );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
static void	MainMapper( MAP_CONT* mapper, VecFx32* pos )
{
	MAP_BLOCK_IDX nowBlockIdx[MAP_BLOCK_SET_COUNT];

 	GetMapperBlockIndex( pos, &nowBlockIdx[0], mapper->gs->g3Dcamera );
	if( ReloadMapperBlock( mapper, &nowBlockIdx[0] ) == TRUE ){
		//OS_Printf("map reloaded\n");
	}
}


//------------------------------------------------------------------
/**
 * @brief	マップ更新ブロック取得
 */
//------------------------------------------------------------------
typedef struct {
	s16	z;
	s16	x;
}BLOCK_OFFS;

static const BLOCK_OFFS blockPat_U[] = {//自分のいるブロックから前方向に９ブロックとる
	{-2,-1},{-2, 0},{-2, 1},{-1,-1},{-1, 0},{-1, 1},{ 0,-1},{ 0, 0},{ 0, 1},	
};
static const BLOCK_OFFS blockPat_UL[] = {//自分のいるブロックから左前方向に９ブロックとる
	{-2,-2},{-2,-1},{-2, 0},{-1,-2},{-1,-1},{-1, 0},{ 0,-2},{ 0,-1},{ 0, 0},
};
static const BLOCK_OFFS blockPat_L[] = {//自分のいるブロックから左方向に９ブロックとる
	{-1,-2},{-1,-1},{-1, 0},{ 0,-2},{ 0,-1},{ 0, 0},{ 1,-2},{ 1,-1},{ 1, 0},
};
static const BLOCK_OFFS blockPat_DL[] = {//自分のいるブロックから左後方向に９ブロックとる
	{ 0,-2},{ 0,-1},{ 0, 0},{ 1,-2},{ 1,-1},{ 1, 0},{ 2,-2},{ 2,-1},{ 2, 0},
};
static const BLOCK_OFFS blockPat_D[] = {//自分のいるブロックから後方向に９ブロックとる
	{ 0,-1},{ 0, 0},{ 0, 1},{ 1,-1},{ 1, 0},{ 1, 1},{ 2,-1},{ 2, 0},{ 2, 1},
};
static const BLOCK_OFFS blockPat_DR[] = {//自分のいるブロックから右後方向に９ブロックとる
	{ 0, 0},{ 0, 1},{ 0, 2},{ 1, 0},{ 1, 1},{ 1, 2},{ 2, 0},{ 2, 1},{ 2, 2},
};
static const BLOCK_OFFS blockPat_R[] = {//自分のいるブロックから右方向に９ブロックとる
	{-1, 0},{-1, 1},{-1, 2},{ 0, 0},{ 0, 1},{ 0, 2},{ 1, 0},{ 1, 1},{ 1, 2},
};
static const BLOCK_OFFS blockPat_UR[] = {//自分のいるブロックから右前方向に９ブロックとる
	{-2, 0},{-2, 1},{-2, 2},{-1, 0},{-1, 1},{-1, 2},{ 0, 0},{ 0, 1},{ 0, 2},
};

static const BLOCK_OFFS blockPat_Around[] = {//自分のいるブロックから周囲方向に９ブロックとる
	{-1,-1},{-1, 0},{-1, 1},{ 0,-1},{ 0, 0},{ 0, 1},{ 1,-1},{ 1, 0},{ 1, 1},
};

static const BLOCK_OFFS* blockIdxOffs_FL[] = {
	blockPat_U, blockPat_UL, blockPat_UL, blockPat_L, 
	blockPat_L, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_U, 
};
static const BLOCK_OFFS* blockIdxOffs_FR[] = {
	blockPat_U, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_R, 
	blockPat_R, blockPat_UR, blockPat_UR, blockPat_U, 
};
static const BLOCK_OFFS* blockIdxOffs_BL[] = {
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_L, 
	blockPat_L, blockPat_DL, blockPat_DL, blockPat_D, 
	blockPat_D, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
};
static const BLOCK_OFFS* blockIdxOffs_BR[] = {
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_D, 
	blockPat_D, blockPat_DR, blockPat_DR, blockPat_R, 
	blockPat_R, blockPat_Around, blockPat_Around, blockPat_Around, 
};

static const BLOCK_OFFS* blockIdxOffs_All[] = {
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
	blockPat_Around, blockPat_Around, blockPat_Around, blockPat_Around, 
};

static const BLOCK_OFFS** blockIdxOffs[] = {
//	blockIdxOffs_FL, blockIdxOffs_FR, blockIdxOffs_BL, blockIdxOffs_BR,
	blockIdxOffs_All, blockIdxOffs_All, blockIdxOffs_All, blockIdxOffs_All,
};

static void GetMapperBlockIndex
		( const VecFx32* pos, MAP_BLOCK_IDX* mapBlockIdx, GFL_G3D_CAMERA* g3Dcamera )
{
	u32		blockIdx, blockx, blockz;
	fx32	posx, posz, scalar;
	int		i, offsx, offsz;
	u16		direction;
	u8		blockPat, dirPat;
	VecFx32	center;
	const BLOCK_OFFS** idxTbl;
	
	//現在ブロックのXZindex取得
	blockx = FX_Whole( FX_Div( pos->x, MAP_BLOCK_LENX ) );
	blockz = FX_Whole( FX_Div( pos->z, MAP_BLOCK_LENZ ) );
	posx = FX_Mod( pos->x, MAP_BLOCK_LENX );
	posz = FX_Mod( pos->z, MAP_BLOCK_LENZ );

	//現在カメラ方向によるパターン取得
	direction = GFL_G3D_CAMERA_GetRadianXZ( g3Dcamera );
	dirPat = (direction >> 12)&0x000f;

	//現在ブロックの中心座標算出
	VEC_Set( &center,	(blockx * MAP_WIDTH + MAP_WIDTH/2) * FX32_ONE, 
						0, 
						(blockz * MAP_WIDTH + MAP_WIDTH/2) * FX32_ONE );
	//scalar = GFL_G3D_CAMERA_GetDotProduct( g3Dcamera, &center );

	//現在ブロック内位置によるパターンテーブル取得
	blockPat = 0;
	if( posx > (MAP_BLOCK_LENX/2) ){
		blockPat += 1;
	}
	if( posz > (MAP_BLOCK_LENZ/2) ){
		blockPat += 2;
	}
	idxTbl = blockIdxOffs[blockPat];

	for( i=0; i<MAP_BLOCK_SET_COUNT; i++ ){
		offsx = blockx + idxTbl[dirPat][i].x;
		offsz = blockz + idxTbl[dirPat][i].z;

		if((offsx < 0)||(offsx >= MAP_BLOCK_SIZX)||(offsz < 0)||(offsz >= MAP_BLOCK_SIZZ)){
			blockIdx = MAPID_NULL;
		} else {
			blockIdx = offsz * MAP_BLOCK_SIZX + offsx;

			if( blockIdx >= MAP_BLOCK_IDXMAX ){
				blockIdx = MAPID_NULL;
			}
		}
		mapBlockIdx[i].mapID = blockIdx;
		mapBlockIdx[i].trans.x = (offsx * MAP_WIDTH + MAP_WIDTH/2) * FX32_ONE;
		mapBlockIdx[i].trans.y = 0;
		mapBlockIdx[i].trans.z = (offsz * MAP_WIDTH + MAP_WIDTH/2) * FX32_ONE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	マップブロック更新チェック
 */
//------------------------------------------------------------------
static BOOL	ReloadMapperBlock( MAP_CONT* mapper, MAP_BLOCK_IDX* new )
{
	BOOL addFlag, delFlag, delProcFlag, addProcFlag, reloadFlag;
	int i, j, c;

	reloadFlag = FALSE;

	//更新ブロックチェック
	delProcFlag = FALSE;
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		if( mapper->mapBlockIdx[i].mapID != MAPID_NULL ){
			delFlag = FALSE;
			for( j=0; j<MAP_BLOCK_SET_COUNT; j++ ){
				if(( mapper->mapBlockIdx[i].mapID == new[j].mapID )&&(delFlag == FALSE )){
					new[j].mapID = MAPID_NULL;
					delFlag = TRUE;
				}
			}
			if( delFlag == FALSE ){
				mapper->mapBlockIdx[i].mapID = MAPID_NULL;
				delProcFlag = TRUE;
			}
		}
	}
	//更新
	addProcFlag = FALSE;
	c = 0;
	for( i=0; i<MAP_BLOCK_SET_COUNT; i++ ){
		if( new[i].mapID != MAPID_NULL ){
			addFlag = FALSE;
			for( j=0; j<MAP_BLOCK_COUNT; j++ ){
				if(( mapper->mapBlockIdx[j].mapID == MAPID_NULL )&&(addFlag == FALSE )){
					{
						u32 datID, texID, attrID;

						GetWorldMapGrahicDataID( new[i].mapID, &datID, &texID, &attrID );
						Set3DmapLoadReq( mapper->gs->sampleMap, j, &new[i].trans,
											datID, texID, attrID );
					}
					mapper->mapBlockIdx[j] = new[i];
					addFlag = TRUE;
					addProcFlag = TRUE;
				}
			}
			if( addFlag == FALSE ){
				GF_ASSERT(0);
			}
		}
	}
	if( addProcFlag == TRUE ){
		reloadFlag = TRUE;
	}
	return reloadFlag;
}


//------------------------------------------------------------------
/**
 * @brief	ブロック情報取得
 */
//------------------------------------------------------------------
static BOOL	GetMapBlock( MAP_CONT* mapper, const VecFx32* pos, 
							u32* blockNum, fx32* inBlockx, fx32* inBlockz ) 
{
	VecFx32 vecTop, vecGrid, inBlockPos;
	VecFx32 vecDefault = {0,0,0};
	fx32	mapLengthX = MAP3D_GRID_SIZEX*MAP3D_GRID_LENX;
	fx32	mapLengthZ = MAP3D_GRID_SIZEZ*MAP3D_GRID_LENZ;
	int i;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		vecTop.x = mapper->mapBlockIdx[i].trans.x - mapLengthX/2;
		vecTop.y = 0;
		vecTop.z = mapper->mapBlockIdx[i].trans.z - mapLengthZ/2;

		if(	(pos->x >= vecTop.x)&&(pos->x < vecTop.x+mapLengthX)
			&&(pos->z >= vecTop.z)&&(pos->z < vecTop.z+mapLengthZ) ){

			*blockNum = i;
			VEC_Subtract( &vecDefault, &vecTop, &vecGrid );
			VEC_Add( &vecGrid, pos, &inBlockPos );
			*inBlockx = inBlockPos.x;
			*inBlockz = inBlockPos.z;
			return TRUE;
		}
	}
	*blockNum = 0;
	*inBlockx = 0;
	*inBlockz = 0;
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	グリッド情報取得
 */
//------------------------------------------------------------------
static void	GetMapGrid( const fx32 inBlockx, const fx32 inBlockz,
						u32* gridIdx, fx32* inGridx, fx32* inGridz ) 
{
	u32 gridx, gridz;

	gridx = inBlockx/MAP3D_GRID_LENX;
	gridz = inBlockz/MAP3D_GRID_LENZ;

	*gridIdx = gridz * MAP3D_GRID_SIZEX + gridx;
	*inGridx = inBlockx%MAP3D_GRID_LENX;
	*inGridz = inBlockz%MAP3D_GRID_LENZ;
}

//------------------------------------------------------------------
/**
 * @brief	グリッド内三角形パターン取得
 */
//------------------------------------------------------------------
static u32 GetMapTriangleID( const fx32 inGridx, const fx32 inGridz, const u32 gridTryPattern )
{
	//グリッド内三角形の判定
	if( gridTryPattern == 0 ){
		//0-2-1,3-1-2のパターン
		if( inGridx + inGridz < MAP3D_GRID_LENX ){	//←ちょっと適当
			return 0;
		} else {
			return 1;
		}
	} else {
		//2-3-0,1-0-3のパターン
		if( inGridx > inGridz ){
			return 0;
		} else {
			return 1;
		}
	}
	return 0;
}

//------------------------------------------------------------------
/**
 * @brief	現在位置地形の法線ベクトル取得
 */
//------------------------------------------------------------------
static void GetWorldMapVecN( MAP_CONT* mapper, VecFx32* pos, VecFx32* vecN )
{
	u32		blockNum;
	fx32	inBlockx, inBlockz;
	u32		gridIdx;
	fx32	inGridx, inGridz;
	NormalVtxSt* mapData;

	if( GetMapBlock( mapper, pos, &blockNum, &inBlockx, &inBlockz )  == FALSE ){
		//OS_Printf("map blockNum not exist");
		vecN->x = 0; 
		vecN->y = FX32_ONE;
		vecN->z = 0; 
		return;
	}
	GetMapGrid( inBlockx, inBlockz, &gridIdx, &inGridx, &inGridz ); 
	mapData = (NormalVtxSt*)Get3DmapAttrData( mapper->gs->sampleMap, blockNum );

	if( GetMapTriangleID( inGridx, inGridz, mapData->tryangleType ) == 0 ){
		vecN->x = mapData->vecN1_x; 
		vecN->y = mapData->vecN1_y; 
		vecN->z = -mapData->vecN1_z; 
	} else {
		vecN->x = mapData->vecN2_x; 
		vecN->y = mapData->vecN2_y; 
		vecN->z = -mapData->vecN2_z; 
	}
}

//------------------------------------------------------------------
/**
 * @brief	現在位置地形の高さ取得
 */
//------------------------------------------------------------------
static void GetWorldMapHeight( MAP_CONT* mapper, VecFx32* pos, fx32* height )
{
	u32		blockNum, blockIdx;
	fx32	inBlockx, inBlockz;
	u32		gridIdx;
	fx32	inGridx, inGridz;
	NormalVtxSt* mapData;
	fx32	by, valD;
	VecFx32 vecN, posLocalMap;

	if( GetMapBlock( mapper, pos, &blockNum, &inBlockx, &inBlockz )  == FALSE ){
		//OS_Printf("map blockNum not exist");
		*height = 0;
		return;
	}
	blockIdx = mapper->mapBlockIdx[blockNum].mapID;
	VEC_Subtract( pos, &mapper->mapBlockIdx[blockNum].trans, &posLocalMap );

	mapData = (NormalVtxSt*)Get3DmapAttrData( mapper->gs->sampleMap, blockNum );
	//OS_Printf("blockIdx = %x, inBlock { %x, %x }\n", blockIdx, inBlockx, inBlockz );
	GetMapGrid( inBlockx, inBlockz, &gridIdx, &inGridx, &inGridz ); 
	//OS_Printf("gridIdx = %x, inGrid { %x, %x }\n", gridIdx, inGridx, inGridz );

	if( GetMapTriangleID( inGridx, inGridz, mapData[gridIdx].tryangleType ) == 0 ){
		vecN.x = mapData[gridIdx].vecN1_x; 
		vecN.y = mapData[gridIdx].vecN1_y; 
		vecN.z = -mapData[gridIdx].vecN1_z;
		valD = mapData[gridIdx].vecN1_D;		//軸の取り方が違うので反転
	} else {
		vecN.x = mapData[gridIdx].vecN2_x; 
		vecN.y = mapData[gridIdx].vecN2_y; 
		vecN.z = -mapData[gridIdx].vecN2_z;
		valD = mapData[gridIdx].vecN2_D;		//軸の取り方が違うので反転
	}
	by = -( FX_Mul( vecN.x, posLocalMap.x ) + FX_Mul( vecN.z, posLocalMap.z ) + valD );
	*height = FX_Div( by, vecN.y );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップディスプレイ（サンプル用）
 */
//------------------------------------------------------------------
static void	Draw3DmapSample( SAMPLE_MAP* sampleMap, GFL_G3D_CAMERA* g3Dcamera )
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
	G3_PolygonAttr(	GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, 
					GX_CULL_NONE, 63, 31, GX_POLYGON_ATTR_MISC_FOG);

	G3_Scale( FX32_ONE*MAPSCALE_WIDTH, FX32_ONE*MAPSCALE_WIDTH, FX32_ONE*MAPSCALE_WIDTH );


	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &sampleMap->mapBlock[i];
		//if(( mapBlock->idx != MAPID_NULL )&&( mapBlock->dataLoadReq == FALSE )){
		if( mapBlock->idx != MAPID_NULL ){
			int		x, z, d;
			fx16	vtx0x, vtx0z, len;
			u32		texDataAdrs, texPlttAdrs;

			texDataAdrs = NNS_GfdGetTexKeyAddr( mapBlock->texKey ); 
			texPlttAdrs = NNS_GfdGetPlttKeyAddr( mapBlock->plttKey ); 

			G3_PushMtx();

			G3_Translate(	mapBlock->trans.x/MAPSCALE_WIDTH,
							mapBlock->trans.y/MAPSCALE_WIDTH,
							mapBlock->trans.z/MAPSCALE_WIDTH );

			G3_TexImageParam(	GX_TEXFMT_PLTT16, GX_TEXGEN_TEXCOORD, 
								GX_TEXSIZE_S32, GX_TEXSIZE_T32,
								GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
								GX_TEXPLTTCOLOR0_TRNS, texDataAdrs );
			G3_TexPlttBase( texPlttAdrs, GX_TEXFMT_PLTT16 );

			G3_Begin( GX_BEGIN_QUADS );

			d = 32;
			len = FX16_ONE*2/d;

			for( z=0; z<d; z++ ){
				for( x=0; x<d; x++ ){
					vtx0x = -FX16_ONE + x*len;
					vtx0z = -FX16_ONE + z*len;

					G3_TexCoord( 0, FX32_ONE*32 );
					G3_Vtx( vtx0x, 0, vtx0z + len );

					G3_TexCoord( FX32_ONE*32, FX32_ONE*32 );
					G3_Vtx( vtx0x + len, 0, vtx0z + len );

					G3_TexCoord( FX32_ONE*32, 0 );
					G3_Vtx( vtx0x + len, 0, vtx0z );

					G3_TexCoord( 0, 0 );
					G3_Vtx( vtx0x, 0, vtx0z );
				}
			}
			G3_End();
			G3_PopMtx(1);
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップデータ作成（サンプル用）
 */
//------------------------------------------------------------------
static void	Make3DmapSample( SAMPLE_MAP* sampleMap, const int blockID, const u32 mapID )
{
	MAP_BLOCK_DATA*		mapBlock;

	GF_ASSERT( blockID < MAP_BLOCK_COUNT );

	mapBlock = &sampleMap->mapBlock[blockID];

	if( mapID != MAPID_NULL ){
		//テクスチャデータ作成
		u32						texData = GFL_G3D_GetAdrsTextureData( mapBlock->g3DresTex );
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
											NNS_GfdGetTexKeyAddr( mapBlock->texKey ), 
											(void*)texData, texSize );
		GFL_BMP_Delete( bmpTex );
	}
	//OS_Printf("mapGraphic block%x is maked id=%x, tAdrs=%x, pAdrs=%x\n", 
	//			blockID, mapBlock->idx, mapBlock->texDataAdrs, mapBlock->texPlttAdrs );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ配置データ（サンプル用）
 */
//------------------------------------------------------------------
typedef struct {
	u16 datID;
	u16 texID;
	u16 attrID;
}WORLDMAP_RESIDX_TBL;

static const WORLDMAP_RESIDX_TBL DPworldMap[] = {
	{ NARC_sample_map_map_a1_nsbmd, NARC_sample_map_map_a1_nsbtx, NARC_sample_map_map_a1_bin },
	{ NARC_sample_map_map_a2_nsbmd, NARC_sample_map_map_a2_nsbtx, NARC_sample_map_map_a2_bin },
	{ NARC_sample_map_map_a3_nsbmd, NARC_sample_map_map_a3_nsbtx, NARC_sample_map_map_a3_bin },
	{ NARC_sample_map_map_a4_nsbmd, NARC_sample_map_map_a4_nsbtx, NARC_sample_map_map_a4_bin },
	{ NARC_sample_map_map_b1_nsbmd, NARC_sample_map_map_b1_nsbtx, NARC_sample_map_map_b1_bin },
	{ NARC_sample_map_map_b2_nsbmd, NARC_sample_map_map_b2_nsbtx, NARC_sample_map_map_b2_bin },
	{ NARC_sample_map_map_b3_nsbmd, NARC_sample_map_map_b3_nsbtx, NARC_sample_map_map_b3_bin },
	{ NARC_sample_map_map_b4_nsbmd, NARC_sample_map_map_b4_nsbtx, NARC_sample_map_map_b4_bin },
	{ NARC_sample_map_map_c1_nsbmd, NARC_sample_map_map_c1_nsbtx, NARC_sample_map_map_c1_bin },
	{ NARC_sample_map_map_c2_nsbmd, NARC_sample_map_map_c2_nsbtx, NARC_sample_map_map_c2_bin },
	{ NARC_sample_map_map_c3_nsbmd, NARC_sample_map_map_c3_nsbtx, NARC_sample_map_map_c3_bin },
	{ NARC_sample_map_map_c4_nsbmd, NARC_sample_map_map_c4_nsbtx, NARC_sample_map_map_c4_bin },
	{ NARC_sample_map_map_d1_nsbmd, NARC_sample_map_map_d1_nsbtx, NARC_sample_map_map_d1_bin },
	{ NARC_sample_map_map_d2_nsbmd, NARC_sample_map_map_d2_nsbtx, NARC_sample_map_map_d2_bin },
	{ NARC_sample_map_map_d3_nsbmd, NARC_sample_map_map_d3_nsbtx, NARC_sample_map_map_d3_bin },
	{ NARC_sample_map_map_d4_nsbmd, NARC_sample_map_map_d4_nsbtx, NARC_sample_map_map_d4_bin },
};

static void GetWorldMapGrahicDataID( u32 idx, u32* datID, u32* texID, u32* attrID )
{
	*datID = (u32)DPworldMap[idx].datID;
	*texID = (u32)DPworldMap[idx].texID;
	*attrID = (u32)DPworldMap[idx].attrID;
}
















#if 0
//	NARC_fld_map_ro_treec_nsbmd = 178,
//	NARC_fld_map_ro_tree2c_nsbmd = 179,
//	NARC_fld_map_ro_tree3c_nsbmd = 180,
//	NARC_fld_map_ro_mountc_nsbmd = 181,
//	NARC_fld_map_ro_seac_nsbmd = 182,
//	NARC_fld_map_ro_snowc_nsbmd = 183,

static const WORLDMAP_RESIDX_TBL DPworldMap[] = {
	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//10
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//11
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_map09_06c_nsbmd, NARC_fld_map_map09_06c_nsbtx },	//9
	{ NARC_fld_map_map10_06c_nsbmd, NARC_fld_map_map10_06c_nsbtx },	//10
	{ NARC_fld_map_map11_06c_nsbmd, NARC_fld_map_map11_06c_nsbtx },	//11
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//22
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//23
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_06c_nsbmd, NARC_fld_map_map28_06c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_map09_07c_nsbmd, NARC_fld_map_map09_07c_nsbtx },	//9
	{ NARC_fld_map_map10_07c_nsbmd, NARC_fld_map_map10_07c_nsbtx },	//10
	{ NARC_fld_map_map11_07c_nsbmd, NARC_fld_map_map11_07c_nsbtx },	//11
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//22
	{ NARC_fld_map_map23_07c_nsbmd, NARC_fld_map_map23_07c_nsbtx },	//23
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_07c_nsbmd, NARC_fld_map_map28_07c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_map01_08c_nsbmd, NARC_fld_map_map01_08c_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_map04_08c_nsbmd, NARC_fld_map_map04_08c_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_map09_08c_nsbmd, NARC_fld_map_map09_08c_nsbtx },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//22
	{ NARC_fld_map_map23_08c_nsbmd, NARC_fld_map_map23_08c_nsbtx },	//23
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_08c_nsbmd, NARC_fld_map_map28_08c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_map09_09c_nsbmd, NARC_fld_map_map09_09c_nsbtx },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//19
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//20
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//21
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//22
	{ NARC_fld_map_map23_09c_nsbmd, NARC_fld_map_map23_09c_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_09c_nsbmd, NARC_fld_map_map28_09c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_map09_10c_nsbmd, NARC_fld_map_map09_10c_nsbtx },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_map19_10c_nsbmd, NARC_fld_map_map19_10c_nsbtx },	//19
	{ NARC_fld_map_map20_10c_nsbmd, NARC_fld_map_map20_10c_nsbtx },	//20
	{ NARC_fld_map_map21_10c_nsbmd, NARC_fld_map_map21_10c_nsbtx },	//21
	{ NARC_fld_map_map22_10c_nsbmd, NARC_fld_map_map22_10c_nsbtx },	//22
	{ NARC_fld_map_map23_10c_nsbmd, NARC_fld_map_map23_10c_nsbtx },	//23
	{ NARC_fld_map_map24_10c_nsbmd, NARC_fld_map_map24_10c_nsbtx },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_10c_nsbmd, NARC_fld_map_map28_10c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_map09_11c_nsbmd, NARC_fld_map_map09_11c_nsbtx },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//10
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx },	//11
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_map19_11c_nsbmd, NARC_fld_map_map19_11c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//23
	{ NARC_fld_map_map24_11c_nsbmd, NARC_fld_map_map24_11c_nsbtx },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_11c_nsbmd, NARC_fld_map_map28_11c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_map09_12c_nsbmd, NARC_fld_map_map09_12c_nsbtx },	//9
	{ NARC_fld_map_map10_12c_nsbmd, NARC_fld_map_map10_12c_nsbtx },	//10
	{ NARC_fld_map_map11_12c_nsbmd, NARC_fld_map_map11_12c_nsbtx },	//11
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_map19_12c_nsbmd, NARC_fld_map_map19_12c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//23
	{ NARC_fld_map_map24_12c_nsbmd, NARC_fld_map_map24_12c_nsbtx },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_12c_nsbmd, NARC_fld_map_map28_12c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//8
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx },	//9
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx },	//10
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx },	//11
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_map19_13c_nsbmd, NARC_fld_map_map19_13c_nsbtx },	//19
	{ NARC_fld_map_map20_13c_nsbmd, NARC_fld_map_map20_13c_nsbtx },	//20
	{ NARC_fld_map_map21_13c_nsbmd, NARC_fld_map_map21_13c_nsbtx },	//21
	{ NARC_fld_map_map22_13c_nsbmd, NARC_fld_map_map22_13c_nsbtx },	//22
	{ NARC_fld_map_map23_13c_nsbmd, NARC_fld_map_map23_13c_nsbtx },	//23
	{ NARC_fld_map_map24_13c_nsbmd, NARC_fld_map_map24_13c_nsbtx },	//24
	{ NARC_fld_map_map25_13c_nsbmd, NARC_fld_map_map25_13c_nsbtx },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_13c_nsbmd, NARC_fld_map_map28_13c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_map25_14c_nsbmd, NARC_fld_map_map25_14c_nsbtx },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_14c_nsbmd, NARC_fld_map_map28_14c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_map03_15c_nsbmd, NARC_fld_map_map03_15c_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//7
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//8
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//16
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//21
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_map28_15c_nsbmd, NARC_fld_map_map28_15c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//5
	{ NARC_fld_map_map06_16c_nsbmd, NARC_fld_map_map06_16c_nsbtx },	//6
	{ NARC_fld_map_map07_16c_nsbmd, NARC_fld_map_map07_16c_nsbtx },	//7
	{ NARC_fld_map_map08_16c_nsbmd, NARC_fld_map_map08_16c_nsbtx },	//8
	{ NARC_fld_map_map09_16c_nsbmd, NARC_fld_map_map09_16c_nsbtx },	//9
	{ NARC_fld_map_map10_16c_nsbmd, NARC_fld_map_map10_16c_nsbtx },	//10
	{ NARC_fld_map_map11_16c_nsbmd, NARC_fld_map_map11_16c_nsbtx },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//12
	{ NARC_fld_map_map13_16c_nsbmd, NARC_fld_map_map13_16c_nsbtx },	//13
	{ NARC_fld_map_map14_16c_nsbmd, NARC_fld_map_map14_16c_nsbtx },	//14
	{ NARC_fld_map_map15_16c_nsbmd, NARC_fld_map_map15_16c_nsbtx },	//15
	{ NARC_fld_map_map16_16c_nsbmd, NARC_fld_map_map16_16c_nsbtx },	//16
	{ NARC_fld_map_map17_16c_nsbmd, NARC_fld_map_map17_16c_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//21
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_map27_16c_nsbmd, NARC_fld_map_map27_16c_nsbtx },	//27
	{ NARC_fld_map_map28_16c_nsbmd, NARC_fld_map_map28_16c_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//5
	{ NARC_fld_map_map06_17c_nsbmd, NARC_fld_map_map06_17c_nsbtx },	//6
	{ NARC_fld_map_map07_17c_nsbmd, NARC_fld_map_map07_17c_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_map09_17c_nsbmd, NARC_fld_map_map09_17c_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//16
	{ NARC_fld_map_map17_17c_nsbmd, NARC_fld_map_map17_17c_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_map26_17c_nsbmd, NARC_fld_map_map26_17c_nsbtx },	//26
	{ NARC_fld_map_map27_17c_nsbmd, NARC_fld_map_map27_17c_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_map05_18c_nsbmd, NARC_fld_map_map05_18c_nsbtx },	//5
	{ NARC_fld_map_map06_18c_nsbmd, NARC_fld_map_map06_18c_nsbtx },	//6
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_map09_18c_nsbmd, NARC_fld_map_map09_18c_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//16
	{ NARC_fld_map_map17_18c_nsbmd, NARC_fld_map_map17_18c_nsbtx },	//17
	{ NARC_fld_map_map18_18c_nsbmd, NARC_fld_map_map18_18c_nsbtx },	//18
	{ NARC_fld_map_map19_18c_nsbmd, NARC_fld_map_map19_18c_nsbtx },	//19
	{ NARC_fld_map_map20_18c_nsbmd, NARC_fld_map_map20_18c_nsbtx },	//20
	{ NARC_fld_map_map21_18c_nsbmd, NARC_fld_map_map21_18c_nsbtx },	//21
	{ NARC_fld_map_map22_18c_nsbmd, NARC_fld_map_map22_18c_nsbtx },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_map26_18c_nsbmd, NARC_fld_map_map26_18c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_map05_19c_nsbmd, NARC_fld_map_map05_19c_nsbtx },	//5
	{ NARC_fld_map_map06_19c_nsbmd, NARC_fld_map_map06_19c_nsbtx },	//6
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_map09_19c_nsbmd, NARC_fld_map_map09_19c_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//16
	{ NARC_fld_map_map17_19c_nsbmd, NARC_fld_map_map17_19c_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_map21_19c_nsbmd, NARC_fld_map_map21_19c_nsbtx },	//21
	{ NARC_fld_map_map22_19c_nsbmd, NARC_fld_map_map22_19c_nsbtx },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_map26_19c_nsbmd, NARC_fld_map_map26_19c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_map05_20c_nsbmd, NARC_fld_map_map05_20c_nsbtx },	//5
	{ NARC_fld_map_map06_20c_nsbmd, NARC_fld_map_map06_20c_nsbtx },	//6
	{ NARC_fld_map_map07_20c_nsbmd, NARC_fld_map_map07_20c_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_map09_20c_nsbmd, NARC_fld_map_map09_20c_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//13
	{ NARC_fld_map_map14_20c_nsbmd, NARC_fld_map_map14_20c_nsbtx },	//14
	{ NARC_fld_map_map15_20c_nsbmd, NARC_fld_map_map15_20c_nsbtx },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//16
	{ NARC_fld_map_map17_20c_nsbmd, NARC_fld_map_map17_20c_nsbtx },	//17
	{ NARC_fld_map_map18_20c_nsbmd, NARC_fld_map_map18_20c_nsbtx },	//18
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//21
	{ NARC_fld_map_map22_20c_nsbmd, NARC_fld_map_map22_20c_nsbtx },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_map26_20c_nsbmd, NARC_fld_map_map26_20c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_map05_21c_nsbmd, NARC_fld_map_map05_21c_nsbtx },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//6
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_map09_21c_nsbmd, NARC_fld_map_map09_21c_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//13
	{ NARC_fld_map_map14_21c_nsbmd, NARC_fld_map_map14_21c_nsbtx },	//14
	{ NARC_fld_map_map15_21c_nsbmd, NARC_fld_map_map15_21c_nsbtx },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//16
	{ NARC_fld_map_map17_21c_nsbmd, NARC_fld_map_map17_21c_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//20
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//21
	{ NARC_fld_map_map22_21c_nsbmd, NARC_fld_map_map22_21c_nsbtx },	//22
	{ NARC_fld_map_map23_21c_nsbmd, NARC_fld_map_map23_21c_nsbtx },	//23
	{ NARC_fld_map_map24_21c_nsbmd, NARC_fld_map_map24_21c_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_map26_21c_nsbmd, NARC_fld_map_map26_21c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//22
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//0
	{ NARC_fld_map_map01_22c_nsbmd, NARC_fld_map_map01_22c_nsbtx },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_map05_22c_nsbmd, NARC_fld_map_map05_22c_nsbtx },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_map09_22c_nsbmd, NARC_fld_map_map09_22c_nsbtx },	//9
	{ NARC_fld_map_map10_22c_nsbmd, NARC_fld_map_map10_22c_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_map12_22c_nsbmd, NARC_fld_map_map12_22c_nsbtx },	//12
	{ NARC_fld_map_map13_22c_nsbmd, NARC_fld_map_map13_22c_nsbtx },	//13
	{ NARC_fld_map_map14_22c_nsbmd, NARC_fld_map_map14_22c_nsbtx },	//14
	{ NARC_fld_map_map15_22c_nsbmd, NARC_fld_map_map15_22c_nsbtx },	//15
	{ NARC_fld_map_map16_22c_nsbmd, NARC_fld_map_map16_22c_nsbtx },	//16
	{ NARC_fld_map_map17_22c_nsbmd, NARC_fld_map_map17_22c_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//21
	{ NARC_fld_map_map22_22c_nsbmd, NARC_fld_map_map22_22c_nsbtx },	//22
	{ NARC_fld_map_map23_22c_nsbmd, NARC_fld_map_map23_22c_nsbtx },	//23
	{ NARC_fld_map_map24_22c_nsbmd, NARC_fld_map_map24_22c_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_map26_22c_nsbmd, NARC_fld_map_map26_22c_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//23
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//0
	{ NARC_fld_map_map01_23c_nsbmd, NARC_fld_map_map01_23c_nsbtx },	//1
	{ NARC_fld_map_map02_23c_nsbmd, NARC_fld_map_map02_23c_nsbtx },	//2
	{ NARC_fld_map_map03_23c_nsbmd, NARC_fld_map_map03_23c_nsbtx },	//3
	{ NARC_fld_map_map04_23c_nsbmd, NARC_fld_map_map04_23c_nsbtx },	//4
	{ NARC_fld_map_map05_23c_nsbmd, NARC_fld_map_map05_23c_nsbtx },	//5
	{ NARC_fld_map_map06_23c_nsbmd, NARC_fld_map_map06_23c_nsbtx },	//6
	{ NARC_fld_map_map07_23c_nsbmd, NARC_fld_map_map07_23c_nsbtx },	//7
	{ NARC_fld_map_map08_23c_nsbmd, NARC_fld_map_map08_23c_nsbtx },	//8
	{ NARC_fld_map_map09_23c_nsbmd, NARC_fld_map_map09_23c_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//13
	{ NARC_fld_map_map14_23c_nsbmd, NARC_fld_map_map14_23c_nsbtx },	//14
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//16
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_map19_23c_nsbmd, NARC_fld_map_map19_23c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_map21_23c_nsbmd, NARC_fld_map_map21_23c_nsbtx },	//21
	{ NARC_fld_map_map22_23c_nsbmd, NARC_fld_map_map22_23c_nsbtx },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//25
	{ NARC_fld_map_map26_23c_nsbmd, NARC_fld_map_map26_23c_nsbtx },	//26
	{ NARC_fld_map_map27_23c_nsbmd, NARC_fld_map_map27_23c_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//24
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//0
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//2
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//3
	{ NARC_fld_map_map04_24c_nsbmd, NARC_fld_map_map04_24c_nsbtx },	//4
	{ NARC_fld_map_map05_24c_nsbmd, NARC_fld_map_map05_24c_nsbtx },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_map09_24c_nsbmd, NARC_fld_map_map09_24c_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//13
	{ NARC_fld_map_map14_24c_nsbmd, NARC_fld_map_map14_24c_nsbtx },	//14
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//16
	{ NARC_fld_map_map17_24c_nsbmd, NARC_fld_map_map17_24c_nsbtx },	//17
	{ NARC_fld_map_map18_24c_nsbmd, NARC_fld_map_map18_24c_nsbtx },	//18
	{ NARC_fld_map_map19_24c_nsbmd, NARC_fld_map_map19_24c_nsbtx },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//20
	{ NARC_fld_map_map21_24c_nsbmd, NARC_fld_map_map21_24c_nsbtx },	//21
	{ NARC_fld_map_map22_24c_nsbmd, NARC_fld_map_map22_24c_nsbtx },	//22
	{ NARC_fld_map_map23_24c_nsbmd, NARC_fld_map_map23_24c_nsbtx },	//23
	{ NARC_fld_map_map24_24c_nsbmd, NARC_fld_map_map24_24c_nsbtx },	//24
	{ NARC_fld_map_map25_24c_nsbmd, NARC_fld_map_map25_24c_nsbtx },	//25
	{ NARC_fld_map_map26_24c_nsbmd, NARC_fld_map_map26_24c_nsbtx },	//26
	{ NARC_fld_map_map27_24c_nsbmd, NARC_fld_map_map27_24c_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//25
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//0
	{ NARC_fld_map_map01_25c_nsbmd, NARC_fld_map_map01_25c_nsbtx },	//1
	{ NARC_fld_map_map02_25c_nsbmd, NARC_fld_map_map02_25c_nsbtx },	//2
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_map05_25c_nsbmd, NARC_fld_map_map05_25c_nsbtx },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//8
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//13
	{ NARC_fld_map_map14_25c_nsbmd, NARC_fld_map_map14_25c_nsbtx },	//14
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//16
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//17
	{ NARC_fld_map_map18_25c_nsbmd, NARC_fld_map_map18_25c_nsbtx },	//18
	{ NARC_fld_map_map19_25c_nsbmd, NARC_fld_map_map19_25c_nsbtx },	//19
	{ NARC_fld_map_map20_25c_nsbmd, NARC_fld_map_map20_25c_nsbtx },	//20
	{ NARC_fld_map_map21_25c_nsbmd, NARC_fld_map_map21_25c_nsbtx },	//21
	{ NARC_fld_map_map22_25c_nsbmd, NARC_fld_map_map22_25c_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//26
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//0
	{ NARC_fld_map_map01_26c_nsbmd, NARC_fld_map_map01_26c_nsbtx },	//1
	{ NARC_fld_map_map02_26c_nsbmd, NARC_fld_map_map02_26c_nsbtx },	//2
	{ NARC_fld_map_map03_26c_nsbmd, NARC_fld_map_map03_26c_nsbtx },	//3
	{ NARC_fld_map_map04_26c_nsbmd, NARC_fld_map_map04_26c_nsbtx },	//4
	{ NARC_fld_map_map05_26c_nsbmd, NARC_fld_map_map05_26c_nsbtx },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//13
	{ NARC_fld_map_map14_26c_nsbmd, NARC_fld_map_map14_26c_nsbtx },	//14
	{ NARC_fld_map_map15_26c_nsbmd, NARC_fld_map_map15_26c_nsbtx },	//15
	{ NARC_fld_map_map16_26c_nsbmd, NARC_fld_map_map16_26c_nsbtx },	//16
	{ NARC_fld_map_map17_26c_nsbmd, NARC_fld_map_map17_26c_nsbtx },	//17
	{ NARC_fld_map_map18_26c_nsbmd, NARC_fld_map_map18_26c_nsbtx },	//18
	{ NARC_fld_map_map19_26c_nsbmd, NARC_fld_map_map19_26c_nsbtx },	//19
	{ NARC_fld_map_map20_26c_nsbmd, NARC_fld_map_map20_26c_nsbtx },	//20
	{ NARC_fld_map_map21_26c_nsbmd, NARC_fld_map_map21_26c_nsbtx },	//21
	{ NARC_fld_map_map22_26c_nsbmd, NARC_fld_map_map22_26c_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//27
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//0
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//2
	{ NARC_fld_map_map03_27c_nsbmd, NARC_fld_map_map03_27c_nsbtx },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//4
	{ NARC_fld_map_map05_27c_nsbmd, NARC_fld_map_map05_27c_nsbtx },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//7
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//8
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//9
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//13
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//16
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//2
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_map05_28c_nsbmd, NARC_fld_map_map05_28c_nsbtx },	//5
	{ NARC_fld_map_map06_28c_nsbmd, NARC_fld_map_map06_28c_nsbtx },	//6
	{ NARC_fld_map_map07_28c_nsbmd, NARC_fld_map_map07_28c_nsbtx },	//7
	{ NARC_fld_map_map08_28c_nsbmd, NARC_fld_map_map08_28c_nsbtx },	//8
	{ NARC_fld_map_map09_28c_nsbmd, NARC_fld_map_map09_28c_nsbtx },	//9
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//8
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//9
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
	//31
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx },	//31
};
#endif



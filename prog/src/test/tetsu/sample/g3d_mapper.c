//============================================================================================
/**
 * @file	g3d_mapper.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system\gfl_use.h"	//乱数用

#include "g3dmapfunc\func_mapeditor_file.h"
#include "g3dmapfunc\func_pmcustom_file.h"

#include "g3d_mapper.h"
//============================================================================================
/**
 *
 *
 *
 * @brief	３Ｄマップコントロール
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
#define MAPID_NULL			(0xffffffff)
#define MAPARC_NULL			(0xffffffff)

#define GLOBAL_OBJ_COUNT	(64)
#define GLOBAL_OBJ_ANMCOUNT	(4)
#define GLOBAL_DDOBJ_COUNT	(32)
//------------------------------------------------------------------
typedef struct {
	u32			blockIdx;
	VecFx32		trans;
}BLOCK_IDX;

//------------------------------------------------------------------
typedef struct {
	GFL_G3D_RES*	g3DresMdl;						//モデルリソース(High Q)
	GFL_G3D_RES*	g3DresTex;						//テクスチャリソース
	GFL_G3D_RES*	g3DresAnm[GLOBAL_OBJ_ANMCOUNT];	//アニメリソース
	GFL_G3D_OBJ*	g3Dobj;							//オブジェクトハンドル
}GLOBALOBJ_RES;

//------------------------------------------------------------------
struct _G3D_MAPPER {
	HEAPID				heapID;
	GFL_G3D_MAP*		g3Dmap[MAP_BLOCK_COUNT];
	BLOCK_IDX			blockIdx[MAP_BLOCK_COUNT];

	u32					nowBlockIdx;				
	VecFx32				posCont;
	u16					sizex;		//横ブロック数
	u16					sizez;		//縦ブロック数
	u32					totalSize;	//配列サイズ
	fx32				width;		//ブロック１辺の幅
	fx32				height;		//ブロック高さ
	G3D_MAPPER_MODE		mode;		//動作モード
	u32					arcID;		//グラフィックアーカイブＩＤ
	const G3D_MAPPER_DATA*	data;	//実マップデータ

	GFL_G3D_RES*			globalTexture;					//共通地形テクスチャ
	GLOBALOBJ_RES*			globalObjRes;					//共通オブジェクト
	u32						globalObjResCount;				//共通オブジェクト数

	GFL_G3D_MAP_GLOBALOBJ	globalObj;						//共通オブジェクト
};

//------------------------------------------------------------------
#define NON_PARAM (0xffffffff)
typedef struct {
	u32 arcID;
	u32	datID;
	u32	inDatNum;
}MAKE_RES_PARAM;

typedef struct {
	MAKE_RES_PARAM	mdl;
	MAKE_RES_PARAM	tex;
	MAKE_RES_PARAM	anm[GLOBAL_OBJ_ANMCOUNT];
}MAKE_OBJ_PARAM;

//------------------------------------------------------------------
static void CreateGlobalTexture( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST* resistData );
static void DeleteGlobalTexture( G3D_MAPPER* g3Dmapper );
static void CreateGlobalObject( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST* resistData );
static void DeleteGlobalObject( G3D_MAPPER* g3Dmapper );

static void CreateGrobalObj_forTbl( G3D_MAPPER* g3Dmapper, const void* resistData );
static void CreateGrobalObj_forBin( G3D_MAPPER* g3Dmapper, const void* resistData );

static void CreateGlobalObj( GLOBALOBJ_RES* objRes, MAKE_OBJ_PARAM* param );
static void DeleteGlobalObj( GLOBALOBJ_RES* objRes );

static void CreateGlobalDDobj( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST_DDOBJ* resistData );
static void DeleteGlobalDDobj( G3D_MAPPER* g3Dmapper );

static void GetMapperBlockIdxAll( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx );
static void GetMapperBlockIdxXZ( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx );
static void GetMapperBlockIdxY( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx );
static BOOL	ReloadMapperBlock( G3D_MAPPER* g3Dmapper, BLOCK_IDX* new );

static const GFL_G3D_MAP_DDOBJ_DATA drawTreeData;
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
static const GFL_G3D_MAP_FILE_FUNC mapFileFuncTbl[] = {
	{ DP3PACK_HEADER, LoadMapData_MapEditorFile, GetAttr_MapEditorFile },
	{ MAPFILE_FUNC_DEFAULT, LoadMapData_PMcustomFile, GetAttr_PMcustomFile },	//TableEnd&default	
};

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム作成
 */
//------------------------------------------------------------------
G3D_MAPPER*	Create3Dmapper( HEAPID heapID )
{
	G3D_MAPPER* g3Dmapper = GFL_HEAP_AllocClearMemory( heapID, sizeof(G3D_MAPPER) );
	int i;

	g3Dmapper->heapID = heapID;

	{
		GFL_G3D_MAP_SETUP setup;

		setup.mapDataHeapSize = MAPMDL_SIZE + MAPTEX_SIZE + MAPATTR_SIZE;
		setup.texVramSize = MAPTEX_SIZE;
		setup.mapFileFunc = mapFileFuncTbl;
		setup.exWork			= NULL;

		//ブロック制御ハンドル作成
		for( i=0; i<MAP_BLOCK_COUNT; i++ ){
			g3Dmapper->g3Dmap[i] = GFL_G3D_MAP_Create( &setup, g3Dmapper->heapID );
		}
	}

	VEC_Set( &g3Dmapper->posCont, 0, 0, 0 );
	g3Dmapper->sizex = 0;
	g3Dmapper->sizez = 0;
	g3Dmapper->totalSize = 0;
	g3Dmapper->width = 0;
	g3Dmapper->height = 0;
	g3Dmapper->mode = G3D_MAPPER_MODE_SCROLL_XZ;
	g3Dmapper->arcID = MAPARC_NULL;
	g3Dmapper->data = NULL;

	g3Dmapper->globalObjRes = NULL;
	g3Dmapper->globalObjResCount = 0;

	g3Dmapper->globalObj.gobj = NULL;
	g3Dmapper->globalObj.gobjCount = 0;
	g3Dmapper->globalObj.gobjIDexchange = NULL;

	g3Dmapper->globalObj.gddobj = NULL;
	g3Dmapper->globalObj.gddobjCount = 0;
	g3Dmapper->globalObj.gddobjIDexchange = NULL;
	
	return g3Dmapper;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム破棄
 */
//------------------------------------------------------------------
void	Delete3Dmapper( G3D_MAPPER* g3Dmapper )
{
	int i;

	GF_ASSERT( g3Dmapper );

	ReleaseData3Dmapper( g3Dmapper );	//登録されたままの場合を想定して削除

	//ブロック制御ハンドル削除
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		GFL_G3D_MAP_Delete( g3Dmapper->g3Dmap[i] );
	}
	GFL_HEAP_FreeMemory( g3Dmapper );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムメイン
 */
//------------------------------------------------------------------
void	Main3Dmapper( G3D_MAPPER* g3Dmapper )
{
	BLOCK_IDX nowBlockIdx[MAP_BLOCK_COUNT];
	int i;

	GF_ASSERT( g3Dmapper );

	if( g3Dmapper->data == NULL ){
		return;
	}
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		nowBlockIdx[i].blockIdx = MAPID_NULL;
	}

	switch( g3Dmapper->mode ){
	case G3D_MAPPER_MODE_SCROLL_NONE: 
		GetMapperBlockIdxAll( g3Dmapper, &g3Dmapper->posCont, &nowBlockIdx[0] );
		break;
	default:
	case G3D_MAPPER_MODE_SCROLL_XZ: 
		GetMapperBlockIdxXZ( g3Dmapper, &g3Dmapper->posCont, &nowBlockIdx[0] );
		break;
	case G3D_MAPPER_MODE_SCROLL_Y: 
		GetMapperBlockIdxY( g3Dmapper, &g3Dmapper->posCont, &nowBlockIdx[0] );
		break;
	}
	ReloadMapperBlock( g3Dmapper, &nowBlockIdx[0] );

	//ブロック制御メイン
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		GFL_G3D_MAP_Main( g3Dmapper->g3Dmap[i] );
	}

	//現在ブロックのindex取得
	{
		u32 blockx = FX_Whole( FX_Div( g3Dmapper->posCont.x, g3Dmapper->width ) );
		u32 blockz = FX_Whole( FX_Div( g3Dmapper->posCont.z, g3Dmapper->width ) );

		g3Dmapper->nowBlockIdx = blockz * g3Dmapper->sizex + blockx;
	}
	//アニメーションコントロール（暫定でフレームループさせているだけ）
	{
		GLOBALOBJ_RES* objRes;
		int j;

		for( i=0; i<g3Dmapper->globalObjResCount; i++ ){
			objRes = &g3Dmapper->globalObjRes[i];

			if( objRes->g3Dobj != NULL ){
#if 0
				for( j=0; j<GLOBAL_OBJ_ANMCOUNT; j++ ){
					GFL_G3D_OBJECT_LoopAnimeFrame( objRes->g3Dobj, j, FX32_ONE ); 
				}
#else
				GFL_G3D_OBJECT_LoopAnimeFrame( objRes->g3Dobj, 0, FX32_ONE ); 
#endif
			}
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムディスプレイ
 */
//------------------------------------------------------------------
void	Draw3Dmapper( G3D_MAPPER* g3Dmapper, GFL_G3D_CAMERA* g3Dcamera )
{
	int i;

	GF_ASSERT( g3Dmapper );

	GFL_G3D_MAP_StartDraw();

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		GFL_G3D_MAP_Draw( g3Dmapper->g3Dmap[i], g3Dcamera );
	}
	GFL_G3D_MAP_EndDraw();
}

//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
void ResistData3Dmapper( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST* resistData )
{
	int i;

	GF_ASSERT( g3Dmapper );

	g3Dmapper->sizex = resistData->sizex;
	g3Dmapper->sizez = resistData->sizez;
	g3Dmapper->totalSize = resistData->totalSize;
	g3Dmapper->width = resistData->width;
	g3Dmapper->height = resistData->height;
	g3Dmapper->mode = resistData->mode;
	switch( g3Dmapper->mode ){
	case G3D_MAPPER_MODE_SCROLL_NONE:
		if( g3Dmapper->totalSize > MAP_BLOCK_COUNT ){
			OS_Printf("mapper mode set Error\n");
			g3Dmapper->mode = G3D_MAPPER_MODE_SCROLL_XZ;
		}
		break;
	case G3D_MAPPER_MODE_SCROLL_Y:
		if( g3Dmapper->sizex * g3Dmapper->sizez > MAP_BLOCK_COUNT/2 ){
			OS_Printf("mapper mode set Error\n");
			g3Dmapper->mode = G3D_MAPPER_MODE_SCROLL_XZ;
		}
		break;
	}
	g3Dmapper->arcID = resistData->arcID;
	g3Dmapper->data = resistData->data;

	//グローバルテクスチャ作成
	CreateGlobalTexture( g3Dmapper, resistData );
	//グローバルオブジェクト作成
	CreateGlobalObject( g3Dmapper, resistData );

	//マップブロック制御設定
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		//新アーカイブＩＤを登録
		GFL_G3D_MAP_ResistArc( g3Dmapper->g3Dmap[i], g3Dmapper->arcID, g3Dmapper->heapID );

		//グローバルリソース登録
		if( g3Dmapper->globalTexture != NULL ){
			GFL_G3D_MAP_ResistGlobalTexResource( g3Dmapper->g3Dmap[i], g3Dmapper->globalTexture );
		} else {
			GFL_G3D_MAP_ReleaseGlobalTexResource( g3Dmapper->g3Dmap[i] );
		}
		GFL_G3D_MAP_ResistGlobalObjResource( g3Dmapper->g3Dmap[i], &g3Dmapper->globalObj );

		//ファイル識別設定（仮）
		GFL_G3D_MAP_ResistFileType( g3Dmapper->g3Dmap[i], resistData->g3DmapFileType );
	}
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		g3Dmapper->blockIdx[i].blockIdx = MAPID_NULL;
		VEC_Set( &g3Dmapper->blockIdx[i].trans, 0, 0, 0 );
	}
}

void ReleaseData3Dmapper( G3D_MAPPER* g3Dmapper )
{
	int i;

	GF_ASSERT( g3Dmapper );

	//マップブロック制御解除
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		GFL_G3D_MAP_ReleaseGlobalObjResource( g3Dmapper->g3Dmap[i] );
		GFL_G3D_MAP_ReleaseGlobalTexResource( g3Dmapper->g3Dmap[i] );
		GFL_G3D_MAP_ReleaseArc( g3Dmapper->g3Dmap[i] );
	}
	DeleteGlobalObject( g3Dmapper );
	DeleteGlobalTexture( g3Dmapper );
}

//------------------------------------------------------------------
/**
 * @brief	グローバルテクスチャ作成
 */
//------------------------------------------------------------------
static void CreateGlobalTexture( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST* resistData )
{
	if( resistData->gtexType != NON_GLOBAL_TEX ){
		G3D_MAPPER_GLOBAL_TEXTURE* gtexData = (G3D_MAPPER_GLOBAL_TEXTURE*)resistData->gtexData;

		g3Dmapper->globalTexture = GFL_G3D_CreateResourceArc( gtexData->arcID, gtexData->datID );
		GFL_G3D_TransVramTexture( g3Dmapper->globalTexture );
	} else {
		g3Dmapper->globalTexture = NULL;
	}
}

static void DeleteGlobalTexture( G3D_MAPPER* g3Dmapper )
{
	if( g3Dmapper->globalTexture != NULL ){
		GFL_G3D_FreeVramTexture( g3Dmapper->globalTexture );
		GFL_G3D_DeleteResource( g3Dmapper->globalTexture );
		g3Dmapper->globalTexture = NULL;
	}
}

//------------------------------------------------------------------
/**
 * @brief	グローバルオブジェクト作成
 */
//------------------------------------------------------------------
static void CreateGlobalObject( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST* resistData )
{
	int i;

	if( (resistData->gobjType != NON_GLOBAL_OBJ)&&(resistData->gobjData != NULL) ){

		if( resistData->gobjType != USE_GLOBAL_OBJSET_BIN ){
			CreateGrobalObj_forTbl( g3Dmapper, resistData->gobjData );
		} else {
			CreateGrobalObj_forBin( g3Dmapper, resistData->gobjData );
		}
	}
}

static void DeleteGlobalObject( G3D_MAPPER* g3Dmapper )
{
	if( g3Dmapper->globalObj.gddobj != NULL ){
		DeleteGlobalDDobj( g3Dmapper );
		GFL_HEAP_FreeMemory( g3Dmapper->globalObj.gddobj );
		g3Dmapper->globalObj.gddobj = NULL;
	}
	if( g3Dmapper->globalObj.gddobjIDexchange != NULL ){
		GFL_HEAP_FreeMemory( g3Dmapper->globalObj.gddobjIDexchange );
		g3Dmapper->globalObj.gddobjIDexchange = NULL;
	}
	if( g3Dmapper->globalObj.gobj != NULL ){
		GFL_HEAP_FreeMemory( g3Dmapper->globalObj.gobj );
		g3Dmapper->globalObj.gobj = NULL;
	}
	if( g3Dmapper->globalObj.gobjIDexchange != NULL ){
		GFL_HEAP_FreeMemory( g3Dmapper->globalObj.gobjIDexchange );
		g3Dmapper->globalObj.gobjIDexchange = NULL;
	}

	if( g3Dmapper->globalObjRes != NULL ){
		int i;

		for( i=0; i<g3Dmapper->globalObjResCount; i++ ){
			DeleteGlobalObj( &g3Dmapper->globalObjRes[i] );
		}
		GFL_HEAP_FreeMemory( g3Dmapper->globalObjRes );
		g3Dmapper->globalObjRes = NULL;
		g3Dmapper->globalObjResCount = 0;
	}
}

//------------------------------------------------------------------
// オブジェクトリソースを作成
//通常MDL
static void CreateGlobalObj( GLOBALOBJ_RES* objRes, MAKE_OBJ_PARAM* param )
{
	GFL_G3D_RND* g3Drnd;
	GFL_G3D_ANM* g3Danm;
	GFL_G3D_RES* resTex;
	GFL_G3D_RES* resAnm;
	GFL_G3D_ANM* anmTbl[GLOBAL_OBJ_ANMCOUNT];
	int i;

	objRes->g3DresMdl = GFL_G3D_CreateResourceArc( param->mdl.arcID, param->mdl.datID );

	if( param->tex.arcID != NON_PARAM ){
		objRes->g3DresTex = GFL_G3D_CreateResourceArc( param->tex.arcID, param->tex.datID );
		resTex = objRes->g3DresTex;
	} else {
		objRes->g3DresTex = NULL;
		resTex = objRes->g3DresMdl;
	}
	GFL_G3D_TransVramTexture( resTex );

	g3Drnd = GFL_G3D_RENDER_Create( objRes->g3DresMdl, 0, resTex );
	for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
		if( param->anm[i].arcID != NON_PARAM ){
			objRes->g3DresAnm[i] = GFL_G3D_CreateResourceArc
										( param->anm[i].arcID, param->anm[i].datID );
			anmTbl[i] = GFL_G3D_ANIME_Create( g3Drnd, objRes->g3DresAnm[i], 0 );  
		} else {
			anmTbl[i] = NULL;
		}
	}
	objRes->g3Dobj = GFL_G3D_OBJECT_Create( g3Drnd, anmTbl, GLOBAL_OBJ_ANMCOUNT );
}

static void DeleteGlobalObj( GLOBALOBJ_RES* objRes )
{
	GFL_G3D_RND*	g3Drnd;
	GFL_G3D_OBJ*	g3Dobj;
	GFL_G3D_RES*	resTex;
	GFL_G3D_ANM*	g3Danm[GLOBAL_OBJ_ANMCOUNT] = { NULL, NULL, NULL, NULL };
	u16				g3DanmCount;
	int i;

	if( objRes->g3Dobj != NULL ){
		//各種ハンドル取得
		g3DanmCount = GFL_G3D_OBJECT_GetAnimeCount( objRes->g3Dobj );
		for( i=0; i<g3DanmCount; i++ ){
			g3Danm[i] = GFL_G3D_OBJECT_GetG3Danm( objRes->g3Dobj, i ); 
		}
		g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( objRes->g3Dobj );
	
		//各種ハンドル＆リソース削除
		GFL_G3D_OBJECT_Delete( objRes->g3Dobj );
		objRes->g3Dobj = NULL;

		for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
			if( g3Danm[i] != NULL ){ 
				GFL_G3D_ANIME_Delete( g3Danm[i] ); 
			}
			if( objRes->g3DresAnm[i] != NULL ){
				GFL_G3D_DeleteResource( objRes->g3DresAnm[i] );
				objRes->g3DresAnm[i] = NULL;
			}
		}
		GFL_G3D_RENDER_Delete( g3Drnd );
		if( objRes->g3DresTex == NULL ){
			resTex = objRes->g3DresMdl;
		} else {
			resTex = objRes->g3DresTex;
			GFL_G3D_DeleteResource( objRes->g3DresTex );
			objRes->g3DresTex = NULL;
		}
		GFL_G3D_FreeVramTexture( resTex );

		if( objRes->g3DresMdl != NULL ){
			GFL_G3D_DeleteResource( objRes->g3DresMdl );
			objRes->g3DresMdl = NULL;
		}
	}
}

//------------------------------------------------------------------
//DirectDraw
static void CreateGlobalDDobj( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST_DDOBJ* resistData )
{
	GFL_G3D_RES*	g3DresTex;
	NNSG3dTexKey	texDataKey;
	NNSG3dPlttKey	texPlttKey;
	int i;

	GF_ASSERT( resistData->count <= GLOBAL_DDOBJ_COUNT );
	
	for( i=0; i< resistData->count; i++ ){
		g3DresTex = GFL_G3D_CreateResourceArc( resistData->arcID, resistData->data[i] );

		g3Dmapper->globalObj.gddobj[i].g3Dres = g3DresTex;
										
		if( GFL_G3D_TransVramTexture( g3DresTex ) == FALSE ){
			GF_ASSERT(0);
		}
		texDataKey = GFL_G3D_GetTextureDataVramKey( g3DresTex );
		texPlttKey = GFL_G3D_GetTexturePlttVramKey( g3DresTex );

		g3Dmapper->globalObj.gddobj[i].texDataAdrs = NNS_GfdGetTexKeyAddr( texDataKey );
		g3Dmapper->globalObj.gddobj[i].texPlttAdrs = NNS_GfdGetPlttKeyAddr( texPlttKey );
		g3Dmapper->globalObj.gddobj[i].data = &drawTreeData;
	}
	g3Dmapper->globalObj.gddobjCount = resistData->count;
}

static void DeleteGlobalDDobj( G3D_MAPPER* g3Dmapper )
{
	int i;

	for( i=0; i<g3Dmapper->globalObj.gddobjCount; i++ ){
		if( g3Dmapper->globalObj.gddobj[i].g3Dres != NULL ){
			GFL_G3D_FreeVramTexture( g3Dmapper->globalObj.gddobj[i].g3Dres );
			GFL_G3D_DeleteResource( g3Dmapper->globalObj.gddobj[i].g3Dres );
			g3Dmapper->globalObj.gddobj[i].g3Dres = NULL;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	マップ位置セット
 */
//------------------------------------------------------------------
void SetPos3Dmapper( G3D_MAPPER* g3Dmapper, const VecFx32* pos )
{
	GF_ASSERT( g3Dmapper );

	VEC_Set( &g3Dmapper->posCont, pos->x, pos->y, pos->z );
}

//------------------------------------------------------------------
/**
 * @brief	マップ更新ブロック取得
 */
//------------------------------------------------------------------
static void GetMapperBlockIdxAll( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx )
{
	u32		idx, county, countxz;
	fx32	width, height;
	int		i, j, offsx, offsz;

	countxz = g3Dmapper->sizex * g3Dmapper->sizez;
	county = g3Dmapper->totalSize / countxz;
	if( g3Dmapper->totalSize % countxz ){
		county++;
	}
	width = g3Dmapper->width;
	height = g3Dmapper->height;

	idx = 0;

	for( i=0; i<county; i++ ){
		for( j=0; j<countxz; j++ ){
			offsx = j % g3Dmapper->sizex;
			offsz = j / g3Dmapper->sizex;

			if( idx >= g3Dmapper->totalSize ){
				idx = MAPID_NULL;
			}
			blockIdx[idx].blockIdx = idx;
			blockIdx[idx].trans.x = offsx * width + width/2;
			blockIdx[idx].trans.y = i * height;
			blockIdx[idx].trans.z = offsz * width + width/2;
			idx++;
		}
	}
}

//------------------------------------------------------------------
typedef struct {
	s16	z;
	s16	x;
}BLOCK_OFFS;

static const BLOCK_OFFS blockPat_Around[] = {//自分のいるブロックから周囲方向に９ブロックとる
	{-1,-1},{-1, 0},{-1, 1},{ 0,-1},{ 0, 0},{ 0, 1},{ 1,-1},{ 1, 0},{ 1, 1},
};

static void GetMapperBlockIdxXZ( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx )
{
	u16		sizex, sizez;
	u32		idx, idxmax, blockx, blockz;
	fx32	inBlockx, inBlockz, width;
	int		i, offsx, offsz;

	sizex = g3Dmapper->sizex;
	sizez = g3Dmapper->sizez;
	idxmax = sizex * sizez;
	width = g3Dmapper->width;

	blockx = FX_Whole( FX_Div( pos->x, width ) );
	blockz = FX_Whole( FX_Div( pos->z, width ) );

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		offsx = blockx + blockPat_Around[i].x;
		offsz = blockz + blockPat_Around[i].z;

		if((offsx < 0)||(offsx >= sizex)||(offsz < 0)||(offsz >= sizez)){
			idx = MAPID_NULL;
		} else {
			idx = offsz * sizex + offsx;

			if( idx >= idxmax ){
				idx = MAPID_NULL;
			}
		}
		blockIdx[i].blockIdx = idx;
		blockIdx[i].trans.x = offsx * width + width/2;
		blockIdx[i].trans.y = 0;
		blockIdx[i].trans.z = offsz * width + width/2;
	}
}

//------------------------------------------------------------------
static void GetMapperBlockIdxY( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx )
{
	u16		sizex, sizez;
	u32		idx, blocky, countxz;
	fx32	width, height;
	int		i, p, offsx, offsy, offsz;

	sizex = g3Dmapper->sizex;
	sizez = g3Dmapper->sizez;
	countxz = sizex * sizez;
	width = g3Dmapper->width;
	height = g3Dmapper->height;

	blocky = FX_Whole( FX_Div( pos->y, height ) );
	if( pos->y - ( blocky * height ) > height/2 ){
		offsy = 1;
	} else {
		offsy = -1;
	}
	p = 0;

	for( i=0; i<countxz; i++ ){
		offsx = i % g3Dmapper->sizex;
		offsz = i / g3Dmapper->sizex;

		idx = blocky * countxz + offsz * sizex + offsx;

		if( idx >= g3Dmapper->totalSize ){
			idx = MAPID_NULL;
		}
		blockIdx[p].blockIdx = idx;
		blockIdx[p].trans.x = offsx * width + width/2;
		blockIdx[p].trans.y = blocky * height;
		blockIdx[p].trans.z = offsz * width + width/2;
		p++;
	}
	for( i=0; i<countxz; i++ ){
		offsx = i % g3Dmapper->sizex;
		offsz = i / g3Dmapper->sizex;

		idx = (blocky + offsy) * countxz + offsz * sizex + offsx;

		if( idx >= g3Dmapper->totalSize ){
			idx = MAPID_NULL;
		}
		blockIdx[p].blockIdx = idx;
		blockIdx[p].trans.x = offsx * width + width/2;
		blockIdx[p].trans.y = (blocky + offsy) * height;
		blockIdx[p].trans.z = offsz * width + width/2;
		p++;
	}
}

//------------------------------------------------------------------
/**
 * @brief	マップブロック更新チェック
 */
//------------------------------------------------------------------
static BOOL	ReloadMapperBlock( G3D_MAPPER* g3Dmapper, BLOCK_IDX* new )
{
	BOOL addFlag, delFlag, delProcFlag, addProcFlag, reloadFlag;
	int i, j, c;

	reloadFlag = FALSE;

	//更新ブロックチェック
	delProcFlag = FALSE;
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		if( g3Dmapper->blockIdx[i].blockIdx != MAPID_NULL ){
			delFlag = FALSE;
			for( j=0; j<MAP_BLOCK_COUNT; j++ ){
				if(( g3Dmapper->blockIdx[i].blockIdx == new[j].blockIdx )&&(delFlag == FALSE )){
					new[j].blockIdx = MAPID_NULL;
					delFlag = TRUE;
				}
			}
			if( delFlag == FALSE ){
				g3Dmapper->blockIdx[i].blockIdx = MAPID_NULL;
				GFL_G3D_MAP_SetDrawSw( g3Dmapper->g3Dmap[i], FALSE );
				delProcFlag = TRUE;
			}
		}
	}
	//更新
	addProcFlag = FALSE;
	c = 0;
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		if( new[i].blockIdx != MAPID_NULL ){
			addFlag = FALSE;
			for( j=0; j<MAP_BLOCK_COUNT; j++ ){
				if(( g3Dmapper->blockIdx[j].blockIdx == MAPID_NULL )&&(addFlag == FALSE )){
					u32 mapdatID = g3Dmapper->data[new[i].blockIdx].datID;

					if( mapdatID != G3D_MAPPER_NOMAP ){
						GFL_G3D_MAP_SetLoadReq( g3Dmapper->g3Dmap[j], mapdatID );
						GFL_G3D_MAP_SetTrans( g3Dmapper->g3Dmap[j], &new[i].trans );
						GFL_G3D_MAP_SetDrawSw( g3Dmapper->g3Dmap[j], TRUE );
					}
					g3Dmapper->blockIdx[j] = new[i];
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


//============================================================================================
/**
 *
 *
 *
 * @brief	３Ｄマップ情報取得
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ワーク初期化
 */
//------------------------------------------------------------------
void InitGet3DmapperGridInfoData( G3D_MAPPER_INFODATA* gridInfoData )
{
	VEC_Fx16Set( &gridInfoData->vecN, 0, 0, 0 );
	gridInfoData->attr = 0;
	gridInfoData->height = 0;
}

void InitGet3DmapperGridInfo( G3D_MAPPER_GRIDINFO* gridInfo )
{
	int i;

	for( i=0; i<G3D_MAPPER_ATTR_MAX; i++ ){
		InitGet3DmapperGridInfoData( &gridInfo->gridData[i] );
	}
	gridInfo->count = 0;
}

//------------------------------------------------------------------
/**
 * @brief	アトリビュート情報取得
 */
//------------------------------------------------------------------
BOOL Get3DmapperGridInfo
	( G3D_MAPPER* g3Dmapper, const VecFx32* pos, G3D_MAPPER_GRIDINFO* gridInfo )
{
	GFL_G3D_MAP_ATTRINFO attrInfo;
	u32 infoCount;
	VecFx32 trans;
	int		i, p;

	GF_ASSERT( g3Dmapper );
	if( g3Dmapper->data == NULL ){
		return FALSE;
	}

	InitGet3DmapperGridInfo( gridInfo );
	
	p = 0;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		if( GFL_G3D_MAP_GetDrawSw( g3Dmapper->g3Dmap[i] ) == TRUE ){
			fx32 min_x, min_z, max_x, max_z;

			GFL_G3D_MAP_GetTrans( g3Dmapper->g3Dmap[i], &trans );

			min_x = trans.x - g3Dmapper->width/2;
			min_z = trans.z - g3Dmapper->width/2;
			max_x = trans.x + g3Dmapper->width/2;
			max_z = trans.z + g3Dmapper->width/2;

			//ブロック範囲内チェック（マップブロックのＸＺ平面上地点）
			if(	(pos->x >= min_x)&&(pos->x < max_x)&&(pos->z >= min_z)&&(pos->z < max_z) ){

				GFL_G3D_MAP_GetAttr( &attrInfo, g3Dmapper->g3Dmap[i], pos, g3Dmapper->width );
				if( attrInfo.mapAttrCount ){
					int j;

					if( (p + attrInfo.mapAttrCount) >= G3D_MAPPER_ATTR_MAX ){
						GF_ASSERT("height count over\n");
					}
					for( j=0; j<attrInfo.mapAttrCount; j++ ){
						gridInfo->gridData[p].vecN = attrInfo.mapAttr[j].vecN;
						gridInfo->gridData[p].attr = attrInfo.mapAttr[j].attr;
						gridInfo->gridData[p].height = attrInfo.mapAttr[j].height;
					}
					p += attrInfo.mapAttrCount;
				}
			}
		}
	}
	gridInfo->count = p;
	if( gridInfo->count ){
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	範囲外チェック
 */
//------------------------------------------------------------------
BOOL Check3DmapperOutRange( G3D_MAPPER* g3Dmapper, const VecFx32* pos )
{
	fx32 widthx, widthz;

	GF_ASSERT( g3Dmapper );
	if( g3Dmapper->data == NULL ){
		return TRUE;
	}

	widthx = g3Dmapper->sizex * g3Dmapper->width;
	widthz = g3Dmapper->sizez * g3Dmapper->width;

	if( ( pos->x >= 0 )&&( pos->x < widthx )&&( pos->z >= 0 )&&( pos->z < widthz ) ){
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	サイズ取得
 */
//------------------------------------------------------------------
void Get3DmapperSize( G3D_MAPPER* g3Dmapper, fx32* x, fx32* z )
{
	GF_ASSERT( g3Dmapper );
	if( g3Dmapper->data == NULL ){
		*x = 0;
		*z = 0;
		return;
	}
	*x = g3Dmapper->sizex * g3Dmapper->width;
	*z = g3Dmapper->sizez * g3Dmapper->width;
	return;
}


//============================================================================================
/**
 *
 *
 *
 * @brief	ジオメトリ直書き関数
 *
 *
 *
 */
//============================================================================================
static void	_drawTree0( u32 texDataAdrs, u32 texPlttAdrs, VecFx16* vecView, BOOL lodSt )
{
	GXTexFmt	texFmt = GX_TEXFMT_PLTT16;
	GXTexSizeS	s;
	GXTexSizeT	t;
	fx32		smax, tmax;

	if( lodSt == TRUE ){
		s = GX_TEXSIZE_S64;
		t = GX_TEXSIZE_T64;
		smax = 64 * FX32_ONE;
		tmax = 64 * FX32_ONE;

		G3_TexImageParam(	texFmt, GX_TEXGEN_TEXCOORD, s, t, GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
						GX_TEXPLTTCOLOR0_TRNS, texDataAdrs + 0x200 );
		G3_TexPlttBase( texPlttAdrs + 0x20, texFmt );

		G3_Begin( GX_BEGIN_QUADS );

		G3_Normal( 0.000 * FX16_ONE, 1.000 * FX16_ONE, 0.000 * FX16_ONE );
		G3_Color( GX_RGB( 31, 31, 31 ) );
		G3_TexCoord( FX_Mul(0.531 * FX32_ONE,smax), FX_Mul(0.000 * FX32_ONE,tmax) );
		G3_Vtx( -1.600 * FX16_ONE, 0.200/*0.091*/ * FX16_ONE, -1.500 * FX16_ONE );	//vtx0
		G3_TexCoord( FX_Mul(0.531 * FX32_ONE,smax), FX_Mul(0.437 * FX32_ONE,tmax) );
		G3_Vtx( -1.600 * FX16_ONE, 0.200/*0.091*/ * FX16_ONE, 1.500 * FX16_ONE );	//vtx2
		G3_TexCoord( FX_Mul(1.000 * FX32_ONE,smax), FX_Mul(0.437 * FX32_ONE,tmax) );
		G3_Vtx( 1.600 * FX16_ONE, 0.200/*0.091*/ * FX16_ONE, 1.500 * FX16_ONE );	//vtx3
		G3_TexCoord( FX_Mul(1.000 * FX32_ONE,smax), FX_Mul(0.000 * FX32_ONE,tmax) );
		G3_Vtx( 1.600 * FX16_ONE, 0.200/*0.091*/ * FX16_ONE, -1.500 * FX16_ONE );	//vtx1

		G3_Normal( 0.000 * FX16_ONE, 0.819 * FX16_ONE, 0.574 * FX16_ONE );
		G3_Color( GX_RGB( 31, 31, 31 ) );
		G3_TexCoord( FX_Mul(0.344 * FX32_ONE,smax), FX_Mul(0.437 * FX32_ONE,tmax) );
		G3_Vtx( -2.000 * FX16_ONE, 2.675 * FX16_ONE, -1.793 * FX16_ONE );	//vtx0
		G3_TexCoord( FX_Mul(0.344 * FX32_ONE,smax), FX_Mul(1.000 * FX32_ONE,tmax) );
		G3_Vtx( -2.000 * FX16_ONE, 0.725 * FX16_ONE, 0.993 * FX16_ONE );	//vtx2
		G3_TexCoord( FX_Mul(1.000 * FX32_ONE,smax), FX_Mul(1.000 * FX32_ONE,tmax) );
		G3_Vtx( 2.000 * FX16_ONE, 0.725 * FX16_ONE, 0.993 * FX16_ONE );		//vtx3
		G3_TexCoord( FX_Mul(1.000 * FX32_ONE,smax), FX_Mul(0.437 * FX32_ONE,tmax) );
		G3_Vtx( 2.000 * FX16_ONE, 2.675 * FX16_ONE, -1.793 * FX16_ONE );	//vtx1

		G3_Normal( 0.000 * FX16_ONE, 0.819 * FX16_ONE, 0.574 * FX16_ONE );
		G3_Color( GX_RGB( 31, 31, 31 ) );
		G3_TexCoord( FX_Mul(0.000 * FX32_ONE,smax), FX_Mul(0.000 * FX32_ONE,tmax) );
		G3_Vtx( -1.650 * FX16_ONE, 3.560 * FX16_ONE, -1.729 * FX16_ONE );	//vtx0
		G3_TexCoord( FX_Mul(0.000 * FX32_ONE,smax), FX_Mul(0.469 * FX32_ONE,tmax) );
		G3_Vtx( -1.650 * FX16_ONE, 1.840 * FX16_ONE, 0.729 * FX16_ONE );	//vtx2
		G3_TexCoord( FX_Mul(0.531 * FX32_ONE,smax), FX_Mul(0.469 * FX32_ONE,tmax) );
		G3_Vtx( 1.650 * FX16_ONE, 1.840 * FX16_ONE, 0.729 * FX16_ONE );		//vtx3
		G3_TexCoord( FX_Mul(0.531 * FX32_ONE,smax), FX_Mul(0.000 * FX32_ONE,tmax) );
		G3_Vtx( 1.650 * FX16_ONE, 3.560 * FX16_ONE, -1.729 * FX16_ONE );	//vtx1

		G3_Normal( 0.000 * FX16_ONE, 0.819 * FX16_ONE, 0.574 * FX16_ONE );
		G3_Color( GX_RGB( 31, 31, 31 ) );
		G3_TexCoord( FX_Mul(0.000 * FX32_ONE,smax), FX_Mul(0.656 * FX32_ONE,tmax) );
		G3_Vtx( -1.100 * FX16_ONE, 4.231 * FX16_ONE, -1.601 * FX16_ONE );	//vtx0
		G3_TexCoord( FX_Mul(0.000 * FX32_ONE,smax), FX_Mul(1.000 * FX32_ONE,tmax) );
		G3_Vtx( -1.100 * FX16_ONE, 2.969 * FX16_ONE, 0.201 * FX16_ONE );	//vtx2
		G3_TexCoord( FX_Mul(0.344 * FX32_ONE,smax), FX_Mul(1.000 * FX32_ONE,tmax) );
		G3_Vtx( 1.100 * FX16_ONE, 2.969 * FX16_ONE, 0.201 * FX16_ONE );		//vtx3
		G3_TexCoord( FX_Mul(0.344 * FX32_ONE,smax), FX_Mul(0.656 * FX32_ONE,tmax) );
		G3_Vtx( 1.100 * FX16_ONE, 4.231 * FX16_ONE, -1.601 * FX16_ONE );	//vtx1

		G3_End();
	} else {
		s = GX_TEXSIZE_S32;
		t = GX_TEXSIZE_T32;
		smax = 32 * FX32_ONE;
		tmax = 32 * FX32_ONE;

		G3_TexImageParam(	texFmt, GX_TEXGEN_TEXCOORD, s, t, GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
						GX_TEXPLTTCOLOR0_TRNS, texDataAdrs + 0x000 );
		G3_TexPlttBase( texPlttAdrs + 0x00, texFmt );

		G3_Begin( GX_BEGIN_QUADS );

		G3_Normal( 0.000 * FX16_ONE, 1.000 * FX16_ONE, 0.000 * FX16_ONE );
		G3_Color( GX_RGB( 31, 31, 31 ) );
		G3_TexCoord( FX_Mul(0.000 * FX32_ONE,smax), FX_Mul(0.000 * FX32_ONE,tmax) );
		G3_Vtx( -2.000 * FX16_ONE, 4.231 * FX16_ONE, 0 * FX16_ONE );	//vtx0
		G3_TexCoord( FX_Mul(0.000 * FX32_ONE,smax), FX_Mul(1.000 * FX32_ONE,tmax) );
		G3_Vtx( -2.000 * FX16_ONE, 0.000 * FX16_ONE, 0 * FX16_ONE );	//vtx2
		G3_TexCoord( FX_Mul(1.000 * FX32_ONE,smax), FX_Mul(1.000 * FX32_ONE,tmax) );
		G3_Vtx( 2.000 * FX16_ONE, 0.000 * FX16_ONE, 0 * FX16_ONE );	//vtx3
		G3_TexCoord( FX_Mul(1.000 * FX32_ONE,smax), FX_Mul(0.000 * FX32_ONE,tmax) );
		G3_Vtx( 2.000 * FX16_ONE, 4.231 * FX16_ONE, 0 * FX16_ONE );	//vtx1

		G3_End();
	}
}

static const GFL_G3D_MAP_DDOBJ_DATA drawTreeData = {
	GFL_G3D_MAP_LIGHT_NONE,
	GX_RGB(31,31,31), GX_RGB(16,16,16), GX_RGB(16,16,16), GX_RGB(0,0,0),
	63, 31, GFL_G3D_MAP_DRAW_YBILLBOARD, 8,
	_drawTree0,
};


//============================================================================================
/**
 *
 *
 *
 * @brief	３Ｄグローバルオブジェクト読み込み
 *
 *
 *
 */
//============================================================================================
//テーブルデータより作成
static void CreateGrobalObj_forTbl( G3D_MAPPER* g3Dmapper, const void* resistData )
{
	G3D_MAPPER_RESIST_OBJ	resistObj;
	G3D_MAPPER_RESIST_DDOBJ resistDDobj;
	G3D_MAPPER_GLOBAL_OBJSET_TBL* gobjTbl;
	int i, p = 0;

	gobjTbl = (G3D_MAPPER_GLOBAL_OBJSET_TBL*)resistData;
             
	if( gobjTbl->objCount ){
		MAKE_OBJ_PARAM objParam;

		objParam.tex.arcID = NON_PARAM;
		objParam.tex.datID = NON_PARAM;
		objParam.tex.inDatNum = 0;
		for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
			objParam.anm[i].arcID = NON_PARAM;
			objParam.anm[i].datID = NON_PARAM;
			objParam.anm[i].inDatNum = 0;
		}
		g3Dmapper->globalObjRes = GFL_HEAP_AllocClearMemory//HQ,LQ の２つ分確保
						( g3Dmapper->heapID, sizeof(GLOBALOBJ_RES) * (gobjTbl->objCount*2) );
		g3Dmapper->globalObj.gobj = GFL_HEAP_AllocClearMemory
						( g3Dmapper->heapID, sizeof(GFL_G3D_MAP_OBJ) * gobjTbl->objCount );

		for( i=0, p=0; i<gobjTbl->objCount; i++ ){
			objParam.mdl.arcID = gobjTbl->objArcID;
			objParam.mdl.inDatNum = 0;

			objParam.mdl.datID = gobjTbl->objData[i].highQ_ID;
			CreateGlobalObj( &g3Dmapper->globalObjRes[p], &objParam );
			g3Dmapper->globalObj.gobj[i].g3DobjHQ = g3Dmapper->globalObjRes[p].g3Dobj;
			p++;

			if( gobjTbl->objData[i].lowQ_ID != NON_LOWQ ){
				objParam.mdl.datID = gobjTbl->objData[i].lowQ_ID;
				CreateGlobalObj( &g3Dmapper->globalObjRes[p], &objParam );
				g3Dmapper->globalObj.gobj[i].g3DobjLQ = g3Dmapper->globalObjRes[p].g3Dobj;
				p++;
			} else {
				g3Dmapper->globalObj.gobj[i].g3DobjLQ = NULL;
			}
		}
		g3Dmapper->globalObj.gobjCount = gobjTbl->objCount;
	}
	g3Dmapper->globalObjResCount = p;

	if( gobjTbl->ddobjCount ){
		g3Dmapper->globalObj.gddobj = GFL_HEAP_AllocClearMemory
					( g3Dmapper->heapID, sizeof(GFL_G3D_MAP_DDOBJ) * gobjTbl->ddobjCount );

		resistDDobj.arcID = gobjTbl->ddobjArcID;
		resistDDobj.data = gobjTbl->ddobjData;
		resistDDobj.count = gobjTbl->ddobjCount;

		CreateGlobalDDobj( g3Dmapper, &resistDDobj );
	}
}

//------------------------------------------------------------------
//バイナリデータより作成(ポケモンＧＳ方式)
typedef struct {
	u16			count;
	u16			data;
}GOBJ_BINDATA;

typedef struct {
	u8 Flg;			//アニメするかどうか
	u8 Type;		//アニメタイプ
	u8 Suicide;		//自殺フラグ
	u8 RepeatEntry;	//重複登録フラグ
	u8 Door;
	u8 Dummy;
	u8 AnmNum;
	u8 SetNum;
	int Code[4];	//アニメコード
}GOBJ_ANMTBL_HEADER;

static void CreateGrobalObj_forBin( G3D_MAPPER* g3Dmapper, const void* resistData )
{
	G3D_MAPPER_RESIST_OBJ			resistObj;
	G3D_MAPPER_RESIST_DDOBJ			resistDDobj;
	G3D_MAPPER_GLOBAL_OBJSET_BIN*	gobjBin;
	GOBJ_BINDATA*					gobjListHeader;
	u16*							gobjList;

	gobjBin = (G3D_MAPPER_GLOBAL_OBJSET_BIN*)resistData;

	gobjListHeader = GFL_ARC_LoadDataAlloc( gobjBin->areaObjArcID, 
											gobjBin->areaObjDatID,
											GetHeapLowID(g3Dmapper->heapID) );
	gobjList = (u16*)&gobjListHeader->data;

	g3Dmapper->globalObjResCount = gobjListHeader->count;

	if( gobjListHeader->count ){
		GOBJ_ANMTBL_HEADER	gobjAnmListHeader;
		MAKE_OBJ_PARAM objParam;
		int i, j;

		g3Dmapper->globalObjRes = GFL_HEAP_AllocClearMemory
				( g3Dmapper->heapID, sizeof(GLOBALOBJ_RES) * gobjListHeader->count );
		g3Dmapper->globalObj.gobj = GFL_HEAP_AllocClearMemory
				( g3Dmapper->heapID, sizeof(GFL_G3D_MAP_OBJ) * gobjListHeader->count );
		g3Dmapper->globalObj.gobjIDexchange = GFL_HEAP_AllocClearMemory
				( g3Dmapper->heapID, sizeof(u16) * gobjListHeader->count );

		//オブジェクトレンダー作成
		for( i=0; i<gobjListHeader->count; i++ ){
			GFL_ARC_LoadDataOfs( &gobjAnmListHeader, gobjBin->areaObjAnmTblArcID, 
									gobjList[i], 0, sizeof(GOBJ_ANMTBL_HEADER) );

			objParam.mdl.arcID = gobjBin->objArcID;
			objParam.mdl.datID = gobjList[i];
			objParam.mdl.inDatNum = 0;
			objParam.tex.arcID = NON_PARAM;
			objParam.tex.datID = NON_PARAM;
			objParam.tex.inDatNum = 0;
			for( j=0; j<GLOBAL_OBJ_ANMCOUNT; j++ ){
				if( j<gobjAnmListHeader.SetNum ){
					objParam.anm[j].arcID = gobjBin->objanmArcID;
					objParam.anm[j].datID = gobjAnmListHeader.Code[j];
					objParam.anm[j].inDatNum = gobjAnmListHeader.AnmNum;
				} else {
					objParam.anm[j].arcID = NON_PARAM;
					objParam.anm[j].datID = NON_PARAM;
					objParam.anm[j].inDatNum = 0;
				}
			}

			(g3Dmapper->globalObj.gobjIDexchange)[i] = gobjList[i];	//配置ＩＤ変換用

			CreateGlobalObj( &g3Dmapper->globalObjRes[i], &objParam );
			g3Dmapper->globalObj.gobj[i].g3DobjHQ = g3Dmapper->globalObjRes[i].g3Dobj;
			g3Dmapper->globalObj.gobj[i].g3DobjLQ = NULL;

			for( j=0; j<gobjAnmListHeader.SetNum; j++ ){
				GFL_G3D_OBJECT_EnableAnime( g3Dmapper->globalObjRes[i].g3Dobj, j ); 
				GFL_G3D_OBJECT_ResetAnimeFrame( g3Dmapper->globalObjRes[i].g3Dobj, j ); 
			}
		}
		g3Dmapper->globalObj.gobjCount = gobjListHeader->count;
	}
	GFL_HEAP_FreeMemory( gobjListHeader );
}





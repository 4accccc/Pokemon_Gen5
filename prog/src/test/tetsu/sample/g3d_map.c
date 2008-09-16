//============================================================================================
/**
 * @file	g3d_map.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "g3d_map.h"
//============================================================================================
/**
 *
 *
 *
 * @brief	３Ｄマップデータコントロール
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
#define OBJID_NULL			(0xffffffff)

#define OBJ_COUNT	(32)
#define DDOBJ_COUNT	(64)

//------------------------------------------------------------------
struct _GFL_G3D_MAP 
{
	BOOL							drawSw;
	VecFx32							trans;
	ARCHANDLE*						arc;
	u32								datID;

	GFL_G3D_MAPDATA_FILETYPE		fileType;

	u32								mapDataHeapSize;
	u32								texVramSize;
	const MAPFILE_FUNC*				mapFileFunc;

	NNSG3dRenderObj*				NNSrnd;	//地形レンダー

	GFL_G3D_RES*					groundResMdl;	//地形モデルリソース
	GFL_G3D_RES*					groundResTex;	//地形テクスチャリソース
	NNSGfdTexKey					groundTexKey;
	NNSGfdPlttKey					groundPlttKey;

	GFL_G3D_RES*					globalResTex;	//グローバル地形テクスチャリソース
	const GFL_G3D_MAP_GLOBALOBJ*	globalResObj;	//配置オブジェクトリソース

	GFL_G3D_MAP_GLOBALOBJ_ST		object[OBJ_COUNT];				//配置オブジェクト配列１
	GFL_G3D_MAP_GLOBALOBJ_ST		directDrawObject[DDOBJ_COUNT];	//配置オブジェクト配列２

	GFL_G3D_MAP_LOAD_STATUS			ldst;
	void*							mapData;
};

typedef BOOL (GFL_G3D_MAPLOAD_FUNC)( GFL_G3D_MAP* g3Dmap );

static const VecFx32 defaultScale = { FX32_ONE, FX32_ONE, FX32_ONE };
static const MtxFx33 defaultRotate = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };
//------------------------------------------------------------------
static BOOL	DrawGround( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera );
static void	DrawObj( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera );
static void	DirectDrawObj( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera );

static void		getViewLength( GFL_G3D_CAMERA* g3Dcamera, const VecFx32* pos, fx32* result );
static void		getYbillboardMtx( GFL_G3D_CAMERA* g3Dcamera, MtxFx33* result );

static void		InitMapObject( GFL_G3D_MAP* g3Dmap );
static void		InitWork( GFL_G3D_MAP* g3Dmap );
static void		MakeMapRender( NNSG3dRenderObj* rndObj, GFL_G3D_RES* mdl, GFL_G3D_RES* tex );
static void		TrashMapRender( NNSG3dRenderObj* rndObj );
static s32		testBoundingBox( const GXBoxTestParam* boundingBox );
static BOOL		checkCullingBoxTest( NNSG3dRenderObj* rnd );

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップシステム作成
 */
//------------------------------------------------------------------
GFL_G3D_MAP*	GFL_G3D_MAP_Create( GFL_G3D_MAP_SETUP* setup, HEAPID heapID )
{
	GFL_G3D_MAP*	g3Dmap = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_MAP) );
	u32				g3DresHeaderSize = GFL_G3D_GetResourceHeaderSize();

	//初期化
	InitWork( g3Dmap );

	//関数テーブル登録
	g3Dmap->mapFileFunc = setup->mapFileFunc;

	//データ格納エリア確保
	g3Dmap->mapDataHeapSize = setup->mapDataHeapSize;
	g3Dmap->mapData = GFL_HEAP_AllocClearMemory( heapID, setup->mapDataHeapSize );

	//レンダー作成
	g3Dmap->NNSrnd = NNS_G3dAllocRenderObj( GFL_G3D_GetAllocaterPointer() );
	g3Dmap->NNSrnd->resMdl = NULL; 

	//アーカイブハンドル作成
	g3Dmap->arc = NULL;

	//モデルリソースヘッダ作成
	g3Dmap->groundResMdl = GFL_HEAP_AllocClearMemory( heapID, g3DresHeaderSize );

	//テクスチャリソースヘッダ作成
	g3Dmap->groundResTex = GFL_HEAP_AllocClearMemory( heapID, g3DresHeaderSize );
	g3Dmap->globalResTex = NULL;

	//テクスチャ＆パレットＶＲＡＭ確保
	if( setup->texVramSize != 0 ){
		g3Dmap->texVramSize = setup->texVramSize;
		g3Dmap->groundTexKey = NNS_GfdAllocTexVram( setup->texVramSize, FALSE, 0 );
		g3Dmap->groundPlttKey = NNS_GfdAllocPlttVram( MAPPLTT_SIZE, FALSE, 0 );

		GF_ASSERT( g3Dmap->groundTexKey != NNS_GFD_ALLOC_ERROR_TEXKEY );
		GF_ASSERT( g3Dmap->groundPlttKey != NNS_GFD_ALLOC_ERROR_PLTTKEY );
	} else {
		g3Dmap->groundTexKey = 0;
		g3Dmap->groundPlttKey = 0;
	}
	return g3Dmap;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップシステム破棄
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_Delete( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	GFL_G3D_MAP_ReleaseArc( g3Dmap );		//アーカイブハンドルを閉じる（保険）

	if( g3Dmap->texVramSize != 0 ){
		NNS_GfdFreePlttVram( g3Dmap->groundPlttKey );
		NNS_GfdFreeTexVram( g3Dmap->groundTexKey );
	}
	GFL_HEAP_FreeMemory( g3Dmap->groundResTex );
	GFL_HEAP_FreeMemory( g3Dmap->groundResMdl );

	NNS_G3dFreeRenderObj( GFL_G3D_GetAllocaterPointer(), g3Dmap->NNSrnd );

	GFL_HEAP_FreeMemory( g3Dmap->mapData );

	GFL_HEAP_FreeMemory( g3Dmap );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップメイン
 *				※データ読み込み
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_Main( GFL_G3D_MAP* g3Dmap )
{
	GFL_G3D_MAP_FILELOAD_FUNC*	loadFunc;

	GF_ASSERT( g3Dmap );

	if( g3Dmap->ldst.seq != LOAD_IDLING ){
		loadFunc = g3Dmap->mapFileFunc[g3Dmap->fileType].loadFunc;

		if( loadFunc != NULL ){
			loadFunc( g3Dmap );
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ描画
 *		開始終了およびグローバルステート設定は関数外で行っておくこと
 *
 *		ex)
 *		{
 *			GFL_G3D_MAP_StartDraw();	//描画開始
 *
 *			for( i=0; i<g3Dmap_count; i++ ){
 *				GFL_G3D_MAP_Draw( ...... );		//各マップ描画関数
 *			
 *			GFL_G3D_MAP_EndDraw();		//描画終了
 *		}
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_StartDraw( void )
{
	GFL_G3D_DRAW_Start();		//描画開始
	GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定		
}

void	GFL_G3D_MAP_Draw( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera )
{
	GF_ASSERT( g3Dmap );

	//地形描画
	if( DrawGround( g3Dmap, g3Dcamera ) == TRUE ){
		//配置オブジェクト描画
		if( g3Dmap->globalResObj != NULL ){
			DrawObj( g3Dmap, g3Dcamera );
			DirectDrawObj( g3Dmap, g3Dcamera );
		}
	}
}

void	GFL_G3D_MAP_EndDraw( void )
{
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップアーカイブデータ登録
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_ResistArc( GFL_G3D_MAP* g3Dmap, const u32 arcID, HEAPID heapID )
{
	GF_ASSERT( g3Dmap );

	//初期化
	InitWork( g3Dmap );

	//アーカイブハンドルが既にオープンされている場合はクローズ
	GFL_G3D_MAP_ReleaseArc( g3Dmap );
	g3Dmap->NNSrnd->resMdl = NULL; 

	//アーカイブハンドル作成
	g3Dmap->arc = GFL_ARC_OpenDataHandle( arcID, heapID );
	GF_ASSERT( g3Dmap->arc );
}

void	GFL_G3D_MAP_ReleaseArc( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	if( g3Dmap->arc != NULL ){
		GFL_ARC_CloseDataHandle( g3Dmap->arc );
		g3Dmap->arc = NULL;
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルテクスチャリソース登録
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_ResistGlobalTexResource
			( GFL_G3D_MAP* g3Dmap, GFL_G3D_RES* globalResTex )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->globalResTex = globalResTex;
}

void	GFL_G3D_MAP_ReleaseGlobalTexResource( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->globalResTex = NULL;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルオブジェクトリソース登録
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_ResistGlobalObjResource
			( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ* globalResObj )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->globalResObj = globalResObj;
}

void	GFL_G3D_MAP_ReleaseGlobalObjResource( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->globalResObj = NULL;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルオブジェクト配置登録
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_ResistGlobalObj
				( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status, u32 idx )
{
	GF_ASSERT( g3Dmap );
	GF_ASSERT( idx < OBJ_COUNT );
	GF_ASSERT( status->id < g3Dmap->globalResObj->gobjCount );

	//配列位置指定
	g3Dmap->object[idx].id = status->id;
	g3Dmap->object[idx].trans = status->trans;
	g3Dmap->object[idx].rotate = status->rotate;
}

u32		GFL_G3D_MAP_ResistAutoGlobalObj
				( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status )
{
	int i;

	GF_ASSERT( g3Dmap );

	//配列位置自動取得
	for( i=0; i<OBJ_COUNT; i++ ){
		if( g3Dmap->object[i].id == OBJID_NULL ){
			GFL_G3D_MAP_ResistGlobalObj( g3Dmap, status, i );
			return i;
		}
	}
	return GLOBALOBJ_SET_ERROR;
}

void	GFL_G3D_MAP_ReleaseGlobalObj( GFL_G3D_MAP* g3Dmap, u32 idx )
{
	GF_ASSERT( g3Dmap );
	GF_ASSERT( idx < OBJ_COUNT );

	if( (idx != GLOBALOBJ_SET_ERROR)&&(idx < OBJ_COUNT) ){
		g3Dmap->object[idx].id = OBJID_NULL;
		VEC_Set( &g3Dmap->object[idx].trans, 0, 0, 0 );
		g3Dmap->object[idx].rotate = 0;
	}
}

//------------------------------------------------------------------
void	GFL_G3D_MAP_ResistGlobalDDobj
				( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status, u32 idx )
{
	GF_ASSERT( g3Dmap );
	GF_ASSERT( idx < DDOBJ_COUNT );
	GF_ASSERT( status->id < g3Dmap->globalResObj->gddobjCount );

	//配列位置指定
	g3Dmap->directDrawObject[idx].id = status->id;
	g3Dmap->directDrawObject[idx].trans = status->trans;
	g3Dmap->directDrawObject[idx].rotate = status->rotate;
}

u32		GFL_G3D_MAP_ResistAutoGlobalDDobj
				( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status )
{
	int i;

	GF_ASSERT( g3Dmap );

	//配列位置自動取得
	for( i=0; i<DDOBJ_COUNT; i++ ){
		if( g3Dmap->directDrawObject[i].id == OBJID_NULL ){
			GFL_G3D_MAP_ResistGlobalDDobj( g3Dmap, status, i );
			return i;
		}
	}
	return GLOBALOBJ_SET_ERROR;
}

void	GFL_G3D_MAP_ReleaseGlobalDDobj( GFL_G3D_MAP* g3Dmap, u32 idx )
{
	GF_ASSERT( g3Dmap );
	GF_ASSERT( idx < DDOBJ_COUNT );

	if( (idx != GLOBALOBJ_SET_ERROR)&&(idx < DDOBJ_COUNT) ){
		g3Dmap->directDrawObject[idx].id = OBJID_NULL;
		VEC_Set( &g3Dmap->directDrawObject[idx].trans, 0, 0, 0 );
		g3Dmap->directDrawObject[idx].rotate = 0;
	}
}

//------------------------------------------------------------------
/**
 * @brief	３ＤマップグローバルオブジェクトＩＤ変換
 */
//------------------------------------------------------------------
BOOL	GFL_G3D_MAP_GetGlobalObjectID( GFL_G3D_MAP* g3Dmap, const u32 datID, u32* exchangeID )
{
	int i;

	if( g3Dmap->globalResObj->gobjIDexchange == NULL ){
		*exchangeID = 0;
		return FALSE;
	}
	for( i=0; i<g3Dmap->globalResObj->gobjCount; i++ ){
		if( datID == g3Dmap->globalResObj->gobjIDexchange[i] ){
			*exchangeID = i;
			return TRUE;
		}
	}
	*exchangeID = 0;
	return FALSE;
}

BOOL	GFL_G3D_MAP_GetGlobalDDobjectID( GFL_G3D_MAP* g3Dmap, const u32 datID, u32* exchangeID )
{
	int i;

	if( g3Dmap->globalResObj->gddobjIDexchange == NULL ){
		*exchangeID = 0;
		return FALSE;
	}
	for( i=0; i<g3Dmap->globalResObj->gddobjCount; i++ ){
		if( datID == g3Dmap->globalResObj->gddobjIDexchange[i] ){
			*exchangeID = i;
			return TRUE;
		}
	}
	*exchangeID = 0;
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロードリクエスト設定
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_SetLoadReq( GFL_G3D_MAP* g3Dmap, const u32 datID )
{
	GF_ASSERT( g3Dmap );
	GF_ASSERT( g3Dmap->arc );

	TrashMapRender( g3Dmap->NNSrnd );
	GFL_G3D_MAP_DeleteResourceMdl( g3Dmap );
	GFL_G3D_MAP_DeleteResourceTex( g3Dmap );

	InitMapObject( g3Dmap );

	g3Dmap->datID = datID;

	g3Dmap->ldst.seq = LOAD_START;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ描画ＯＮ／ＯＦＦ＆取得
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_SetDrawSw( GFL_G3D_MAP* g3Dmap, BOOL drawSw )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->drawSw = drawSw;
}

BOOL	GFL_G3D_MAP_GetDrawSw( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	return g3Dmap->drawSw;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ位置設定＆取得
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_SetTrans( GFL_G3D_MAP* g3Dmap, const VecFx32* trans )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->trans = *trans;
}

void	GFL_G3D_MAP_GetTrans( GFL_G3D_MAP* g3Dmap, VecFx32* trans )
{
	GF_ASSERT( g3Dmap );

	*trans = g3Dmap->trans;
}

//------------------------------------------------------------------
/**
 * @brief	ロードステータスポインタ取得
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_GetLoadStatusPointer( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_LOAD_STATUS** ldst )
{
	GF_ASSERT( g3Dmap );

	*ldst = &g3Dmap->ldst;
}

//------------------------------------------------------------------
/**
 * @brief	ロードメモリポインタ取得
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_GetLoadMemoryPointer( GFL_G3D_MAP* g3Dmap, void** mem )
{
	GF_ASSERT( g3Dmap );

	*mem = g3Dmap->mapData;
}

//------------------------------------------------------------------
/**
 * @brief	アーカイブハンドル取得
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_GetLoadArcHandle( GFL_G3D_MAP* g3Dmap, ARCHANDLE** handle )
{
	GF_ASSERT( g3Dmap );

	*handle = g3Dmap->arc;
}

//------------------------------------------------------------------
/**
 * @brief	ロードデータＩＤ取得
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_GetLoadDatID( GFL_G3D_MAP* g3Dmap, u32* ID )
{
	GF_ASSERT( g3Dmap );

	*ID = g3Dmap->datID;
}

//------------------------------------------------------------------
/**
 * @brief	モデルリソース設定
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_CreateResourceMdl( GFL_G3D_MAP* g3Dmap, void* mem )
{
	GF_ASSERT( g3Dmap );

	GFL_G3D_CreateResource( g3Dmap->groundResMdl, GFL_G3D_RES_CHKTYPE_MDL, mem );
}

void GFL_G3D_MAP_DeleteResourceMdl( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	GFL_G3D_CreateResource( g3Dmap->groundResMdl, GFL_G3D_RES_CHKTYPE_MDL, NULL );
}

//------------------------------------------------------------------
/**
 * @brief	テクスチャリソース設定
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_CreateResourceTex( GFL_G3D_MAP* g3Dmap, void* mem )
{
	GF_ASSERT( g3Dmap );

	GFL_G3D_CreateResource( g3Dmap->groundResTex, GFL_G3D_RES_CHKTYPE_TEX, mem );
}

void GFL_G3D_MAP_DeleteResourceTex( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	GFL_G3D_CreateResource( g3Dmap->groundResTex, GFL_G3D_RES_CHKTYPE_TEX, NULL );
}

//------------------------------------------------------------------
/**
 * @brief	レンダー作成
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_MakeRenderObj( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	if( (GFL_G3D_GetResourceFileHeader( g3Dmap->groundResTex ) == NULL )
		&&( g3Dmap->globalResTex != NULL )){
		MakeMapRender( g3Dmap->NNSrnd, g3Dmap->groundResMdl, g3Dmap->globalResTex );
	} else {
		MakeMapRender( g3Dmap->NNSrnd, g3Dmap->groundResMdl, g3Dmap->groundResTex );
	}
}

//------------------------------------------------------------------
/**
 * @brief	ロードステータス初期化
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_ResetLoadStatus( GFL_G3D_MAP* g3Dmap )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->ldst.mOffs = 0;
	g3Dmap->ldst.tOffs = 0;
	g3Dmap->ldst.mdlLoaded = FALSE;
	g3Dmap->ldst.texLoaded = FALSE;
	g3Dmap->ldst.attrLoaded = FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	ファイル読み込み関数
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_StartFileLoad( GFL_G3D_MAP* g3Dmap, u32 datID )
{
	void* dst = (void*)((u32)g3Dmap->mapData + g3Dmap->ldst.mOffs);

	GF_ASSERT( (g3Dmap->ldst.mOffs + MAPLOAD_SIZE) <= g3Dmap->mapDataHeapSize );

	//ロード開始
	g3Dmap->ldst.fSize = GFL_ARC_GetDataSizeByHandle( g3Dmap->arc, datID );
	g3Dmap->ldst.fOffs = 0;

	GFL_ARC_LoadDataOfsByHandle( g3Dmap->arc, datID, 0, MAPLOAD_SIZE, dst ); 

	g3Dmap->ldst.mOffs += MAPLOAD_SIZE;
	g3Dmap->ldst.fOffs += MAPLOAD_SIZE;
}

BOOL GFL_G3D_MAP_ContinueFileLoad( GFL_G3D_MAP* g3Dmap )
{
	void*	dst = (void*)((u32)g3Dmap->mapData + g3Dmap->ldst.mOffs);
	u32		rest;

	if( g3Dmap->ldst.fSize < g3Dmap->ldst.fOffs){
		return FALSE;
	}
	rest = g3Dmap->ldst.fSize - g3Dmap->ldst.fOffs;

	if( rest > MAPLOAD_SIZE ){ 
		GF_ASSERT( (g3Dmap->ldst.mOffs + MAPLOAD_SIZE) <= g3Dmap->mapDataHeapSize );

		//ロード継続
		GFL_ARC_LoadDataByHandleContinue( g3Dmap->arc, MAPLOAD_SIZE, dst ); 

		g3Dmap->ldst.mOffs += MAPLOAD_SIZE;
		g3Dmap->ldst.fOffs += MAPLOAD_SIZE;
		return TRUE;
	}
	GF_ASSERT( (g3Dmap->ldst.mOffs + rest) <= g3Dmap->mapDataHeapSize );

	//最終ロード
	GFL_ARC_LoadDataByHandleContinue( g3Dmap->arc, rest, dst );

	g3Dmap->ldst.mOffs += rest;
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	ローカルテクスチャVRAM転送関数
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_SetTransVramParam( GFL_G3D_MAP* g3Dmap )
{
	if( g3Dmap->texVramSize == 0 ){
		return;
	}
	//テクスチャリソースへのＶＲＡＭキーの設定
	NNS_G3dTexSetTexKey( GFL_G3D_GetResTex(g3Dmap->groundResTex), g3Dmap->groundTexKey, 0 );
	NNS_G3dPlttSetPlttKey( GFL_G3D_GetResTex(g3Dmap->groundResTex), g3Dmap->groundPlttKey );

	g3Dmap->ldst.tSize = g3Dmap->texVramSize;	//テクスチャ転送量設定
	g3Dmap->ldst.tOffs = 0;
}

BOOL GFL_G3D_MAP_TransVram( GFL_G3D_MAP* g3Dmap )
{
	void*	src;
	u32		dst;
	u32		rest;

	if( g3Dmap->texVramSize == 0 ){
		return FALSE;
	}
	if( g3Dmap->ldst.tOffs >= g3Dmap->ldst.tSize ){
		src = (void*)GFL_G3D_GetAdrsTexturePltt(g3Dmap->groundResTex);
		dst = NNS_GfdGetPlttKeyAddr( g3Dmap->groundPlttKey );

		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, dst, src, MAPPLTT_SIZE );
		return FALSE;
	}
	src = (void*)(GFL_G3D_GetAdrsTextureData(g3Dmap->groundResTex) + g3Dmap->ldst.tOffs );
	dst = NNS_GfdGetTexKeyAddr( g3Dmap->groundTexKey ) + g3Dmap->ldst.tOffs;
	rest = g3Dmap->ldst.tSize - g3Dmap->ldst.tOffs;

	if( rest > MAPTRANS_SIZE){
		//転送継続
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, dst, src, MAPTRANS_SIZE );
		g3Dmap->ldst.tOffs += MAPTRANS_SIZE;
	} else {
		//最終転送
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, dst, src, rest );
		g3Dmap->ldst.tOffs = g3Dmap->ldst.tSize;
	}
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	ファイル識別設定（仮）※いずれはデータファイルの中に識別を埋め込む
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_ResistFileType( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAPDATA_FILETYPE fileType )
{
	GF_ASSERT( g3Dmap );

	g3Dmap->fileType = fileType;
}





//============================================================================================
/**
 *
 *
 *
 * @brief	各種描画
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	地形描画
 */
//------------------------------------------------------------------
static BOOL	DrawGround( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera )
{
	if( (g3Dmap->drawSw != FALSE)&&(NNS_G3dRenderObjGetResMdl(g3Dmap->NNSrnd) != NULL )){
		
		NNS_G3dGlbSetBaseTrans( &g3Dmap->trans );	// 位置設定
		NNS_G3dGlbSetBaseRot( &defaultRotate );		// 角度設定
		NNS_G3dGlbSetBaseScale( &defaultScale );	// スケール設定
		NNS_G3dGlbFlush();							//グローバルステート反映

		if( checkCullingBoxTest( g3Dmap->NNSrnd ) == TRUE ){
			//地形描画
			NNS_G3dDraw( g3Dmap->NNSrnd );	
			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	配置オブジェクト描画
 */
//------------------------------------------------------------------
static void	DrawObj( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera )
{
	GFL_G3D_MAP_OBJ*	obj = g3Dmap->globalResObj->gobj;
	u32					count = g3Dmap->globalResObj->gobjCount;
	NNSG3dRenderObj	*NNSrnd, *NNSrnd_L;
	VecFx32			globalTrans;
	fx32			length;
	int				i;

	if(( count == 0 )||( obj == NULL )){ return; }

	for( i=0; i<OBJ_COUNT; i++ ){
		if(	(g3Dmap->object[i].id != OBJID_NULL)&&(g3Dmap->object[i].id < count) ){

			VEC_Add( &g3Dmap->object[i].trans, &g3Dmap->trans, &globalTrans );

			getViewLength( g3Dcamera, &globalTrans, &length );

			if( length <= DRAW_LIMIT ){
	
				NNSrnd = obj[ g3Dmap->object[i].id ].NNSrnd_H;
				NNSrnd_L = obj[ g3Dmap->object[i].id ].NNSrnd_L;

				if( ( length > LOD_LIMIT )&&( NNSrnd_L != NULL ) ){
					NNSrnd = NNSrnd_L;
				}
				if( NNS_G3dRenderObjGetResMdl( NNSrnd ) != NULL ){
	
					NNS_G3dGlbSetBaseTrans( &globalTrans );		// 位置設定
					NNS_G3dGlbSetBaseRot( &defaultRotate );		// 角度設定
					NNS_G3dGlbSetBaseScale( &defaultScale );	// スケール設定
					NNS_G3dGlbFlush();							//グローバルステート反映
	
					if( checkCullingBoxTest( NNSrnd ) == TRUE ){
						//オブジェクト描画
						NNS_G3dDraw( NNSrnd );
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	配置オブジェクト描画(ジオメトリ直書き)
 */
//------------------------------------------------------------------
static void	DirectDrawObj( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera )
{
	GFL_G3D_MAP_DDOBJ*	ddobj = g3Dmap->globalResObj->gddobj;
	u32					count = g3Dmap->globalResObj->gddobjCount;
	const GFL_G3D_MAP_DDOBJ*	objData;
	GFL_G3D_MAP_GLOBALOBJ_ST*	ddObject;
	MtxFx33		mtxBillboard;
	VecFx32		trans, globalTrans;
	VecFx16		vecView;
	fx32		length;
	int			i;

	if(( count == 0 )||( ddobj == NULL )){ return; }

	G3X_Reset();

	//カメラ設定
	{
		VecFx32	camPos, camUp, target, tmp;

		GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );
		G3_LookAt( &camPos, &camUp, &target, NULL );

		VEC_Subtract( &camPos, &target, &tmp );
		VEC_Normalize( &tmp, &tmp );
		VEC_Fx16Set( &vecView, tmp.x, tmp.y, tmp.z );
	}
	//Y軸ビルボード用回転MTX取得
	getYbillboardMtx( g3Dcamera, &mtxBillboard );

	for( i=0; i<DDOBJ_COUNT; i++ ){
		ddObject = &g3Dmap->directDrawObject[i];

		if( (ddObject->id != OBJID_NULL)&&(ddObject->id < count) ){
			objData = &ddobj[ ddObject->id ];
			if( objData->g3Dres != NULL ){

				VEC_Add( &ddObject->trans, &g3Dmap->trans, &globalTrans );

				getViewLength( g3Dcamera, &globalTrans, &length );

				if( length <= DRAW_LIMIT ){
					G3_PushMtx();

					//スケール設定
					G3_Scale(	FX32_ONE * objData->data->scaleVal, 
								FX32_ONE * objData->data->scaleVal, 
								FX32_ONE * objData->data->scaleVal );
					//回転、平行移動パラメータ設定
					trans.x = globalTrans.x / objData->data->scaleVal;
					trans.y = globalTrans.y / objData->data->scaleVal;
					trans.z = globalTrans.z / objData->data->scaleVal;
					if( objData->data->drawType == DRAW_YBILLBOARD ){
						G3_MultTransMtx33( &mtxBillboard, &trans );
					} else {
						G3_Translate( trans.x, trans.y, trans.z );
					}
					//マテリアル設定
					G3_MaterialColorDiffAmb(	objData->data->diffuse, 
												objData->data->ambient, 
												TRUE );
					G3_MaterialColorSpecEmi(	objData->data->specular, 
												objData->data->emission, 
												FALSE );
					G3_PolygonAttr(	objData->data->lightMask, 
									GX_POLYGONMODE_MODULATE, 
									GX_CULL_NONE, 
									objData->data->polID, 
									objData->data->alpha, 
									GX_POLYGON_ATTR_MISC_FOG );
	
					if( objData->data->func != NULL ){
						objData->data->func(	ddobj[ ddObject->id ].texDataAdrs, 
												ddobj[ ddObject->id ].texPlttAdrs, 
												&vecView, (length < LOD_LIMIT) );
					}
					G3_PopMtx(1);
				}
			}
		}
	}
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
 * @brief	アトリビュート取得ワーク初期化
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_InitAttr( GFL_G3D_MAP_ATTRINFO* attrInfo )
{
	int i;

	for( i=0; i<GFL_G3D_MAP_ATTR_GETMAX; i++ ){
		VEC_Fx16Set( &attrInfo->mapAttr[i].vecN, 0, 0, 0 );
		attrInfo->mapAttr[i].attr = 0;
		attrInfo->mapAttr[i].height = 0;
	}
	attrInfo->mapAttrCount = 0;
}

//------------------------------------------------------------------
/**
 * @brief	アトリビュート取得
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_GetAttr( GFL_G3D_MAP_ATTRINFO* attrInfo, GFL_G3D_MAP* g3Dmap,
							const VecFx32* pos, const fx32 map_width )
{
	GFL_G3D_MAP_FILEATTR_FUNC*	attrFunc;
	VecFx32 posInBlock;
	fx32	map_height;

	GF_ASSERT( g3Dmap );

	if( g3Dmap->ldst.attrLoaded == FALSE ){
		attrInfo->mapAttrCount = 0;
		return;
	}
	map_height = g3Dmap->trans.y;

	//ブロック内情報取得
	VEC_Subtract( pos, &g3Dmap->trans, &posInBlock );

	attrFunc = g3Dmap->mapFileFunc[g3Dmap->fileType].attrFunc;

	if( attrFunc != NULL ){
		attrFunc( attrInfo, g3Dmap->mapData, &posInBlock, map_width, map_height );
	}
}





//============================================================================================
/**
 *
 *
 *
 * @brief	ローカル関数
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
static void	InitMapObject( GFL_G3D_MAP* g3Dmap )
{
	int i;

	for( i=0; i<OBJ_COUNT; i++ ){
		g3Dmap->object[i].id = OBJID_NULL;
		VEC_Set( &g3Dmap->object[i].trans, 0, 0, 0 );
		g3Dmap->object[i].rotate = 0;
	}
	for( i=0; i<DDOBJ_COUNT; i++ ){
		g3Dmap->directDrawObject[i].id = OBJID_NULL;
		VEC_Set( &g3Dmap->directDrawObject[i].trans, 0, 0, 0 );
		g3Dmap->object[i].rotate = 0;
	}
}

static void	InitWork( GFL_G3D_MAP* g3Dmap )
{
	g3Dmap->drawSw = FALSE;
	VEC_Set( &g3Dmap->trans, 0, 0, 0 );

	g3Dmap->ldst.seq = LOAD_IDLING;
	g3Dmap->datID = 0;

	InitMapObject( g3Dmap );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップレンダー作成
 */
//------------------------------------------------------------------
static void MakeMapRender( NNSG3dRenderObj* rndObj, GFL_G3D_RES* mdl, GFL_G3D_RES* tex )
{
	NNSG3dResFileHeader* mdlFH = GFL_G3D_GetResourceFileHeader( mdl );
	NNSG3dResFileHeader* texFH = GFL_G3D_GetResourceFileHeader( tex );
	NNSG3dResMdl* pMdl = NULL;
	NNSG3dResTex* pTex = NULL;

	//モデル＆テクスチャリソースポインタ取得
	pMdl = NNS_G3dGetMdlByIdx( NNS_G3dGetMdlSet( mdlFH ), 0 );	//0th mdl
	GF_ASSERT(pMdl);
	if( texFH != NULL ){
		pTex = NNS_G3dGetTex( texFH );
	}

	//モデルとテクスチャリソースとの関連付け
	if( pTex ){
		NNS_G3dBindMdlTex( pMdl, pTex );
		NNS_G3dBindMdlPltt( pMdl, pTex );
	}
	//レンダリングオブジェクト初期化
	NNS_G3dRenderObjInit( rndObj, pMdl );
}

static void TrashMapRender( NNSG3dRenderObj* rndObj )
{
	rndObj->resMdl = NULL; 
}

//--------------------------------------------------------------------------------------------
//ボックス視体積内外テストによるカリングチェック
//（オブジェクトのグローバルステータス反映後に呼ぶこと）
static s32 testBoundingBox( const GXBoxTestParam* boundingBox );

static BOOL checkCullingBoxTest( NNSG3dRenderObj* rnd )
{
	NNSG3dResMdlInfo*	modelInfo;		// モデルデータからのボックステスト用パラメータ取得用
	BOOL				result = TRUE;
	GXBoxTestParam		boundingBox;

	//ボックステスト用パラメータ取得
	modelInfo = NNS_G3dGetMdlInfo( NNS_G3dRenderObjGetResMdl(rnd) );

	//バウンディングボックス作成
	boundingBox.x		= modelInfo->boxX;
	boundingBox.y		= modelInfo->boxY;
	boundingBox.z		= modelInfo->boxZ;
	boundingBox.width	= modelInfo->boxW;
	boundingBox.height	= modelInfo->boxH;
	boundingBox.depth	= modelInfo->boxD;

	//上のボックステストの箱の値をposScaleでかけることにより本当の値になる
	NNS_G3dGePushMtx();
	NNS_G3dGeScale( modelInfo->boxPosScale, modelInfo->boxPosScale, modelInfo->boxPosScale );
	
	//チェック
	if( !testBoundingBox( &boundingBox ) ){
		result = FALSE;
	}
	NNS_G3dGePopMtx(1);

	return result;
}

static s32 testBoundingBox( const GXBoxTestParam* boundingBox )
{
	s32 testResult = 1;

	//ポリゴン属性の「FAR面交差クリッピングフラグ」と「１ドットポリゴン表示フラグ」を1に設定
	//※ライブラリからの要求
	NNS_G3dGePolygonAttr( GX_LIGHTMASK_0, GX_POLYGONMODE_MODULATE, GX_CULL_NONE,           
					0, 0, GX_POLYGON_ATTR_MISC_FAR_CLIPPING | GX_POLYGON_ATTR_MISC_DISP_1DOT );

	//ポリゴンアトリビュート反映 ※ライブラリ指定手順
	NNS_G3dGeBegin( GX_BEGIN_TRIANGLES );							
	NNS_G3dGeEnd();	

	NNS_G3dGeBoxTest( boundingBox );	// ボックステスト
	NNS_G3dGeFlushBuffer();				// 同期を取る

	//チェック本体（返り値が0の場合テスト終了）。 0:視体積内、それ以外の値は視体積外
	while ( G3X_GetBoxTestResult( &testResult ) != 0 ) ;

	return testResult;
}

//--------------------------------------------------------------------------------------------
// カメラとの距離計算
static void getViewLength( GFL_G3D_CAMERA* g3Dcamera, const VecFx32* pos, fx32* result )
{
	VecFx32 camPos, vecLength;

	GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );

	VEC_Subtract( pos, &camPos, &vecLength );

	*result = VEC_Mag( &vecLength );
}

//Y軸ビルボード用回転MTX計算
static void getYbillboardMtx( GFL_G3D_CAMERA* g3Dcamera, MtxFx33* result )
{
	VecFx32	camPos, target, tmp;

	GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
	GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

	VEC_Subtract( &camPos, &target, &tmp );
	tmp.y = 0;
	VEC_Normalize( &tmp, &tmp );
	MTX_Identity33( result );
	MTX_RotY33( result, tmp.x, tmp.z );
}





//============================================================================================
/**
 *
 *
 *
 * @brief	テスト関数
 *
 *
 *
 */
//============================================================================================
#include "system\gfl_use.h"	//乱数用

static void makeRandomPos( GFL_G3D_MAP* g3Dmap, u32 idx, GFL_G3D_MAP_GLOBALOBJ_ST* status )
{
	fx32	centeringOffs, randomOffs, gridWidth, mapWidth;
	GFL_G3D_MAP_ATTRINFO attrInfo;
	VecFx32 pWorld;

	mapWidth = 512 * FX32_ONE;
	centeringOffs = -mapWidth/2;
	gridWidth = mapWidth/MAP_GRIDCOUNT;

	status->id = idx;
	status->rotate = 0;

	randomOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gridWidth;
	status->trans.x = randomOffs + centeringOffs;
	status->trans.y = 0;
	randomOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gridWidth;
	status->trans.z = randomOffs + centeringOffs;

	VEC_Add( &status->trans, &g3Dmap->trans, &pWorld );
	GFL_G3D_MAP_GetAttr( &attrInfo, g3Dmap, &pWorld, mapWidth );
	status->trans.y = attrInfo.mapAttr[0].height;
}

void GFL_G3D_MAP_MakeTestPos( GFL_G3D_MAP* g3Dmap )
{
	int i;
	GFL_G3D_MAP_GLOBALOBJ_ST status;

	if( g3Dmap->trans.y == 0 ){
		for( i=0; i<4; i++ ){
			if( i<2 ){
				makeRandomPos( g3Dmap, 0, &status );
			} else {
				makeRandomPos( g3Dmap, 1, &status );
			}
			GFL_G3D_MAP_ResistGlobalObj( g3Dmap, &status, i );
		}
		for( i=0; i<32; i++ ){
			makeRandomPos( g3Dmap, 0, &status );
			GFL_G3D_MAP_ResistGlobalDDobj( g3Dmap, &status, i );
		}
	}
}



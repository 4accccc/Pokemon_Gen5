//============================================================================================
/**
 * @file	g3d_map.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system\gfl_use.h"	//乱数用

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

//#define LOCAL_OBJRES_COUNT	(16)
#define OBJ_COUNT	(32)
#define DDOBJ_COUNT	(64)

//------------------------------------------------------------------
//高さデータ取得用
typedef struct {
	fx16	vecN1_x;
	fx16	vecN1_y;
	fx16	vecN1_z;

	fx16	vecN2_x;
	fx16	vecN2_y;
	fx16	vecN2_z;

	fx32	vecN1_D;
	fx32	vecN2_D;

	u32		attr:31;
	u32		tryangleType:1;
}NormalVtxSt;

//ポリゴンデータ部
typedef struct SPLIT_GRID_DATA_tag{
	u16		NumX;		//グリッドＸ数
	u16		NumZ;		//グリッドＺ数

	fx32	StartX;		//始点位置Ｘ
	fx32	StartZ;		//始点位置Ｚ
	fx32	EndX;		//終点位置Ｘ
	fx32	EndZ;		//終点位置Ｚ
	fx32	SizeX;		//グリッドＸサイズ
	fx32	SizeZ;		//グリッドＺサイズ
}SPLIT_GRID_DATA;

typedef struct POLYGON_DATA_tag{
	u16 vtx_idx0;
	u16 vtx_idx1;
	u16 vtx_idx2;	//3角ポリゴンを形成する、頂点データ配列へのインデックスNo
	u16	nrm_idx;	//法線データ配列(正規化済み)へのインデックスNo
	
	fx32	paramD;		//平面の方程式から導いた、補正値D
}POLYGON_DATA;

typedef struct HEIGHT_ARRAY_tag
{
	fx32 Height;
	int Prev;
	int Next;
}HEIGHT_ARRAY;

typedef struct MAP_HEIGHT_INFO_tag{
	SPLIT_GRID_DATA	*SplitGridData;
	POLYGON_DATA	*PolygonData;
	u16				*GridDataTbl;
	u16				*LineDataTbl;
	VecFx32			*VertexArray;
	VecFx32			*NormalArray;
	u16				*PolyIDList;
//	BOOL			LoadOK;
	BOOL			DataValid;		//データ有効有無
}MAP_HEIGHT_INFO;

typedef struct {
	int VtxNum;
	int NrmNum;
	int PolygonNum;
	int GridNum;
	int TotalPolyIDListSize;
	int TotalLineListSize;
	int LineEntryMax;
}READ_INFO;

//------------------------------------------------------------------
typedef struct {
	u32			id;
	VecFx32		trans;
}MAP_OBJECT_DATA;

struct _GFL_G3D_MAP 
{
	BOOL				drawSw;
	VecFx32				trans;
	ARCHANDLE*			arc;

	u32					mdlHeapSize;
	u32					texHeapSize;
	u32					attrHeapSize;

	u32					loadSeq;
	u32					loadCount;

	NNSG3dRenderObj*	NNSrnd;	//地形レンダー

	BOOL				mdlLoadReq;
	u32					mdlID;
	GFL_G3D_RES*		groundResMdl;	//地形モデルリソース
	u8*					mdl;

	BOOL				texLoadReq;
	u32					texID;
	GFL_G3D_RES*		groundResTex;	//ローカル地形テクスチャリソース
	u8*					tex;

	BOOL				attrLoadReq;
	u32					attrID;
	u8*					attr;

	NNSGfdTexKey		groundTexKey;
	NNSGfdPlttKey		groundPlttKey;

	MAP_OBJECT_DATA		object[OBJ_COUNT];	//配置オブジェクト
	MAP_OBJECT_DATA		directDrawObject[DDOBJ_COUNT];

#if 0
	BOOL				objLoadReq;
	u32					objID;
	GFL_G3D_RES*		g3DobjectResMdl[LOCAL_OBJRES_COUNT];//ローカルオブジェクトモデルリソース
	u8					obj[MAPOBJ_SIZE];

	NNSGfdTexKey		objectTexKey;
	NNSGfdPlttKey		objectPlttKey;
#endif
};

enum {
	LOAD_IDLING = 0,
	MDL_LOAD_START,
	MDL_LOAD,
	TEX_LOAD_START,
	TEX_LOAD,
	RND_CREATE,
	TEX_TRANS,
	PLTT_TRANS,
	ATTR_LOAD_START,
	ATTR_LOAD,
	LOAD_END,
};

static const VecFx32 defaultScale = { FX32_ONE, FX32_ONE, FX32_ONE };
static const MtxFx33 defaultRotate = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };
//------------------------------------------------------------------
static BOOL	DrawGround( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera );
static void	DrawObj
			( GFL_G3D_MAP* g3Dmap, const GFL_G3D_MAP_OBJ* obj, GFL_G3D_CAMERA* g3Dcamera );
static void	DirectDrawObj
			( GFL_G3D_MAP* g3Dmap, const GFL_G3D_MAP_DDOBJ* ddobj, GFL_G3D_CAMERA* g3Dcamera );

static void		getViewLength( GFL_G3D_CAMERA* g3Dcamera, const VecFx32* pos, fx32* result );
static void		getYbillboardMtx( GFL_G3D_CAMERA* g3Dcamera, MtxFx33* result );

static void		InitMapObject( GFL_G3D_MAP* g3Dmap );
static void		InitWork( GFL_G3D_MAP* g3Dmap );
static void		MakeMapRender( NNSG3dRenderObj* rndObj, GFL_G3D_RES* mdl, GFL_G3D_RES* tex );
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

	//データ格納エリア確保
	g3Dmap->mdlHeapSize = setup->mdlHeapSize;
	g3Dmap->texHeapSize = setup->texHeapSize;
	g3Dmap->attrHeapSize = setup->attrHeapSize;
	g3Dmap->mdl = GFL_HEAP_AllocClearMemory( heapID, g3Dmap->mdlHeapSize );
	g3Dmap->tex = GFL_HEAP_AllocClearMemory( heapID, g3Dmap->texHeapSize );
	g3Dmap->attr = GFL_HEAP_AllocClearMemory( heapID, g3Dmap->attrHeapSize );

	//レンダー作成
	g3Dmap->NNSrnd = NNS_G3dAllocRenderObj( GFL_G3D_GetAllocaterPointer() );
	g3Dmap->NNSrnd->resMdl = NULL; 

	//アーカイブハンドル作成
	g3Dmap->arc = NULL;

	//モデルリソースヘッダ作成
	g3Dmap->groundResMdl = GFL_HEAP_AllocClearMemory( heapID, g3DresHeaderSize );
	GFL_G3D_CreateResource( g3Dmap->groundResMdl, 
								GFL_G3D_RES_CHKTYPE_MDL,(void*)g3Dmap->mdl );

	//テクスチャリソースヘッダ作成
	g3Dmap->groundResTex = GFL_HEAP_AllocClearMemory( heapID, g3DresHeaderSize );
	GFL_G3D_CreateResource( g3Dmap->groundResTex, 
								GFL_G3D_RES_CHKTYPE_TEX, (void*)g3Dmap->tex );

	//テクスチャ＆パレットＶＲＡＭ確保
	g3Dmap->groundTexKey = NNS_GfdAllocTexVram( g3Dmap->texHeapSize, FALSE, 0 );
	g3Dmap->groundPlttKey = NNS_GfdAllocPlttVram( MAPPLTT_SIZE, FALSE, 0 );

	GF_ASSERT( g3Dmap->groundTexKey != NNS_GFD_ALLOC_ERROR_TEXKEY );
	GF_ASSERT( g3Dmap->groundPlttKey != NNS_GFD_ALLOC_ERROR_PLTTKEY );

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

	if( g3Dmap->arc != NULL ){
		GFL_ARC_CloseDataHandle( g3Dmap->arc );
	}
	NNS_GfdFreePlttVram( g3Dmap->groundPlttKey );
	NNS_GfdFreeTexVram( g3Dmap->groundTexKey );

	GFL_HEAP_FreeMemory( g3Dmap->groundResTex );
	GFL_HEAP_FreeMemory( g3Dmap->groundResMdl );

	NNS_G3dFreeRenderObj( GFL_G3D_GetAllocaterPointer(), g3Dmap->NNSrnd );

	GFL_HEAP_FreeMemory( g3Dmap->attr );
	GFL_HEAP_FreeMemory( g3Dmap->tex );
	GFL_HEAP_FreeMemory( g3Dmap->mdl );

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
	u32	dataAdrs;

	GF_ASSERT( g3Dmap );

	switch( g3Dmap->loadSeq ){
	case LOAD_IDLING:
		break;

	case MDL_LOAD_START:
		g3Dmap->NNSrnd->resMdl = NULL; 
		InitMapObject( g3Dmap );

		//モデルデータロード開始
		GFL_ARC_LoadDataOfsByHandle( g3Dmap->arc, g3Dmap->mdlID, 
										0, MAPLOAD_SIZE, (void*)g3Dmap->mdl );
		g3Dmap->loadCount = 0;
		g3Dmap->loadCount++;
		g3Dmap->loadSeq = MDL_LOAD;
		break;
	case MDL_LOAD:
		dataAdrs = g3Dmap->loadCount * MAPLOAD_SIZE;

		if( (dataAdrs + MAPLOAD_SIZE) < g3Dmap->mdlHeapSize){
			//ロード継続
			GFL_ARC_LoadDataByHandleContinue(	g3Dmap->arc, MAPLOAD_SIZE, 
												(void*)((u32)g3Dmap->mdl + dataAdrs) );
			g3Dmap->loadCount++;
			break;
		}
		//最終ロード
		GFL_ARC_LoadDataByHandleContinue(	g3Dmap->arc, g3Dmap->mdlHeapSize - dataAdrs,
											(void*)((u32)g3Dmap->mdl + dataAdrs) );
		g3Dmap->mdlLoadReq = FALSE;
		g3Dmap->loadSeq = TEX_LOAD_START;
		break;
	case TEX_LOAD_START:
		if( g3Dmap->texLoadReq == FALSE ){
			g3Dmap->loadSeq = RND_CREATE;
			break;
		}
		//テクスチャロード開始
		GFL_ARC_LoadDataOfsByHandle( g3Dmap->arc, g3Dmap->texID, 
										0, MAPLOAD_SIZE, (void*)g3Dmap->tex );
		g3Dmap->loadCount = 0;
		g3Dmap->loadCount++;
		g3Dmap->loadSeq = TEX_LOAD;
		break;
	case TEX_LOAD:
		dataAdrs = g3Dmap->loadCount * MAPLOAD_SIZE;

		if( (dataAdrs + MAPLOAD_SIZE) < g3Dmap->texHeapSize){
			//ロード継続
			GFL_ARC_LoadDataByHandleContinue(	g3Dmap->arc, MAPLOAD_SIZE, 
												(void*)((u32)g3Dmap->tex + dataAdrs) );
			g3Dmap->loadCount++;
			break;
		}
		//最終ロード
		GFL_ARC_LoadDataByHandleContinue(	g3Dmap->arc, g3Dmap->texHeapSize - dataAdrs,
											(void*)((u32)g3Dmap->tex + dataAdrs) );
		g3Dmap->texLoadReq = FALSE;
		//テクスチャリソースへのＶＲＡＭキーの設定
		NNS_G3dTexSetTexKey( GFL_G3D_GetResTex(g3Dmap->groundResTex), 
								g3Dmap->groundTexKey, 0 );
		NNS_G3dPlttSetPlttKey( GFL_G3D_GetResTex(g3Dmap->groundResTex), 
								g3Dmap->groundPlttKey );
		g3Dmap->loadSeq = RND_CREATE;
		break;
	case RND_CREATE:
		//レンダー作成
		MakeMapRender
			( g3Dmap->NNSrnd, g3Dmap->groundResMdl, g3Dmap->groundResTex );
		g3Dmap->loadCount = 0;
		g3Dmap->loadSeq = TEX_TRANS;
		break;
	case TEX_TRANS:
		dataAdrs = g3Dmap->loadCount * MAPTRANS_SIZE;

		if( (dataAdrs + MAPTRANS_SIZE) < g3Dmap->texHeapSize){
			NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, 
				NNS_GfdGetTexKeyAddr( g3Dmap->groundTexKey ) + dataAdrs,
				(void*)(GFL_G3D_GetAdrsTextureData( g3Dmap->groundResTex ) + dataAdrs),
				MAPTRANS_SIZE );
			//転送継続
			g3Dmap->loadCount++;
			break;
		}
		//最終転送
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, 
				NNS_GfdGetTexKeyAddr( g3Dmap->groundTexKey ) + dataAdrs,
				(void*)(GFL_G3D_GetAdrsTextureData( g3Dmap->groundResTex ) + dataAdrs),
				g3Dmap->texHeapSize - dataAdrs );
		g3Dmap->loadSeq = PLTT_TRANS;
		break;
	case PLTT_TRANS:
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, 
				NNS_GfdGetPlttKeyAddr( g3Dmap->groundPlttKey ),
				(void*)GFL_G3D_GetAdrsTexturePltt( g3Dmap->groundResTex ),
				MAPPLTT_SIZE );
		g3Dmap->loadSeq = ATTR_LOAD_START;
		break;
	case ATTR_LOAD_START:
		//アトリビュートデータロード開始
		if( g3Dmap->attrID == NON_ATTR ){
			GFL_STD_MemClear32( (void*)g3Dmap->attr, g3Dmap->attrHeapSize );
			g3Dmap->loadSeq = LOAD_END;
			break;
		}
		GFL_ARC_LoadDataOfsByHandle( g3Dmap->arc, g3Dmap->attrID, 
										0, MAPLOAD_SIZE, (void*)g3Dmap->attr );
		g3Dmap->loadCount = 0;
		g3Dmap->loadCount++;
		g3Dmap->loadSeq = ATTR_LOAD;
		break;
	case ATTR_LOAD:
		dataAdrs = g3Dmap->loadCount * MAPLOAD_SIZE;

		if( (dataAdrs + MAPLOAD_SIZE) < g3Dmap->attrHeapSize){
			//ロード継続
			GFL_ARC_LoadDataByHandleContinue(	g3Dmap->arc, MAPLOAD_SIZE, 
												(void*)((u32)g3Dmap->attr + dataAdrs) );
			g3Dmap->loadCount++;
			break;
		}
		//最終ロード
		GFL_ARC_LoadDataByHandleContinue(	g3Dmap->arc, g3Dmap->attrHeapSize - dataAdrs,
											(void*)((u32)g3Dmap->attr + dataAdrs) );
		g3Dmap->attrLoadReq = FALSE;
		g3Dmap->loadSeq = LOAD_END;
		break;
	case LOAD_END:
		g3Dmap->loadSeq = LOAD_IDLING;
#if 1
		//--------------------
		if( g3Dmap->trans.y == 0 ){
			int i, r;
			fx32 randTransx, randTransz;
			fx32 cOffs, rOffs, gWidth, mapWidth;
			VecFx32 pWorld;
			GFL_G3D_MAP_ATTRINFO attrInfo;
			BOOL f;

			mapWidth = 512 * FX32_ONE;
			cOffs = -mapWidth/2;
			gWidth = mapWidth/MAP_GRIDCOUNT;
			for( i=0; i<4; i++ ){
				if( i<2 ){
					g3Dmap->object[i].id = 0;
				} else {
					g3Dmap->object[i].id = 1;
				}

				rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
				g3Dmap->object[i].trans.x = rOffs + cOffs;
				g3Dmap->object[i].trans.y = 0;
				rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
				g3Dmap->object[i].trans.z = rOffs + cOffs;

				VEC_Add( &g3Dmap->object[i].trans, &g3Dmap->trans, &pWorld );
				GFL_G3D_MAP_GetAttr( &attrInfo, g3Dmap, &pWorld, mapWidth );
				g3Dmap->object[i].trans.y = attrInfo.height;
			}
			for( i=0; i<32; i++ ){
				g3Dmap->directDrawObject[i].id = 0;

				rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
				g3Dmap->directDrawObject[i].trans.x = rOffs + cOffs;
				g3Dmap->directDrawObject[i].trans.y = 0;
				rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
				g3Dmap->directDrawObject[i].trans.z = rOffs + cOffs;
				VEC_Add( &g3Dmap->directDrawObject[i].trans, &g3Dmap->trans, &pWorld );
				GFL_G3D_MAP_GetAttr( &attrInfo, g3Dmap, &pWorld, mapWidth );
				g3Dmap->directDrawObject[i].trans.y = attrInfo.height;
			}
		}
		//--------------------
#endif
		break;
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

void	GFL_G3D_MAP_Draw( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera,
							const GFL_G3D_MAP_OBJ* obj, const GFL_G3D_MAP_DDOBJ* ddobj )
{
	GF_ASSERT( g3Dmap );

	//地形描画
	if( DrawGround( g3Dmap, g3Dcamera ) == TRUE ){
		//配置オブジェクト描画
		if( obj != NULL ){
			DrawObj( g3Dmap, obj, g3Dcamera );
		}
		if( ddobj != NULL ){
			DirectDrawObj( g3Dmap, ddobj, g3Dcamera );
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
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロードリクエスト設定
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_SetLoadReq( GFL_G3D_MAP* g3Dmap,
								const u32 datID, const u32 texID, const u32 attrID )
{
	GF_ASSERT( g3Dmap );
	GF_ASSERT( g3Dmap->arc );

	if( g3Dmap->texID != texID ){
		g3Dmap->texID = texID;
		g3Dmap->texLoadReq = TRUE;
	}
	g3Dmap->attrID = attrID;
	g3Dmap->attrLoadReq = TRUE;
	g3Dmap->mdlID = datID;
	g3Dmap->mdlLoadReq = TRUE;

	g3Dmap->loadCount = 0;
	g3Dmap->loadSeq = MDL_LOAD_START;
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
static void	DrawObj
	( GFL_G3D_MAP* g3Dmap, const GFL_G3D_MAP_OBJ* obj, GFL_G3D_CAMERA* g3Dcamera )
{
	NNSG3dRenderObj	*NNSrnd, *NNSrnd_L;
	VecFx32			grobalTrans;
	fx32			length;
	int				i;

	for( i=0; i<OBJ_COUNT; i++ ){
		if(	g3Dmap->object[i].id != OBJID_NULL ){

			VEC_Add( &g3Dmap->object[i].trans, &g3Dmap->trans, &grobalTrans );

			getViewLength( g3Dcamera, &grobalTrans, &length );

			if( length <= DRAW_LIMIT ){
	
				NNSrnd = obj[ g3Dmap->object[i].id ].NNSrnd_H;
				NNSrnd_L = obj[ g3Dmap->object[i].id ].NNSrnd_L;

				if( ( length > LOD_LIMIT )&&( NNSrnd_L != NULL ) ){
					NNSrnd = NNSrnd_L;
				}

				if( NNS_G3dRenderObjGetResMdl( NNSrnd ) != NULL ){
	
					NNS_G3dGlbSetBaseTrans( &grobalTrans );		// 位置設定
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
static void	DirectDrawObj
	( GFL_G3D_MAP* g3Dmap, const GFL_G3D_MAP_DDOBJ* ddobj, GFL_G3D_CAMERA* g3Dcamera )
{
	const GFL_G3D_MAP_DDOBJ*	objData;
	MAP_OBJECT_DATA*			ddObject;
	MtxFx33		mtxBillboard;
	VecFx32		trans, grobalTrans;
	VecFx16		vecView;
	fx32		length;
	int			i;

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

		if( ddObject->id != OBJID_NULL ){
			objData = &ddobj[ ddObject->id ];

			VEC_Add( &ddObject->trans, &g3Dmap->trans, &grobalTrans );

			getViewLength( g3Dcamera, &grobalTrans, &length );

			if( length <= DRAW_LIMIT ){
				G3_PushMtx();

				//スケール設定
				G3_Scale(	FX32_ONE * objData->data->scaleVal, 
							FX32_ONE * objData->data->scaleVal, 
							FX32_ONE * objData->data->scaleVal );
				//回転、平行移動パラメータ設定
				trans.x = grobalTrans.x / objData->data->scaleVal;
				trans.y = grobalTrans.y / objData->data->scaleVal;
				trans.z = grobalTrans.z / objData->data->scaleVal;
				if( objData->data->drawType == DRAW_YBILLBOARD ){
					G3_MultTransMtx33( &mtxBillboard, &trans );
				} else {
					G3_Translate( trans.x, trans.y, trans.z );
				}
				//マテリアル設定
				G3_MaterialColorDiffAmb( objData->data->diffuse, objData->data->ambient, TRUE );
				G3_MaterialColorSpecEmi( objData->data->specular, objData->data->emission, FALSE );
				G3_PolygonAttr(	objData->data->lightMask, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 
							objData->data->polID, objData->data->alpha, GX_POLYGON_ATTR_MISC_FOG );

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
	VEC_Fx16Set( &attrInfo->vecN, 0, 0, 0 );
	attrInfo->attr = 0;
	attrInfo->height = 0;
}

//------------------------------------------------------------------
/**
 * @brief	アトリビュート取得
 */
//------------------------------------------------------------------
void GFL_G3D_MAP_GetAttr( GFL_G3D_MAP_ATTRINFO* attrInfo, const GFL_G3D_MAP* g3Dmap,
							const VecFx32* pos, const fx32 map_width )
{
	const VecFx32*	mapTrans;
	fx32			block_hw, grid_w, block_x, block_z, grid_x, grid_z;
	u32				grid_idx;
	VecFx32			vecN;
	fx32			by, valD;
	NormalVtxSt*	nvs;
	u8				triangleType;

	if( g3Dmap->attrLoadReq == TRUE ){
		VEC_Fx16Set( &attrInfo->vecN, 0, 0, 0 );
		attrInfo->height = 0;
		attrInfo->attr = 0;
		return;
	}
	mapTrans = &g3Dmap->trans;
	block_hw = map_width / 2;			//マップ幅の半分
	grid_w = map_width / MAP_GRIDCOUNT;	//マップ幅をグリッド数で分割

	//ブロック内情報取得
	block_x = pos->x - ( mapTrans->x - block_hw );
	block_z = pos->z - ( mapTrans->z - block_hw );

	//グリッド内情報取得
	grid_idx = ( block_z / grid_w ) * MAP_GRIDCOUNT + ( block_x / grid_w );
	grid_x = block_x % grid_w;
	grid_z = block_z % grid_w;

	//情報取得(軸の取り方が違うので法線ベクトルはZ反転)
	nvs = (NormalVtxSt*)(g3Dmap->attr + grid_idx * sizeof(NormalVtxSt));

	//グリッド内三角形の判定
	if( nvs->tryangleType == 0 ){
		//0-2-1,3-1-2のパターン
		if( grid_x + grid_z < grid_w ){
			triangleType = 0;
		} else {
			triangleType = 1;
		}
	} else {
		//2-3-0,1-0-3のパターン
		if( grid_x > grid_z ){
			triangleType = 0;
		} else {
			triangleType = 1;
		}
	}
	if( triangleType == 0 ){
		VEC_Fx16Set( &attrInfo->vecN, nvs->vecN1_x, nvs->vecN1_y, -nvs->vecN1_z );
		valD = nvs->vecN1_D;
	} else {
		VEC_Fx16Set( &attrInfo->vecN, nvs->vecN2_x, nvs->vecN2_y, -nvs->vecN2_z );
		valD = nvs->vecN2_D;
	}
	VEC_Set( &vecN, attrInfo->vecN.x, attrInfo->vecN.y, attrInfo->vecN.z );
	by = -( FX_Mul(vecN.x, pos->x - mapTrans->x) + FX_Mul(vecN.z, pos->z - mapTrans->z) + valD );
	attrInfo->attr = nvs->attr;

	attrInfo->height = FX_Div( by, attrInfo->vecN.y ) + mapTrans->y;
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
	}
	for( i=0; i<DDOBJ_COUNT; i++ ){
		g3Dmap->directDrawObject[i].id = OBJID_NULL;
		VEC_Set( &g3Dmap->directDrawObject[i].trans, 0, 0, 0 );
	}
}

static void	InitWork( GFL_G3D_MAP* g3Dmap )
{
	g3Dmap->drawSw = FALSE;
	VEC_Set( &g3Dmap->trans, 0, 0, 0 );
	g3Dmap->loadSeq = LOAD_IDLING;
	g3Dmap->loadCount = 0;

	g3Dmap->mdlLoadReq = FALSE;
	g3Dmap->mdlID = 0;
	g3Dmap->texLoadReq = FALSE;
	g3Dmap->texID = 0;
	g3Dmap->attrLoadReq = FALSE;
	g3Dmap->attrID = 0;

	InitMapObject( g3Dmap );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップレンダー作成
 */
//------------------------------------------------------------------
static void MakeMapRender( NNSG3dRenderObj* rndObj, GFL_G3D_RES* mdl, GFL_G3D_RES* tex )
{
	NNSG3dResFileHeader*	mdlFH = GFL_G3D_GetResourceFileHeader( mdl );
	NNSG3dResFileHeader*	texFH = GFL_G3D_GetResourceFileHeader( tex );
	//モデル＆テクスチャリソースポインタ取得
	NNSG3dResMdl*			pMdl = NNS_G3dGetMdlByIdx( NNS_G3dGetMdlSet( mdlFH ), 0 );	//0th mdl
	NNSG3dResTex*			pTex = NNS_G3dGetTex( texFH );

	GF_ASSERT(pMdl);

	//モデルとテクスチャリソースとの関連付け
	if( pTex ){
		NNS_G3dBindMdlTex( pMdl, pTex );
		NNS_G3dBindMdlPltt( pMdl, pTex );
	}
	//レンダリングオブジェクト初期化
	NNS_G3dRenderObjInit( rndObj, pMdl );
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


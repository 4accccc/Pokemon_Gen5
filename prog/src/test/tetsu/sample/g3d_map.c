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

typedef struct {
	u16			seq;
	u16			loadCount;
	u32			mOffs;
	u32			fOffs;
	u32			fSize;
	u32			tOffs;
	u32			tSize;

	BOOL		mdlLoaded;
	BOOL		texLoaded;
	BOOL		attrLoaded;
}LOAD_STATUS;

struct _GFL_G3D_MAP 
{
	BOOL				drawSw;
	VecFx32				trans;
	ARCHANDLE*			arc;

	u32					mapDataHeapSize;
	u32					texVramSize;

	NNSG3dRenderObj*	NNSrnd;	//地形レンダー

	u32					mdlID;
	GFL_G3D_RES*		groundResMdl;	//地形モデルリソース
	u8*					mdl;

	u32					texID;
	GFL_G3D_RES*		groundResTex;	//ローカル地形テクスチャリソース
	u8*					tex;

	u32					attrID;
	u8*					attr;

	NNSGfdTexKey		groundTexKey;
	NNSGfdPlttKey		groundPlttKey;

	MAP_OBJECT_DATA		object[OBJ_COUNT];	//配置オブジェクト
	MAP_OBJECT_DATA		directDrawObject[DDOBJ_COUNT];

	LOAD_STATUS			ldst;
	void*				mapData;

	GFL_G3D_RES*		grobalResTex;	//グローバル地形テクスチャリソース
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
static void		StartFileLoad( GFL_G3D_MAP* g3Dmap, u32 datID );
static BOOL		ContinueFileLoad( GFL_G3D_MAP* g3Dmap );
static void		SetTransVramParam( GFL_G3D_MAP* g3Dmap );
static BOOL		TransVram( GFL_G3D_MAP* g3Dmap );
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
	g3Dmap->grobalResTex = NULL;

	//テクスチャ＆パレットＶＲＡＭ確保
	g3Dmap->texVramSize = setup->texVramSize;
	g3Dmap->groundTexKey = NNS_GfdAllocTexVram( setup->texVramSize, FALSE, 0 );
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
	GF_ASSERT( g3Dmap );

	switch( g3Dmap->ldst.seq ){
	case LOAD_IDLING:
		break;

	case MDL_LOAD_START:
		TrashMapRender( g3Dmap->NNSrnd );
		InitMapObject( g3Dmap );
		g3Dmap->ldst.mOffs = 0;
		g3Dmap->ldst.tOffs = 0;
		g3Dmap->ldst.mdlLoaded = FALSE;
		g3Dmap->ldst.texLoaded = FALSE;
		g3Dmap->ldst.attrLoaded = FALSE;

		//モデルデータロード開始
		g3Dmap->mdl = (u8*)((u32)g3Dmap->mapData + g3Dmap->ldst.mOffs);

		StartFileLoad( g3Dmap, g3Dmap->mdlID );
		g3Dmap->ldst.seq = MDL_LOAD;
		break;
	case MDL_LOAD:
		if( ContinueFileLoad(g3Dmap) == FALSE ){
			g3Dmap->ldst.mdlLoaded = TRUE;

			GFL_G3D_CreateResource
				( g3Dmap->groundResMdl, GFL_G3D_RES_CHKTYPE_MDL,(void*)g3Dmap->mdl );

			g3Dmap->ldst.seq = TEX_LOAD_START;
		}
		break;

	case TEX_LOAD_START:
		if( g3Dmap->texID == NON_TEX ){
			g3Dmap->tex = NULL;
			g3Dmap->ldst.seq = RND_CREATE;
		}
		//テクスチャロード開始
		g3Dmap->tex = (u8*)((u32)g3Dmap->mapData + g3Dmap->ldst.mOffs);

		StartFileLoad( g3Dmap, g3Dmap->texID );
		g3Dmap->ldst.seq = TEX_LOAD;
		break;
	case TEX_LOAD:
		if( ContinueFileLoad(g3Dmap) == FALSE ){
			g3Dmap->ldst.texLoaded = TRUE;

			GFL_G3D_CreateResource
				( g3Dmap->groundResTex, GFL_G3D_RES_CHKTYPE_TEX, (void*)g3Dmap->tex );

			//テクスチャリソースへのＶＲＡＭキーの設定
			NNS_G3dTexSetTexKey( GFL_G3D_GetResTex(g3Dmap->groundResTex), g3Dmap->groundTexKey, 0 );
			NNS_G3dPlttSetPlttKey( GFL_G3D_GetResTex(g3Dmap->groundResTex), g3Dmap->groundPlttKey );
			SetTransVramParam( g3Dmap );	//テクスチャ転送設定

			g3Dmap->ldst.seq = TEX_TRANS;
		}
		break;

	case TEX_TRANS:
		if( TransVram(g3Dmap) == FALSE ){
			g3Dmap->ldst.seq = RND_CREATE;
		}
		break;

	case RND_CREATE:
		//レンダー作成
		if( g3Dmap->texID == NON_TEX ){
			MakeMapRender( g3Dmap->NNSrnd, g3Dmap->groundResMdl, g3Dmap->grobalResTex );
		} else {
			MakeMapRender( g3Dmap->NNSrnd, g3Dmap->groundResMdl, g3Dmap->groundResTex );
		}
		g3Dmap->ldst.seq = ATTR_LOAD_START;
		break;

	case ATTR_LOAD_START:
		if( g3Dmap->attrID == NON_ATTR ){
			g3Dmap->attr = NULL;
			g3Dmap->ldst.seq = LOAD_END;
			break;
		}
		//アトリビュートデータロード開始
		g3Dmap->attr = (u8*)((u32)g3Dmap->mapData + g3Dmap->ldst.mOffs);

		StartFileLoad( g3Dmap, g3Dmap->attrID );
		g3Dmap->ldst.seq = ATTR_LOAD;
		break;
	case ATTR_LOAD:
		if( ContinueFileLoad(g3Dmap) == FALSE ){
			g3Dmap->ldst.attrLoaded = TRUE;
			g3Dmap->ldst.seq = LOAD_END;
		}
		break;

	case LOAD_END:
//		OS_Printf(" mapDataTop(%x), mdl(%x), tex(%x), attr(%x)\n ", 
//					g3Dmap->mapData, g3Dmap->mdl, g3Dmap->tex, g3Dmap->attr );
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
		g3Dmap->ldst.seq = LOAD_IDLING;
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
 * @brief	３Ｄマップグローバルテクスチャリソース登録
 */
//------------------------------------------------------------------
void	GFL_G3D_MAP_ResistGrobalTex( GFL_G3D_MAP* g3Dmap, GFL_G3D_RES* grobalResTex )
{
	g3Dmap->grobalResTex = grobalResTex;
}

void	GFL_G3D_MAP_ReleaseGrobalTex( GFL_G3D_MAP* g3Dmap )
{
	g3Dmap->grobalResTex = NULL;
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

	g3Dmap->mdlID = datID;
	g3Dmap->texID = texID;
	g3Dmap->attrID = attrID;

	g3Dmap->ldst.seq = MDL_LOAD_START;
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

	if( g3Dmap->ldst.attrLoaded == FALSE ){
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

	g3Dmap->ldst.seq = LOAD_IDLING;
	g3Dmap->mdlID = 0;
	g3Dmap->texID = NON_TEX;
	g3Dmap->attrID = NON_ATTR;

	InitMapObject( g3Dmap );
}

//------------------------------------------------------------------
/**
 * @brief	読み込み関数
 */
//------------------------------------------------------------------
static void StartFileLoad( GFL_G3D_MAP* g3Dmap, u32 datID )
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

static BOOL ContinueFileLoad( GFL_G3D_MAP* g3Dmap )
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
 * @brief	転送関数
 */
//------------------------------------------------------------------
static void SetTransVramParam( GFL_G3D_MAP* g3Dmap )
{
	//g3Dmap->ldst.tSize = g3Dmap->ldst.fSize;
	g3Dmap->ldst.tSize = g3Dmap->texVramSize;	//テクスチャ転送量設定
	g3Dmap->ldst.tOffs = 0;
}

static BOOL TransVram( GFL_G3D_MAP* g3Dmap )
{
	void*	src;
	u32		dst;
	u32		rest;

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


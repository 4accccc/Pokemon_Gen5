//============================================================================================
/**
 * @file	g3d_mapper.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "include\system\gfl_use.h"	//乱数用

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
#define MAP_BLOCK_COUNT		(9)
#define MAPID_NULL			(0xffffffff)
#define MAPARC_NULL			(0xffffffff)
#define LOCAL_OBJID_NULL	(0xffffffff)
#define DDRAW_OBJID_NULL	(0xffffffff)

#define GROBAL_OBJ_COUNT	(32)
#define GROBAL_DDOBJ_COUNT	(32)
#define COMMON_OBJ_RESCOUNT	(64)
#define LOCAL_OBJRES_COUNT	(16)
#define LOCAL_OBJRND_COUNT	(32)
#define LOCAL_DIRECTDRAW_OBJ_COUNT	(64)

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
//#define MAP_BLOCK_LEN		(FX32_ONE * 256 * 2 )

typedef struct {
	u32			blockIdx;
	VecFx32		trans;
}BLOCK_IDX;

typedef struct {
	u32			id;
	VecFx32		trans;
}MAP_OBJECT_DATA;

typedef struct {
	u32			id;
	VecFx32		trans;
}DIRECT_OBJ;

typedef struct {
	NNSG3dRenderObj*	NNSrnd_H;	//レンダー(High Q)
	GFL_G3D_RES*		g3Dres_H;	//リソース(High Q)
	NNSG3dRenderObj*	NNSrnd_L;	//レンダー(Low Q)
	GFL_G3D_RES*		g3Dres_L;	//リソース(High Q)
}MAP_GROBAL_OBJECT_DATA;

typedef struct {
	GFL_G3D_RES*	g3Dres;
	u32				texDataAdrs;
	u32				texPlttAdrs;
}MAP_GROBAL_DDOBJECT_DATA;

typedef struct {
	u32				idx;
	VecFx32			trans;
	ARCHANDLE*		arc;

	u32				loadSeq;
	u32				loadCount;

	NNSG3dRenderObj*	NNSrnd;	//地形レンダー

	BOOL			dataLoadReq;
	u32				dataID;
	GFL_G3D_RES*	g3DgroundResMdl;	//地形モデルリソース
	u8				data[MAPDATA_SIZE];

	BOOL			texLoadReq;
	u32				texID;
	GFL_G3D_RES*	g3DgroundResTex;	//ローカル地形テクスチャリソース
	u8				tex[MAPTEX_SIZE];

	BOOL			attrLoadReq;
	u32				attrID;
	u8				attr[MAPATTR_SIZE];

	NNSGfdTexKey	groundTexKey;
	NNSGfdPlttKey	groundPlttKey;

	MAP_OBJECT_DATA	object[LOCAL_OBJRND_COUNT];	//配置オブジェクト
#if 0
	BOOL			objLoadReq;
	u32				objID;
	GFL_G3D_RES*	g3DobjectResMdl[LOCAL_OBJRES_COUNT];//ローカルオブジェクトモデルリソース
	u8				obj[MAPOBJ_SIZE];

	NNSGfdTexKey	objectTexKey;
	NNSGfdPlttKey	objectPlttKey;
#endif
	DIRECT_OBJ		directDrawObject[LOCAL_DIRECTDRAW_OBJ_COUNT];
}MAP_BLOCK_DATA;

struct _G3D_MAPPER {
	HEAPID				heapID;
	MAP_BLOCK_DATA		mapBlock[MAP_BLOCK_COUNT];

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

	MAP_GROBAL_OBJECT_DATA		grobalObject[GROBAL_OBJ_COUNT];	//共通オブジェクト
	MAP_GROBAL_DDOBJECT_DATA	grobalDDobject[GROBAL_DDOBJ_COUNT];	//共通オブジェクト(直書き)
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

//------------------------------------------------------------------
static void	Load3Dmap( G3D_MAPPER* g3Dmapper );

static void	CreateMapGraphicResource( G3D_MAPPER* g3Dmapper );
static void	DeleteMapGraphicResource( G3D_MAPPER* g3Dmapper );
static void	InitMapGraphicResource( MAP_BLOCK_DATA*	mapBlock );

static void MakeMapRender( NNSG3dRenderObj* rndObj, GFL_G3D_RES* mdl, GFL_G3D_RES* tex );

static void GetMapperBlockIdxAll( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx );
static void GetMapperBlockIdxXZ( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx );
static void GetMapperBlockIdxY( G3D_MAPPER* g3Dmapper, const VecFx32* pos, BLOCK_IDX* blockIdx );
static BOOL	ReloadMapperBlock( G3D_MAPPER* g3Dmapper, BLOCK_IDX* new );

static void	DirectDraw3Dmapper
	( G3D_MAPPER* g3Dmapper, MAP_BLOCK_DATA* mapBlock, GFL_G3D_CAMERA* g3Dcamera );

//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
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

	CreateMapGraphicResource( g3Dmapper );

	VEC_Set( &g3Dmapper->posCont, 0, 0, 0 );
	g3Dmapper->sizex = 0;
	g3Dmapper->sizez = 0;
	g3Dmapper->totalSize = 0;
	g3Dmapper->width = 0;
	g3Dmapper->height = 0;
	g3Dmapper->mode = G3D_MAPPER_MODE_SCROLL_XZ;
	g3Dmapper->arcID = MAPARC_NULL;
	g3Dmapper->data = NULL;
	
	for( i=0; i<GROBAL_OBJ_COUNT; i++ ){
		g3Dmapper->grobalObject[i].NNSrnd_H = NULL;
		g3Dmapper->grobalObject[i].g3Dres_H = NULL;
		g3Dmapper->grobalObject[i].NNSrnd_L = NULL;
		g3Dmapper->grobalObject[i].g3Dres_L = NULL;
	}
	for( i=0; i<GROBAL_DDOBJ_COUNT; i++ ){
		g3Dmapper->grobalDDobject[i].g3Dres = NULL;
		g3Dmapper->grobalDDobject[i].texDataAdrs = 0;
		g3Dmapper->grobalDDobject[i].texPlttAdrs = 0;
	}

	OS_Printf("g3DmapperSize = %x\n", sizeof(G3D_MAPPER));
	OS_Printf("mapBlockDataSize = %x\n", sizeof(MAP_BLOCK_DATA));
	OS_Printf("rndObjSize = %x\n", sizeof(NNSG3dRenderObj));
	
	return g3Dmapper;
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
	Load3Dmap( g3Dmapper );

	//現在ブロックのindex取得
	{
		u32 blockx = FX_Whole( FX_Div( g3Dmapper->posCont.x, g3Dmapper->width ) );
		u32 blockz = FX_Whole( FX_Div( g3Dmapper->posCont.z, g3Dmapper->width ) );

		g3Dmapper->nowBlockIdx = blockz * g3Dmapper->sizex + blockx;
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムディスプレイ
 */
static BOOL checkCullingBoxTest( NNSG3dRenderObj* rnd );
static BOOL checkCullingLength( GFL_G3D_CAMERA* g3Dcamera, VecFx32* pos );
static fx32 getCullingLength( GFL_G3D_CAMERA* g3Dcamera, VecFx32* pos );
//------------------------------------------------------------------
void	Draw3Dmapper( G3D_MAPPER* g3Dmapper, GFL_G3D_CAMERA* g3Dcamera )
{
	MAP_BLOCK_DATA*		mapBlock;
	BOOL				cullResult; 
	VecFx32				trans;
	VecFx32				scale = { FX32_ONE, FX32_ONE, FX32_ONE };
	MtxFx33				rotate = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };
	NNSG3dRenderObj		*NNSrnd, *NNSrnd_L;
	int					i, j;
	fx32				length, limitLength = 512 * FX32_ONE;

	GF_ASSERT( g3Dmapper );

	//描画開始
	GFL_G3D_DRAW_Start();
	//カメラグローバルステート設定		
 	GFL_G3D_DRAW_SetLookAt();

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &g3Dmapper->mapBlock[i];
		if( (mapBlock->idx != MAPID_NULL)
		  &&(NNS_G3dRenderObjGetResMdl( mapBlock->NNSrnd ) != NULL )){
			
			NNS_G3dGlbSetBaseTrans( &mapBlock->trans );	// 位置設定
			NNS_G3dGlbSetBaseRot( &rotate );	// 角度設定
			NNS_G3dGlbSetBaseScale( &scale );	// スケール設定
			NNS_G3dGlbFlush();					//グローバルステート反映

			if( checkCullingBoxTest( mapBlock->NNSrnd ) == TRUE ){
				//地形描画
				NNS_G3dDraw( mapBlock->NNSrnd );	

				//配置オブジェクト描画
				for( j=0; j<LOCAL_OBJRND_COUNT; j++ ){
					if(	mapBlock->object[j].id != LOCAL_OBJID_NULL ){
//						if( checkCullingLength(g3Dcamera, &mapBlock->object[j].trans) == TRUE){
							VEC_Add( &mapBlock->object[j].trans, &mapBlock->trans, &trans );

							length = getCullingLength( g3Dcamera, &trans );
				
							NNSrnd = g3Dmapper->grobalObject[ mapBlock->object[j].id ].NNSrnd_H;
							NNSrnd_L = g3Dmapper->grobalObject[ mapBlock->object[j].id ].NNSrnd_L;

							if( ( length > limitLength )&&( NNSrnd_L != NULL ) ){
								NNSrnd = NNSrnd_L;
							}

//							if( ( checkCullingBoxTest( NNSrnd ) == TRUE )
//								&&( NNS_G3dRenderObjGetResMdl( NNSrnd ) != NULL ) ){
							if( NNS_G3dRenderObjGetResMdl( NNSrnd ) != NULL ){
	
								NNS_G3dGlbSetBaseTrans( &trans );// 位置設定
								NNS_G3dGlbSetBaseRot( &rotate );		// 角度設定
								NNS_G3dGlbSetBaseScale( &scale );		// スケール設定
								NNS_G3dGlbFlush();						//グローバルステート反映

								//オブジェクト描画
								NNS_G3dDraw( NNSrnd );
							}
//						}
					}
				}
				DirectDraw3Dmapper( g3Dmapper, mapBlock, g3Dcamera );
			}
		}
	}
	//描画終了（バッファスワップ）
	//GFL_G3D_DRAW_End();							
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
// カメラとの距離によるカリングチェック
static fx32 getCullingLength( GFL_G3D_CAMERA* g3Dcamera, VecFx32* pos )
{
	VecFx32 camPos, vecLength;

	GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );

	VEC_Subtract( pos, &camPos, &vecLength );

	return VEC_Mag( &vecLength );
}

static BOOL checkCullingLength( GFL_G3D_CAMERA* g3Dcamera, VecFx32* pos )
{
	fx32 limitLength, length;

	GFL_G3D_CAMERA_GetFar( g3Dcamera, &limitLength );

	length = getCullingLength( g3Dcamera, pos );

	if( length < limitLength ){
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム破棄
 */
//------------------------------------------------------------------
void	Delete3Dmapper( G3D_MAPPER* g3Dmapper )
{
	GF_ASSERT( g3Dmapper );

	ReleaseObjRes3Dmapper( g3Dmapper );	//登録されたままの場合を想定して削除
	DeleteMapGraphicResource( g3Dmapper );

	GFL_HEAP_FreeMemory( g3Dmapper );
}


//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
void ResistData3Dmapper( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST* resistData )
{
	MAP_BLOCK_DATA*	mapBlock;
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

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &g3Dmapper->mapBlock[i];

		//初期化
		InitMapGraphicResource( mapBlock );

		//アーカイブハンドル作成
		if( mapBlock->arc != NULL ){
			GFL_ARC_CloseDataHandle( mapBlock->arc );
		}
		mapBlock->NNSrnd->resMdl = NULL; 
		mapBlock->arc = GFL_ARC_OpenDataHandle( g3Dmapper->arcID, g3Dmapper->heapID );
		GF_ASSERT( mapBlock->arc );
	}
}

//------------------------------------------------------------------
/**
 * @brief	オブジェクトリソース登録
 */
//------------------------------------------------------------------
//通常MDL
void ResistObjRes3Dmapper( G3D_MAPPER* g3Dmapper, const G3D_MAPPEROBJ_RESIST* resistData )
{
	int i;

	GF_ASSERT( g3Dmapper );
	GF_ASSERT( resistData->count <= GROBAL_OBJ_COUNT );
	
	ReleaseObjRes3Dmapper( g3Dmapper );

	for( i=0; i< resistData->count; i++ ){
		g3Dmapper->grobalObject[i].g3Dres_H = GFL_G3D_CreateResourceArc
										( resistData->arcID, resistData->data[i].highQ_ID );
		GFL_G3D_TransVramTexture( g3Dmapper->grobalObject[i].g3Dres_H ); 

		g3Dmapper->grobalObject[i].NNSrnd_H = NNS_G3dAllocRenderObj
												( GFL_G3D_GetAllocaterPointer() );

		MakeMapRender(	g3Dmapper->grobalObject[i].NNSrnd_H,
						g3Dmapper->grobalObject[i].g3Dres_H,
						g3Dmapper->grobalObject[i].g3Dres_H );

		if( resistData->data[i].lowQ_ID != NON_LOWQ ){
			g3Dmapper->grobalObject[i].g3Dres_L = GFL_G3D_CreateResourceArc
											( resistData->arcID, resistData->data[i].lowQ_ID );
			GFL_G3D_TransVramTexture( g3Dmapper->grobalObject[i].g3Dres_L ); 

			g3Dmapper->grobalObject[i].NNSrnd_L = NNS_G3dAllocRenderObj
													( GFL_G3D_GetAllocaterPointer() );

			MakeMapRender(	g3Dmapper->grobalObject[i].NNSrnd_L,
							g3Dmapper->grobalObject[i].g3Dres_L,
							g3Dmapper->grobalObject[i].g3Dres_L );
		}
	}
}

void ReleaseObjRes3Dmapper( G3D_MAPPER* g3Dmapper )
{
	int i;

	GF_ASSERT( g3Dmapper );
	for( i=0; i<GROBAL_OBJ_COUNT; i++ ){
		if( g3Dmapper->grobalObject[i].NNSrnd_L != NULL ){
			NNS_G3dFreeRenderObj(	GFL_G3D_GetAllocaterPointer(), 
									g3Dmapper->grobalObject[i].NNSrnd_L );
			g3Dmapper->grobalObject[i].NNSrnd_L = NULL;
		}
		if( g3Dmapper->grobalObject[i].g3Dres_L != NULL ){
			GFL_G3D_FreeVramTexture( g3Dmapper->grobalObject[i].g3Dres_L );
			GFL_G3D_DeleteResource( g3Dmapper->grobalObject[i].g3Dres_L );
			g3Dmapper->grobalObject[i].g3Dres_L = NULL;
		}
		if( g3Dmapper->grobalObject[i].NNSrnd_H != NULL ){
			NNS_G3dFreeRenderObj(	GFL_G3D_GetAllocaterPointer(), 
									g3Dmapper->grobalObject[i].NNSrnd_H );
			g3Dmapper->grobalObject[i].NNSrnd_H = NULL;
		}
		if( g3Dmapper->grobalObject[i].g3Dres_H != NULL ){
			GFL_G3D_FreeVramTexture( g3Dmapper->grobalObject[i].g3Dres_H );
			GFL_G3D_DeleteResource( g3Dmapper->grobalObject[i].g3Dres_H );
			g3Dmapper->grobalObject[i].g3Dres_H = NULL;
		}
	}
}

//------------------------------------------------------------------
//DirectDraw
void ResistDDobjRes3Dmapper( G3D_MAPPER* g3Dmapper, const G3D_MAPPERDDOBJ_RESIST* resistData )
{
	GFL_G3D_RES*	g3DresTex;
	NNSG3dTexKey	texDataKey;
	NNSG3dPlttKey	texPlttKey;
	int i;

	GF_ASSERT( g3Dmapper );
	GF_ASSERT( resistData->count <= GROBAL_DDOBJ_COUNT );
	
	ReleaseDDobjRes3Dmapper( g3Dmapper );

	for( i=0; i< resistData->count; i++ ){
		g3DresTex = GFL_G3D_CreateResourceArc( resistData->arcID, resistData->data[i] );

		g3Dmapper->grobalDDobject[i].g3Dres = g3DresTex;
										
		if( GFL_G3D_TransVramTexture( g3DresTex ) == FALSE ){
			GF_ASSERT(0);
		}
		texDataKey = GFL_G3D_GetTextureDataVramKey( g3DresTex );
		texPlttKey = GFL_G3D_GetTexturePlttVramKey( g3DresTex );

		g3Dmapper->grobalDDobject[i].texDataAdrs = NNS_GfdGetTexKeyAddr( texDataKey );
		g3Dmapper->grobalDDobject[i].texPlttAdrs = NNS_GfdGetPlttKeyAddr( texPlttKey );
	}
}

void ReleaseDDobjRes3Dmapper( G3D_MAPPER* g3Dmapper )
{
	int i;

	GF_ASSERT( g3Dmapper );
	for( i=0; i<GROBAL_DDOBJ_COUNT; i++ ){
		if( g3Dmapper->grobalDDobject[i].g3Dres != NULL ){
			GFL_G3D_FreeVramTexture( g3Dmapper->grobalDDobject[i].g3Dres );
			GFL_G3D_DeleteResource( g3Dmapper->grobalDDobject[i].g3Dres );
			g3Dmapper->grobalDDobject[i].g3Dres = NULL;
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
 * @brief	３Ｄマップデータセットアップ
 */
//------------------------------------------------------------------
static void	CreateMapGraphicResource( G3D_MAPPER* g3Dmapper )
{
	MAP_BLOCK_DATA*	mapBlock;
	int				i;
	u32				g3DresHeaderSize = GFL_G3D_GetResourceHeaderSize();

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &g3Dmapper->mapBlock[i];

		//初期化
		InitMapGraphicResource( mapBlock );

		mapBlock->NNSrnd = NNS_G3dAllocRenderObj( GFL_G3D_GetAllocaterPointer() );
		mapBlock->NNSrnd->resMdl = NULL; 

		//アーカイブハンドル作成
		//mapBlock->arc = GFL_ARC_OpenDataHandle( g3Dmapper->arcID, g3Dmapper->heapID );
		//GF_ASSERT( mapBlock->arc );
		mapBlock->arc = NULL;
		//モデルリソースヘッダ作成
		mapBlock->g3DgroundResMdl = GFL_HEAP_AllocClearMemory(g3Dmapper->heapID,g3DresHeaderSize);
		//テクスチャリソースヘッダ作成
		mapBlock->g3DgroundResTex = GFL_HEAP_AllocClearMemory(g3Dmapper->heapID,g3DresHeaderSize);

		//モデルリソースハンドル内容設定
		GFL_G3D_CreateResource( mapBlock->g3DgroundResMdl, 
								GFL_G3D_RES_CHKTYPE_MDL,(void*)&mapBlock->data[0] );
		//テクスチャリソースハンドル内容設定
		GFL_G3D_CreateResource( mapBlock->g3DgroundResTex, 
								GFL_G3D_RES_CHKTYPE_TEX, (void*)&mapBlock->tex[0] );

		//テクスチャＶＲＡＭ確保
		mapBlock->groundTexKey = NNS_GfdAllocTexVram( MAPTEX_SIZE, FALSE, 0 );
		//パレットＶＲＡＭ確保
		mapBlock->groundPlttKey = NNS_GfdAllocPlttVram( MAPPLTT_SIZE, FALSE, 0 );

		GF_ASSERT( mapBlock->groundTexKey != NNS_GFD_ALLOC_ERROR_TEXKEY );
		GF_ASSERT( mapBlock->groundPlttKey != NNS_GFD_ALLOC_ERROR_PLTTKEY );
	}
}

//------------------------------------------------------------------
static void	DeleteMapGraphicResource( G3D_MAPPER* g3Dmapper )
{
	MAP_BLOCK_DATA*	mapBlock;
	int				i, j;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &g3Dmapper->mapBlock[i];

		NNS_G3dFreeRenderObj( GFL_G3D_GetAllocaterPointer(), mapBlock->NNSrnd );

		NNS_GfdFreePlttVram( mapBlock->groundPlttKey );
		NNS_GfdFreeTexVram( mapBlock->groundTexKey );

		GFL_HEAP_FreeMemory( mapBlock->g3DgroundResTex );
		GFL_HEAP_FreeMemory( mapBlock->g3DgroundResMdl );
		if( mapBlock->arc != NULL ){
			GFL_ARC_CloseDataHandle( mapBlock->arc );
		}
	}
}

//------------------------------------------------------------------
static void	InitMapGraphicData( MAP_BLOCK_DATA*	mapBlock )
{
	int i;

	for( i=0; i<LOCAL_OBJRND_COUNT; i++ ){
		mapBlock->object[i].id = LOCAL_OBJID_NULL;
		VEC_Set( &mapBlock->object[i].trans, 0, 0, 0 );
	}
	for( i=0; i<LOCAL_DIRECTDRAW_OBJ_COUNT; i++ ){
		mapBlock->directDrawObject[i].id = DDRAW_OBJID_NULL;
		VEC_Set( &mapBlock->directDrawObject[i].trans, 0, 0, 0 );
	}
}

static void	InitMapGraphicResource( MAP_BLOCK_DATA*	mapBlock )
{
	mapBlock->idx = MAPID_NULL;
	VEC_Set( &mapBlock->trans, 0, 0, 0 );
	mapBlock->loadSeq = LOAD_IDLING;
	mapBlock->loadCount = 0;

	mapBlock->dataLoadReq = FALSE;
	mapBlock->dataID = 0;
	mapBlock->texLoadReq = FALSE;
	mapBlock->texID = 0;
	mapBlock->attrLoadReq = FALSE;
	mapBlock->attrID = 0;

	InitMapGraphicData( mapBlock );
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

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロード
 */
//------------------------------------------------------------------
static void	Load3Dmap( G3D_MAPPER* g3Dmapper )
{
	MAP_BLOCK_DATA*	mapBlock;
	int				i;
	u32				dataAdrs;

	GF_ASSERT( g3Dmapper );

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &g3Dmapper->mapBlock[i];

		switch( mapBlock->loadSeq ){
		case LOAD_IDLING:
			break;

		case MDL_LOAD_START:
			mapBlock->NNSrnd->resMdl = NULL; 
			InitMapGraphicData( mapBlock );

			//モデルデータロード開始
			GFL_ARC_LoadDataOfsByHandle( mapBlock->arc, mapBlock->dataID, 
											0, MAPLOAD_SIZE, (void*)mapBlock->data );
			mapBlock->loadCount = 0;
			mapBlock->loadCount++;
			mapBlock->loadSeq = MDL_LOAD;
			break;
		case MDL_LOAD:
			dataAdrs = mapBlock->loadCount * MAPLOAD_SIZE;

			if( (dataAdrs + MAPLOAD_SIZE) < MAPDATA_SIZE){
				//ロード継続
				GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPLOAD_SIZE, 
													(void*)&mapBlock->data[dataAdrs] );
				mapBlock->loadCount++;
				break;
			}
			//最終ロード
			GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPDATA_SIZE - dataAdrs,
												(void*)&mapBlock->data[dataAdrs] );
			mapBlock->dataLoadReq = FALSE;
			mapBlock->loadSeq = TEX_LOAD_START;
			break;
		case TEX_LOAD_START:
			if( mapBlock->texLoadReq == FALSE ){
				mapBlock->loadSeq = RND_CREATE;
				break;
			}
			//テクスチャロード開始
			GFL_ARC_LoadDataOfsByHandle( mapBlock->arc, mapBlock->texID, 
											0, MAPLOAD_SIZE, (void*)mapBlock->tex );
			mapBlock->loadCount = 0;
			mapBlock->loadCount++;
			mapBlock->loadSeq = TEX_LOAD;
			break;
		case TEX_LOAD:
			dataAdrs = mapBlock->loadCount * MAPLOAD_SIZE;

			if( (dataAdrs + MAPLOAD_SIZE) < MAPTEX_SIZE){
				//ロード継続
				GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPLOAD_SIZE, 
													(void*)&mapBlock->tex[dataAdrs] );
				mapBlock->loadCount++;
				break;
			}
			//最終ロード
			GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPTEX_SIZE - dataAdrs,
												(void*)&mapBlock->tex[dataAdrs] );
			mapBlock->texLoadReq = FALSE;
			//テクスチャリソースへのＶＲＡＭキーの設定
			NNS_G3dTexSetTexKey( GFL_G3D_GetResTex(mapBlock->g3DgroundResTex), 
									mapBlock->groundTexKey, 0 );
			NNS_G3dPlttSetPlttKey( GFL_G3D_GetResTex(mapBlock->g3DgroundResTex), 
									mapBlock->groundPlttKey );
			mapBlock->loadSeq = RND_CREATE;
			break;
		case RND_CREATE:
			//レンダー作成
			MakeMapRender
				( mapBlock->NNSrnd, mapBlock->g3DgroundResMdl, mapBlock->g3DgroundResTex );
			mapBlock->loadCount = 0;
			mapBlock->loadSeq = TEX_TRANS;
			break;
		case TEX_TRANS:
			dataAdrs = mapBlock->loadCount * MAPTRANS_SIZE;

			if( (dataAdrs + MAPTRANS_SIZE) < MAPTEX_SIZE){
				NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, 
					NNS_GfdGetTexKeyAddr( mapBlock->groundTexKey ) + dataAdrs,
					(void*)(GFL_G3D_GetAdrsTextureData( mapBlock->g3DgroundResTex ) + dataAdrs),
					MAPTRANS_SIZE );
				//転送継続
				mapBlock->loadCount++;
				break;
			}
			//最終転送
			NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, 
					NNS_GfdGetTexKeyAddr( mapBlock->groundTexKey ) + dataAdrs,
					(void*)(GFL_G3D_GetAdrsTextureData( mapBlock->g3DgroundResTex ) + dataAdrs),
					MAPTEX_SIZE - dataAdrs );
			mapBlock->loadSeq = PLTT_TRANS;
			break;
		case PLTT_TRANS:
			NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, 
					NNS_GfdGetPlttKeyAddr( mapBlock->groundPlttKey ),
					(void*)GFL_G3D_GetAdrsTexturePltt( mapBlock->g3DgroundResTex ),
					MAPPLTT_SIZE );
			mapBlock->loadSeq = ATTR_LOAD_START;
			break;
		case ATTR_LOAD_START:
			//アトリビュートデータロード開始
			if( mapBlock->attrID == NON_ATTR ){
				GFL_STD_MemClear32( (void*)mapBlock->attr, MAPATTR_SIZE );
				mapBlock->loadSeq = LOAD_END;
				break;
			}
			GFL_ARC_LoadDataOfsByHandle( mapBlock->arc, mapBlock->attrID, 
											0, MAPLOAD_SIZE, (void*)mapBlock->attr );
			mapBlock->loadCount = 0;
			mapBlock->loadCount++;
			mapBlock->loadSeq = ATTR_LOAD;
			break;
		case ATTR_LOAD:
			dataAdrs = mapBlock->loadCount * MAPLOAD_SIZE;

			if( (dataAdrs + MAPLOAD_SIZE) < MAPATTR_SIZE){
				//ロード継続
				GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPLOAD_SIZE, 
													(void*)&mapBlock->attr[dataAdrs] );
				mapBlock->loadCount++;
				break;
			}
			//最終ロード
			GFL_ARC_LoadDataByHandleContinue(	mapBlock->arc, MAPATTR_SIZE - dataAdrs,
												(void*)&mapBlock->attr[dataAdrs] );
			mapBlock->attrLoadReq = FALSE;
			mapBlock->loadSeq = LOAD_END;
			break;
		case LOAD_END:
#if 1
			//--------------------
			if( mapBlock->trans.y == 0 ){
				int j, r;
				fx32 randTransx, randTransz;
				fx32 cOffs, rOffs, gWidth;
				VecFx32 pWorld;
				G3D_MAPPER_INFODATA gridInfoData;
				BOOL f;

				cOffs = -g3Dmapper->width/2;
				gWidth = g3Dmapper->width/MAP_GRIDCOUNT;
				for( j=0; j<4; j++ ){
					if( j<2 ){
						mapBlock->object[j].id = 0;
					} else {
						mapBlock->object[j].id = 1;
					}

					rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
					mapBlock->object[j].trans.x = rOffs + cOffs;
					mapBlock->object[j].trans.y = 0;
					rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
					mapBlock->object[j].trans.z = rOffs + cOffs;

					VEC_Add( &mapBlock->object[j].trans, &mapBlock->trans, &pWorld );
					Get3DmapperGridInfoData( g3Dmapper, &pWorld, &gridInfoData );
					mapBlock->object[j].trans.y = gridInfoData.height;
				}
				for( j=0; j<32; j++ ){
					mapBlock->directDrawObject[j].id = 0;

					rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
					mapBlock->directDrawObject[j].trans.x = rOffs + cOffs;
					mapBlock->directDrawObject[j].trans.y = 0;
					rOffs = GFUser_GetPublicRand( MAP_GRIDCOUNT ) * gWidth;
					mapBlock->directDrawObject[j].trans.z = rOffs + cOffs;
					VEC_Add( &mapBlock->directDrawObject[j].trans, &mapBlock->trans, &pWorld );
					Get3DmapperGridInfoData( g3Dmapper, &pWorld, &gridInfoData );
					mapBlock->directDrawObject[j].trans.y = gridInfoData.height;
				}
			}
			//--------------------
#endif
			mapBlock->loadSeq = LOAD_IDLING;
			break;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロードリクエスト設定
 */
//------------------------------------------------------------------
static void	Set3DmapLoadReq( G3D_MAPPER* g3Dmapper, const int blockID, const VecFx32* trans, 
								const u32 datID, const u32 texID, const u32 attrID )
{
	MAP_BLOCK_DATA* mapBlock;

	GF_ASSERT( blockID < MAP_BLOCK_COUNT );

	mapBlock = &g3Dmapper->mapBlock[blockID];

	if( mapBlock->texID != texID ){
		mapBlock->texID = texID;
		mapBlock->texLoadReq = TRUE;
	}
	mapBlock->attrID = attrID;
	mapBlock->attrLoadReq = TRUE;
	mapBlock->dataID = datID;
	mapBlock->dataLoadReq = TRUE;

	mapBlock->loadCount = 0;
	mapBlock->loadSeq = MDL_LOAD_START;

	VEC_Set( &mapBlock->trans, trans->x, trans->y, trans->z );
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
		if( g3Dmapper->mapBlock[i].idx != MAPID_NULL ){
			delFlag = FALSE;
			for( j=0; j<MAP_BLOCK_COUNT; j++ ){
				if(( g3Dmapper->mapBlock[i].idx == new[j].blockIdx )&&(delFlag == FALSE )){
					new[j].blockIdx = MAPID_NULL;
					delFlag = TRUE;
				}
			}
			if( delFlag == FALSE ){
				g3Dmapper->mapBlock[i].idx = MAPID_NULL;
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
				if(( g3Dmapper->mapBlock[j].idx == MAPID_NULL )&&(addFlag == FALSE )){

					Set3DmapLoadReq( g3Dmapper, j, &new[i].trans, 
										(u32)g3Dmapper->data[new[i].blockIdx].datID,
										(u32)g3Dmapper->data[new[i].blockIdx].texID,
										(u32)g3Dmapper->data[new[i].blockIdx].attrID );
					g3Dmapper->mapBlock[j].idx = new[i].blockIdx;
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
 * @brief	ジオメトリ直書き関数
 *
 *
 *
 */
//============================================================================================
typedef void (DIRECT_DRAW_FUNC)( u32 texDataAdrs, u32 texPlttAdrs, VecFx16* vecN, BOOL lodSt );

enum {
	DRAW_NORMAL = 0,
	DRAW_YBILLBOARD,
};

typedef struct {
	GXRgb				diffuse;
    GXRgb				ambient;
	GXRgb				specular;
    GXRgb				emission;
	u8					polID;
	u8					alpha;
	u8					drawType;	
	DIRECT_DRAW_FUNC*	func;

}DIRECT_DRAW_DATA;

static const DIRECT_DRAW_DATA drawTreeData;

static const DIRECT_DRAW_DATA* directDrawDataTable[] = {
	&drawTreeData,
};

static void	DirectDraw3Dmapper
	( G3D_MAPPER* g3Dmapper, MAP_BLOCK_DATA* mapBlock, GFL_G3D_CAMERA* g3Dcamera )
{
	const DIRECT_DRAW_DATA* ddData;
	DIRECT_OBJ*		ddObject;
	MtxFx33	mtxBillboard;
	VecFx32	trans, gTrans;
	VecFx16	vecN;
	u32		texDataAdrs, texPlttAdrs;
	int		scaleVal = 8;
	int		i;

	G3X_Reset();

	//カメラ設定取得
	{
		VecFx32		camPos, camUp, target, vecNtmp;

		GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

		G3_LookAt( &camPos, &camUp, &target, NULL );

		VEC_Subtract( &camPos, &target, &vecNtmp );
		VEC_Normalize( &vecNtmp, &vecNtmp );
		VEC_Fx16Set( &vecN, vecNtmp.x, vecNtmp.y, vecNtmp.z );

		vecNtmp.y = 0;
		VEC_Normalize( &vecNtmp, &vecNtmp );
		MTX_Identity33( &mtxBillboard );
		MTX_RotY33( &mtxBillboard, vecNtmp.x, vecNtmp.z );
	}
	//GFL_G3D_LIGHT_Switching( g3Dlightset );

	//グローバルスケール設定
	G3_Scale( FX32_ONE * scaleVal, FX32_ONE * scaleVal, FX32_ONE * scaleVal );

	for( i=0; i<LOCAL_DIRECTDRAW_OBJ_COUNT; i++ ){
		ddObject = &mapBlock->directDrawObject[i];

		if( ddObject->id != DDRAW_OBJID_NULL ){
			GF_ASSERT( ddObject->id  < NELEMS(directDrawDataTable) );

			texDataAdrs = g3Dmapper->grobalDDobject[ ddObject->id ].texDataAdrs;
			texPlttAdrs = g3Dmapper->grobalDDobject[ ddObject->id ].texPlttAdrs;

			VEC_Add( &ddObject->trans, &mapBlock->trans, &gTrans );

			if( checkCullingLength( g3Dcamera, &gTrans ) == TRUE ){
				G3_PushMtx();

				ddData = directDrawDataTable[ ddObject->id ]; 

				//回転、平行移動パラメータ設定
				trans.x = gTrans.x / scaleVal;
				trans.y = gTrans.y / scaleVal;
				trans.z = gTrans.z / scaleVal;
				if( ddData->drawType == DRAW_YBILLBOARD ){
					G3_MultTransMtx33( &mtxBillboard, &trans );
				} else {
					G3_Translate( trans.x, trans.y, trans.z );
				}

				//マテリアル設定
				G3_MaterialColorDiffAmb( ddData->diffuse, ddData->ambient, TRUE );
				G3_MaterialColorSpecEmi( ddData->specular, ddData->emission, FALSE );
				G3_PolygonAttr(	0/*ライトマスク*/, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 
							ddData->polID, ddData->alpha, GX_POLYGON_ATTR_MISC_FOG );

				if( ddData->func != NULL ){
					fx32 limitLength = 512 * FX32_ONE;
					fx32 length = getCullingLength( g3Dcamera, &gTrans );
				
					ddData->func(	texDataAdrs, texPlttAdrs, &vecN, ( length < limitLength) );
				}

				G3_PopMtx(1);
			}
		}
	}
	//↓後にG3D_Systemで行うので、ここではやらない
	//G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

//------------------------------------------------------------------
/**
 *
 * @brief	木
 *
 */
//------------------------------------------------------------------
static void	_drawTree0( u32 texDataAdrs, u32 texPlttAdrs, VecFx16* vecN, BOOL lodSt )
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

static const DIRECT_DRAW_DATA drawTreeData = {
	GX_RGB(31,31,31), GX_RGB(16,16,16), GX_RGB(16,16,16), GX_RGB(0,0,0),
	63, 31, DRAW_YBILLBOARD,
	_drawTree0,
};





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

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		InitGet3DmapperGridInfoData( &gridInfo->gridData[i] );
	}
	gridInfo->count = 0;
}

//------------------------------------------------------------------
/**
 * @brief	グリッド情報計算
 */
//------------------------------------------------------------------
static void CalcGridInfoData( G3D_MAPPER_INFODATA* gridInfo, const MAP_BLOCK_DATA* mapBlock,
								const VecFx32* pos, const fx32 map_width )
{
	const VecFx32*	mapTrans;
	fx32			block_hw, grid_w, block_x, block_z, grid_x, grid_z;
	u32				grid_idx;
	VecFx32			vecN;
	fx32			by, valD;
	NormalVtxSt*	nvs;
	u8				triangleType;

	mapTrans = &mapBlock->trans;
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
	nvs = &((NormalVtxSt*)&mapBlock->attr)[grid_idx];

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
		VEC_Fx16Set( &gridInfo->vecN, nvs->vecN1_x, nvs->vecN1_y, -nvs->vecN1_z );
		valD = nvs->vecN1_D;
	} else {
		VEC_Fx16Set( &gridInfo->vecN, nvs->vecN2_x, nvs->vecN2_y, -nvs->vecN2_z );
		valD = nvs->vecN2_D;
	}
	VEC_Set( &vecN, gridInfo->vecN.x, gridInfo->vecN.y, gridInfo->vecN.z );
	by = -( FX_Mul(vecN.x, pos->x - mapTrans->x) + FX_Mul(vecN.z, pos->z - mapTrans->z) + valD );
	gridInfo->attr = nvs->attr;

	gridInfo->height = FX_Div( by, gridInfo->vecN.y ) + mapTrans->y;
}
	
//------------------------------------------------------------------
/**
 * @brief	グリッド情報取得
 */
//------------------------------------------------------------------
BOOL Get3DmapperGridInfoData
	( G3D_MAPPER* g3Dmapper, const VecFx32* pos, G3D_MAPPER_INFODATA* gridInfoData )
{
	MAP_BLOCK_DATA* mapBlock;
	int		i;

	GF_ASSERT( g3Dmapper );
	if( g3Dmapper->data == NULL ){
		return FALSE;
	}
	InitGet3DmapperGridInfoData( gridInfoData );
	
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &g3Dmapper->mapBlock[i];
		if( (mapBlock->idx != MAPID_NULL)&&( mapBlock->attrLoadReq == FALSE)){
			fx32 min_x = mapBlock->trans.x - g3Dmapper->width/2;
			fx32 min_y = mapBlock->trans.y;
			fx32 min_z = mapBlock->trans.z - g3Dmapper->width/2;
			fx32 max_x = mapBlock->trans.x + g3Dmapper->width/2;
			fx32 max_y = mapBlock->trans.y + g3Dmapper->height;
			fx32 max_z = mapBlock->trans.z + g3Dmapper->width/2;

			//ブロック範囲内チェック
			if(	(pos->x >= min_x)&&(pos->x < max_x)
				&&(pos->y >= min_y)&&(pos->y < max_y)
				&&(pos->z >= min_z)&&(pos->z < max_z) ){

				CalcGridInfoData( gridInfoData, mapBlock, pos, g3Dmapper->width );
				return TRUE;
			}
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	グリッド情報取得（ＸＺ平面上複数）
 */
//------------------------------------------------------------------
BOOL Get3DmapperGridInfo
	( G3D_MAPPER* g3Dmapper, const VecFx32* pos, G3D_MAPPER_GRIDINFO* gridInfo )
{
	MAP_BLOCK_DATA* mapBlock;
	int		i, p;

	GF_ASSERT( g3Dmapper );
	if( g3Dmapper->data == NULL ){
		return FALSE;
	}

	InitGet3DmapperGridInfo( gridInfo );
	
	p = 0;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mapBlock = &g3Dmapper->mapBlock[i];
		if( (mapBlock->idx != MAPID_NULL)&&( mapBlock->attrLoadReq == FALSE)){

			fx32 min_x = mapBlock->trans.x - g3Dmapper->width/2;
			fx32 min_z = mapBlock->trans.z - g3Dmapper->width/2;
			fx32 max_x = mapBlock->trans.x + g3Dmapper->width/2;
			fx32 max_z = mapBlock->trans.z + g3Dmapper->width/2;

			//ブロック範囲内チェック（マップブロックのＸＺ平面上地点）
			if(	(pos->x >= min_x)&&(pos->x < max_x)&&(pos->z >= min_z)&&(pos->z < max_z) ){

				CalcGridInfoData( &gridInfo->gridData[p], mapBlock, pos, g3Dmapper->width );
				p++;
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






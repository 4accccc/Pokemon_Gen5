//============================================================================================
/**
 * @file	map3d.c
 * @brief	マップ生成
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"

//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
#define EXOBJ_MAX	(64)
#define EXOBJ_NULL	(0xffff)

typedef struct {
	u16	id;
	u16	count;
}MAPOBJ_HEADER;

struct _SCENE_MAP {
	GFL_G3D_SCENE*		g3Dscene;
	u16					unitIdx;
	u16					resIdx;
	u16					objIdx;
	u16*				mapAttr;
	MAPOBJ_HEADER		defaultFloor;
	MAPOBJ_HEADER		extraObject[EXOBJ_MAX];
};

typedef struct {
	const char*				data;
	GFL_G3D_SCENEOBJ_DATA	floor;
}MAPDATA;

typedef struct {
	const GFL_G3D_SCENEOBJ_DATA*	data;
	int								count;
}EXOBJ_DATATABLE;

typedef struct { 
	int			seq;

}EXMAPOBJ_WORK;//setup.cで指定されているサイズに収めるように注意

enum {
	QUAD_TYPE_PLANE = 0,
	QUAD_TYPE_SLIDE,
	TRIANGLE_TYPE_021_312,
	TRIANGLE_TYPE_230_103,
};

typedef struct {
	u8	vtxY0;	
	u8	vtxY1;
	u8	vtxY2;
	u8	vtxY3;
	u8	triangleType;

}MAP_GRIDMAKE_DATA;

typedef struct {
	VecFx16	vtx0;
	VecFx16	vtx1;
	VecFx16	vtx2;
	VecFx32	vecN;
	fx32	valD;

}MAP_PLANE_DATA;

typedef struct {
	MAP_PLANE_DATA	triangle[2];
	u32				planeType;

}MAP_GRID_DATA;

static MAP_GRID_DATA*	mapGridData;

#define mapSizeX	(32)
#define mapSizeZ	(32)
#define mapGrid		(FX32_ONE*64)
#define defaultMapX	(-mapGrid*16+mapGrid/2)
#define defaultMapZ	(-mapGrid*16+mapGrid/2)

static void	MapDataCreate( SCENE_MAP* sceneMap, const MAPDATA* map, HEAPID heapID );
static void MapDataDelete( SCENE_MAP* sceneMap );
static void AddExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj, int objID, VecFx32* trans );
static void RemoveExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj );
static const	MAPDATA mapDataTbl;
static const	EXOBJ_DATATABLE extraObjectDataTbl[5];

static void CreateMapGridData( HEAPID heapID );
static void DeleteMapGridData( void );
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
#include "src/sample_graphic/haruka.naix"
static const char g3DarcPath2[] = {"src/sample_graphic/haruka.narc"};

enum {
	G3DRES_MAP_FLOOR = 0,
	G3DRES_EFFECT_WALL,
	G3DRES_FIELD_TEX1,
	G3DRES_FIELD_TEX2,
};
//３Ｄグラフィックリソーステーブル
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ (u32)g3DarcPath2, NARC_haruka_test_floor3_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_test_wall_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_field_tex1_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_field_tex2_nsbmd,  GFL_G3D_UTIL_RESPATH },
};

//---------------------
enum {
	G3DOBJ_MAP_FLOOR = 0,
	G3DOBJ_EFFECT_WALL,
	G3DOBJ_FIELD_TEX1,
	G3DOBJ_FIELD_TEX2,
};
//３Ｄオブジェクト定義テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_MAP_FLOOR, 0, G3DRES_MAP_FLOOR, NULL, 0 },
	{ G3DRES_EFFECT_WALL, 0, G3DRES_EFFECT_WALL, NULL, 0 },
	{ G3DRES_FIELD_TEX1, 0, G3DRES_FIELD_TEX1, NULL, 0 },
	{ G3DRES_FIELD_TEX2, 0, G3DRES_FIELD_TEX2, NULL, 0 },
};

//---------------------
//g3Dscene 初期設定テーブルデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl, NELEMS(g3Dutil_resTbl),
	g3Dutil_objTbl, NELEMS(g3Dutil_objTbl),
};

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ生成
 */
//------------------------------------------------------------------
SCENE_MAP*	Create3Dmap( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	SCENE_MAP* sceneMap = GFL_HEAP_AllocMemory( heapID, sizeof(SCENE_MAP) );
	GFL_G3D_SCENEOBJ_DATA* data;
	int	i;

	//３Ｄデータセットアップ
	sceneMap->unitIdx = GFL_G3D_SCENE_AddG3DutilUnit( g3Dscene, &g3Dutil_setup );
	sceneMap->resIdx = GFL_G3D_SCENE_GetG3DutilUnitResIdx( g3Dscene, sceneMap->unitIdx );
	sceneMap->objIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( g3Dscene, sceneMap->unitIdx );

	//マップ作成
	sceneMap->g3Dscene = g3Dscene;
	MapDataCreate( sceneMap, &mapDataTbl, heapID );

	for( i=0; i<EXOBJ_MAX; i++ ){
		sceneMap->extraObject[i].id = EXOBJ_NULL;
		sceneMap->extraObject[i].count = 0;
	}

	CreateMapGridData( heapID );

	return sceneMap;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ破棄
 */
//------------------------------------------------------------------
void	Delete3Dmap( SCENE_MAP* sceneMap )
{
	int	i;

	DeleteMapGridData();

	for( i=0; i<EXOBJ_MAX; i++ ){
		if( sceneMap->extraObject[i].id != EXOBJ_NULL ){
			RemoveExtraObject( sceneMap, &sceneMap->extraObject[i] );
		}
	}
	MapDataDelete( sceneMap );
	GFL_G3D_SCENE_DelG3DutilUnit( sceneMap->g3Dscene, sceneMap->unitIdx );
	GFL_HEAP_FreeMemory( sceneMap );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップオブジェクト追加
 */
//------------------------------------------------------------------
int		AddObject3Dmap( SCENE_MAP* sceneMap, int objID, VecFx32* trans )
{
	int	i;

	for( i=0; i<EXOBJ_MAX; i++ ){
		if( sceneMap->extraObject[i].id == EXOBJ_NULL ){
			AddExtraObject( sceneMap, &sceneMap->extraObject[i], objID, trans );
			return i;
		}
	}
	GF_ASSERT(0);
	return 0;
}	

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップオブジェクト破棄
 */
//------------------------------------------------------------------
void	RemoveObject3Dmap( SCENE_MAP* sceneMap, int mapobjID )
{
	if( sceneMap->extraObject[mapobjID].id != EXOBJ_NULL ){
		RemoveExtraObject( sceneMap, &sceneMap->extraObject[mapobjID] );
		sceneMap->extraObject[mapobjID].id = EXOBJ_NULL;
	}
}	

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップオブジェクト描画スイッチ設定
 */
//------------------------------------------------------------------
void	Set3DmapDrawSw( SCENE_MAP* sceneMap, int mapobjID, BOOL* sw )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneMap->g3Dscene, 
													sceneMap->extraObject[mapobjID].id );
	GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, sw );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップオブジェクト描画スイッチ取得
 */
//------------------------------------------------------------------
void	Get3DmapDrawSw( SCENE_MAP* sceneMap, int mapobjID, BOOL* sw )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneMap->g3Dscene, 
													sceneMap->extraObject[mapobjID].id );
	GFL_G3D_SCENEOBJ_GetDrawSW( g3DsceneObj, sw );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップアトリビュート取得
 */
//------------------------------------------------------------------
BOOL	Get3DmapAttr( SCENE_MAP* sceneMap, VecFx32* pos, u16* attr )
{
	fx32 wx = pos->x + mapGrid*16;
	fx32 wz = pos->z + mapGrid*16;
	int x = wx/mapGrid;
	int z = wz/mapGrid;
	if(( wx < 0 )||( wx >= mapGrid * mapSizeX )||( wz < 0 )||( wz >= mapGrid * mapSizeZ )){
		return FALSE;
	}
	*attr = sceneMap->mapAttr[ z * mapSizeX + x ];
	return TRUE;
}


//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ生成メイン
 */
//------------------------------------------------------------------
static const  GFL_G3D_OBJSTATUS defaultStatus = {
	{ 0, 0, 0 },
	{ FX32_ONE, FX32_ONE, FX32_ONE },
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
};
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

//------------------------------------------------------------------
/**
 * @brief		３Ｄマップデータセットアップ
 */
//------------------------------------------------------------------
static void MapDataCreate( SCENE_MAP* sceneMap, const MAPDATA* map, HEAPID heapID )
{
	GFL_G3D_SCENEOBJ_DATA*			pdata;
	GFL_G3D_SCENEOBJ_DATA			data;
	u16*							mapAttr;
	int		count = 0;
	int		x,z;
	u16*	mapData = (u16*)map->data;	//めんどいので全角のみに制限
	int		sizeX = mapSizeX;
	int		sizeZ = mapSizeZ;
	u16		mapCode;

	mapAttr = GFL_HEAP_AllocMemory( heapID, 2*sizeX*sizeZ );
	pdata = GFL_HEAP_AllocMemoryLo( heapID, sizeof(GFL_G3D_SCENEOBJ_DATA)*sizeX*sizeZ+1 );

	pdata[ count ] = map->floor;
	count++;

	for( z=0; z<sizeZ; z++ ){
		for( x=0; x<sizeX; x++ ){
			mapCode = GET_MAPCODE( mapData, x, z );
			switch( mapCode ){
			default:
			case '　':
				mapAttr[ z * sizeX + x ] = 0;
				break;
			case '■':	//壁
				mapAttr[ z * sizeX + x ] = 1;
				break;
			case '○':	//配置人物１
#if 0
				data.objID = G3DOBJ_HUMAN2_STOP; 
				data.movePriority = 0;
				data.drawPriority = 250;
				data.cullingFlag = TRUE;
				data.drawSW = TRUE;
				data.status = defaultStatus;
				data.blendAlpha = 31;
				data.status.trans.x = defaultMapX + (mapGrid * x);
				data.status.trans.z = defaultMapZ + (mapGrid * z);
				data.status.scale.x = FX32_ONE/4;
				data.status.scale.y = FX32_ONE/4;
				data.status.scale.z = FX32_ONE/4;
				data.func = moveStopHaruka;
				pdata[ count ] = data;
				count++;
#endif
				mapAttr[z*sizeX+x] = 0;
				break;
			case '◎':	//プレーヤー
#if 0
				tetsuWork->contPos.x = defaultMapX + (mapGrid * x);
				tetsuWork->contPos.y = 0;
				tetsuWork->contPos.z = defaultMapZ + (mapGrid * z);
#endif
				mapAttr[z*sizeX+x] = 0;
				break;
			}
		}
	}

	//マップオブジェクト追加
//	sceneMap->defaultFloor.id = GFL_G3D_SCENEOBJ_Add
//								( sceneMap->g3Dscene, pdata, count, sceneMap->objIdx );
	sceneMap->defaultFloor.count = count;
	sceneMap->mapAttr = mapAttr;

	//マップオブジェクト設定後、セットアップ用配列は破棄
	GFL_HEAP_FreeMemory( pdata );
}

//------------------------------------------------------------------
static void MapDataDelete( SCENE_MAP* sceneMap )
{
//	GFL_G3D_SCENEOBJ_Remove
//			(sceneMap->g3Dscene, sceneMap->defaultFloor.id, sceneMap->defaultFloor.count ); 
	GFL_HEAP_FreeMemory( sceneMap->mapAttr );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄマップオブジェクトデータセットアップ
 */
//------------------------------------------------------------------
static void AddExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj, int objID, VecFx32* trans )
{
	int	i;
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	VecFx32	transTmp;

	exobj->count = extraObjectDataTbl[objID].count;
	exobj->id = GFL_G3D_SCENEOBJ_Add
			( sceneMap->g3Dscene, extraObjectDataTbl[objID].data, exobj->count, sceneMap->objIdx );

	for( i=0; i<exobj->count; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneMap->g3Dscene, exobj->id + i );
		GFL_G3D_SCENEOBJ_GetPos( g3DsceneObj, &transTmp );
		transTmp.x = trans->x;
		transTmp.z = trans->z;
		GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, &transTmp );
	}
}

//------------------------------------------------------------------
static void RemoveExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj )
{
	GFL_G3D_SCENEOBJ_Remove( sceneMap->g3Dscene, exobj->id, exobj->count ); 
}


//------------------------------------------------------------------
/**
 * @brief		３Ｄマップオブジェクトデータ動作
 */
//------------------------------------------------------------------
static void moveExtraObject( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	EXMAPOBJ_WORK*	exw = (EXMAPOBJ_WORK*)work;

	if( exw->seq == 3 ){
		return;
	}
	if( exw->seq == 2 ){
		u8	blendAlphaEnd = 31;
		GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &blendAlphaEnd );
		exw->seq++;
	}
	{
		VecFx32	transTmp;
		fx32	heightLimit = 0;

		GFL_G3D_SCENEOBJ_GetPos( sceneObj, &transTmp );
		if( transTmp.y < heightLimit ){	//高さチェック
			transTmp.y += FX32_ONE/16;
			if( transTmp.y >= heightLimit ){
				transTmp.y = heightLimit;
				exw->seq++;
			}
			GFL_G3D_SCENEOBJ_SetPos( sceneObj, &transTmp );
		}
	}
	{
		u8	blendAlphaTmp;
		u8	alphaLimit = 20;

		GFL_G3D_SCENEOBJ_GetBlendAlpha( sceneObj, &blendAlphaTmp );
		if( blendAlphaTmp < alphaLimit ){	//透明度チェック
			blendAlphaTmp++;
			if( blendAlphaTmp >= alphaLimit ){
				blendAlphaTmp = alphaLimit;
				exw->seq++;
			}
			GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &blendAlphaTmp );
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief		データ
 */
//------------------------------------------------------------------
static const char mapData0[] = {
"　　　　■■■■■■■■　　　　　　　　■■■■■■■■　　　　"	//16
"　　　　■■■■■■■■　　　　　　　　■■■■■■■■　　　　"	//
"　　　　■■■■■■■■　　　　　　　　■■■■■■■■　　　　"	//
"　　　　■■■■■■■■　　　　　　　　■■■■■■■■　　　　"	//
"　　　　■■■■■■■■　　　　　　　　■■■■■■■■　　　　"	//
"　　　　■■■■■■■■　　　　　　　　■■■■　　　■　　　　"	//
"　　　　■■■■■■■■　　　　　　　　■　　■　　　■　　　　"	//
"　　　　■■■■■■■■■■　　　　■■■　　■■　　■　　　　"	//
"　　　　■■　　　　　　　　　　　　　　　　　■　　　■　　　　"	//
"　　　　■■　　　　　　　　　　　　　　　　　■　　　■　　　　"	//
"　　　　■■　　　　　　　　　　　　　　　　　■　　　　　　　　"	//
"　　　　■■　　　　　　　　　　　　　　　　　■　　　　　　　　"	//
"　　　　■■■■■■　　　■■■■■■■■■■■　■■■■■■■"	//
"　　　　■■　　　　　　　■　　　　　　　　■　　　　　　　　　"	//
"　　　　■■　　　　　　　■　　　　　　　　■　　　　　　　　　"	//
"　　　　■■　　　　　　　■　　　　■　　　■　　　　　　　　　"	//
"　　　　■■　　　　　　　■　　　　■　　　■　　　　　　　　　"	//
"　　　　■■■■■■　　　■■■■■■■　　■　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　■　　■■　■■■■　　　"	//
"　　　　　　　　　　　　　　　　　　　■　　　　　■　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　■　　　　　■　　　　　　"	//
"　　　　■■■■■　　■■■■■　　　■　　　　　■　　　　　　"	//
"　　　　■■　　　　　　　　　■　　　■　　　　　■　　　　　　"	//
"　　　　■■　　　　　　　　　■　　　■　　　　　■　　　　　　"	//
"　　　　■■　　　　　　　■　■■■■■■■■■■■■■■　　　"	//
"　　　　■■　　　　　　　■　■■■■　　　　　　　　　■　　　"	//
"　　　　■■■■■■　　　■　■■■■　　　　　　　　　■　　　"	//
"　　　　■■■■■■■■■■　　　　■　　■■■■■　　　　　　"	//
"　　　　■■■■■■■■■■　　　　■　　■■■■■　　　　　　"	//
"　　　　■■■■■■■■■■■■　　　　　■■■■■　　　　　　"	//
"　　　　■■■■■■■■■■■■　　　　　　　　　　　　　　　　"	//
"　　　　■■■■■■■■■■■■　　　　　　　　　　　　　　　　"	//
};

static const char mapData1[] = {
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//0
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//1
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//2
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//3
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//4
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//5
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//6
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//7
"　　　　　　　　　□□□□□□　　　　　　　　　　　　　　　　　"	//8
"　　　　□□□□□□　　　　□　　　　　　　　　　　　　　　　　"	//9
"　　　　□□□□□□　　　　□　□□□□□□　　　　　　　　　　"	//10
"　　　　□□　□□□　　　　□　□　　　　□　　　　　　　　　　"	//11
"　　　　□□　□□□　　　　□　□　　　　□　　　　　　　　　　"	//12
"　　　　□□□□□□　　　　□　□□□□□□　　　　　　　　　　"	//13
"　　　　□□□□□□　　　　□　　　　　　　　　　　　　　　　　"	//14
"　　　　　　　　　□□□□□□　　　■　　　　　　　　　　　　　"	//15
"　　　　　　　　　　　　　　　　　　■　　　　　　　　　　　　　"	//16
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//17
"　　　　　　　　　　　　　　　□□□□□□□□□□□　　　　　　"	//18
"　　　　　　　　　　　　　　　□　　　　□□　　　□　　　　　　"	//19
"　　　　　　　　　　　　　　　□　　　　□□　　　□　　　　　　"	//20
"　　　　　　　　　　　　　　　□　　　　□□　　　□　　　　　　"	//21
"　　　　　　　　　　　　　　　□□□□□□□□□□□　　　　　　"	//22
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//23
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//24
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//25
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//26
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//27
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//28
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//29
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//30
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//31
};

static const MAP_GRIDMAKE_DATA mapGrid1[] = {
//0//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//1//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//2//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//3//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//4//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//5//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//6//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//7//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//8//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,1,0},{0,0,1,1,0},{0,0,1,1,0},{0,0,1,1,0},{0,0,1,1,0},{0,0,1,0,1},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//9//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,1,0},{0,0,1,1,0},{0,0,1,1,0},{0,0,1,1,0},
{0,0,1,0,1},{0,1,0,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,0,1,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//10//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,1,0},{1,1,1,3,1},{1,1,3,3,0},{1,1,3,1,0},
{1,0,1,0,0},{0,1,0,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,0,1,0,0},{0,0,0,0,0},
{0,0,0,4,1},{0,0,4,4,0},{0,0,4,4,0},{0,0,4,4,0},{0,0,4,4,0},{0,0,4,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//11//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,1,0},{1,3,1,3,0},{3,3,3,3,0},{3,1,3,1,0},
{1,0,1,0,0},{0,1,0,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,0,1,0,0},{0,0,0,0,0},
{0,4,0,4,0},{4,4,4,4,0},{4,4,4,4,0},{4,4,4,4,0},{4,4,4,4,0},{4,0,4,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//12//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,1,0},{1,3,1,3,0},{3,3,3,3,0},{3,1,3,1,0},
{1,0,1,0,0},{0,1,0,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,0,1,0,0},{0,0,0,0,0},
{0,4,0,4,0},{4,4,4,4,0},{4,4,4,4,0},{4,4,4,4,0},{4,4,4,4,0},{4,0,4,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//13//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,1,0},{1,3,1,1,0},{3,3,1,1,0},{3,1,1,1,1},
{1,0,1,0,0},{0,1,0,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,0,1,0,0},{0,0,0,0,0},
{0,4,0,0,0},{4,4,0,0,0},{4,4,0,0,0},{4,4,0,0,0},{4,4,0,0,0},{4,0,0,0,1},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//14//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,0,1},{1,1,0,0,0},{1,1,0,0,0},{1,1,0,0,0},
{1,0,0,0,0},{0,1,0,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,1,1,1,0},{1,0,1,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//15//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,1,0,0,1},{1,1,0,0,0},{1,1,0,0,0},{1,1,0,0,0},{1,1,0,0,0},{1,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//16//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//17//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//18//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,2,1},
{0,0,2,2,0},{0,0,2,2,0},{0,0,2,2,0},{0,0,2,2,0},{0,0,2,0,0},{0,0,0,2,1},{0,0,2,2,0},{0,0,2,2,0},
{0,0,2,2,0},{0,0,2,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//19//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,2,0,2,0},
{2,2,2,2,0},{2,2,2,2,0},{2,2,2,2,0},{2,2,2,2,0},{2,0,2,0,0},{0,2,0,2,0},{2,2,2,2,0},{2,2,2,2,0},
{2,2,2,2,0},{2,0,2,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//20//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,2,0,2,0},
{2,2,2,2,0},{2,2,2,2,0},{2,2,2,2,0},{2,2,2,2,0},{2,0,2,0,0},{0,2,0,2,0},{2,2,2,2,0},{2,2,2,2,0},
{2,2,2,2,0},{2,0,2,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//21//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,2,0,2,0},
{2,2,2,2,0},{2,2,2,2,0},{2,2,2,2,0},{2,2,2,2,0},{2,0,2,0,0},{0,2,0,2,0},{2,2,2,2,0},{2,2,2,2,0},
{2,2,2,2,0},{2,0,2,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//22//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,2,0,0,0},
{2,2,0,0,0},{2,2,0,0,0},{2,2,0,0,0},{2,2,0,0,0},{2,0,0,0,1},{0,2,0,0,0},{2,2,0,0,0},{2,2,0,0,0},
{2,2,0,0,0},{2,0,0,0,1},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//23//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//24//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//25//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//26//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//27//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//28//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//29//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//30//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//31//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
};

static const MAPDATA mapDataTbl = {
	mapData0,
	{	G3DOBJ_MAP_FLOOR, 0, 1, 31, FALSE, TRUE,
		{	{ 0, 0, 0 },
			{ FX32_ONE*4, FX32_ONE*4, FX32_ONE*4 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

static const GFL_G3D_SCENEOBJ_DATA extraObject1[] = {
	{	G3DOBJ_EFFECT_WALL, 0, 1, 8, TRUE, TRUE,
		{	{ 0, -FX32_ONE*64, 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveExtraObject,
	},
};

static const	EXOBJ_DATATABLE extraObjectDataTbl[5] = {
	{ extraObject1, NELEMS(extraObject1) },
	{ extraObject1, NELEMS(extraObject1) },
	{ extraObject1, NELEMS(extraObject1) },
	{ extraObject1, NELEMS(extraObject1) },
	{ extraObject1, NELEMS(extraObject1) },
};

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ高さデータ作成
 */
//------------------------------------------------------------------
static void MakeGridData( int x, int z, int y0, int y1, int y2, int y3 );
static void MakeTriangleData
		( MAP_PLANE_DATA* triData, VecFx32* posRef, VecFx16* vtx0, VecFx16* vtx1, VecFx16* vtx2 );

static void CreateMapGridData( HEAPID heapID )
{
	int		i, x, z;
	VecFx32	posRef;
	VecFx16	vtx0, vtx1, vtx2, vtx3;

	mapGridData = GFL_HEAP_AllocClearMemory( heapID, sizeof(MAP_GRID_DATA)*mapSizeX*mapSizeZ );

	for( z=0; z<mapSizeZ; z++ ){
		for( x=0; x<mapSizeX; x++ ){
#if 1
			i = z * mapSizeX + x;

			if( (x == 0)||(x == 31)||(z == 0)||(z == 31)){
				int height = 4;

				if((x == 0)&&(z == 0)){
					MakeGridData( x, z, height, height, height, 0 ); 
				} else if((x == 31)&&(z == 0)){
					MakeGridData( x, z, height, height, 0, height ); 
				} else if((x == 0)&&(z == 31)){
					MakeGridData( x, z, height, 0, height, height ); 
				} else if((x == 31)&&(z == 31)){
					MakeGridData( x, z, 0, height, height, height ); 
				} else if(x == 0){
					MakeGridData( x, z, height, 0, height, 0 ); 
				} else if(x == 31){
					MakeGridData( x, z, 0, height, 0, height ); 
				} else if(z == 0){
					MakeGridData( x, z, height, height, 0, 0 ); 
				} else {
					MakeGridData( x, z, 0, 0, height, height ); 
				}
			} else {
				MakeGridData( x, z, 
				mapGrid1[i].vtxY0, mapGrid1[i].vtxY1, mapGrid1[i].vtxY2, mapGrid1[i].vtxY3 );
			}
#else
			MakeGridData( x, z, 0, 0, 0, 0 );
#endif
		}
	}
}

static void DeleteMapGridData( void )
{
	GFL_HEAP_FreeMemory( mapGridData );
}

static void	GetGroundGridData( const VecFx32* pos, int* x, int* z, u16* offset )
{
	*x = (pos->x + mapGrid * mapSizeX/2)/mapGrid;
	*z = (pos->z + mapGrid * mapSizeZ/2)/mapGrid;
	*offset = *z * mapSizeX + *x;
}

//------------------------------------------------------------------
static void inline posGridCalc( int x, int z, VecFx16* vtx, VecFx32* dst )
{
	VecFx32	tmp;

	tmp.x = (x - mapSizeX/2) * FX16_ONE + vtx->x;
	tmp.y = vtx->y;
	tmp.z = (z - mapSizeZ/2) * FX16_ONE + vtx->z;

	dst->x = FX_Mul( mapGrid, tmp.x );
	dst->y = FX_Mul( mapGrid, tmp.y );
	dst->z = FX_Mul( mapGrid, tmp.z );
}

static void MakeGridData( int gridx, int gridz, int y0, int y1, int y2, int y3 )
{
	MAP_GRID_DATA* grid = &mapGridData[ gridz * mapSizeX + gridx ];
	VecFx32	posRef;
	VecFx16	vtx0, vtx1, vtx2, vtx3;

	//法線ベクトル算出用、頂点方向データ作成（描画にも使える）
	vtx0.x = 0;
	vtx0.y = y0 * FX16_ONE/4;
	vtx0.z = 0;

	vtx1.x = FX16_ONE;
	vtx1.y = y1 * FX16_ONE/4;
	vtx1.z = 0;

	vtx2.x = 0;
	vtx2.y = y2 * FX16_ONE/4;
	vtx2.z = FX16_ONE;

	vtx3.x = FX16_ONE;
	vtx3.y = y3 * FX16_ONE/4;
	vtx3.z = FX16_ONE;

	if(( y0 == y3 )&&( y1 == y2 )&&( y0 == y1 )){
		grid->planeType = QUAD_TYPE_PLANE;
	} else if( y0 == y3 ){
		grid->planeType = TRIANGLE_TYPE_230_103;
	} else if( y1 == y2 ){
		grid->planeType = TRIANGLE_TYPE_021_312;
	} else {
		grid->planeType = QUAD_TYPE_SLIDE;
	}
	//データ格納(法線が上の場合は左回りに頂点設定)
	if( grid->planeType == TRIANGLE_TYPE_021_312 ){
		//0-2-1 triangle設定
		posGridCalc( gridx, gridz, &vtx0, &posRef );	//vtx0の実座標計算
		MakeTriangleData( &grid->triangle[0], &posRef, &vtx0, &vtx2, &vtx1 );
		//3-1-2 triangle設定
		posGridCalc( gridx, gridz, &vtx3, &posRef );	//vtx3の実座標計算
		MakeTriangleData( &grid->triangle[1], &posRef, &vtx3, &vtx1, &vtx2 );
	} else {
		//2-3-0 triangle設定
		posGridCalc( gridx, gridz, &vtx2, &posRef );	//vtx2の実座標計算
		MakeTriangleData( &grid->triangle[0], &posRef, &vtx2, &vtx3, &vtx0 );
		//1-0-3 triangle設定
		posGridCalc( gridx, gridz, &vtx1, &posRef );	//vtx1の実座標計算
		MakeTriangleData( &grid->triangle[1], &posRef, &vtx1, &vtx0, &vtx3 );
	}
}

//------------------------------------------------------------------
static void MakeTriangleData
		( MAP_PLANE_DATA* triData, VecFx32* posRef, VecFx16* vtx0, VecFx16* vtx1, VecFx16* vtx2 )
{
	VecFx16	vec0, vec1, vecN;

	triData->vtx0 = *vtx0;
	triData->vtx1 = *vtx1;
	triData->vtx2 = *vtx2;

	VEC_Fx16Subtract( vtx1, vtx0, &vec0 );
	VEC_Fx16Subtract( vtx2, vtx0, &vec1 );
	VEC_Fx16CrossProduct( &vec0, &vec1, &vecN );
	VEC_Fx16Normalize( &vecN, &vecN );

	VEC_Set( &triData->vecN, (fx32)vecN.x, (fx32)vecN.y, (fx32)vecN.z );

	triData->valD = -(	  FX_Mul( triData->vecN.x, posRef->x ) 
						+ FX_Mul( triData->vecN.y, posRef->y ) 
						+ FX_Mul( triData->vecN.z, posRef->z ) ); 
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ高さデータ取得
 */
//------------------------------------------------------------------
static BOOL	CheckGetGroundOutSideData( const int x, const int z )
{
	if(( x < 0 )||( x >= mapSizeX )||( z < 0 )||( z >= mapSizeZ ) ){ 
		return TRUE;
	} else {
		return FALSE;
	}
}

static BOOL	GetGroundTriangleID( const VecFx32* pos, u16* gridOffs, u16* ID )
{
	int		gridx, gridz;
	fx32	fx, fz;

	GetGroundGridData( pos, &gridx, &gridz, gridOffs );
	fx = ( pos->x + mapGrid * mapSizeX/2 )%mapGrid;
	fz = ( pos->z + mapGrid * mapSizeZ/2 )%mapGrid;

	if( CheckGetGroundOutSideData( gridx, gridz ) == TRUE ){
		return FALSE;
	}
	//グリッド内三角形の判定
	if( mapGridData[ *gridOffs ].planeType == TRIANGLE_TYPE_021_312 ){
		//0-2-1,3-1-2のパターン
		if( fx + fz < mapGrid ){
			*ID = 0;
			return TRUE;	//0-1-2三角形内
		} else {
			*ID = 1;
			return TRUE;	//3-2-1三角形内
		}
	} else {
		//2-3-0,1-0-3のパターン
		if( fx < fz ){
			*ID = 0;
			return TRUE;	//2-0-3三角形内
		} else {
			*ID = 1;
			return TRUE;	//1-3-0三角形内
		}
	}
	return FALSE;
}

BOOL	GetGroundPlaneData( const VecFx32* pos, VecFx32* vecN, fx32* valD )
{
	u16		gridOffs, triangleID;

	if( GetGroundTriangleID( pos, &gridOffs, &triangleID ) == FALSE ){
		return FALSE;
	}
	*vecN	= mapGridData[ gridOffs ].triangle[ triangleID ].vecN;
	*valD	= mapGridData[ gridOffs ].triangle[ triangleID ].valD;

	return TRUE;
}

void	GetGroundPlaneVecN( const VecFx32* pos, VecFx32* vecN )
{
	u16		gridOffs, triangleID;

	if( GetGroundTriangleID( pos, &gridOffs, &triangleID ) == FALSE ){
		vecN->x = 0;
		vecN->y = FX32_ONE;
		vecN->z = 0;
		return;
	}
	*vecN	= mapGridData[ gridOffs ].triangle[ triangleID ].vecN;
}

void	GetGroundPlaneHeight( const VecFx32* pos, fx32* height ) 
{
	u16		gridOffs, triangleID;
	VecFx32 vecN;
	fx32	by, valD;

	if( GetGroundTriangleID( pos, &gridOffs, &triangleID ) == FALSE ){
		*height = 0;
		return;
	}
	vecN	= mapGridData[ gridOffs ].triangle[ triangleID ].vecN;
	valD	= mapGridData[ gridOffs ].triangle[ triangleID ].valD;

	by = -( FX_Mul( vecN.x, pos->x ) + FX_Mul( vecN.z, pos->z ) + valD );
	*height = FX_Div( by, vecN.y );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップリソースデータ取得
 */
//------------------------------------------------------------------
static void GetMapTexAddress
			( const SCENE_MAP* sceneMap, const u16 idx, u32* texData, u32* texPltt )
{
	NNSG3dTexKey	texKey;
	NNSG3dPlttKey	plttKey;

	texKey = GFL_G3D_GetTextureDataVramKey
		( GFL_G3D_SCENE_GetG3DutilResHandle( sceneMap->g3Dscene, idx + sceneMap->resIdx ) );
	plttKey = GFL_G3D_GetTexturePlttVramKey
		( GFL_G3D_SCENE_GetG3DutilResHandle( sceneMap->g3Dscene, idx + sceneMap->resIdx ) );

	*texData = NNS_GfdGetTexKeyAddr( texKey );
	*texPltt = NNS_GfdGetPlttKeyAddr( plttKey );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップディスプレイ
 */
//------------------------------------------------------------------
void	Draw3Dmap( SCENE_MAP* sceneMap, GFL_G3D_CAMERA* g3Dcamera )
{
	u32			texData, texPltt, texData1, texPltt1, texData2, texPltt2;
	int			x, z;
	fx32		px, pz;
	int			offset;
	VecFx16		*pVtx0, *pVtx1, *pVtx2, *pVtx3;
	fx32		s0, t0, s1, t1, s2, t2;
	GXRgb		vtx0col, vtx1col, vtx2col;
	fx16		col;
	GXTexFmt	texFmt;
	int			i;

	GetMapTexAddress( sceneMap, G3DRES_FIELD_TEX1, &texData1, &texPltt1 );
	GetMapTexAddress( sceneMap, G3DRES_FIELD_TEX2, &texData2, &texPltt2 );

	//水平線描画
	G3X_Reset();
	{
		//カメラ設定
		VecFx32 camPos, camUp, target;

		GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

		G3_LookAt( &camPos, &camUp, &target, NULL);
	}
	//マテリアル設定
	G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), TRUE );
	G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE );
	G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_BACK, 63, 8, 0);

	G3_Scale( FX32_ONE*64*32, FX32_ONE*64*32, FX32_ONE*64*32 );
	G3_PushMtx();
	G3_Translate( 0, 0, 0 );

	G3_Begin( GX_BEGIN_QUADS );

	//ライトを使用しないのでNormal(法線ベクトル設定)コマンドは発行しない
	G3_Color( GX_RGB(0, 0, 16) );
	G3_Vtx( -FX16_ONE, -4, FX16_ONE );
	G3_Color( GX_RGB(0, 0, 31) );
	G3_Vtx( FX16_ONE, -4, FX16_ONE );
	G3_Color( GX_RGB(0, 0, 16) );
	G3_Vtx( FX16_ONE, -4, -FX16_ONE );
	G3_Color( GX_RGB(0, 0, 31) );
	G3_Vtx( -FX16_ONE, -4, -FX16_ONE );

	G3_End();
	G3_PopMtx(1);

	//地形描画
	G3X_Reset();
	{
		//カメラ設定
		VecFx32 camPos, camUp, target;

		GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

		G3_LookAt( &camPos, &camUp, &target, NULL);
	}
	{
		G3_MtxMode(GX_MTXMODE_TEXTURE);
		G3_Identity();
		// Use an identity matrix for the texture matrix for simplicity
		G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
	}
	//マテリアル設定
	G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), TRUE );
	G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE );
	G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_BACK, 63, 31, 0);
	G3_Scale( mapGrid, mapGrid, mapGrid );

	for( z=0; z<mapSizeZ; z++ ){
		for( x=0; x<mapSizeX; x++ ){

			px = (x - mapSizeX/2) * FX16_ONE; 
			pz = (z - mapSizeZ/2) * FX16_ONE;
			offset = z * mapSizeX + x;

			G3_PushMtx();

			G3_Translate( px, 0, pz );

			G3_Begin( GX_BEGIN_TRIANGLES );

			for( i=0; i<2; i++ ){
				pVtx0 = &mapGridData[offset].triangle[i].vtx0;
				pVtx1 = &mapGridData[offset].triangle[i].vtx1;
				pVtx2 = &mapGridData[offset].triangle[i].vtx2;

				if(( pVtx0->y == pVtx1->y )&&( pVtx0->y == pVtx2->y )){
					//平面（草）
					texFmt	= GX_TEXFMT_PLTT16;// use 16 colors palette texture
					texData = texData1;
					texPltt = texPltt1;
					s0 = 0;
					t0 = 0;
					s1 = 0;
					t1 = 64 * FX32_ONE;
					s2 = 64 * FX32_ONE;
					t2 = 0;
				} else {
					//斜面（土）
					texFmt	= GX_TEXFMT_PLTT4;// use 4 colors palette texture
					texData = texData2;
					texPltt = texPltt2;
					s0 = 0;
					t0 = 0;
					s1 = 0;
					t1 = 64 * FX32_ONE;
					s2 = 64 * FX32_ONE;
					t2 = 0;
				}
				G3_TexImageParam(	texFmt, GX_TEXGEN_TEXCOORD,// use texcoord
									GX_TEXSIZE_S64, GX_TEXSIZE_T64,// 64 pixels
									GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
									GX_TEXPLTTCOLOR0_USE, texData );
				G3_TexPlttBase( texPltt, texFmt );

				//gxSt0 = GX_ST( pVtx0->x * 64, pVtx0->z * 64 );
				//gxSt1 = GX_ST( pVtx1->x * 64, pVtx1->z * 64 );
				//gxSt2 = GX_ST( pVtx2->x * 64, pVtx2->z * 64 );
				vtx0col = GX_RGB(16, 16, 16);
				vtx1col = GX_RGB(24, 24, 24);
				vtx2col = GX_RGB(31, 31, 31);

				//ライトを使用しないのでNormal(法線ベクトル設定)コマンドは発行しない
				G3_Color( vtx0col );
				G3_TexCoord( s0, t0 );
				G3_Vtx( pVtx0->x, pVtx0->y, pVtx0->z );

				G3_Color( vtx1col );
				G3_TexCoord( s1, t1 );
				G3_Vtx( pVtx1->x, pVtx1->y, pVtx1->z );

				G3_Color( vtx2col );
				G3_TexCoord( s2, t2 );
				G3_Vtx( pVtx2->x, pVtx2->y, pVtx2->z );
			}
			G3_End();

			G3_PopMtx(1);
		}
	}
	//↓後にG3D_Systemで行うので、ここではやらない
	//G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ高さ変更
 */
//------------------------------------------------------------------
static void	GetGridVtxY( const int gridx, const int gridz, 
							fx16* y0, fx16* y1, fx16* y2, fx16* y3 )
{
	MAP_GRID_DATA* grid = &mapGridData[ gridz * mapSizeX + gridx ];

	if( grid->planeType == TRIANGLE_TYPE_021_312 ){
		*y0 = grid->triangle[0].vtx0.y;
		*y1 = grid->triangle[0].vtx2.y;
		*y2 = grid->triangle[1].vtx2.y;
		*y3 = grid->triangle[1].vtx0.y;
	} else {
		*y0 = grid->triangle[0].vtx2.y;
		*y1 = grid->triangle[1].vtx0.y;
		*y2 = grid->triangle[0].vtx0.y;
		*y3 = grid->triangle[1].vtx2.y;
	}
}

static u32	GetGridPlaneData( const int gridx, const int gridz )
{
	MAP_GRID_DATA* grid = &mapGridData[ gridz * mapSizeX + gridx ];

	return  grid->planeType;
}

static void	GetGridVtxYLen( const int gridx, const int gridz, 
							int* y0, int* y1, int* y2, int* y3 )
{
	fx16 fy0, fy1, fy2, fy3;

	GetGridVtxY( gridx, gridz, &fy0, &fy1, &fy2, &fy3 );
	*y0 = fy0/(FX16_ONE/4);
	*y1 = fy1/(FX16_ONE/4);
	*y2 = fy2/(FX16_ONE/4);
	*y3 = fy3/(FX16_ONE/4);
}

static void	SetGridAround( const int gridx, const int gridz )
{
	int y0, y1, y2, y3;
	int ty0, ty1, ty2, ty3;

	GetGridVtxYLen( gridx, gridz, &y0, &y1, &y2, &y3 );

	if( CheckGetGroundOutSideData( gridx-1, gridz-1 ) == FALSE ){
		GetGridVtxYLen( gridx-1, gridz-1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx-1, gridz-1, ty0, ty1, ty2, y0 );
	}
	if( CheckGetGroundOutSideData( gridx, gridz-1 ) == FALSE ){
		GetGridVtxYLen( gridx, gridz-1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx, gridz-1, ty0, ty1, y0, y1 );
	}
	if( CheckGetGroundOutSideData( gridx+1, gridz-1 ) == FALSE ){
		GetGridVtxYLen( gridx+1, gridz-1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx+1, gridz-1, ty0, ty1, y1, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx-1, gridz ) == FALSE ){
		GetGridVtxYLen( gridx-1, gridz, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx-1, gridz, ty0, y0, ty2, y2 );
	}
	if( CheckGetGroundOutSideData( gridx+1, gridz ) == FALSE ){
		GetGridVtxYLen( gridx+1, gridz, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx+1, gridz, y1, ty1, y3, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx-1, gridz+1 ) == FALSE ){
		GetGridVtxYLen( gridx-1, gridz+1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx-1, gridz+1, ty0, y2, ty2, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx, gridz+1 ) == FALSE ){
		GetGridVtxYLen( gridx, gridz+1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx, gridz+1, y2, y3, ty2, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx+1, gridz+1 ) == FALSE ){
		GetGridVtxYLen( gridx+1, gridz+1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( gridx+1, gridz+1, y3, ty1, ty2, ty3 );
	}
}

static void	SetGridUp( const int gridx, const int gridz,
							const int y0, const int y1, const int y2, const int y3 )
{
	int	ymax;

	//OS_Printf("対象地形 y0 = %x, y1 = %x, y2 = %x, y3 = %x\n", y0, y1, y2, y3 );
	ymax = y0;
	if( ymax < y1 ){ ymax = y1; }
	if( ymax < y2 ){ ymax = y2; }
	if( ymax < y3 ){ ymax = y3; }

	if( GetGridPlaneData( gridx, gridz ) == QUAD_TYPE_PLANE ){
		if( ymax < (32-1) ){
			ymax++;	//有効値は、fx16上位が3bit幅なので(fx16/4)単位だと-32<y<32
		}
	}
	MakeGridData( gridx, gridz, ymax, ymax, ymax, ymax );
	SetGridAround( gridx, gridz );
}

static void	SetGridDown( const fx32 gridx, const fx32 gridz,
							const int y0, const int y1, const int y2, const int y3 )
{
	int	ymin;

	//OS_Printf("対象地形 y0 = %x, y1 = %x, y2 = %x, y3 = %x\n", y0, y1, y2, y3 );
	ymin = y0;
	if( ymin > y1 ){ ymin = y1; }
	if( ymin > y2 ){ ymin = y2; }
	if( ymin > y3 ){ ymin = y3; }

	if( GetGridPlaneData( gridx, gridz ) == QUAD_TYPE_PLANE ){
		if( ymin > -(32-1) ){
			ymin--;	//有効値は、fx16上位が3bit幅なので(fx16/4)単位だと-32<y<32
		}
	}
	MakeGridData( gridx, gridz, ymin, ymin, ymin, ymin );
	SetGridAround( gridx, gridz );
}

static BOOL	CheckGridVtxUp( const int gridx, const int gridz )
{
	int		ymax, y0, y1, y2, y3;
	BOOL	result = TRUE;

	GetGridVtxYLen( gridx, gridz, &y0, &y1, &y2, &y3 );
	ymax = y0;
	if( ymax < y1 ){ ymax = y1; }
	if( ymax < y2 ){ ymax = y2; }
	if( ymax < y3 ){ ymax = y3; }

	if( y0 < (ymax - 1) ){
		y0 = ymax - 1;
		result = FALSE;
	}
	if( y1 < (ymax - 1) ){
		y1 = ymax - 1;
		result = FALSE;
	}
	if( y2 < (ymax - 1) ){
		y2 = ymax - 1;
		result = FALSE;
	}
	if( y3 < (ymax - 1) ){
		y3 = ymax - 1;
		result = FALSE;
	}
	if( result == FALSE ){
		MakeGridData( gridx, gridz, y0, y1, y2, y3 );
		SetGridAround( gridx, gridz );
	}
	return 	result;
}

static BOOL	CheckGridVtxDown( const int gridx, const int gridz )
{
	int		ymin, y0, y1, y2, y3;
	BOOL	result = TRUE;

	GetGridVtxYLen( gridx, gridz, &y0, &y1, &y2, &y3 );
	ymin = y0;
	if( ymin > y1 ){ ymin = y1; }
	if( ymin > y2 ){ ymin = y2; }
	if( ymin > y3 ){ ymin = y3; }

	if( y0 > (ymin + 1) ){
		y0 = ymin + 1;
		result = FALSE;
	}
	if( y1 > (ymin + 1) ){
		y1 = ymin + 1;
		result = FALSE;
	}
	if( y2 > (ymin + 1) ){
		y2 = ymin + 1;
		result = FALSE;
	}
	if( y3 > (ymin + 1) ){
		y3 = ymin + 1;
		result = FALSE;
	}
	if( result == FALSE ){
		MakeGridData( gridx, gridz, y0, y1, y2, y3 );
		SetGridAround( gridx, gridz );
	}
	return 	result;
}

void	SetMapGroundUp( VecFx32* pos )
{
	int		gridx, gridz;
	int		y0, y1, y2, y3;
	u16		gridOffs;

//	OS_Printf("地形アップ\n");
	GetGroundGridData( pos, &gridx, &gridz, &gridOffs );
	GetGridVtxYLen( gridx, gridz, &y0, &y1, &y2, &y3 );

	SetGridUp( gridx, gridz, y0, y1, y2, y3 );
#if 0
	{
		BOOL endFlag = FALSE;

		while( endFlag == FALSE ){
			int x, z;
			endFlag = TRUE;

			for( z=0; z<mapSizeZ; z++ ){
				for( x=0; x<mapSizeX; x++ ){
					if( CheckGridVtxUp( x, z ) == FALSE ){
						endFlag = FALSE;
					}
				}
			}
		}
	}
#endif
}

void	SetMapGroundDown( VecFx32* pos )
{
	int		gridx, gridz;
	int		y0, y1, y2, y3;
	u16		gridOffs;

//	OS_Printf("地形ダウン\n");
	GetGroundGridData( pos, &gridx, &gridz, &gridOffs );
	GetGridVtxYLen( gridx, gridz, &y0, &y1, &y2, &y3 );

	SetGridDown( gridx, gridz, y0, y1, y2, y3 );
#if 0
	{
		BOOL endFlag = FALSE;

		while( endFlag == FALSE ){
			int x, z;
			endFlag = TRUE;

			for( z=0; z<mapSizeZ; z++ ){
				for( x=0; x<mapSizeX; x++ ){
					if( CheckGridVtxDown( x, z ) == FALSE ){
						endFlag = FALSE;
					}
				}
			}
		}
	}
#endif
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップとレイとの当たり座標取得
 */
//------------------------------------------------------------------
static BOOL checkOnTriangle( VecFx32* pos, 
		VecFx32* posRef0, VecFx32* posRef1, VecFx32* posRef2, VecFx32* vecN )
{
	VecFx32 vecEdge, vecVtx, vecTmp;
	fx32	scalar0, scalar1, scalar2;
			
	//一定回りに頂点を結び符号を確認する
	VEC_Subtract( pos, posRef0, &vecVtx );
	VEC_Subtract( posRef1, posRef0, &vecEdge );
	VEC_CrossProduct( &vecVtx, &vecEdge, &vecTmp );
	scalar0 = VEC_DotProduct( &vecTmp, vecN );

	VEC_Subtract( pos, posRef1, &vecVtx );
	VEC_Subtract( posRef2, posRef1, &vecEdge );
	VEC_CrossProduct( &vecVtx, &vecEdge, &vecTmp );
	scalar1 = VEC_DotProduct( &vecTmp, vecN );

	VEC_Subtract( pos, posRef2, &vecVtx );
	VEC_Subtract( posRef0, posRef2, &vecEdge );
	VEC_CrossProduct( &vecVtx, &vecEdge, &vecTmp );
	scalar2 = VEC_DotProduct( &vecTmp, vecN );

	if(( scalar0 > 0 )&&( scalar1 > 0 )&&( scalar2 > 0 )){
		return TRUE;
	}
	if(( scalar0 < 0 )&&( scalar1 < 0 )&&( scalar2 < 0 )){
		return TRUE;
	}
	return FALSE;
}

BOOL GetRayPosOnMap( const VecFx32* posRay, const VecFx32* vecRay, VecFx32* dst )
{
	VecFx32 vecRayXZ, vecLength, posStart, pos;
	fx32	limitLength;
	int		x, z;
	u16		offset, prevOffset; 

	//レイの方向成分をXZ方向だけ抜き出して正規化
	VEC_Set( &vecRayXZ, vecRay->x, 0, vecRay->z );
	VEC_Normalize( &vecRayXZ, &vecRayXZ );

	VEC_Set( &posStart, posRay->x, 0, posRay->z );		//検出開始地点
	prevOffset = 0xffff;	//検出初期化
	vecLength.x = 0;
	vecLength.y = 0;
	vecLength.z = 0;
	{
		fx32 x_x = FX_Mul( mapSizeX * FX32_ONE, mapSizeX * FX32_ONE ); 
		fx32 z_z = FX_Mul( mapSizeZ * FX32_ONE, mapSizeZ * FX32_ONE ); 
		fx32 xz = FX_Sqrt( x_x + z_z );

		limitLength = FX_Mul( mapGrid, xz );
	}
	while( VEC_Mag( &vecLength ) <= limitLength ){
		VEC_Add( &posStart, &vecLength, &pos );

		GetGroundGridData( &pos, &x, &z, &offset );
		if( CheckGetGroundOutSideData( x, z ) == FALSE ){

			if( offset != prevOffset ){	//同じデータ(offset)はパス
				MAP_GRID_DATA*	grid = &mapGridData[ offset ];
				VecFx32 posRef0, posRef1, posRef2, vecN, posCalc;
				BOOL result;
				int i;

				for( i=0; i<2; i++ ){
					//グリッド内三角形１が作る平面との交差判定
					posGridCalc( x, z, &grid->triangle[i].vtx0, &posRef0 );
					posGridCalc( x, z, &grid->triangle[i].vtx1, &posRef1 );
					posGridCalc( x, z, &grid->triangle[i].vtx2, &posRef2 );
					vecN = grid->triangle[i].vecN;
	
					result = GFL_G3D_Calc_GetClossPointRayPlane
								( posRay, vecRay, &posRef0, &vecN, &posCalc, 0 );
					if( result == GFL_G3D_CALC_TRUE ){
						if( checkOnTriangle(&posCalc, &posRef0, &posRef1, &posRef2, &vecN)==TRUE ){
							*dst = posCalc;
							return TRUE;
						}
					}
				}
				prevOffset = offset;
			}
		}
		VEC_Add( &vecLength, &vecRayXZ, &vecLength );
	}
	//レンジオーバーで検出失敗
	//意味はないが念のためdstに問題が発生しないような値を入れる
	dst->x = 0;
	dst->y = 0;
	dst->z = 0;
	return FALSE;
}



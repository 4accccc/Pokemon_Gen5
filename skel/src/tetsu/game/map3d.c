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

typedef struct {
	u8	vtxY0;	
	u8	vtxY1;
	u8	vtxY2;
	u8	vtxY3;
	u8	triangleType;

}MAP_GRIDMAKE_DATA;

typedef struct {
	VecFx32	vtx0;
	VecFx32	vtx1;
	VecFx32	vtx2;
	VecFx32	vecN;
	fx32	valD;

}MAP_TRI_DATA;

typedef struct {
	MAP_TRI_DATA	triangle[2];
	u32				triangleType;

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
};
//３Ｄグラフィックリソーステーブル
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ (u32)g3DarcPath2, NARC_haruka_test_floor3_nsbmd,  GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath2, NARC_haruka_test_wall_nsbmd,  GFL_G3D_UTIL_RESPATH },
};

//---------------------
enum {
	G3DOBJ_MAP_FLOOR = 0,
	G3DOBJ_EFFECT_WALL,
};
//３Ｄオブジェクト定義テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_MAP_FLOOR, 0, G3DRES_MAP_FLOOR, NULL, 0 },
	{ G3DRES_EFFECT_WALL, 0, G3DRES_EFFECT_WALL, NULL, 0 },
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
	sceneMap->defaultFloor.id = GFL_G3D_SCENEOBJ_Add
								( sceneMap->g3Dscene, pdata, count, sceneMap->objIdx );
	sceneMap->defaultFloor.count = count;
	sceneMap->mapAttr = mapAttr;

	//マップオブジェクト設定後、セットアップ用配列は破棄
	GFL_HEAP_FreeMemory( pdata );
}

//------------------------------------------------------------------
static void MapDataDelete( SCENE_MAP* sceneMap )
{
	GFL_G3D_SCENEOBJ_Remove
			(sceneMap->g3Dscene, sceneMap->defaultFloor.id, sceneMap->defaultFloor.count ); 
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

static const u8 mapGridPat[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//0
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//1
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//2
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//3
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//4
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//5
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//6
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//7
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//8
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//9
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,//10
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,//11
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,//12
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,8,8,8,8,9,0,0,0,0,0,0,0,0,0,0,//13
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//14
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,0,0,0,0,0,0,0,0,0,0,0,0,0,//15
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,0,0,0,0,0,0,0,0,0,0,0,0,0,//16
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//17
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,11,11,11,11,12,10,11,11,11,12,0,0,0,0,0,0,//18
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,14,14,14,14,15,13,14,14,14,15,0,0,0,0,0,0,//19
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,14,14,14,14,15,13,14,14,14,15,0,0,0,0,0,0,//20
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,14,14,14,14,15,13,14,14,14,15,0,0,0,0,0,0,//21
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,17,17,17,17,18,16,17,17,17,18,0,0,0,0,0,0,//22
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//23
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//24
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//25
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//26
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//27
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//28
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//29
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//30
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//31
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
{0,0,0,0,0},{0,0,0,0,0},{1,1,1,1,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
//16//
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
{0,0,0,0,0},{0,0,0,0,0},{2,2,2,2,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},
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
 * @brief	３Ｄマップ高さデータ作成（テスト用）
 */
//------------------------------------------------------------------
static void MakeTriangleData( MAP_TRI_DATA* triData, VecFx32* vtx0, VecFx32* vtx1, VecFx32* vtx2 );

static void CreateMapGridData( HEAPID heapID )
{
	int		i, x, z;
	VecFx32	vtx0, vtx1, vtx2, vtx3;

	mapGridData = GFL_HEAP_AllocClearMemory( heapID, sizeof(MAP_GRID_DATA)*mapSizeX*mapSizeZ );

	for( z=0; z<mapSizeZ; z++ ){
		for( x=0; x<mapSizeX; x++ ){
			i = z * mapSizeZ + x;

			vtx0.x = (x - 16) * mapGrid;
			vtx0.y = mapGrid1[i].vtxY0 * mapGrid/2;
			vtx0.z = (z - 16) * mapGrid;

			vtx1.x = (x - 16) * mapGrid + mapGrid;
			vtx1.y = mapGrid1[i].vtxY1 * mapGrid/2;
			vtx1.z = (z - 16) * mapGrid;

			vtx2.x = (x - 16) * mapGrid;
			vtx2.y = mapGrid1[i].vtxY2 * mapGrid/2;
			vtx2.z = (z - 16) * mapGrid + mapGrid;

			vtx3.x = (x - 16) * mapGrid + mapGrid;
			vtx3.y = mapGrid1[i].vtxY3 * mapGrid/2;
			vtx3.z = (z - 16) * mapGrid + mapGrid;

			mapGridData[i].triangleType = mapGrid1[i].triangleType;

			if( mapGridData[i].triangleType == 0 ){
				//0-1-2,3-2-1のパターン
				MakeTriangleData( &mapGridData[i].triangle[0], &vtx0, &vtx1, &vtx2 );
				MakeTriangleData( &mapGridData[i].triangle[1], &vtx3, &vtx2, &vtx1 );
			} else {
				//2-0-3,1-3-0のパターン
				MakeTriangleData( &mapGridData[i].triangle[0], &vtx2, &vtx0, &vtx3 );
				MakeTriangleData( &mapGridData[i].triangle[1], &vtx1, &vtx3, &vtx0 );
			}
		}
	}
}

static void DeleteMapGridData( void )
{
	GFL_HEAP_FreeMemory( mapGridData );
}

//------------------------------------------------------------------
static void MakeTriangleData( MAP_TRI_DATA* triData, VecFx32* vtx0, VecFx32* vtx1, VecFx32* vtx2 )
{
	VecFx32	vec0, vec1, vecN;

	triData->vtx0 = *vtx0;
	triData->vtx1 = *vtx1;
	triData->vtx2 = *vtx2;

	VEC_Subtract( vtx1, vtx0, &vec0 );
	VEC_Subtract( vtx2, vtx0, &vec1 );
	VEC_CrossProduct( &vec0, &vec1, &vecN );
	VEC_Normalize( &vecN, &vecN );

	if( vecN.y < 0 ){	//Y法線は+方向が表としてしまう
		VEC_Set( &triData->vecN, -vecN.x, -vecN.y, -vecN.z );
	} else {
		VEC_Set( &triData->vecN, vecN.x, vecN.y, vecN.z );
	}
	triData->valD = -( FX_Mul( triData->vecN.x, vtx0->x ) 
						+ FX_Mul( triData->vecN.y, vtx0->y ) 
						+ FX_Mul( triData->vecN.z, vtx0->z) );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ高さデータ取得(暫定)
 */
//------------------------------------------------------------------
static BOOL	CheckGetGroundOutSideData( const fx32 x, const fx32 z )
{
	if(( x < 0 )||( x >= mapGrid * mapSizeX )||( z < 0 )||( z >= mapGrid * mapSizeZ ) ){ 
		return TRUE;
	} else {
		return FALSE;
	}
}

static BOOL	GetGroundTraiangleID( const VecFx32* pos, u16* gridOffs, u16* ID )
{
	fx32	wx = pos->x + mapGrid*16;
	fx32	wz = pos->z + mapGrid*16;
	u16		offs = ( wz/mapGrid ) * mapSizeX + ( wx/mapGrid );
	int		fx = wx%mapGrid;
	int		fz = wz%mapGrid;

	if( CheckGetGroundOutSideData( wx, wz ) == TRUE ){
		return FALSE;
	}
	*gridOffs = offs;

	//グリッド内三角形の判定
	if( mapGridData[ offs ].triangleType == 0 ){
		//0-1-2,3-2-1のパターン
		if( fx + fz < mapGrid ){
			*ID = 0;
			return TRUE;	//0-1-2三角形内
		} else {
			*ID = 1;
			return TRUE;	//3-2-1三角形内
		}
	} else {
		//2-0-3,1-3-0のパターン
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

	if( GetGroundTraiangleID( pos, &gridOffs, &triangleID ) == FALSE ){
		return FALSE;
	}
	*vecN	= mapGridData[ gridOffs ].triangle[ triangleID ].vecN;
	*valD	= mapGridData[ gridOffs ].triangle[ triangleID ].valD;

	return TRUE;
}

void	GetGroundPlaneVecN( const VecFx32* pos, VecFx32* vecN )
{
	u16		gridOffs, triangleID;

	if( GetGroundTraiangleID( pos, &gridOffs, &triangleID ) == FALSE ){
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

	if( GetGroundTraiangleID( pos, &gridOffs, &triangleID ) == FALSE ){
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
 *
 * @brief		マップ上でのベクトル移動（重力あり）
 *
 */
//------------------------------------------------------------------
struct _CALC_PH_MV {
	fx32	gravityMove;
	fx32	gravityFall;
	fx32	planeMargin;
	fx32	absorbVal;
	u16		enableMoveTheta;

	VecFx32	vecDir;
	fx32	speed;
	u16		theta;

	VecFx32	vecSpeed;
	VecFx32	vecMove;
};

static void	inline VEC_Mult( const fx32 a, const VecFx32* b, VecFx32* dst ) 
{
	dst->x = FX_Mul( a, b->x );
	dst->y = FX_Mul( a, b->y );
	dst->z = FX_Mul( a, b->z );
}

static void	inline VEC_Divt( const fx32 a, const VecFx32* b, VecFx32* dst ) 
{
	GF_ASSERT( a );

	dst->x = FX_Div( b->x, a );
	dst->y = FX_Div( b->y, a );
	dst->z = FX_Div( b->z, a );
}

static void	inline VEC_Proj( const VecFx32* a, const VecFx32* b, VecFx32* dst ) 
{
	// Ｂは正規化済みであること
	// 　※ＡをＢに投影するのは、Ｂは正規化済みであることを前提で (Ａ・Ｂ)Ｂ
	// 　　正確には(Ａ・Ｂ)Ｂ／(Ｂの長さ)
	fx32 scalar = VEC_DotProduct( a, b );
	VEC_Mult( scalar, b, dst );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL	CheckOnGround( VecFx32* pos, fx32* y )
{
	fx32 groundHeight;

	GetGroundPlaneHeight( pos, &groundHeight );
	if( pos->y <= groundHeight + (0.001f * FX32_ONE) ){	//0.1fは誤差許容幅
		if( y != NULL ){
			*y = groundHeight;
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

static void	GetGroundGravity( const VecFx32* pos, VecFx32* vecG, const fx32 gravity )
{
	VecFx32 vecN, vecH, vecV;
	VecFx32 vecGravity = { 0, -FX32_ONE, 0 };

	//平面の法線ベクトルにより地面に垂直で斜面に並行なベクトルを算出
	GetGroundPlaneVecN( pos, &vecN );				//平面の法線を取得
	VEC_CrossProduct( &vecN, &vecGravity, &vecH );	//平面上の水平ベクトル算出
	VEC_CrossProduct( &vecN, &vecH, &vecV );		//平面上の斜面ベクトル算出
	if( vecV.y > 0 ){
		//正方向（天向き）へのベクトルが出てしまった場合負方向（地向き）へ修正
		VEC_Set( &vecV, -vecV.x, -vecV.y, -vecV.z );
	}
	VEC_Normalize( &vecV, &vecV );

	//重力ベクトルを投影し、かかるべき重力ベクトルを算出
	vecGravity.y = -gravity;
	VEC_Proj( &vecGravity, &vecV, vecG ); 
}

static void	GetGroundVec( const VecFx32* pos, const VecFx32* vecDir, VecFx32* vecGround )
{
	VecFx32 vecN, vecH, vecV;

	//平面の法線ベクトルにより移動ベクトルに垂直で斜面に並行なベクトルを算出
	GetGroundPlaneVecN( pos, &vecN );				//平面の法線を取得
	VEC_CrossProduct( &vecN, vecDir, &vecH );		//平面上の水平ベクトル算出
	VEC_CrossProduct( &vecN, &vecH, &vecV );		//平面上の斜面ベクトル算出

	if( VEC_DotProduct( &vecV, vecDir ) < 0 ){
		//逆方向へのベクトルが出てしまった場合修正
		VEC_Set( vecGround, -vecV.x, -vecV.y, -vecV.z );
	} else {
		VEC_Set( vecGround, vecV.x, vecV.y, vecV.z );
	}
	VEC_Normalize( vecGround, vecGround );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
CALC_PH_MV*	CreateCalcPhisicsMoving( HEAPID heapID, PHMV_SETUP* setup )
{
	CALC_PH_MV*	calcPHMV = GFL_HEAP_AllocClearMemory( heapID, sizeof(CALC_PH_MV) );

	calcPHMV->gravityMove		= setup->gravityMove;
	calcPHMV->gravityFall		= setup->gravityFall;
	calcPHMV->planeMargin		= -FX_SinIdx( setup->planeMarginTheta );
	calcPHMV->absorbVal			= setup->absorbVal;
	calcPHMV->enableMoveTheta	= setup->enableMoveTheta;
	
	ResetMovePHMV( calcPHMV );
	return calcPHMV;
}

void DeleteCalcPhisicsMoving( CALC_PH_MV* calcPHMV )
{
	GFL_HEAP_FreeMemory( calcPHMV );
}

//------------------------------------------------------------------
void SetAbsorbPHMV( CALC_PH_MV* calcPHMV, fx32 absorbVal )
{
	calcPHMV->absorbVal = absorbVal;
}

//------------------------------------------------------------------
void SetMoveVecPHMV( CALC_PH_MV* calcPHMV, VecFx32* dirXZ )
{
	VecFx32 vecDir, vecMove;

	vecDir.x = FX_Mul( dirXZ->x, FX_CosIdx( calcPHMV->theta ) );
	vecDir.y = FX_SinIdx( calcPHMV->theta );
	vecDir.z = FX_Mul( dirXZ->z, FX_CosIdx( calcPHMV->theta ) );
	VEC_Normalize( &vecDir, &vecDir );
	VEC_Mult( calcPHMV->speed, &vecDir, &vecDir );

	VEC_Add( &vecDir, &calcPHMV->vecSpeed, &vecDir );
	VEC_Normalize( &vecDir, &vecDir );

	VEC_Mult( VEC_Mag( &calcPHMV->vecSpeed ), &vecDir, &calcPHMV->vecSpeed );
}

//------------------------------------------------------------------
void GetMoveDirPHMV( CALC_PH_MV* calcPHMV, VecFx32* vecDir )
{
	VEC_Normalize( &calcPHMV->vecSpeed, vecDir );
}

//------------------------------------------------------------------
void GetMoveVecPHMV( CALC_PH_MV* calcPHMV, VecFx32* vecMove )
{
	*vecMove = calcPHMV->vecMove;
}

//------------------------------------------------------------------
fx32 GetMoveSpeedPHMV( CALC_PH_MV* calcPHMV )
{
	return VEC_Mag( &calcPHMV->vecSpeed );
}

//------------------------------------------------------------------
void SetMoveSpeedPHMV( CALC_PH_MV* calcPHMV, fx32 speed )
{
	VecFx32 vecSpeed;

	VEC_Normalize( &calcPHMV->vecSpeed, &vecSpeed );
	VEC_Mult( speed, &vecSpeed, &calcPHMV->vecSpeed );
}
//------------------------------------------------------------------
void AddMoveSpeedPHMV( CALC_PH_MV* calcPHMV, fx32 speed )
{
	VecFx32 vecSpeed;
	fx32	valSpeed = VEC_Mag( &calcPHMV->vecSpeed ) + speed;

	VEC_Normalize( &calcPHMV->vecSpeed, &vecSpeed );
	VEC_Mult( valSpeed, &vecSpeed, &calcPHMV->vecSpeed );
}

//------------------------------------------------------------------
BOOL CheckOnFloorPHMV( CALC_PH_MV* calcPHMV, VecFx32* pos )
{
	return CheckOnGround( pos, NULL );	//TRUE = 地上, FALSE = 空中
}

//------------------------------------------------------------------
BOOL CheckGroundGravityPHMV( CALC_PH_MV* calcPHMV, VecFx32* pos )
{
	VecFx32 vecG;

	GetGroundGravity( pos, &vecG, FX32_ONE );
	if( vecG.y < 0 ){
		return TRUE;	//空中or地上斜面(重力による仕事が発生している状態)
	} else {
		return FALSE;	//地上平面
	}
}

//------------------------------------------------------------------
void StartMovePHMV( CALC_PH_MV* calcPHMV, VecFx32* dirXZ, fx32 speed, u16 theta )
{
	calcPHMV->speed = speed;
	calcPHMV->theta = theta;

	calcPHMV->vecDir.x = FX_Mul( dirXZ->x, FX_CosIdx( calcPHMV->theta ) );
	calcPHMV->vecDir.y = FX_SinIdx( calcPHMV->theta );
	calcPHMV->vecDir.z = FX_Mul( dirXZ->z, FX_CosIdx( calcPHMV->theta ) );
	VEC_Normalize( &calcPHMV->vecDir, &calcPHMV->vecDir );

	VEC_Mult( calcPHMV->speed, &calcPHMV->vecDir, &calcPHMV->vecSpeed ); 
}

//------------------------------------------------------------------
void ResetMovePHMV( CALC_PH_MV* calcPHMV )
{
	calcPHMV->speed = 0;
	calcPHMV->theta = 0;

	calcPHMV->vecDir.x = 0;
	calcPHMV->vecDir.y = 0;
	calcPHMV->vecDir.z = 0;

	calcPHMV->vecSpeed.x = 0;
	calcPHMV->vecSpeed.y = 0;
	calcPHMV->vecSpeed.z = 0;
}

//------------------------------------------------------------------
BOOL	CalcMovePHMV( CALC_PH_MV* calcPHMV, VecFx32* posNow )
{
	VecFx32 vecMove, vecG, vecSpeed, posNext;
	VecFx32	vecSpeed_next;
	u16		thetaGround;

	if( !calcPHMV->vecDir.y ){
		//水平移動

		//移動方向の地形に沿った単位ベクトル取得
		GetGroundVec( posNow, &calcPHMV->vecSpeed, &vecMove );
		//移動ベクトル取得
		VEC_Mult( VEC_Mag( &calcPHMV->vecSpeed ), &vecMove, &calcPHMV->vecSpeed );
		//予想移動位置の計算
		VEC_Add( posNow, &calcPHMV->vecSpeed, &posNext );
		GetGroundGravity( &posNext, &vecG, calcPHMV->gravityMove );

		//速度の平均を算出( V0+V1 / 2 )
		VEC_Add( &calcPHMV->vecSpeed, &vecG, &vecSpeed_next );
		VEC_Add( &calcPHMV->vecSpeed, &vecSpeed_next, &vecSpeed );
		VEC_Divt( 2 * FX32_ONE, &vecSpeed, &vecSpeed );

		calcPHMV->vecSpeed = vecSpeed_next;
		//OS_Printf("speed{ %d }\n ", VEC_Mag( &calcPHMV->vecSpeed ));

		//移動位置の計算
		VEC_Add( posNow, &vecSpeed, &posNext );
		GetGroundPlaneHeight( &posNext, &posNext.y );
	} else {
		//空中移動
		vecG.x = 0;
		vecG.y = -calcPHMV->gravityFall;
		vecG.z = 0;
		
		//速度の平均を算出( V0+V1 / 2 )
		VEC_Add( &calcPHMV->vecSpeed, &vecG, &vecSpeed_next );
		VEC_Add( &calcPHMV->vecSpeed, &vecSpeed_next, &vecSpeed );
		VEC_Divt( 2 * FX32_ONE, &vecSpeed, &vecSpeed );

		calcPHMV->vecSpeed = vecSpeed_next;

		//移動位置の計算
		VEC_Add( posNow, &vecSpeed, &posNext );
		CheckOnGround( &posNext, &posNext.y );
	}
	VEC_Subtract( &posNext, posNow, &calcPHMV->vecMove );
	*posNow = posNext;

	return TRUE;
}




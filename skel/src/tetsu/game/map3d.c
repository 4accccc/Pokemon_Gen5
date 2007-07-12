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
struct _SCENE_MAP {
	GFL_G3D_SCENE*	g3Dscene;
	u16*			mapAttr;
	int				objID;
	int				objCount;
};

typedef struct {
	const char*				data;
	GFL_G3D_SCENEOBJ_DATA	floor;
}MAPDATA;

#define mapSizeX	(32)
#define mapSizeZ	(32)
#define mapGrid		(FX32_ONE*64)
#define defaultMapX	(-mapGrid*16+mapGrid/2)
#define defaultMapZ	(-mapGrid*16+mapGrid/2)

static GFL_G3D_SCENEOBJ_DATA*	
		MapDataCreate( SCENE_MAP* sceneMap, const MAPDATA* map, HEAPID heapID );
static const	MAPDATA mapDataTbl;

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ生成
 */
//------------------------------------------------------------------
SCENE_MAP*	Create3Dmap( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	GFL_G3D_SCENEOBJ_DATA* data;
	SCENE_MAP* sceneMap = GFL_HEAP_AllocMemory( heapID, sizeof(SCENE_MAP) );

	//マップ作成
	data = MapDataCreate( sceneMap, &mapDataTbl, heapID );

	//マップオブジェクト追加
	sceneMap->objID = GFL_G3D_SCENEOBJ_Add( g3Dscene, data, sceneMap->objCount );
	sceneMap->g3Dscene = g3Dscene;

	//マップオブジェクト設定後、セットアップ用配列は破棄
	GFL_HEAP_FreeMemory( data );

	return sceneMap;
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ破棄
 */
//------------------------------------------------------------------
void	Delete3Dmap( SCENE_MAP* sceneMap )
{
	GFL_G3D_SCENEOBJ_Remove(sceneMap->g3Dscene, sceneMap->objID, sceneMap->objCount ); 
	GFL_HEAP_FreeMemory( sceneMap->mapAttr );
	GFL_HEAP_FreeMemory( sceneMap );
}

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップアトリビュート取得
 */
//------------------------------------------------------------------
int work[2];
BOOL		Get3DmapAttr( SCENE_MAP* sceneMap, VecFx32* pos, u16* attr )
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
 * @brief		３Ｄマップデータ作成
 */
//------------------------------------------------------------------
static GFL_G3D_SCENEOBJ_DATA*	
		MapDataCreate( SCENE_MAP* sceneMap, const MAPDATA* map, HEAPID heapID )
{
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

	sceneMap->objCount = count;
	sceneMap->mapAttr = mapAttr;

	return pdata;
}

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

static const MAPDATA mapDataTbl = {
	mapData0,
	{	G3DOBJ_MAP_FLOOR, 0, 1, 31, FALSE, TRUE,
		{	{ 0, 0, 0 },
			{ FX32_ONE*8, FX32_ONE*8, FX32_ONE*8 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};





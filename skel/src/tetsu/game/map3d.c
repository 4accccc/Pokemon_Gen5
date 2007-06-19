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

#define mapSizeX	16//(32)
#define mapSizeZ	16//(32)
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
u16		Get3DmapAttr( SCENE_MAP* sceneMap, VecFx32* pos )
{
	int x = (pos->x + mapGrid*16)/mapGrid;
	int z = (pos->z + mapGrid*16)/mapGrid;
	return sceneMap->mapAttr[z*mapSizeX+x];
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
#if 0
	for( z=0; z<sizeZ; z++ ){
		for( x=0; x<sizeX; x++ ){
			mapCode = GET_MAPCODE( mapData, x, z );
			switch( mapCode ){
			default:
			case '　':
				mapAttr[z*sizeX+x] = 0;
				break;
			case '■':	//壁
				GetWallData( &data, mapCode, 
							GET_MAPCODE( mapData, x, z-1 ), GET_MAPCODE( mapData, x, z+1 ), 
							GET_MAPCODE( mapData, x-1, z ), GET_MAPCODE( mapData, x+1, z ) );
				data.status.trans.x = defaultMapX + (mapGrid * x);
				data.status.trans.z = defaultMapZ + (mapGrid * z);
				data.func = moveWall;
				pdata[ count ] = data;
				count++;
				mapAttr[z*sizeX+x] = 1;
				break;
			case '□':	//透過壁
				GetWallData( &data, mapCode, 
							GET_MAPCODE( mapData, x, z-1 ), GET_MAPCODE( mapData, x, z+1 ), 
							GET_MAPCODE( mapData, x-1, z ), GET_MAPCODE( mapData, x+1, z ) );
				data.status.trans.x = defaultMapX + (mapGrid * x);
				data.status.trans.z = defaultMapZ + (mapGrid * z);
				data.func = moveSkelWall;
				pdata[ count ] = data;
				count++;
				mapAttr[z*sizeX+x] = 1;
				break;
			case '○':	//配置人物１
				//data.objID = G3DOBJ_HARUKA_STOP; 
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
				mapAttr[z*sizeX+x] = 0;
				break;
			case '◎':	//プレーヤー
				tetsuWork->contPos.x = defaultMapX + (mapGrid * x);
				tetsuWork->contPos.y = 0;
				tetsuWork->contPos.z = defaultMapZ + (mapGrid * z);
				mapAttr[z*sizeX+x] = 0;
				break;
			case '●':	//配置人物２
				mapAttr[z*sizeX+x] = 0;
				break;
			}
		}
	}
#endif
	sceneMap->objCount = count;
	sceneMap->mapAttr = mapAttr;

	return pdata;
}

static const char mapData0[] = {
"■■■■■■■■■■■■■■■■"
"■　　　○　　　　○　　　　　■"
"■　■■■■■■■■■■■■　■"
"■　■　　　　　　○　　　■　■"
"■○■　■■■　■■■■　■○■"
"■　■　■　■　　　　■○■　■"
"■　■　■　■○　　　■　■　■"
"■　　　■　■■■　　■　■　■"
"■　■　■　　　■　　　　■　■"
"■　■　■■■　■■■■　■○■"
"■　■　　　　　　　　■○■　■"
"■　■■■　■■■■■■　■　■"
"■　◎　■　　　　○　　　■　■"
"■　　　■■■■■　■■■■　■"
"■　　　　　○　　　　　　　○■"
"■■■■■■■■■■■■■■■■"
};

static const char mapData1[] = {
"■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■"	//32
"■　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　■"	//2///34
"■　　　○　　　　　　　　　　　　　　　　○　　　　　　　　　■"	//2
"■　　■■■■■■■■■■■■■■■■■■■■■■■■■■　　■"	//28
"■　　■　　　　　　　　　　　　○　　　　　　　　　　　■　　■"	//4
"■　　■　■■■■■■■■■■■■■■■■■■■■■■　■　　■"	//26
"■　　■　■　　　　○　　　　　　　　　　　　　　　■　■　　■"	//6
"■　　■　■　■■■■■■■■■■■■■■■■■■　■　■　　■"	//24
"■　　■　■　■　　　　　　　　　　　　○　　　■　■　■　　■"	//8
"■　　■　■　■　■■■■■■■■■■■■■■　■○■　■　　■"	//22
"■　　■　■　■　■　　　　　　　○　　　　■　■　■　■　　■"	//10
"■　　■　■　■　■○■■■■■■■■■■　■　■　■　■　　■"	//20
"■　　■　■　■　■　■　　　　　　　　■　■　■　■○■　　■"	//12
"■　　■　■　■　■　■　■■■■■■　■　■　■　■　■　　■"	//18///180
"■　　■　■　■　■　■　■　　　　■○■　■　■　■　■　　■"	//14
"■　　■○■　■　■　■　■　●　　■　■　■　■　■　■○　■"	//14
"■　　■　■　■　■　■○■　　　　■　■　■　■　■　■　　■"	//14
"■　　■　■　■　■　■　■　　　　■　■　■　■　■　■　　■"	//14///56////270
"■　○■　■　■　■　■　■　■■■■　■○■　■　■　■　　■"	//17
"■　　■　■　■　■　　　　　　　　　　■　■　■　■　■　　■"	//11
"■　　■　■　■　■　■■■■■■■■■■　■　■　■　■　　■"	//20
"■　　■　■○■　■　　　　　○　　　　　　■　■　■　■　　■"	//10///58
"■　　■　■　■　■　■■■■■■■■■■■■　■　■　■　　■"	//21
"■　○■　■　　　　　○　　　　　　　　　○　　■　■　■　　■"	//7
"■　　■　■　■■■■■■■■■■■■■■■■■■　■　■　　■"	//24
"■　　■　■　　　　　　　　　　○　　　　　　　　　■　■　　■"	//6///58
"■　　■　■　■■■■■■■■■■■■■■■■■■■■　■○　■"	//25
"■　　　　　　　　　　○　　　　　　　　　　○　　　　　■　　■"	//3
"■　　□■■■■■■■■■■■■■■■■■■■■■■■■■　　■"	//28
"■　　　　　○　　　　　　　　　　○　　　　　　　　　　　　　■"	//2///58
"■　　　　　　　　　　　　○　　　　　　　　　　　　　　　　　■"	//2
"■　◎　■■■■■■■■■■■■■■■■■■■■■■■■■■■■"	//32///34////208
};//478+27

static const char mapData2[] = {
"■　■　　　　　　　　　　　■　　■　　　■　　　　■　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　■　　　　■　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　■　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　■　　　　■　　　　　"	//0
"■　■■■■　■■■■■■■■　　■　　　■　　　　　　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　■■■■■■■■■■■"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　　　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　　　　　　　"	//0
"■　■　　　　　　　　　　　■　　■　　　　　　　　　　　　　　"	//0
"■　■■■■■■■■　■■■■　　■■■■■■■　　■■■■■■"	//0
"■　■　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//0
"■　■　　　　　■■■■■■■■■■■■■■■■■■■■　　■■"	//0
"■　■　　　　　■　　　　　　　　　　　　　■　　　　　　　　　"	//0
"■　■　　　　　■　　□□□■■■□□□　　■　　　　　　　　　"	//0
"■　■　　　　　■　　□　　　　　　　□　　■　　　　　　　　　"	//0
"■　■■■■　　■　　□　　　　　　　□　　■　　　　　　　　　"	//0
"　　　　　　　　■　　□　　　　　　　□　　■　　■■■■■■■"	//0
"　　　　　　　　■　　□　　　　　　　□　　■　　■　　　　　　"	//0
"　　　■　　　　■　　□□□　　　□□□　　■　　■　　　　　　"	//0
"　　　■　　　　■　　　　　　　　　　　　　■　　■■■■■■　"	//0
"　　　■　　　　■　　　　　　　　　　　　　■　　■　　　　　　"	//0
"　　　■　　　　■　　　　　　　　　　　　　■　　■　　　　　　"	//0
"　　　■■■■■■■■■■■■■■■■■　　■　　■■■■■■　"	//0
"　　　■　　　　　　　　　　　　　　　　　　　　　■　　　　　　"	//0
"　　　■　　　　　　　　　　　　　　　　　　　　　■　　　　　　"	//0
"　　　■■■■■■■■■■■■■■■■■■■■■■■■■■■■　"	//0
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//0
"　　　■■■■■■■■■■■■　　■■■■■■■■■■　■■■■"	//0
"　　　■■　　　　　　　　　　　　　　　　　　　　■■　　　　　"	//0
"　　　■■　　　　　■■■■■■■■■■■■■　　■■　　　　　"	//0
"　◎　■■　　　　　■■■■■■■■■■■■■　　■■　　　　　"	//0
"　　　■■　　　　　　　　　　　　　　　　　　　　■■　　　　　"	//0
};//210+56+203-19 = 450

static const char mapData3[] = {
"○　○　○　○　○　○　○　○　○　○　○　○　○　○　○　○　"	//16
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　■　■　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　■◎■　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
};

static const char mapData4[] = {
"　　○　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//16
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　◎　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
"　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　"	//
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

#if 0
typedef struct {
	u16		wallCode;
	fx32	rotateY;
}WALL_CODE_TBL;

static const WALL_CODE_TBL wallCodeTbl[] = {	//上下左右の壁あり状態
	{ G3DOBJ_MAP_WALL,		0x0000 },			//空空空空
	{ G3DOBJ_MAP_WALL3,		0x8000 },			//壁空空空
	{ G3DOBJ_MAP_WALL3,		0x0000 },			//空壁空空
	{ G3DOBJ_MAP_WALL2_2,	0x0000 },			//壁壁空空
	{ G3DOBJ_MAP_WALL3,		0xc000 },			//空空壁空
	{ G3DOBJ_MAP_WALL2_1,	0xc000 },			//壁空壁空
	{ G3DOBJ_MAP_WALL2_1,	0x0000 },			//空壁壁空
	{ G3DOBJ_MAP_WALL1,		0x4000 },			//壁壁壁空
	{ G3DOBJ_MAP_WALL3,		0x4000 },			//空空空壁
	{ G3DOBJ_MAP_WALL2_1,	0x8000 },			//壁空空壁
	{ G3DOBJ_MAP_WALL2_1,	0x4000 },			//空壁空壁
	{ G3DOBJ_MAP_WALL1,		0xc000 },			//壁壁空壁
	{ G3DOBJ_MAP_WALL2_2,	0x4000 },			//空空壁壁
	{ G3DOBJ_MAP_WALL1,		0x0000 },			//壁空壁壁
	{ G3DOBJ_MAP_WALL1,		0x8000 },			//空壁壁壁
	{ G3DOBJ_MAP_WALL0,		0x0000 },			//壁壁壁壁
};

static void GetWallData
			( GFL_G3D_SCENEOBJ_DATA* data, u16 chkCode, u16 up, u16 down, u16 left, u16 right ) 
{
	u16 pattern = 0;
	VecFx32 rotate = {0,0,0};
	MtxFx33 rotMtx = defaultStatus.rotate;

	if( up == chkCode ){
		pattern |= 0x0001;
	}
	if( down == chkCode ){
		pattern |= 0x0002;
	}
	if( left == chkCode ){
		pattern |= 0x0004;
	}
	if( right == chkCode ){
		pattern |= 0x0008;
	}
	data->objID = wallCodeTbl[ pattern ].wallCode;
	data->movePriority = 0;
	data->drawPriority = 2;
	data->cullingFlag = TRUE;
	data->drawSW = TRUE;
	data->blendAlpha = 31;
	data->status = defaultStatus;

	rotate.y = wallCodeTbl[ pattern ].rotateY;
	rotateCalc( &rotate, &rotMtx );
	data->status.rotate = rotMtx;
}

#endif

#if 0
//------------------------------------------------------------------
/**
 * @brief	３Ｄ動作
 */
//------------------------------------------------------------------
#define VIEW_LENGTH		(64*2)
#define VIEW_SCALAR_MAX	(0x20000)

#define BETWEEN_SCALAR	(-0x01000000)
#define BETWEEN_THETA	(0x01100)

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

static inline void SetDrawSW( GFL_G3D_SCENEOBJ* sceneObj, BOOL sw )
{
	BOOL swBuf = sw;
	GFL_G3D_SCENEOBJ_SetDrawSW( sceneObj, &swBuf );
}

static inline void SetAlpha( GFL_G3D_SCENEOBJ* sceneObj, u8 alpha, int scalar )
{
	//半透明処理の関係上、奥に見えるものから優先( 距離を200分割 )
	u8	drawPri = 249 - ( scalar * 200 / (VIEW_LENGTH * VIEW_LENGTH) );
	u8	alphaBuf = alpha;

	GFL_G3D_SCENEOBJ_SetDrawPri( sceneObj, &drawPri );
	GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &alphaBuf );
}

static inline void ResetAlpha( GFL_G3D_SCENEOBJ* sceneObj )
{
	u8	alphaBuf = 31;
	u8	drawPri = 3;

	GFL_G3D_SCENEOBJ_SetDrawPri( sceneObj, &drawPri );
	GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &alphaBuf );
}

//対象物とカメラとの視界位置によるカリング（ＸＺのみ）
static BOOL checkPos_bitweenCamera( GFL_G3D_CAMERA* camera, u16 theta, VecFx32* objPos )
{
	VecFx32 cameraPos, cameraTarget;
	VecFx32 vecView, vecObj;
	VecFx32 vecA_cross, vecB_cross;
	fx32	sin, cos;
	int		scalarA, scalarB;
	s16		tmpTheta;

	//カメラ情報取得
	GFL_G3D_CAMERA_GetPos( camera, &cameraPos );
	GFL_G3D_CAMERA_GetTarget( camera, &cameraTarget );

	//視界ベクトル計算（整数部のみ）
	vecView.x = ( cameraTarget.x - cameraPos.x ) >> FX32_SHIFT;
	vecView.z = ( cameraTarget.z - cameraPos.z ) >> FX32_SHIFT;

	//対象物体ベクトル計算（整数部のみ）
	vecObj.x = ( objPos->x - cameraPos.x ) >> FX32_SHIFT;
	vecObj.z = ( objPos->z - cameraPos.z ) >> FX32_SHIFT;

	//カメラの方向から左側境界ベクトルＡに直交するベクトルを作成
	tmpTheta = -theta + 0x4000;	//負方向θ+90°の角度値
	sin = FX_SinIdx((u16)tmpTheta);
	cos = FX_CosIdx((u16)tmpTheta);
	vecA_cross.x = (vecView.x * cos ) - (vecView.z * sin );
	vecA_cross.z = (vecView.x * sin ) + (vecView.z * cos );

	//カメラの方向から右側境界ベクトルＢに直交するベクトルを作成
	tmpTheta = theta - 0x4000;	//正方向θ-90°の角度値
	sin = FX_SinIdx((u16)tmpTheta);
	cos = FX_CosIdx((u16)tmpTheta);
	vecB_cross.x = (vecView.x * cos ) - (vecView.z * sin );
	vecB_cross.z = (vecView.x * sin ) + (vecView.z * cos );

	//ベクトルＡに直交するベクトルとベクトルＢに直交するベクトルに対しての内積が
	//ともに指定スカラー値範囲内であれば２方向間に存在すると決定する
	scalarA = vecA_cross.x * vecObj.x + vecA_cross.z * vecObj.z;
	scalarB = vecB_cross.x * vecObj.x + vecB_cross.z * vecObj.z;
	if(( scalarA < BETWEEN_SCALAR )||( scalarB < BETWEEN_SCALAR )){
		return FALSE;
	} else {
		return TRUE;
	}
}

//対象物の周囲位置によるカリング（ＸＺのみ）
static BOOL checkPos_aroundTarget( GFL_G3D_CAMERA* camera, fx32 len, VecFx32* objPos )
{
	VecFx32 cameraTarget;
	fx32	diffX, diffZ, diffLen;
				
	GFL_G3D_CAMERA_GetTarget( camera, &cameraTarget );

	diffX = ( cameraTarget.x - objPos->x ) >> FX32_SHIFT;
	diffZ = ( cameraTarget.z - objPos->z ) >> FX32_SHIFT;
	diffLen = ( diffX * diffX + diffZ * diffZ );
	if( diffLen > len ){
		return FALSE;
	} else {
		return TRUE;
	}
}

//２Ｄカリング
static BOOL culling2DView( GFL_G3D_SCENEOBJ* sceneObj, VecFx32* objPos, int* scalar )
{
	GFL_G3D_CAMERA*	camera;

	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		//カメラポインタ取得
		//camera = tetsuWork->g3Dcamera[0];	//下カメラ
		camera = Get_GS_G3Dcamera( tetsuWork->gs, MAINCAMERA_ID );	//下カメラ
		//カメラ位置による事前カリング
		*scalar = GFL_G3D_CAMERA_GetDotProductXZfast( camera, objPos );
		//カメラとのスカラーによる位置判定(0は水平、正は前方、負は後方)
		if( *scalar < -0x800 ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
		//ターゲット視界判定によるカリング
		if( checkPos_bitweenCamera( camera, BETWEEN_THETA, objPos ) == FALSE ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
		DebugDrawCount++;
	} else {
		//カメラポインタ取得
		//camera = tetsuWork->g3Dcamera[1];	//上カメラ
		camera = Get_GS_G3Dcamera( tetsuWork->gs, SUBCAMERA_ID );	//下カメラ
		//カメラ位置による事前カリング
		*scalar = GFL_G3D_CAMERA_GetDotProductXZfast( camera, objPos );
		//カメラとのスカラーによる位置判定(0は水平、正は前方、負は後方)
		if( *scalar < -0x800 ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
		//ターゲット周囲判定によるカリング
		if( checkPos_aroundTarget( camera, VIEW_LENGTH * VIEW_LENGTH, objPos ) == FALSE ){
			DebugCullingCount++;
			SetDrawSW( sceneObj, FALSE );
			return FALSE;
		}
	}
	SetDrawSW( sceneObj, TRUE );
	return TRUE;
}
//------------------------------------------------------------------
static void moveWall( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	minePos;
	int		scalar;
			
	GFL_G3D_SCENEOBJ_GetPos( sceneObj, &minePos );
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
	//スカラーによる位置判定により半透明処理をする※ターゲット位置に相当するスカラー値
	if(( scalar < 0x800 )&& GFL_G3D_DOUBLE3D_GetFlip()){ 
		SetAlpha( sceneObj, 16, scalar );
	} else {
		ResetAlpha( sceneObj );
	}
}

//------------------------------------------------------------------
static void moveSkelWall( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	minePos;
	int		scalar;
			
	GFL_G3D_SCENEOBJ_GetPos( sceneObj, &minePos );
	if( culling2DView( sceneObj, &minePos, &scalar ) == FALSE ) return;
	SetAlpha( sceneObj, 8, scalar );
}

#endif





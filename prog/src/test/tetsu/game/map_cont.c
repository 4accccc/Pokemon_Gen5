//============================================================================================
/**
 * @file	map_cont.c
 * @brief	マップコントロール
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"

typedef struct _MAP_CONTROL MAP_CONTROL;

//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
#define MAP_BLOCK_COUNT	(4)
#define MAP_BLOCK_NULL	(0xffff)

typedef struct {
	void*	p;
}MAP3D_SYS;

typedef struct {
	u16		block[MAP_BLOCK_COUNT];
}MAP_BLOCK_IDX;

typedef struct {
	int		idx;
}MAP_BLOCK_DATA;

struct _MAP_CONTROL {
	HEAPID			heapID;
	MAP3D_SYS*		map3Dsys;
	MAP_BLOCK_IDX	mapBlockIdx;
	MAP_BLOCK_DATA	mapBlock[MAP_BLOCK_COUNT];
};

#define MAP_BLOCK_LENX		(MAP3D_GRID_LEN * MAP3D_GRID_SIZEX)
#define MAP_BLOCK_LENZ		(MAP3D_GRID_LEN * MAP3D_GRID_SIZEZ)
#define MAP_BLOCK_SIZX		(32)
#define MAP_BLOCK_SIZZ		(32)
#define MAP_BLOCK_IDXMAX	(MAP_BLOCK_SIZX * MAP_BLOCK_SIZZ)

MAP_CONTROL*	Create3DmapControl( MAP3D_SYS* map3Dsys,  HEAPID heapID );
void			Delete3DmapControl( MAP_CONTROL* mc );
void			Main3DmapControl( MAP_CONTROL* mc, VecFx32* pos );

void  	Get3DmapBlockIndex( const VecFx32* pos, MAP_BLOCK_IDX* mapBlockIdx );
//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
/**
 * @brief	マップコントローラ生成
 */
//------------------------------------------------------------------
MAP_CONTROL*	Create3DmapControl( MAP3D_SYS* map3Dsys,  HEAPID heapID )
{
	MAP_CONTROL* mc = GFL_HEAP_AllocClearMemory( heapID, sizeof(MAP_CONTROL) );
	int i;

	mc->heapID = heapID;
	mc->map3Dsys = map3Dsys;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		mc->mapBlockIdx.block[i] = MAP_BLOCK_NULL;
	}
	return mc;
}

//------------------------------------------------------------------
/**
 * @brief	マップコントローラ破棄
 */
//------------------------------------------------------------------
void	Delete3DmapControl( MAP_CONTROL* mc )
{
	int	i;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		//if( mc->mapBlockIdx != MAP_BLOCK_NULL ){
			//Delete
		//}
	}
	GFL_HEAP_FreeMemory( mc );
}

//------------------------------------------------------------------
/**
 * @brief	マップコントローラメイン
 */
//------------------------------------------------------------------
void	Main3DmapControl( MAP_CONTROL* mc, VecFx32* pos )
{
	MAP_BLOCK_IDX nowBlockIdx;
	int	i;

 	Get3DmapBlockIndex( pos, &nowBlockIdx );

	if( mc->mapBlockIdx.block[0] != nowBlockIdx.block[0] ){
		//ＩＮＤＥＸの小さい順に格納されているので先頭が異なれば全部違う

		//reload
		
		//情報更新
		for( i=0; i<MAP_BLOCK_COUNT; i++ ){
			mc->mapBlockIdx.block[i] = nowBlockIdx.block[i];
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	マップブロック取得
 */
//------------------------------------------------------------------
static const int blockIdxOffs[][MAP_BLOCK_COUNT][2] = {
	{ {-1,-1},{-1, 0},{ 0,-1},{ 0, 0} },	//自分のいるブロックから左上方向に４ブロックとる
	{ {-1, 0},{-1, 1},{ 0, 0},{ 0, 1} },	//自分のいるブロックから右上方向に４ブロックとる
	{ { 0,-1},{ 0, 0},{ 1,-1},{ 1, 0} },	//自分のいるブロックから左下方向に４ブロックとる
	{ { 0, 0},{ 0, 1},{ 1, 0},{ 1, 1} },	//自分のいるブロックから右下方向に４ブロックとる
};

void  	Get3DmapBlockIndex( const VecFx32* pos, MAP_BLOCK_IDX* mapBlockIdx )
{
	u16		blockIdx, blockx, blockz;
	fx32	posx, posz;
	int		i, blockPat = 0;
	
	blockx = pos->x/MAP_BLOCK_LENX;
	blockz = pos->z/MAP_BLOCK_LENZ;

	posx = pos->x%MAP_BLOCK_LENX;
	posz = pos->z%MAP_BLOCK_LENZ;

	if( posx > (MAP_BLOCK_LENX/2) ){
		blockPat += 1;
	}
	if( posz > (MAP_BLOCK_LENZ/2) ){
		blockPat += 2;
	}
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		blockIdx = (blockz + blockIdxOffs[blockPat][i][0]) * MAP_BLOCK_SIZX 
					+ (blockx + blockIdxOffs[blockPat][i][1]);
		if( blockIdx > MAP_BLOCK_IDXMAX ){
			blockIdx = MAP_BLOCK_NULL;
		}
		mapBlockIdx->block[i] = blockIdx;
	}
}



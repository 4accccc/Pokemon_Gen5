//======================================================================
/**
 * @file	map_matrix.c
 * @brief	フィールドマップマトリクス
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "map_matrix.h"

//======================================================================
//	define
//======================================================================
#define MAP_MATRIX_WIDTH_MAX (30)
#define MAP_MATRIX_HEIGHT_MAX (30)
#define MAP_MATRIX_MAX (MAP_MATRIX_WIDTH_MAX*MAP_MATRIX_HEIGHT_MAX)
#define MAP_MATRIX_PREFIX_MAX (16)

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	MAP_MATRIX_HEADER
//--------------------------------------------------------------
typedef struct
{
	u16 flag;
	u16 kind;
	u16 size_h;
	u16 size_v;
}MAP_MATRIX_HEADER;

//--------------------------------------------------------------
//	MAP_MATRIX
//--------------------------------------------------------------
struct _TAG_MAP_MATRIX
{
	HEAPID heapID;
	u32 zone_id;
	u32 matrix_id;
	
	u16 size_w;
	u16 size_h;
	u16 status_flag;
	u32 table_size;
	u32 zone_id_tbl[MAP_MATRIX_MAX];
	u32 map_res_id_tbl[MAP_MATRIX_MAX];
};

//======================================================================
//	proto
//======================================================================
static void MapMatrix_SetData(
	MAP_MATRIX *pMat, const void *pMatData, u32 matrix_id, u32 zone_id );

//======================================================================
//	マップマトリクス
//======================================================================
//--------------------------------------------------------------
/**
 * MAP_MATRIX生成
 * @param	heapID	ワークを確保するHEAPID
 * @retval	MAP_MATRIX* MAP_MATRIX*
 */
//--------------------------------------------------------------
MAP_MATRIX * MAP_MATRIX_Create( HEAPID heapID )
{
	MAP_MATRIX *pMat;
	pMat = GFL_HEAP_AllocClearMemory( heapID, sizeof(MAP_MATRIX) );
	pMat->heapID = heapID;
	return( pMat );
}

//--------------------------------------------------------------
/**
 * マトリクスIDからMAP_MATRIX初期化
 * @param	matrix_id 生成するマトリクスID
 * @param	zone_id	matrix_idが属するゾーンID
 * @retval	MAP_MATRIX*
 */
//--------------------------------------------------------------
void MAP_MATRIX_Init(
	MAP_MATRIX *pMat, const u16 matrix_id, const u16 zone_id )
{
	void *pMatData = GFL_ARC_LoadDataAlloc(
		ARCID_FLDMAP_MAPMATRIX, matrix_id, pMat->heapID );
	MapMatrix_SetData( pMat, pMatData, matrix_id, zone_id );
	GFL_HEAP_FreeMemory( pMatData );
}

//--------------------------------------------------------------
/**
 * 生成されたMAP_MATRIX削除
 * @param	pMat 生成したマトリクスID
 * @retval	nothing
 */
//--------------------------------------------------------------
void MAP_MATRIX_Delete( MAP_MATRIX *pMat )
{
	GFL_HEAP_FreeMemory( pMat );
}

//--------------------------------------------------------------
/**
 * マトリクスアーカイブデータ -> MAP_MATRIX
 * @param	pMat MAP_MATRIX
 * @param	pMatData マトリクスアーカイブデータ
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MapMatrix_SetData(
	MAP_MATRIX *pMat, const void *pMatData, u32 matrix_id, u32 zone_id )
{
	u32 *pMatTbl;
	const u32 *pResTbl;
	const MAP_MATRIX_HEADER *pMatH;
	
	pMatH = pMatData;
	pMat->zone_id = zone_id;
	pMat->matrix_id = matrix_id;
	
	pMat->size_w = pMatH->size_h;
	pMat->size_h = pMatH->size_v;
	pMat->table_size = pMat->size_w * pMat->size_h;
	pMat->status_flag = pMatH->flag;
	
	GF_ASSERT( pMat->size_w );
	GF_ASSERT( pMat->size_h );
	
	pMatTbl = pMat->map_res_id_tbl;
	pResTbl = (const u32*)((const u32)pMatData + sizeof(MAP_MATRIX_HEADER));
	MI_CpuCopy32( pResTbl, pMatTbl, sizeof(u32)*pMat->table_size );
	
	pMatTbl = pMat->zone_id_tbl;
	
	if( pMat->status_flag == 1 ){ //ゾーンID指定チェック
		pResTbl = (const u32*)((const u32)pMatData +
			sizeof(MAP_MATRIX_HEADER) + (sizeof(u32)*pMat->table_size) );
		MI_CpuCopy32( pResTbl, pMatTbl, sizeof(u32)*pMat->table_size );
	}else{ //ゾーンID指定なし
		MI_CpuFill32( pMatTbl, zone_id, sizeof(u32)*MAP_MATRIX_MAX );
	}
	
#ifdef DEBUG_ONLY_FOR_kagaya
	OS_Printf( "マトリクス情報 ID=%d,X=%d,Z=%d\n",
		matrix_id, pMat->size_w, pMat->size_h );
#endif
}

//======================================================================
//	MAP_MATRIX 参照
//======================================================================
//--------------------------------------------------------------
/**
 * MAP_MATRIX セットされているマトリクスIDを取得
 * @param	pMat	MAP_MATRIX
 * @retval	u16		マップマトリクスID
 */
//--------------------------------------------------------------
u32 MAP_MATRIX_GetMatrixID( const MAP_MATRIX *pMat )
{
	return( pMat->matrix_id );
}

//--------------------------------------------------------------
/**
 * MAP_MATRIX 指定位置のゾーンIDを取得
 * @param	pMat	MAP_MATRIX
 * @param	x		X座標(ブロック単位)
 * @param	z		Z座標(ブロック単位)
 * @retval	u32		ZONE ID
 */
//--------------------------------------------------------------
u32 MAP_MATRIX_GetBlockPosZoneID( const MAP_MATRIX *pMat, int x, int z )
{
	u32 zone_id;
	GF_ASSERT( 0 <= x && x < pMat->size_w );
	GF_ASSERT( 0 <= z && z < pMat->size_h );
	zone_id = pMat->zone_id_tbl[x + (z*pMat->size_h)];
	return( zone_id );
}

//--------------------------------------------------------------
/**
 * MAP_MATRIX マップ横ブロックサイズ取得
 * @param	pMat	MAP_MATRIX
 * @retval	u16	Xサイズ
 */
//--------------------------------------------------------------
u16 MAP_MATRIX_GetMapBlockSizeWidth( const MAP_MATRIX *pMat )
{
	return( pMat->size_w );
}

//--------------------------------------------------------------
/**
 * MAP_MATRIX マップZサイズ取得
 * @param	pMat	MAP_MATRIX
 * @retval	u16	Xサイズ
 */
//--------------------------------------------------------------
u16 MAP_MATRIX_GetMapBlockSizeHeight( const MAP_MATRIX *pMat )
{
	return( pMat->size_h );
}

//--------------------------------------------------------------
/**
 * MAP_MATRIX マップXxZの総サイズ取得
 * @param	pMat	MAP_MATRIX
 * @retval	u32		総サイズ
 */
//--------------------------------------------------------------
u32 MAP_MATRIX_GetMapBlockTotalSize( const MAP_MATRIX *pMat )
{
	return( pMat->table_size );
}

//--------------------------------------------------------------
/**
 * MAP_MATRIX リソースデータIDが格納されたブロックテーブルを取得
 * @param	pMat	MAP_MATRIX
 * @retval	u32*	ブロックテーブルのポインタ
 */
//--------------------------------------------------------------
const u32 * MAP_MATRIX_GetMapResIDTable( const MAP_MATRIX *pMat )
{
	return( pMat->map_res_id_tbl );
}

#if 0
		u8 *tbl;
		u32 matID;
		const MAP_MATRIX_HEADER *matH;
		
		matID = ZONEDATA_GetMatrixID( mapid );
		GFL_ARC_LoadData( matrix_buf, ARCID_FLDMAP_MAPMATRIX, matID );
		
		matH = matrix_buf;
		tbl = (u8*)matrix_buf + sizeof(MAP_MATRIX_HEADER);
		
		map_res->sizex = matH->size_h;
		map_res->sizez = matH->size_v;
		map_res->totalSize = matH->size_h * matH->size_v;
		map_res->blocks = (const FLDMAPPER_MAPDATA *)tbl;
#endif

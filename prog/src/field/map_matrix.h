//======================================================================
/**
 *
 * @file	map_matrix.h
 * @brief	マップマトリクス
 * @author	kagaya
 * @data	05.07.13
 *
 */
//======================================================================
#ifndef MAP_MATRIX_H_FILE
#define MAP_MATRIX_H_FILE

#include <gflib.h>

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
typedef struct _TAG_MAP_MATRIX MAP_MATRIX; ///<MAP_MATRIX

//======================================================================
//	extern
//======================================================================
extern MAP_MATRIX * MAP_MATRIX_Create( HEAPID heapID );
extern void MAP_MATRIX_Init(
	MAP_MATRIX *pMat, const u16 matrix_id, const u16 zone_id );
extern void MAP_MATRIX_Delete( MAP_MATRIX *pMat );

extern u32 MAP_MATRIX_GetMatrixID( const MAP_MATRIX *pMat );
extern u32 MAP_MATRIX_GetBlockPosZoneID(
		const MAP_MATRIX *pMat, int x, int z );
extern u16 MAP_MATRIX_GetMapBlockSizeWidth( const MAP_MATRIX *pMat );
extern u16 MAP_MATRIX_GetMapBlockSizeHeight( const MAP_MATRIX *pMat );
extern u32 MAP_MATRIX_GetMapBlockTotalSize( const MAP_MATRIX *pMat );
extern const u32 * MAP_MATRIX_GetMapResIDTable( const MAP_MATRIX *pMat );

#endif //MAP_MATRIX_H_FILE

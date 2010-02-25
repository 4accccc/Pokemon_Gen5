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

#include "gamesystem/gamesystem.h"

//======================================================================
//	define
//======================================================================
#define MAP_MATRIX_ZONE_ID_NON (0xffff)
#define MAP_MATRIX_RES_ID_NON (0xffffffff)

#define MAP_MATRIX_WIDTH_MAX (30)
#define MAP_MATRIX_HEIGHT_MAX (30)
#define MAP_MATRIX_MAX (MAP_MATRIX_WIDTH_MAX*MAP_MATRIX_HEIGHT_MAX)
#define MAP_MATRIX_PREFIX_MAX (16)

//======================================================================
//	struct
//======================================================================
typedef struct _TAG_MAP_MATRIX MAP_MATRIX; ///<MAP_MATRIX
//typedef u16 ZONE_ID;

//======================================================================
//	extern
//======================================================================
extern MAP_MATRIX * MAP_MATRIX_Create( HEAPID heapID );
extern void MAP_MATRIX_Init(
	MAP_MATRIX *pMat, const u16 matrix_id, const u16 zone_id );
extern void MAP_MATRIX_Delete( MAP_MATRIX *pMat );

extern u32 MAP_MATRIX_GetMatrixID( const MAP_MATRIX *pMat );
extern u16 MAP_MATRIX_GetBlockPosZoneID(
		const MAP_MATRIX *pMat, int x, int z );
extern u16 MAP_MATRIX_GetVectorPosZoneID(
		const MAP_MATRIX *pMat, fx32 x, fx32 z );
extern u16 MAP_MATRIX_GetMapBlockSizeWidth( const MAP_MATRIX *pMat );
extern u16 MAP_MATRIX_GetMapBlockSizeHeight( const MAP_MATRIX *pMat );
extern u32 MAP_MATRIX_GetMapBlockTotalSize( const MAP_MATRIX *pMat );
extern const u32 * MAP_MATRIX_GetMapResIDTable( const MAP_MATRIX *pMat );
extern BOOL MAP_MATRIX_CheckBlockPosRange(
		const MAP_MATRIX *pMat, int x, int z );
extern BOOL MAP_MATRIX_CheckVectorPosRange(
		const MAP_MATRIX *pMat, fx32 x, fx32 z );

extern void MAP_MATRIX_SetTableSize( 
    MAP_MATRIX *pMat, u8 xsize, u8 zsize );

extern void MAP_MATRIX_CheckReplace(
    MAP_MATRIX *pMat, GAMESYS_WORK * gamework );
//--------------------------------------------------------------
/**
 * @brief   MAP_MATRIXへのポインタ取得
 * @param	  gamedata	GAMEDATAへのポインタ
 * @retval  MAP_MATRIXへのポインタ
 *
 * 実態はgamesystem/game_data.cにあるが、アクセス制限のため
 * こちらに配置する
 */
//--------------------------------------------------------------
extern MAP_MATRIX * GAMEDATA_GetMapMatrix(GAMEDATA * gamedata);

#endif //MAP_MATRIX_H_FILE

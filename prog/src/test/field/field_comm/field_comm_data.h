//======================================================================
/**
 * @file	field_comm_data.h
 * @brief	通信で共有するデータ(各キャラの座標とか)
 * @author	ariizumi
 * @data	08/11/13
 */
//======================================================================

#include "gamesystem/playerwork.h"

#ifndef FIELD_COMM_DATA_H__
#define FIELD_COMM_DATA_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================
extern void FIELD_COMM_DATA_InitSystem( HEAPID heapID );
extern void	FIELD_COMM_DATA_TermSystem();
extern BOOL	FIELD_COMM_DATA_IsExistSystem(void);

extern void	FIELD_COMM_DATA_SetSelfData_Pos( const ZONEID *zoneID , const VecFx32 *pos , const u16 *dir );
extern PLAYER_WORK*	FIELD_COMM_DATA_GetSelfData_PlayerWork(void);

extern void	FIELD_COMM_DATA_SetCharaData_IsExist( const idx , const BOOL );
extern void	FIELD_COMM_DATA_SetCharaData_IsValid( const idx , const BOOL );
extern const BOOL	FIELD_COMM_DATA_GetCharaData_IsExist( const idx );
extern const BOOL	FIELD_COMM_DATA_GetCharaData_IsValid( const idx );
extern PLAYER_WORK*	FIELD_COMM_DATA_GetCharaData_PlayerWork( const idx );



#endif //FIELD_COMM_DATA_H__


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
extern void FieldCommData_InitSystem( HEAPID heapID );
extern void	FieldCommData_TermSystem();
extern BOOL	FieldCommData_IsExistSystem(void);

extern void	FieldCommData_SetSelfData_Pos( const ZONEID *zoneID , const VecFx32 *pos , const u16 *dir );
extern PLAYER_WORK*	FieldCommData_GetSelfData_PlayerWork(void);

extern void	FieldCommData_SetCharaData_IsExist( const idx , const BOOL );
extern void	FieldCommData_SetCharaData_IsValid( const idx , const BOOL );
extern const BOOL	FieldCommData_GetCharaData_IsExist( const idx );
extern const BOOL	FieldCommData_GetCharaData_IsValid( const idx );
extern PLAYER_WORK*	FieldCommData_GetCharaData_PlayerWork( const idx );



#endif //FIELD_COMM_DATA_H__


//======================================================================
/**
 * @file	field_comm_main.c
 * @brief	フィールド通信の処理の外と中をつなぐ関数とか群
 * @author	ariizumi
 * @data	08/11/11
 */
//======================================================================

#ifndef FIELD_COMM_MAIN__
#define FIELD_COMM_MAIN__

#include "../field_main.h"
#include "../field_player.h"
#include "gamesystem/gamesystem.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct _FIELD_COMM_MAIN FIELD_COMM_MAIN;

//======================================================================
//	proto
//======================================================================
extern FIELD_COMM_MAIN* FieldCommMain_InitSystem( int heapID );
extern void	FieldCommMain_TermSystem( FIELD_COMM_MAIN *commSys );
extern void	FieldCommMain_UpdateCommSystem( FIELD_MAIN_WORK *fieldWork , GAMESYS_WORK *gameSys , PC_ACTCONT *pcActor , FIELD_COMM_MAIN *commSys );
extern void	FieldCommMain_InitStartCommMenu( FIELD_COMM_MAIN *commSys );
extern void	FieldCommMain_TermStartCommMenu( FIELD_COMM_MAIN *commSys );
extern const BOOL	FieldCommMain_LoopStartCommMenu( FIELD_COMM_MAIN *commSys );
extern void	FieldCommMain_InitStartInvasionMenu( FIELD_COMM_MAIN *commSys );
extern void	FieldCommMain_TermStartInvasionMenu( FIELD_COMM_MAIN *commSys );
extern const BOOL	FieldCommMain_LoopStartInvasionMenu( FIELD_COMM_MAIN *commSys );




#endif //FIELD_COMM_MAIN__


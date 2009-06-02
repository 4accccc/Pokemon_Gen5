//======================================================================
/**
 * @file	mus_comm_func.h
 * @brief	ミュージカル通信機能
 * @author	ariizumi
 * @data	09/05/29
 *
 * モジュール名：MUS_COMM_FUNC
 */
//======================================================================
#pragma once

#include "gamesystem/game_comm.h"
#include "savedata/save_control.h"
#include "savedata/mystatus.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MCM_NONE,
  MCM_PARENT,
  MCM_CHILD,
  MUS_COMM_MODE_MAX
}MUS_COMM_MODE;


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  STRCODE name[BUFLEN_PERSON_NAME+EOM_SIZE];
}MUS_COMM_BEACON;

typedef struct _MUS_COMM_WORK MUS_COMM_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

MUS_COMM_WORK* MUS_COMM_CreateWork( HEAPID heapId , GAME_COMM_SYS_PTR gameComm , SAVE_CONTROL_WORK *saveCtrl );
void MUS_COMM_DeleteWork( MUS_COMM_WORK* work );

void MUS_COMM_InitComm( MUS_COMM_WORK* work );
void MUS_COMM_ExitComm( MUS_COMM_WORK* work );
void MUS_COMM_UpdateComm( MUS_COMM_WORK* work );
const BOOL MUS_COMM_IsInitComm( MUS_COMM_WORK* work );

void MUS_COMM_StartParent( MUS_COMM_WORK* work );
void MUS_COMM_StartChild( MUS_COMM_WORK* work );

MYSTATUS *MUS_COMM_GetPlayerMyStatus( MUS_COMM_WORK* work , u8 idx );
const BOOL MUS_COMM_IsRefreshUserData( MUS_COMM_WORK *work );
void MUS_COMM_ResetRefreshUserData( MUS_COMM_WORK *work );

GAME_COMM_SYS_PTR MUS_COMM_GetGameComm( MUS_COMM_WORK* work );

MUS_COMM_MODE MUS_COMM_GetMode( MUS_COMM_WORK* work );

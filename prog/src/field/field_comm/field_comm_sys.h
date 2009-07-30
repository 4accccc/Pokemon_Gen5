//==============================================================================
/**
 * @file  field_comm_sys.h
 * @brief
 * @author  matsuda
 * @date  2009.04.30(ñÿ)
 */
//==============================================================================
#pragma once

#include "field/field_comm/field_comm_def.h"
#include "gamesystem/game_comm.h"


//==============================================================================
//  äOïîä÷êîêÈåæ
//==============================================================================
extern COMM_FIELD_SYS_PTR FIELD_COMM_SYS_Alloc(HEAPID commHeapID, GAME_COMM_SYS_PTR game_comm);
extern void FIELD_COMM_SYS_Free(COMM_FIELD_SYS_PTR comm_field);
extern FIELD_COMM_FUNC* FIELD_COMM_SYS_GetCommFuncWork( COMM_FIELD_SYS_PTR commField );
extern FIELD_COMM_DATA* FIELD_COMM_SYS_GetCommDataWork( COMM_FIELD_SYS_PTR commField );
extern GAME_COMM_SYS_PTR FIELD_COMM_SYS_GetGameCommSys( COMM_FIELD_SYS_PTR commField );
extern BOOL * FIELD_COMM_SYS_GetCommActorVanishFlag(COMM_FIELD_SYS_PTR commField, int net_id);
extern void FIELD_COMM_SYS_SetInvalidNetID(COMM_FIELD_SYS_PTR commField, int invalid_netid);
extern int FIELD_COMM_SYS_GetInvalidNetID(COMM_FIELD_SYS_PTR commField);
extern void FIELD_COMM_SYS_SetExitReq(COMM_FIELD_SYS_PTR commField);
extern BOOL FIELD_COMM_SYS_GetExitReq(COMM_FIELD_SYS_PTR commField);
extern void FIELD_COMM_SYS_SetRecvProfile(COMM_FIELD_SYS_PTR commField, int net_id);
extern BOOL FIELD_COMM_SYS_GetRecvProfile(COMM_FIELD_SYS_PTR commField, int net_id);


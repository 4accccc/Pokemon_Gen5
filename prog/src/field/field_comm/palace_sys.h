//==============================================================================
/**
 * @file    palace_sys.h
 * @brief   フィールド通信：パレスのヘッダ
 * @author  matsuda
 * @date    2009.05.01(金)
 */
//==============================================================================
#pragma once

#include "field/field_player.h"
#include "intrude_types.h"


//==============================================================================
//  外部関数宣言
//==============================================================================
extern PALACE_SYS_PTR PALACE_SYS_Alloc(HEAPID heap_id, GAME_COMM_SYS_PTR game_comm);
extern void PALACE_SYS_InitWork(PALACE_SYS_PTR palace);
extern void PALACE_SYS_Free(PALACE_SYS_PTR palace);
extern void PALACE_SYS_Update(PALACE_SYS_PTR palace, PLAYER_WORK *plwork, FIELD_PLAYER *fldply, INTRUDE_COMM_SYS_PTR intcomm, FIELD_MAIN_WORK *fieldWork, GAME_COMM_SYS_PTR game_comm);
extern void PALACE_SYS_SetArea(PALACE_SYS_PTR palace, u8 palace_area);
extern u8 PALACE_SYS_GetArea(PALACE_SYS_PTR palace);
extern void PALACE_SYS_FriendPosConvert(PALACE_SYS_PTR palace, int friend_area, 
  PLAYER_WORK *my_plwork, PLAYER_WORK *friend_plwork);
extern int PALACE_SYS_GetMissionNo(PALACE_SYS_PTR palace);

//--------------------------------------------------------------
//  デバッグ用
//--------------------------------------------------------------
extern BOOL PALACE_DEBUG_CreateNumberAct(PALACE_SYS_PTR palace, HEAPID heap_id, FIELD_MAIN_WORK *fieldWork);
extern void PALACE_DEBUG_DeleteNumberAct(PALACE_SYS_PTR palace);
extern GMEVENT * EVENT_DebugPalaceNGWin( GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork, FIELD_PLAYER *fld_player, BOOL left_right );
extern void PALACE_DEBUG_EnableNumberAct(PALACE_SYS_PTR palace, BOOL enable);

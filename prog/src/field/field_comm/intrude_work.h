//==============================================================================
/**
 * @file    intrude_work.h
 * @brief   侵入システムワークに外部から触る為の関数群
 * @author  matsuda
 * @date    2009.10.10(土)
 */
//==============================================================================
#pragma once

#include "gamesystem/game_comm.h"
#include "field/field_comm/intrude_types.h"


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void IntrudeWork_SetActionStatus(GAME_COMM_SYS_PTR game_comm, INTRUDE_ACTION action);
extern void Intrude_SetWarpTown(GAME_COMM_SYS_PTR game_comm, int town_tblno);
extern int Intrude_GetWarpTown(GAME_COMM_SYS_PTR game_comm);
extern BOOL Intrude_GetTargetTimingNo(INTRUDE_COMM_SYS_PTR intcomm, u8 timing_no, u8 target_netid);

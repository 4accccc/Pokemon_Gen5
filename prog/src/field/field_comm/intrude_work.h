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
extern void Intrude_SetWarpPlayerNetID(GAME_COMM_SYS_PTR game_comm, int net_id);
extern int Intrude_GetWarpPlayerNetID(GAME_COMM_SYS_PTR game_comm);
extern void Intrude_SetWarpTown(GAME_COMM_SYS_PTR game_comm, int town_tblno);
extern int Intrude_GetWarpTown(GAME_COMM_SYS_PTR game_comm);
extern BOOL Intrude_GetTargetTimingNo(INTRUDE_COMM_SYS_PTR intcomm, u8 timing_no, u8 target_netid);
extern OCCUPY_INFO * Intrude_GetOccupyInfo(INTRUDE_COMM_SYS_PTR intcomm, int palace_area);
extern MYSTATUS * Intrude_GetMyStatus(INTRUDE_COMM_SYS_PTR intcomm, int net_id);
extern MISSION_CHOICE_LIST * Intrude_GetChoiceList(INTRUDE_COMM_SYS_PTR intcomm, int palace_area);
extern u8 Intrude_GetPalaceArea(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL Intrude_GetRecvWfbc(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL Intrude_GetNewMissionRecvFlag(INTRUDE_COMM_SYS_PTR intcomm);

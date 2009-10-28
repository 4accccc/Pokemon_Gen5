//==============================================================================
/**
 * @file    intrude_mission.h
 * @brief   ミッション処理のヘッダ
 * @author  matsuda
 * @date    2009.10.26(月)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void MISSION_Init(MISSION_SYSTEM *mission);
extern void MISSION_Set_DataSendReq(MISSION_SYSTEM *mission);
extern BOOL MISSION_SetEntry(INTRUDE_COMM_SYS_PTR intcomm, MISSION_SYSTEM *mission, const MISSION_REQ *req, int accept_netid);
extern void MISSION_SetMissionData(MISSION_SYSTEM *mission, const MISSION_DATA *src);
extern void MISSION_SendUpdate(INTRUDE_COMM_SYS_PTR intcomm, MISSION_SYSTEM *mission);
extern BOOL MISSION_RecvCheck(const MISSION_SYSTEM *mission);
extern u16 MISSION_GetMissionMsgID(const MISSION_SYSTEM *mission);
extern u16 MISSION_GetAchieveMsgID(const MISSION_SYSTEM *mission, int my_netid);
extern BOOL MISSION_EntryAchieve(MISSION_SYSTEM *mission, const MISSION_DATA *mdata, int achieve_netid);
extern BOOL MISSION_EntryAchieve(MISSION_SYSTEM *mission, const MISSION_DATA *mdata, int achieve_netid);
extern void MISSION_SetResult(MISSION_SYSTEM *mission, const MISSION_RESULT *cp_result);
extern BOOL MISSION_RecvAchieve(const MISSION_SYSTEM *mission);
extern BOOL MISSION_Talk_CheckAchieve(const MISSION_SYSTEM *mission, int talk_netid);
extern s32 MISSION_GetPoint(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_RESULT *result);
extern BOOL MISSION_AddPoint(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission);


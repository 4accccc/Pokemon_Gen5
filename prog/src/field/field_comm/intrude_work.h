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
#include "field\field_wfbc_data.h"


//==============================================================================
//  定数定義
//==============================================================================
typedef enum{
  GRAYSCALE_TYPE_NULL,    ///<グレースケールにする必要無し
  GRAYSCALE_TYPE_WHITE,   ///<白バージョンのグレースケール
  GRAYSCALE_TYPE_BLACK,   ///<黒バージョンのグレースケール
  GRAYSCALE_TYPE_NEXT,    ///<次回バージョンのグレースケール
}GRAYSCALE_TYPE;


///ライブ通信画面用：侵入状態
typedef enum{
  LIVE_COMM_NULL,             ///<侵入されていない
  LIVE_COMM_INTRUDE,          ///<侵入されている
  LIVE_COMM_INTRUDE_PARTNER,  ///<協力者になった
  LIVE_COMM_INTRUDE_OUT,      ///<協力者になった後、ライブ通信画面を一度抜けている
}LIVE_COMM;


///侵入の接続状況
typedef enum{
  INTRUDE_CONNECT_NULL,              ///<接続されていない
  INTRUDE_CONNECT_INTRUDE,           ///<ハイリンク接続時(ミッションが行われていない)
  INTRUDE_CONNECT_MISSION_TARGET,    ///<ハイリンク接続時(ミッションのターゲットになっている)
  INTRUDE_CONNECT_MISSION_PARTNER,   ///<ハイリンク接続時(協力者である)
}INTRUDE_CONNECT;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern BOOL Intrude_Check_AlwaysBoot(GAMESYS_WORK *gsys);
extern INTRUDE_COMM_SYS_PTR Intrude_Check_CommConnect(GAME_COMM_SYS_PTR game_comm);
extern void Intrude_SetWarpPlayerNetID(GAME_COMM_SYS_PTR game_comm, int net_id);
extern int Intrude_GetWarpPlayerNetID(GAME_COMM_SYS_PTR game_comm);
extern void Intrude_SetWarpTown(GAME_COMM_SYS_PTR game_comm, int town_tblno);
extern int Intrude_GetWarpTown(GAME_COMM_SYS_PTR game_comm);
extern u8 Intrude_GetIntrudeRomVersion(u8 pm_version, u32 trainer_id);
extern BOOL Intrude_GetMissionBattleTimingNo(INTRUDE_COMM_SYS_PTR intcomm, u8 timing_bit, u8 target_netid);
extern OCCUPY_INFO * Intrude_GetOccupyInfo(GAMEDATA *gamedata, int palace_area);
extern MYSTATUS * Intrude_GetMyStatus(GAMEDATA *gamedata, int net_id);
extern MISSION_CHOICE_LIST * Intrude_GetChoiceList(INTRUDE_COMM_SYS_PTR intcomm, int palace_area);
extern u8 Intrude_GetPalaceArea(const GAMEDATA *gamedata);
extern BOOL Intrude_GetRecvWfbc(INTRUDE_COMM_SYS_PTR intcomm);
extern WFBC_COMM_DATA * Intrude_GetWfbcCommData(INTRUDE_COMM_SYS_PTR intcomm);
extern u8 Intrude_GetRomVersion(GAME_COMM_SYS_PTR game_comm);
extern u8 Intrude_GetSeasonID(GAME_COMM_SYS_PTR game_comm);
extern BOOL Intrude_CheckNextPalaceAreaMine(GAME_COMM_SYS_PTR game_comm, const GAMEDATA *gamedata, u16 dir);
extern GRAYSCALE_TYPE Intrude_CheckGrayScaleMap(GAME_COMM_SYS_PTR game_comm, GAMEDATA *gamedata);
extern BOOL Intrude_CheckPalaceConnect(GAME_COMM_SYS_PTR game_comm);
extern BOOL Intrude_CheckZonePalaceConnect(u16 zone_id);
extern BOOL Intrude_CheckTutorialComplete(GAMEDATA *gamedata);
extern BOOL Intrude_CheckStartTimeoutMsgON(INTRUDE_COMM_SYS_PTR intcomm);
extern LIVE_COMM Intrude_CheckLiveCommStatus(GAME_COMM_SYS_PTR game_comm);
extern void Intrude_SetLiveCommStatus_IntrudeOut(GAME_COMM_SYS_PTR game_comm);
extern INTRUDE_CONNECT Intrude_GetIntrudeStatus(GAME_COMM_SYS_PTR game_comm);
extern int Intrude_GetMissionEntryNum(GAME_COMM_SYS_PTR game_comm);
extern BOOL Intrude_GetTargetName(GAME_COMM_SYS_PTR game_comm, STRBUF *dest_buf);


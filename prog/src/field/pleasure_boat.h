//======================================================================
/**
 * @file  pleasure_boat.h
 * @brief 遊覧船
 * @author  Saito
 */
//======================================================================

#pragma once

#include "pleasure_boat_ptr.h"
#include "gamesystem/gamesystem.h"    //for GMEVENT

extern PL_BOAT_WORK_PTR PL_BOAT_Init(void);
extern void PL_BOAT_End(PL_BOAT_WORK_PTR *work);
extern void PL_BOAT_Main(PL_BOAT_WORK_PTR work);
extern GMEVENT *PL_BOAT_CheckEvt(GAMESYS_WORK *gsys, PL_BOAT_WORK_PTR work);
extern void PL_BOAT_AddTimeEvt(PL_BOAT_WORK_PTR work, const int inAddSec);
extern int PL_BOAT_GetTrNum(PL_BOAT_WORK_PTR work, const int inSearchType);

//部屋情報取得
extern int PL_BOAT_GetObjCode(PL_BOAT_WORK_PTR work, const int inRoomIdx);
extern int PL_BOAT_GetNpcType(PL_BOAT_WORK_PTR work, const int inRoomIdx);
extern int PL_BOAT_GetTrID(PL_BOAT_WORK_PTR work, const int inRoomIdx);
extern int PL_BOAT_GetMsg(PL_BOAT_WORK_PTR work, const int inRoomIdx, const inMsgKind);
extern BOOL PL_BOAT_CheckBtlFlg(PL_BOAT_WORK_PTR work, const int inRoomIdx);
//extern BOOL PL_BOAT_CheckDblBtl(PL_BOAT_WORK_PTR work, const int inRoomIdx);

//部屋情報セット
extern void PL_BOAT_SetBtlFlg(PL_BOAT_WORK_PTR work, const int inRoomIdx, const BOOL inFlg);

//時間取得・セット
extern int PL_BOAT_GetTime(PL_BOAT_WORK_PTR work);
extern void PL_BOAT_SetTime(PL_BOAT_WORK_PTR work, const int inTime);

GMEVENT * PL_BOAT_CreateDemoEvt(GAMESYS_WORK *gsys, PL_BOAT_WORK_PTR work, u16 *ret_wk);



//======================================================================
/**
 * @file  united_nations.h
 * @brief  çëòAä÷òA
 * @author  Saito
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "savedata/un_savedata.h"

#define UN_IDX_OVER   (0xff) 

extern void UN_SetData(GAMESYS_WORK *gsys, const int inCountryCode);
extern u8 UN_GetUnIdx(UNSV_WORK* work, const u32 inObjIdx);
extern u32 UN_GetRoomObjCode(UNSV_WORK* work, const u32 inObjIdx);
extern u32 UN_GetRoomObjNum(UNSV_WORK* work);
extern u32 UN_GetRoomObjMsg(GAMESYS_WORK *gsys, UNSV_WORK* work, const int inObjIdx, const BOOL inFirstMsg);

#ifdef PM_DEBUG
extern u32 DebugGetUnCouontry(GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param);
extern void DebugSetUnCouontry(GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param, u32 value);
#endif



//============================================================================================
/**
 * @file  sheimi_normalform.h
 * @bfief シェイミ時間越えフォルム戻り
 * @author  Saito
 */
//============================================================================================
#pragma once

#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"
#include "gamesystem/gamedata_def.h"

extern void SHEIMI_NFORM_ChangeNormal(GAMEDATA * gdata, POKEPARTY *ppt);
extern BOOL SHEIMI_NFORM_ChangeNormal_TimeUpdate(GAMEDATA * gdata, POKEPARTY *ppt, int min_diff, const RTCTime * now, int inSeason);



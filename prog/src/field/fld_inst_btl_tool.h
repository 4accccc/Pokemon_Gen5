//======================================================================
/**
 * @file  fld_inst_btl_tool.h
 * @brief  施設バトル呼び出し等の共通部分
 *
 * @note  施設：institution
 * モジュール名：
 */
//======================================================================
#pragma once

#include "app/pokelist.h"
#include "gamesystem/gamesystem.h"

extern GMEVENT *FIB_TOOL_CreatePokeListEvt(
    GAMESYS_WORK *gsys,
    const int inType, const int inReg, POKEPARTY *pp,
    u8 *outSelNoAry, PL_SELECT_POS *outResult, PL_RETURN_TYPE *outRetMode );


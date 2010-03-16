//==============================================================================
/**
 * @file    event_symbol.h
 * @brief   侵入：シンボルポケモン
 * @author  matsuda
 * @date    2010.03.16(火)
 */
//==============================================================================
#pragma once

#include "savedata/symbol_save.h"


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * EVENT_SymbolPokeBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const SYMBOL_POKEMON *spoke, u16 *result_ptr, HEAPID heap_id);

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
extern GMEVENT * EVENT_ReqIntrudeSymbolParam(GAMESYS_WORK *gsys, u16 *result_ptr, SYMBOL_ZONE_TYPE zone_type, u8 map_no);
extern GMEVENT * EVENT_SymbolMapWarp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u16 *result_ptr, u16 warp_zone_id, const VecFx32 *warp_pos, u16 warp_dir, BOOL seasonUpdateEnable, SYMBOL_ZONE_TYPE zone_type, u8 map_no);

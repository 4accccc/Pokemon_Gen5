//==============================================================================
/**
 * @file  symbol_map.h
 * @brief シンボルエンカウント用マップ関連
 * @author  tamada GAME FREAK inc.
 * @date  2010.03.19
 */
//==============================================================================

#pragma once
#include <gflib.h>
#include "field/intrude_symbol.h"
#include "gamesystem/gamesystem.h"
//--------------------------------------------------------------
//--------------------------------------------------------------
extern u16 SYMBOLMAP_GetZoneID( GAMESYS_WORK * gsys, SYMBOL_MAP_ID symmap_id );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern SYMBOL_MAP_ID SYMBOLMAP_GetNextSymbolMapID(
    GAMESYS_WORK * gsys, SYMBOL_MAP_ID now_symmap_id, u16 dir_id );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern BOOL SYMBOLMAP_IsEntranceID( SYMBOL_MAP_ID symmap_id );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern BOOL SYMBOLMAP_IsKeepzoneID( SYMBOL_MAP_ID symmap_id );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern INTRUDE_SYMBOL_WORK * SYMBOLMAP_AllocSymbolWork( HEAPID heapID, GAMESYS_WORK * gsys, u32 * no );


//======================================================================
/**
 * @file	musical_debug.h
 * @brief	ミュージカルデバッグ用
 * @author	ariizumi
 * @data	100322
 *
 * モジュール名：MUSICAL_DEBUG
 */
//======================================================================

#pragma once

#include "musical/musical_define.h"

#if PM_DEBUG
extern const BOOL MUSICAL_DEBUG_CreateDummyData( MUSICAL_SHOT_DATA* shotData , const u16 monsNo , const HEAPID heapId );
#endif
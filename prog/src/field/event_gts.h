//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_gts.h
 *	@brief  イベント：GTS
 *	@author	Toru=Nagihashi
 *	@date		2010.03.26
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern GMEVENT* EVENT_Gts( GAMESYS_WORK * p_gamesys, FIELDMAP_WORK * p_fieldmap );


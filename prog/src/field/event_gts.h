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

FS_EXTERN_OVERLAY(event_gts);

//-------------------------------------
///	EVENT_CallGts用パラメータ
//=====================================
typedef struct {
  FIELDMAP_WORK* fieldmap;
  BOOL is_debug;
} EV_GTS_PARAM;


//=============================================================================
/**
 *					外部公開
///	GMEVENT_CreateOverlayEventCall関数用コールバック関数
*/
//=============================================================================
extern GMEVENT* EVENT_CallGts( GAMESYS_WORK * p_gamesys, void* work );


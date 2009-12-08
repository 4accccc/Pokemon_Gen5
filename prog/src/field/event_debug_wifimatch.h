//============================================================================================
/**
 * @file	  event_wifibattlematch.h
 * @brief	  イベント：WIFI世界対戦
 * @author	Toru=Nagihashi
 * @date	  2009.11.24
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "field/fieldmap.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "net_app/debug_wifi_match.h"


extern GMEVENT* EVENT_DEBUG_WifiMatch( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, DEBUG_WIFIBATTLEMATCH_MODE mode );


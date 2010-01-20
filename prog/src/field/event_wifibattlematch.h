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
#include "net_app/wifibattlematch.h"


extern GMEVENT* EVENT_WifiBattleMatch( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, WIFIBATTLEMATCH_MODE mode, WIFIBATTLEMATCH_POKE  poke, WIFIBATTLEMATCH_BTLRULE btl_rule );


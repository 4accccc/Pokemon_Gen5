//============================================================================================
/**
 * @file	  event_gtsnego.h
 * @brief	  イベント：GTSネゴシエーション
 * @author	ohno
 * @date	  2009.11.15
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "net_app/gtsnego.h"


extern GMEVENT* EVENT_GTSNego( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap );
extern void EVENT_GTSNegoChange(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * event);


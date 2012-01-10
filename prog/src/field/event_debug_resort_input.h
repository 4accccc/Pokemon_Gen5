//============================================================================================
/**
 * @file	  event_debug_resort_input.h
 * @brief	  イベント：リゾートデバッグを呼ぶ
 * @author	Toru=Nagihashi
 * @date	  2012.01.10
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "field/fieldmap.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "net_app/debug_wifi_match.h"

FS_EXTERN_OVERLAY(event_debug_resort_input);

extern GMEVENT* EVENT_CallDebugInputResort( GAMESYS_WORK * gsys, void* work );


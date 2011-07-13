//============================================================================================
/**
 * @file	  event_push_medal.h
 * @brief	  イベント：すれ違い詳細画面
 * @author  iwasawa	
 * @date	  2010.02.01
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"
#include "net_app/wifi_login.h"
#include "app/box2.h"
#include "app/box_mode.h"

FS_EXTERN_OVERLAY( notwifi_event_push_medal );

extern GMEVENT* EVENT_PushMedal(GAMESYS_WORK * gsys, void* work );



//============================================================================================
/**
 * @file	  event_beacon_detail.h
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
#include "net_app/comm_tvt_sys.h"  //TVトランシーバ

extern GMEVENT* EVENT_BeaconDetail( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap );


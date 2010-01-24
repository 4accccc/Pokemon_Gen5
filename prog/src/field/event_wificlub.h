//============================================================================================
/**
 * @file	event_wificlub.h
 * @brief	イベント：WIFIクラブ
 * @author	ohno
 * @date	2009.03.31
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

#include "battle/battle.h"
//typedef struct _EVENT_WIFICLUB_WORK EVENT_WIFICLUB_WORK;

typedef struct {
  GMEVENT * event;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  SAVE_CONTROL_WORK *ctrl;
  void* pWork;
  BATTLE_SETUP_PARAM para;
  BOOL isEndProc;
  BOOL bFieldEnd;
  int selectType;
} EVENT_WIFICLUB_WORK;


extern GMEVENT* EVENT_WiFiClub( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap );
extern void EVENT_WiFiClubChange(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * event);


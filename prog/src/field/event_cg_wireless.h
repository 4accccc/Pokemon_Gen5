//============================================================================================
/**
 * @file	  event_cg_wireless.h
 * @brief	  イベント：パレスとTVトランシーバー
 * @author	k.ohno
 * @date	  2009.12.13
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


typedef enum{
  WIRELESS_CALLTYPE_PALACE,
  WIRELESS_CALLTYPE_TV,
} WIRELESS_CALLTYPE_ENUM;



typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gameData;
  SAVE_CONTROL_WORK *ctrl;
  BOOL isEndProc;
  int selectType;
  BOOL push;
} EVENT_CG_WIRELESS_WORK;

extern void EVENT_CG_WIRELESS_SetEnd(EVENT_CG_WIRELESS_WORK* pWk);


extern GMEVENT* EVENT_CG_Wireless(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * prevevent,BOOL bCreate);


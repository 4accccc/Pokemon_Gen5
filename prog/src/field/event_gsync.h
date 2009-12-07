//============================================================================================
/**
 * @file	  event_gsync.h
 * @brief	  イベント：GAMESYNC
 * @author	k.ohno
 * @date	  2009.01.19
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"
#include "net_app/wifi_login.h"


typedef enum{
  GSYNC_CALLTYPE_INFO,
  GSYNC_CALLTYPE_BOXSET,
} GSYNC_CALLTYPE_ENUM;



typedef struct {
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  SAVE_CONTROL_WORK *ctrl;
  BATTLE_SETUP_PARAM para;
  BOOL isEndProc;
  int selectType;
  BOOL push;
  u16 boxNo;
  u16 boxIndex;
  WIFILOGIN_PARAM aLoginWork;
} EVENT_GSYNC_WORK;

extern void EVENT_GSYNC_SetEnd(EVENT_GSYNC_WORK* pWk);


extern GMEVENT* EVENT_GSync(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * prevevent,BOOL bCreate);


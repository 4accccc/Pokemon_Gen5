//======================================================================
/**
 * @file  event_geonet.c
 * @brief 地球儀呼び出し用イベント
 * @date  2010.02.24
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "fieldmap.h"

#include "proc_gameclear_save.h"
#include "event_fieldmap_control.h"

#include "event_geonet.h"
#include "net_app/wifi_earth.h"

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  u8 a; //適当
}GEONET_WORK;
//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT geonetEvent( GMEVENT * event, int *seq, void *work )
{
  GEONET_WORK * geo_work = work;
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork( event );
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK * fieldmap;

  switch (*seq)
  {
  case 0:
    fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    GMEVENT_CallEvent(event, EVENT_FieldFadeOut_Black(gsys, fieldmap, FIELD_FADE_WAIT ) );
    (*seq) ++;
    break;
  case 1:
    fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq) ++;
    break;
  case 2:
    GMEVENT_CallProc( event, FS_OVERLAY_ID( geonet ), &Earth_Demo_proc_data, gamedata );
    (*seq) ++;
    break;
  case 3:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case 4:
    fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black(gsys, fieldmap, FIELD_FADE_WAIT) );
    (*seq) ++;
    break;
  case 5:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * EVENT_Geonet( GAMESYS_WORK * gsys )
{
  GMEVENT * event;
  GEONET_WORK * geo_work;
  event = GMEVENT_Create( gsys, NULL, geonetEvent, sizeof(GEONET_WORK) );
  geo_work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( geo_work, sizeof( GEONET_WORK ) );
  return event;
}

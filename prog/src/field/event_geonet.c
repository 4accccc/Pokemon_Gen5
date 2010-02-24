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

FS_EXTERN_OVERLAY( watanabe_sample );
extern const GFL_PROC_DATA Earth_Demo_proc_data;

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
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq) ++;
    break;
  case 1:
    //GMEVENT_CallProc( event, FS_OVERLAY_ID(gameclear_demo), &GameClearMsgProcData, gsys );
    GMEVENT_CallProc( event, FS_OVERLAY_ID(watanabe_sample), &Earth_Demo_proc_data, gamedata );
    (*seq) ++;
    break;
  case 2:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case 3:
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

//============================================================================================
/**
 * @file	  event_debug_resort_input.c
 * @brief	  イベント：リゾートデバッグを呼ぶ
 * @author	Toru=Nagihashi
 * @date	  2012.01.10
 */
//============================================================================================
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"      //GFL_HEAPID_APP参照

//フィールド
#include "event_fieldmap_control.h"

//PROC
#include "field/resonance_resort/resort_debug_input.h"

//外部公開
#include "field/event_debug_resort_input.h"

//--------------------------------------------------------------
/**
 *  Event
 */
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK      * gsys;
  FIELDMAP_WORK     * fieldmap;
  SAVE_CONTROL_WORK * ctrl;
}DEBUG_INPUT_RESORT_DATA;
static GMEVENT_RESULT EVENT_DebugInputResort(GMEVENT * event, int *  seq, void * work);
GMEVENT * EVENT_CallDebugInputResort( GAMESYS_WORK *p_gsys, void *work )
{
  GMEVENT * event = GMEVENT_Create(p_gsys, NULL, EVENT_DebugInputResort, sizeof(DEBUG_INPUT_RESORT_DATA));
  DEBUG_INPUT_RESORT_DATA *p_wk = GMEVENT_GetEventWork(event);

  p_wk->gsys = p_gsys;
  p_wk->fieldmap = GAMESYSTEM_GetFieldMapWork(p_gsys);
  p_wk->ctrl = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(p_gsys));

  return event;
}

static GMEVENT_RESULT EVENT_DebugInputResort(GMEVENT * event, int *  seq, void * work)
{
  enum{
    _FIELD_COMM,
    _FIELD_FADEOUT,
    _FIELD_CLOSE,
    _CALL_WIFIBTLMATCH,
    _WAIT_WIFIBTLMATCH,
    _FIELD_OPEN,
    _FIELD_FADEIN,
    _FIELD_END
  };

  DEBUG_INPUT_RESORT_DATA * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;


  switch (*seq) {
  case _FIELD_COMM:
      PMSND_PushBGM();
      (*seq)++;
    break;
  case _FIELD_FADEOUT:
    GMEVENT_CallEvent(event, EVENT_FieldFadeOut(gsys, dbw->fieldmap, FIELD_FADE_BLACK, FIELD_FADE_WAIT));
    (*seq)++;
    break;
  case _FIELD_CLOSE:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
    (*seq)++;
    break;
  case _CALL_WIFIBTLMATCH:
    { 
      GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(debug_resort_app), &DEBUG_INPUT_Resort_ProcData, GAMESYSTEM_GetGameData( gsys ) );
    }
    (*seq)++;
    break;
  case _WAIT_WIFIBTLMATCH:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      (*seq) ++;
    }
    break;
    case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
   case _FIELD_FADEIN:
    GMEVENT_CallEvent(event,EVENT_FieldFadeIn_Black(gsys, dbw->fieldmap, FIELD_FADE_WAIT ));
    (*seq) ++;
    break;
  case _FIELD_END:
    PMSND_PopBGM();
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}



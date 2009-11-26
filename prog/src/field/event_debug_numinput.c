/*
 *  @file   event_debug_numinput.c
 *  @brief  デバッグ数値入力
 *  @author Miyuki Iwasawa
 *  @date   09.11.25
 */
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "font/font.naix"

#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "field/eventdata_sxy.h"

#include "sound/pm_sndsys.h"

#include "field_sound.h"
#include "eventwork.h"

#include "event_debug_local.h"
#include "event_debug_numinput.h"

#include "message.naix"
#include "msg/msg_d_field.h"

typedef struct _DMENU_NUMINPUT_EVWK{
  int i;
  GAMESYS_WORK* gsys;

}DMENU_NUMINPUT_EVWK;

static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work);

//--------------------------------------------------------------
/**
 * 数値入力デバッグメニューイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
GMEVENT* EVENT_DMenuNumInput( GAMESYS_WORK* gsys, void* work )
{
  DEBUG_MENU_EVENT_WORK* dwk = (DEBUG_MENU_EVENT_WORK*)work;
  DMENU_NUMINPUT_EVWK* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, dninput_MainEvent, sizeof(DMENU_NUMINPUT_EVWK) );
  evwk = GMEVENT_GetEventWork( event );
  evwk->gsys = gsys; 

  return event;
}

/*
 *  @brief  数値入力メインメニューイベント
 */
static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  switch(*seq)
  {
  case 0:
    break;
  }
  return GMEVENT_RES_FINISH;
  return GMEVENT_RES_CONTINUE;
}


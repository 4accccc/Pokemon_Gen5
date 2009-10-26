//==============================================================================
/**
 * @file    event_intrude_subscreen.c
 * @brief   侵入によるサブスクリーン切り替えイベント
 * @author  matsuda
 * @date    2009.10.18(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/field_subscreen.h"
#include "event_intrude_subscreen.h"
#include "fieldmap.h"
#include "event_mapchange.h"
#include "field_comm/intrude_types.h"
#include "field_comm/intrude_main.h"
#include "field_comm/intrude_work.h"


//==============================================================================
//  構造体定義
//==============================================================================
///イベント管理ワーク
typedef struct{
  GMEVENT *gmEvent;
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldWork;
  
  FIELD_SUBSCREEN_MODE change_mode;
  u8 seq;
  u8 padding[3];
}CISS_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT ChangeIntrudeSubScreenEvent( GMEVENT *event, int *seq, void *wk );



//==============================================================================
//  イベント
//==============================================================================
//==================================================================
/**
 * 侵入によるサブスクリーン切り替えイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   change_mode		切り替え後のサブスクリーンモード
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_ChangeIntrudeSubScreen(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_SUBSCREEN_MODE change_mode)
{
  CISS_WORK *ciss;
  GMEVENT * event;
  FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
  
  event = GMEVENT_Create(
    gsys, NULL, ChangeIntrudeSubScreenEvent, sizeof(CISS_WORK));
  
  ciss = GMEVENT_GetEventWork(event);
  MI_CpuClear8( ciss, sizeof(CISS_WORK) );
  
  ciss->gsys = gsys;
  ciss->gmEvent = event;
  ciss->fieldWork = fieldWork;
  ciss->change_mode = change_mode;
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント：侵入によるサブスクリーン切り替え
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ChangeIntrudeSubScreenEvent( GMEVENT *event, int *seq, void *wk )
{
  CISS_WORK *ciss = wk;
  switch (ciss->seq){
  case 0:
    if( FIELD_SUBSCREEN_CanChange( FIELDMAP_GetFieldSubscreenWork(ciss->fieldWork) ) == TRUE )
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( ciss->fieldWork );
      GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
      MMDLSYS_PauseMoveProc( fldMdlSys );
      GAMEDATA_SetSubScreenType( gamedata , FMIT_POKEMON );
      FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(ciss->fieldWork), ciss->change_mode);
      ciss->seq++;
    }
    break;
  case 1:
    if( FIELD_SUBSCREEN_CanChange( FIELDMAP_GetFieldSubscreenWork(ciss->fieldWork) ) == TRUE )
    {
      GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
      GAMEDATA_SetSubScreenMode(gamedata, ciss->change_mode);
      ciss->seq++;
    }
    break;
  case 2:
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( ciss->fieldWork );
      MMDLSYS_ClearPauseMoveProc( fldMdlSys );
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;

}


//==============================================================================
//  ワープイベント
//==============================================================================
//==================================================================
/**
 * 侵入によるサブスクリーン切り替えイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudeTownWarp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT * event;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  int town_tblno;
  ZONEID warp_zone_id;
  VecFx32 pos;
  
  town_tblno = Intrude_GetWarpTown(game_comm);
  if(town_tblno == PALACE_TOWN_DATA_NULL){
    return NULL;
  }
  
  warp_zone_id = Intrude_GetPalaceTownZoneID(town_tblno);
  Intrude_GetPalaceTownRandPos(town_tblno, &pos);
  
  event = DEBUG_EVENT_ChangeMapPos(gsys, fieldWork, warp_zone_id, &pos, DIR_UP);
  return event;
}


#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『ゾーンジャンプ』
 * @file   event_debug_menu_zone_jump.c
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"
#include "debug/debug_str_conv.h"
#include "event_debug_local.h"
#include "event_fieldmap_control.h"
#include "event_mapchange.h"

#include "message.naix"
#include "msg/debug/msg_d_field.h"

#include "event_debug_menu_zone_jump.h"


//======================================================================
//  デバッグメニュー ゾーンジャンプ
//======================================================================
#include "debug/debug_zone_jump.h"
FS_EXTERN_OVERLAY(debug_zone_jump);
static GMEVENT_RESULT debugMenuZoneJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// デバッグ国連データ作成用ワーク
//=====================================
typedef struct
{
  HEAPID HeapID;
  GAMESYS_WORK    *gsys;
  GMEVENT         *Event;
  FIELDMAP_WORK *FieldWork;
  PROCPARAM_DEBUG_ZONE_JUMP p_work;
} DEBUG_ZONE_JUMP_EVENT_WORK;


//--------------------------------------------------------------
/**
 * イベント：ゾーンジャンプ
 * @param gsys
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_ZoneJump( GAMESYS_WORK *gsys, void* wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  FIELDMAP_WORK *fieldWork  = debug_work->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_ZONE_JUMP_EVENT_WORK *evt_work;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));
  GMEVENT *event;

  //イベント生成
  event = GMEVENT_Create( gsys, debug_work->gmEvent, debugMenuZoneJump, sizeof(DEBUG_ZONE_JUMP_EVENT_WORK) );
  evt_work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( evt_work, sizeof(DEBUG_ZONE_JUMP_EVENT_WORK) );

  //ワーク設定
  evt_work->gsys = gsys;
  evt_work->Event = event;
  evt_work->FieldWork = fieldWork;
  evt_work->HeapID = heapID;

  return event;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグゾーンジャンプイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_RESULT,
    SEQ_PROC_END,
  };

  DEBUG_ZONE_JUMP_EVENT_WORK *evt_work = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( evt_work->Event, EVENT_FieldSubProc( evt_work->gsys, evt_work->FieldWork,
        FS_OVERLAY_ID(debug_zone_jump), &ProcData_DebugZoneJump, &evt_work->p_work ) );
    *p_seq  = SEQ_PROC_RESULT;
    break;
  case SEQ_PROC_RESULT:
    if ( evt_work->p_work.Ret )
    {
      GMEVENT * mapchange_event;
      OS_Printf("ゾーンID　%d にジャンプ\n", evt_work->p_work.ZoneID);
      mapchange_event = DEBUG_EVENT_QuickChangeMapDefaultPos( evt_work->gsys, evt_work->FieldWork, evt_work->p_work.ZoneID );
      GMEVENT_ChangeEvent( p_event, mapchange_event );
    }
    else *p_seq  = SEQ_PROC_END;
    break;
  case SEQ_PROC_END:
    return GMEVENT_RES_FINISH;
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

#endif // PM_DEBUG 


#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『フィールド技』
 * @file   event_debug_menu_fskill.c
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
#include "event_mapchange.h"
#include "fieldmap.h"
#include "field_event_check.h"
#include "script.h"

#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/script/hiden_def.h"  //SCRID_HIDEN_DIVING

#include "event_debug_menu_fskill.h"



//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuFieldSkillListEvent(GMEVENT *event, int *seq, void *wk );
static BOOL debugMenuCallProc_Naminori( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_Kairiki( DEBUG_MENU_EVENT_WORK *wk ); 
static BOOL debugMenuCallProc_Ananukenohimo( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Anawohoru( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Teleport( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Diving( DEBUG_MENU_EVENT_WORK *p_wk );



static const FLDMENUFUNC_LIST DATA_SubFieldSkillList[] =
{
  { DEBUG_FIELD_STR31, debugMenuCallProc_Naminori },              //波乗り
  { DEBUG_FIELD_STR45, debugMenuCallProc_Kairiki },               //怪力
  { DEBUG_FIELD_ANANUKENOHIMO, debugMenuCallProc_Ananukenohimo }, //穴ヌケの紐
  { DEBUG_FIELD_ANAWOHORU, debugMenuCallProc_Anawohoru },         //穴を掘る
  { DEBUG_FIELD_TELEPORT, debugMenuCallProc_Teleport },           //テレポート
  { DEBUG_FIELD_FSKILL_01, debugMenuCallProc_Diving },            //ダイビング
};

static const DEBUG_MENU_INITIALIZER DebugSubFieldSkillListSelectData = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_SubFieldSkillList),
  DATA_SubFieldSkillList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};


//--------------------------------------------------------------
/**
 * イベント：フィールド技
 * @param gsys
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_Fskill( GAMESYS_WORK *gsys, void* wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  DEBUG_MENU_EVENT_WORK   *work;
  GMEVENT *event;

  event = GMEVENT_Create( gsys, debug_work->gmEvent,
    debugMenuFieldSkillListEvent, sizeof(DEBUG_MENU_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MENU_EVENT_WORK) );

  *work  = *debug_work;
  work->gmEvent = event;
  work->call_proc = NULL;

  work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugSubFieldSkillListSelectData );

  return event;
}


//--------------------------------------------------------------
/**
 * イベント：フィールド技リスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuFieldSkillListEvent(GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MENU_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }else if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        (*seq)++;
      }else if( ret != FLDMENUFUNC_CANCEL ){  //決定
        work->call_proc = (DEBUG_MENU_CALLPROC)ret;
        (*seq)++;
      }
    }
    break;

  case 2:
    {
      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( work->call_proc != NULL ){
        if( work->call_proc(work) == TRUE ){
          return( GMEVENT_RES_CONTINUE );
        }
      }

      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー　波乗り
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りイベント呼び出し
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @retval BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Naminori( DEBUG_MENU_EVENT_WORK *wk )
{
  if( FIELDMAP_GetMapControlType(wk->fieldWork) != FLDMAP_CTRLTYPE_GRID )
  {
    // レールのとき
    FIELD_PLAYER* p_fld_player = FIELDMAP_GetFieldPlayer( wk->fieldWork );
    if( FIELD_PLAYER_GetMoveForm( p_fld_player ) != PLAYER_MOVE_FORM_SWIM ){
      FIELD_PLAYER_SetNaminori( p_fld_player );
    }else{
      FIELD_PLAYER_SetNaminoriEnd( p_fld_player );
    }
    return( FALSE );
  }

  FIELD_EVENT_ChangeNaminoriStart( wk->gmEvent, wk->gmSys, wk->fieldWork );
  return( TRUE );
}

//======================================================================
//  怪力
//======================================================================
//--------------------------------------------------------------
/**
 * 怪力実行
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @retval BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Kairiki( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( wk->gdata );
  EVENTWORK_SetEventFlag( evwork, SYS_FLAG_KAIRIKI );
  return( FALSE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  あなぬけのヒモ
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Ananukenohimo( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT*      parent = wk->gmEvent;
  GAMESYS_WORK* gsys = wk->gmSys;
  FIELDMAP_WORK* fieldmap = wk->fieldWork;

  GMEVENT_ChangeEvent( parent, EVENT_ChangeMapByAnanukenohimo( fieldmap, gsys ) );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  あなをほる
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Anawohoru( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT*      parent = wk->gmEvent;
  GAMESYS_WORK* gsys = wk->gmSys;

  GMEVENT_ChangeEvent( parent, EVENT_ChangeMapByAnawohoru( gsys ) );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  テレポート
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Teleport( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT*      parent = wk->gmEvent;
  GAMESYS_WORK* gsys = wk->gmSys;

  GMEVENT_ChangeEvent( parent, EVENT_ChangeMapByTeleport( gsys ) );
  return TRUE;
} 
//----------------------------------------------------------------------------
/**
 *  @brief  フィールド技：ダイビング
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Diving( DEBUG_MENU_EVENT_WORK *wk )
{
  if( FIELDMAP_GetMapControlType(wk->fieldWork) != FLDMAP_CTRLTYPE_GRID )
  {
    FIELD_PLAYER* p_fld_player = FIELDMAP_GetFieldPlayer( wk->fieldWork );

    // レール上ではこっち
    FIELD_PLAYER_SetRequest( p_fld_player, FIELD_PLAYER_REQBIT_DIVING );
    return FALSE;
  }
  SCRIPT_ChangeScript( wk->gmEvent, SCRID_HIDEN_DIVING_MENU, NULL, HEAPID_FIELDMAP );
  return TRUE;
}


#endif // PM_DEBUG 

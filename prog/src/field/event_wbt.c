//======================================================================
/**
 * @file  event_wbt.c
 * @brief  WBT　イベント
 * @date  2011.02.27
 * @author  tamada
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem\btl_setup.h"
#include "event_fieldmap_control.h"
#include "event_fldmmdl_control.h"
#include "poke_tool/poke_regulation.h"
#include "app/pokelist.h"
#include "app/p_status.h"
#include "app/leader_board.h"
#include "battle/battle.h"

#include "event_battle.h"
#include "field/event_battle_call.h"
#include "event_fieldmap_control.h"
#include "event_ircbattle.h"

#include "savedata/save_tbl.h"

#include "fieldmap.h"

#include "msgdata.h"
#include "field/field_msgbg.h"
#include "print/wordset.h"
#include "message.naix"

//#include "bsubway_scr.h"
//#include "bsubway_scrwork.h"
//#include "scrcmd_bsubway.h"
//#include "bsubway_tr.h"

#include "wbt_system.h"

#include "event_wbt.h"

//#include "effect_encount.h"

//#include "scrcmd_menuwin.h"

#include "script_message.naix"
//#include "../../../resource/message/dst/script/msg_bsubway_scr.h"
//#include "../../../resource/message/dst/msg_tower_trainer.h"

#include "system/net_err.h"

FS_EXTERN_OVERLAY(pokelist);

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//======================================================================
//  WBT　イベント　ポケモンリスト
//======================================================================
//--------------------------------------------------------------
/// ポケモンリスト選択イベント
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldmap;
  WBTSYS * wbtsys;
  
  PSTATUS_DATA status_data;

  PLIST_DATA list_data;
  
}EVENT_WORK_POKE_LIST;

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_SelectPokeList( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_POKE_LIST *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
		GMEVENT_CallEvent( event,
        EVENT_FieldFadeOut_Black(gsys,work->fieldmap,FIELD_FADE_WAIT) );
    (*seq)++;
    break;
  case 1:
		GMEVENT_CallEvent( event, EVENT_FieldClose(gsys,work->fieldmap) );
    (*seq)++;
    break;
  case 2:
    GMEVENT_CallEvent( event, EVENT_PokeSelect(
          gsys,work->fieldmap,&work->list_data,&work->status_data) );
    (*seq)++;
    break;
  case 3:
		GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
    (*seq)++;
    break;
  case 4:
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black(
          gsys,work->fieldmap, FIELD_FADE_WAIT) );
    (*seq)++;
    break;
  case 5:
    WBTSYS_SetPokelistResult( work->wbtsys, &work->list_data );
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント　呼び出し
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * WBT_EVENT_SetSelectPokeList(
    WBTSYS *wbtsys, GAMESYS_WORK *gsys, BOOL btl_box )
{
  GMEVENT *event;
  GAMEDATA *gdata;
  POKEPARTY *pp;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_POKE_LIST *work;
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  event = GMEVENT_Create( gsys, NULL,
      ev_SelectPokeList, sizeof(EVENT_WORK_POKE_LIST) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldmap = fieldmap;
  work->wbtsys = wbtsys;
  
  WBTSYS_SetupPokeListData( wbtsys, gdata, &work->list_data );
  
  WBTSYS_SetupPokeStatusData( wbtsys, gdata, &work->status_data );
   
  return( event );
}

//=============================================================================
//
//    WBTトーナメントリスト画面呼び出し
//
//=============================================================================
//  nsbtx_to_clwkを使いたいので名前入力と同じところにおいてみた
FS_EXTERN_OVERLAY(namein);
//-------------------------------------
///
//=====================================
typedef struct
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK       *fieldmap;
  WBTLIST_PARAM       *wbtlist_param;
  BOOL                call_fade_flag;
} WBTLIST_EVENT_WORK;

static GMEVENT_RESULT eventWBTList( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * WBT_EVENT_ShowTournamentList(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    WBTLIST_PARAM * wbtlist_param, BOOL call_fade_flag )
{
  WBTLIST_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イヴェント
  new_event = GMEVENT_Create( gsys, NULL, eventWBTList, sizeof(WBTLIST_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(WBTLIST_EVENT_WORK) );

  p_param->gsys  = gsys;
  p_param->fieldmap    = fieldmap;
  p_param->wbtlist_param = wbtlist_param;
  p_param->call_fade_flag = call_fade_flag;

  OS_Printf( "WBT List Start\n" );

  return new_event;

}
//----------------------------------------------------------------------------
/**
 *  @brief  WBTリストアプリ呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventWBTList( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  WBTLIST_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    if ( p_wk->call_fade_flag )
    {
      call_event = EVENT_FieldSubProc(
          p_wk->gsys, p_wk->fieldmap, FS_OVERLAY_ID(namein), &WBTListProcData, p_wk->wbtlist_param );
    } else {
      call_event = EVENT_FieldSubProcNoFade(
          p_wk->gsys, p_wk->fieldmap, FS_OVERLAY_ID(namein), &WBTListProcData, p_wk->wbtlist_param );
    }
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    WBTLIST_PARAM_Delete( p_wk->wbtlist_param );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}


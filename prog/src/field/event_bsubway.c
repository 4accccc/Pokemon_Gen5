//======================================================================
/**
 * @file  event_bsubway.c
 * @brief  バトルサブウェイ　イベント
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "event_fieldmap_control.h"
#include "poke_tool/poke_regulation.h"
#include "app/pokelist.h"
#include "app/p_status.h"
#include "battle/battle.h"

#include "event_battle.h"

#include "fieldmap.h"

#include "field/field_msgbg.h"
#include "message.naix"

#include "bsubway_scr.h"
#include "bsubway_scr_common.h"
#include "scrcmd_bsubway.h"
#include "bsubway_tr.h"

#include "event_bsubway.h"

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
//  バトルサブウェイ　イベント　ポケモンリスト
//======================================================================
//--------------------------------------------------------------
/// ポケモンリスト選択イベント
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldmap;
  
  PSTATUS_DATA status_data;
  PLIST_DATA list_data;
  
  u16 *return_mode;
  u16 *result_select;
  u8 *result_num;
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
    MI_CpuCopy8( work->list_data.in_num, work->result_num, 6 );
    *work->result_select = work->list_data.ret_sel;
    *work->return_mode = work->list_data.ret_mode;
    GFL_HEAP_FreeMemory( work->list_data.reg );
    (*seq)++;
    break;
  case 5:
    
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
GMEVENT * BSUBWAY_EVENT_SetSelectPokeList(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys )
{
  GMEVENT *event;
  GAMEDATA *gdata;
  POKEPARTY *pp;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_POKE_LIST *work;
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  pp = GAMEDATA_GetMyPokemon( gdata );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  event = GMEVENT_Create( gsys, NULL,
      ev_SelectPokeList, sizeof(EVENT_WORK_POKE_LIST) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldmap = fieldmap;
  work->result_num = bsw_scr->pokelist_select_num;
  work->result_select = &bsw_scr->pokelist_result_select;
  work->return_mode = &bsw_scr->pokelist_return_mode;
  
  {
    int reg = REG_SUBWAY_SINGLE;
    int type = PL_TYPE_SINGLE;
    PLIST_DATA *list = &work->list_data;
    
    MI_CpuClear8( list, sizeof(PLIST_DATA) );
    list->pp = pp;
    
    switch( bsw_scr->play_mode ){
    case BSWAY_MODE_DOUBLE:
      reg = REG_SUBWAY_DOUBLE;
      type = PL_TYPE_DOUBLE;
      break;
    }
    
    list->reg = (void*)PokeRegulation_LoadDataAlloc( reg, HEAPID_PROC );
    list->mode = PL_MODE_BATTLE;
    list->type = type;
  }
  
  {
    PSTATUS_DATA *st = &work->status_data;
    MI_CpuClear8( st, sizeof(PSTATUS_DATA) );
    st->ppd = pp;
    st->ppt = PST_PP_TYPE_POKEPARTY;
    st->max = PokeParty_GetPokeCount( pp );
    st->mode = PST_MODE_NORMAL;
    st->page = PPT_INFO;
  }
   
  return( event );
}

//======================================================================
//  バトルサブウェイ　戦闘
//======================================================================
//--------------------------------------------------------------
/**
 * バトルサブウェイ　トレーナーバトル
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_TrainerBattle(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT * event;
  BATTLE_SETUP_PARAM *bp;
  
  bp = BtlTower_CreateBattleParam( bsw_scr, gsys );
  event = EVENT_BSubwayTrainerBattle( gsys, fieldmap, bp );
  
  return( event );
}

//======================================================================
//  バトルサブウェイ　イベント　対戦前メッセージ
//======================================================================
//--------------------------------------------------------------
/// 対戦前メッセージ
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  BSUBWAY_SCRWORK *bsw_scr;
  
  FLDSYSWIN_STREAM *sysWin;
  STRBUF *strBuf;
}EVENT_WORK_TRAINER_BEFORE_MSG;

//--------------------------------------------------------------
/**
 * 対戦前メッセージ
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_TrainerMsgBefore(
    GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_TRAINER_BEFORE_MSG *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
    FLDSYSWIN_STREAM_PrintStrBufStart( work->sysWin, 0, 0, work->strBuf );
    (*seq)++;
    break;
  case 1:
    if( FLDSYSWIN_STREAM_Print(work->sysWin) == TRUE ){
      (*seq)++;
    }
    break;
  case 2:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      FLDSYSWIN_STREAM_Delete( work->sysWin );
      GFL_STR_DeleteBuffer( work->strBuf );
      return( GMEVENT_RES_FINISH );
    }
    break;
  }
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * 対戦前メッセージ 呼び出し
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_TrainerBeforeMsg(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, int tr_idx )
{
  GMEVENT *event;
  GAMEDATA *gdata;
  FLDMSGBG *msgbg;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_TRAINER_BEFORE_MSG *work;
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  msgbg = FIELDMAP_GetFldMsgBG( fieldmap );
  
  event = GMEVENT_Create( gsys, NULL,
    ev_TrainerMsgBefore, sizeof(EVENT_WORK_POKE_LIST) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->strBuf = GFL_STR_CreateBuffer( 92, HEAPID_PROC );
  
  if( bsw_scr->tr_data[tr_idx].bt_trd.appear_word[0] == 0xffff ){ //ROM MSG
    GFL_MSGDATA *msgdata;
    
    msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_tower_trainer_dat, HEAPID_PROC );
    
    GFL_MSG_GetString(
        msgdata, 
        bsw_scr->tr_data[tr_idx].bt_trd.appear_word[1],
        work->strBuf );
    
    GFL_MSG_Delete( msgdata );
  }else{ //簡易会話 kari
    GFL_MSGDATA *msgdata;
    
    msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_tower_trainer_dat, HEAPID_PROC );
    
    GFL_MSG_GetString(
        msgdata,
        0,
        work->strBuf );
    
    GFL_MSG_Delete( msgdata );
  }
  
  work->sysWin = FLDSYSWIN_STREAM_Add( msgbg, NULL, 19 );
  return( event );
}


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

#include "bsubway_scr.h"
#include "bsubway_scrwork.h"
#include "scrcmd_bsubway.h"
#include "bsubway_tr.h"


#include "event_wbt.h"

#include "effect_encount.h"

#include "scrcmd_menuwin.h"

#include "script_message.naix"
//#include "../../../resource/message/dst/script/msg_bsubway_scr.h"
#include "../../../resource/message/dst/msg_tower_trainer.h"

#include "system/net_err.h"

FS_EXTERN_OVERLAY(pokelist);

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// EVENT_BSW_COMM_BATTLE_WORK
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  BATTLE_SETUP_PARAM *para;
  COMM_BTL_DEMO_PARAM *demo_prm;
}EVENT_BSW_COMM_BATTLE_WORK;

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
  
  PSTATUS_DATA status_data;
  PLIST_DATA list_data;
  
  u16 *return_mode;
  u16 *result_select;
  u8 *result_num;
  
  POKEPARTY *pp_btl_box;
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
GMEVENT * WBT_EVENT_SetSelectPokeList(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, BOOL btl_box )
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
  work->result_num = bsw_scr->pokelist_select_num;
  work->result_select = &bsw_scr->pokelist_result_select;
  work->return_mode = &bsw_scr->pokelist_return_mode;
  
  if( btl_box == FALSE ){
    pp = GAMEDATA_GetMyPokemon( gdata );
  }else{
    pp = bsw_scr->btl_box_party;
  }
  
  GF_ASSERT( pp != NULL );

  {
    int reg = REG_SUBWAY_SINGLE;
    int type = PL_TYPE_SINGLE;
    PLIST_DATA *list = &work->list_data;
    
    MI_CpuClear8( list, sizeof(PLIST_DATA) );
    list->pp = pp;
    
    switch( bsw_scr->play_mode ){
    case BSWAY_MODE_DOUBLE:
    case BSWAY_MODE_S_DOUBLE:
      reg = REG_SUBWAY_DOUBLE;
      type = PL_TYPE_DOUBLE;
      break;
    case BSWAY_MODE_MULTI:
    case BSWAY_MODE_S_MULTI:
    case BSWAY_MODE_COMM_MULTI:
    case BSWAY_MODE_S_COMM_MULTI:
      reg = REG_SUBWAY_MALTI;
      type = PL_TYPE_MULTI;
      break;
    }
    
#ifdef DEBUG_BSW_PRINT    
    KAGAYA_Printf( "レギュレーション　%d\n", reg );
#endif
    
    list->reg = (void*)PokeRegulation_LoadDataAlloc( reg, HEAPID_PROC );
    list->mode = PL_MODE_BATTLE_SUBWAY;
    list->type = type;
    
    //debug
    #ifdef DEBUG_ONLY_FOR_kagaya
    {
      REGULATION *pReg = list->reg;
      
#ifdef DEBUG_BSW_PRINT    
      KAGAYA_Printf( "BSWAY REG NUM_LO = %d, HI =%d, LV = %d\n",
        pReg->NUM_LO, pReg->NUM_HI, pReg->LEVEL );
#endif
    }
    #endif
  }
  
  {
    PSTATUS_DATA *st = &work->status_data;
    ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gdata );
    MI_CpuClear8( st, sizeof(PSTATUS_DATA) );
    st->ppd = pp;
    st->ppt = PST_PP_TYPE_POKEPARTY;
    st->max = PokeParty_GetPokeCount( pp );
    st->mode = PST_MODE_NORMAL;
    st->page = PPT_INFO;
    st->game_data = gdata;

    st->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
  }
   
  return( event );
}



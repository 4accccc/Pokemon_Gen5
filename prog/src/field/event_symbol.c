//==============================================================================
/**
 * @file    event_symbol.c
 * @brief   侵入：シンボルエンカウント
 * @author  matsuda
 * @date    2010.03.16(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "event_comm_talk.h"
#include "field/event_fieldmap_control.h" //EVENT_FieldSubProc
#include "event_symbol.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "app/pdc.h"
#include "poke_tool/poke_tool.h"
#include "sound/pm_sndsys.h"


//==============================================================================
//  定数定義
//==============================================================================
///シンボルポケモン用バトルBGM
#define SND_SYMBOL_BATTLE_BGM   (SEQ_BGM_VS_NORAPOKE)
///シンボルポケモン用エンカウントエフェクトID
#define SYMBOL_ENCEFF_ID        (ENCEFFID_WILD_NORMAL)

///シンボルポケモンのレベル
#define SYMBOL_POKE_LEVEL       (30)


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct{
  FIELDMAP_WORK *fieldWork;
  PDC_SETUP_PARAM *pdc_setup;
  POKEMON_PARAM *pp;
  BTL_FIELD_SITUATION bfs;
  HEAPID heap_id;
  u16 *result_ptr;
}EVENT_SYMBOL_BATTLE;

typedef struct{
  u16 *result_ptr;
  SYMBOL_ZONE_TYPE zone_type;
  u8 map_no;
}EVENT_REQ_SYMBOL_PARAM;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT EventSymbolPokeBattle( GMEVENT *event, int *seq, void *wk );



//==================================================================
/**
 * シンボルポケモン：バトル
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   spoke		      出現させるシンボルポケモンデータへのポインタ
 * @param   result_ptr		結果代入先(TRUE:シンボルポケモンを捕獲した。　FALSE:捕獲しなかった)
 * @param   heap_id		    
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_SymbolPokeBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const SYMBOL_POKEMON *spoke, u16 *result_ptr, HEAPID heap_id)
{
	GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  EVENT_SYMBOL_BATTLE *esb;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventSymbolPokeBattle, sizeof(EVENT_SYMBOL_BATTLE) );
  
	esb = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( esb, sizeof(EVENT_SYMBOL_BATTLE) );

  esb->fieldWork = fieldWork;
  esb->result_ptr = result_ptr;
  esb->heap_id = heap_id;
  
  { //シンボルポケモンをPokemonParamとして生成
    u32 personal_rnd, id;
    
    id = MyStatus_GetID( GAMEDATA_GetMyStatus(gamedata) );
    
    esb->pp = PP_Create( spoke->monsno, SYMBOL_POKE_LEVEL, id, heap_id );
    personal_rnd = POKETOOL_CalcPersonalRandEx( 
      id, spoke->monsno, spoke->form_no, spoke->sex, 0, FALSE );
    PP_SetupEx( esb->pp, spoke->monsno, SYMBOL_POKE_LEVEL, id, PTL_SETUP_POW_AUTO, personal_rnd );
    PP_SetTokusei3( esb->pp, spoke->monsno, spoke->form_no );
  }

  BTL_FIELD_SITUATION_SetFromFieldStatus( &esb->bfs, gamedata, fieldWork );

	return( event );
}

//--------------------------------------------------------------
/**
 * シンボルポケモン：バトル
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventSymbolPokeBattle( GMEVENT *event, int *seq, void *wk )
{
  EVENT_SYMBOL_BATTLE *esb = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
	enum{
    SEQ_BATTLE_BGM,
    SEQ_ENCEFF,
    SEQ_FADEOUT,
    SEQ_FIELD_CLOSE,
    SEQ_BATTLE,
    SEQ_RESULT,
    SEQ_FIELD_OPEN,
    SEQ_FADEIN,
    SEQ_BGMPOP,
  };
	
	switch( *seq ){
  case SEQ_BATTLE_BGM:
    // 戦闘用ＢＧＭセット
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, SND_SYMBOL_BATTLE_BGM));
    (*seq)++;
    break;
  case SEQ_ENCEFF:
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(esb->fieldWork), event, SYMBOL_ENCEFF_ID);
    (*seq)++;
    break;
  case SEQ_FADEOUT:
  #if 0 //エンカウントエフェクトでやっているのでいらない
    { // フェードアウト
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, esb->fieldWork, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
  #endif
		(*seq) ++;
		break;
  case SEQ_FIELD_CLOSE:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, esb->fieldWork));
    (*seq)++;
    break;
	case SEQ_BATTLE:
    { // サブプロセス呼び出しイベント
      GMEVENT* ev_sub;
      SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
      
      GFL_OVERLAY_Load( FS_OVERLAY_ID(pdc) );
      esb->pdc_setup = PDC_MakeSetUpParam( esb->pp, &esb->bfs, 
        GAMEDATA_GetMyStatus(gamedata), GAMEDATA_GetMyItem(gamedata), 
        SaveData_GetConfig( sv ), ZUKAN_SAVEDATA_GetZukanSave(sv), esb->heap_id );
      GMEVENT_CallProc( event, NO_OVERLAY_ID, &PDC_ProcData, esb->pdc_setup );
    }
		(*seq) ++;
		break;
  case SEQ_RESULT:
    //PDC_RESULTの結果取得
    if(PDC_GetResult( esb->pdc_setup ) == PDC_RESULT_CAPTURE){
      *(esb->result_ptr) = TRUE;
    }
    else{
      *(esb->result_ptr) = FALSE;
    }
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(pdc) );
    (*seq)++;
    break;
  case SEQ_FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq)++;
    break;
  case SEQ_FADEIN:
    { // フェードイン
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, esb->fieldWork, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
		(*seq) ++;
		break;
	case SEQ_BGMPOP:
	  // フィールドBGM復帰
    GMEVENT_CallEvent(event, EVENT_FSND_PopPlayBGM_fromBattle(gsys));
    (*seq)++;
    break;

  default:
    GFL_HEAP_FreeMemory(esb->pp);
    GFL_HEAP_FreeMemory(esb->pdc_setup);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}



#if 0
//==============================================================================
//
//  侵入先のシンボルデータを要求し、受信
//
//==============================================================================
//==================================================================
/**
 * 侵入先のシンボルデータを要求し、受信するまでの処理をイベントとして行います
 *
 * @param   gsys		
 * @param   result_ptr		結果代入先(TRUE:正常にデータ受信。　FALSE:受信出来なかった(通信エラー))
 * @param   zone_type	  	相手へ要求するSYMBOL_ZONE_TYPE
 * @param   map_no	  	  相手へ要求するマップ番号(キープゾーンの場合は0固定)
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_ReqIntrudeSymbolParam(GAMESYS_WORK *gsys, u16 *result_ptr, SYMBOL_ZONE_TYPE zone_type, u8 map_no)
{
	GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  EVENT_REQ_SYMBOL_PARAM *ersp;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventReqIntrudeSymbolParam, sizeof(EVENT_REQ_SYMBOL_PARAM) );
  
	ersp = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( ersp, sizeof(EVENT_REQ_SYMBOL_PARAM) );

  ersp->result_ptr = result_ptr;
  ersp->zone_type = zone_type;
  ersp->map_no = map_no;

	return( event );
}

//--------------------------------------------------------------
/**
 * 侵入先のシンボルデータを要求し、受信するまでの処理をイベントとして行います
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventReqIntrudeSymbolParam( GMEVENT *event, int *seq, void *wk )
{
  EVENT_REQ_SYMBOL_PARAM *ersp = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	
	intcomm = Intrude_Check_CommConnect(game_comm);
	if(intcomm == NULL){
    if(ersp->result_ptr != NULL){
      *(ersp->result_ptr) = FALSE;
    }
    return GMEVENT_RES_FINISH;
  }

	switch( *seq ){
	case 0:
	  if(IntrudeSymbol_ReqSymbolData(intcomm, ersp->zone_type, ersp->map_no) == TRUE){
      (*seq)++;
    }
    break;
  case 1:
    if(IntrudeSymbol_CheckRecvSymbolData(intcomm) == TRUE){
      (*seq)++;
    }
    break;
  case 2:
    if(ersp->result_ptr != NULL){
      *(ersp->result_ptr) = TRUE;
    }
    return GMEVENT_RES_FINISH;
  }
  
	return GMEVENT_RES_CONTINUE;
}




//==============================================================================
//
//  シンボルマップ遷移
//
//==============================================================================
//==================================================================
/**
 * シンボルマップ遷移
 *    シンボルマップのデータ要求 ＞ マップ遷移
 * までをイベントとして行います
 *
 * @param   gsys		
 * @param   result_ptr		結果代入先  TRUE:正常にデータ受信してマップ遷移した。
 *                                    FALSE:受信出来なかった為マップ遷移もしていない(通信エラー)
 * @param   zone_type	  	相手へ要求するSYMBOL_ZONE_TYPE
 * @param   map_no	  	  相手へ要求するマップ番号(キープゾーンの場合は0固定)
 *
 * @retval  GMEVENT *		
 *
 * 侵入先のシンボルデータ要求中に通信エラーが起きた場合はマップ遷移をせずにイベント終了します。
 */
//==================================================================
GMEVENT * EVENT_SymbolMapWarp(GAMESYS_WORK *gsys, u16 *result_ptr, SYMBOL_ZONE_TYPE zone_type, u8 map_no)
{
	GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  EVENT_REQ_SYMBOL_PARAM *ersp;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventReqIntrudeSymbolParam, sizeof(EVENT_REQ_SYMBOL_PARAM) );
  
	ersp = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( ersp, sizeof(EVENT_REQ_SYMBOL_PARAM) );

  ersp->result_ptr = result_ptr;
  ersp->zone_type = zone_type;
  ersp->map_no = map_no;

	return( event );
}

//--------------------------------------------------------------
/**
 * 侵入先のシンボルデータを要求し、受信するまでの処理をイベントとして行います
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventReqIntrudeSymbolParam( GMEVENT *event, int *seq, void *wk )
{
  EVENT_REQ_SYMBOL_PARAM *ersp = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	
	switch( *seq ){
	case 0:
	  GMEVENT_CallEvent(event, 
	    EVENT_ReqIntrudeSymbolParam(gsys, &ersp->data_recv_result, ersp->zone_type, ersp->map_no));
    (*seq)++;
    break;
  case 1:
    if(ersp->data_recv_result == FALSE){
      if(ersp->result_ptr != NULL){
        *(ersp->result_ptr) = FALSE;
      }
      return GMEVENT_RES_FINISH;
    }
    
    if(IntrudeSymbol_CheckRecvSymbolData(intcomm) == TRUE){
      (*seq)++;
    }
    break;
  case 2:
    if(ersp->result_ptr != NULL){
      *(ersp->result_ptr) = TRUE;
    }
    return GMEVENT_RES_FINISH;
  }
  
	return GMEVENT_RES_CONTINUE;
}

#endif

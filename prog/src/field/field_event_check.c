//======================================================================
/**
 * @file  field_event_check.c
 * @brief
 * @author  tamada GAME FREAK inc.
 * @date  2009.06.09
 *
 * 2009.06.09 fieldmap.cからイベント起動チェック部分を独立させた
 */
//======================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_event.h"
#include "field/zonedata.h"
//#include "field/areadata.h"

#include "field_event_check.h"

#include "fieldmap.h"
#include "field/fieldmap_proc.h"
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"
#include "eventdata_local.h"

#include "system/wipe.h"      //WIPE_SYS_EndCheck
#include "event_mapchange.h"
#include "event_ircbattle.h"      //EVENT_IrcBattle
#include "field_comm/field_comm_main.h"   //DEBUG_PalaceJamp
#include "field_comm/field_comm_event.h"   //FIELD_COMM_EVENT_～
#include "event_debug_menu.h"     //DEBUG_EVENT_DebugMenu
#include "event_battle.h"         //EVENT_Battle
#include "event_fieldtalk.h"      //EVENT_FieldTalk
#include "event_fieldmap_menu.h"  //EVENT_FieldMapMenu
#include "script.h"     //SCRIPT_SetScript

#include "field_encount.h"      //FIELD_ENCOUNT_CheckEncount

//======================================================================
//======================================================================
//event
static GMEVENT * fldmap_Event_CheckConnect(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const VecFx32 *now_pos );
static GMEVENT * fldmap_Event_CheckPushConnect(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );

//======================================================================
//	フィールドマップ　サブ　イベント
//======================================================================
//--------------------------------------------------------------
/**
 * イベント マップ接続チェック
 * @param gsys GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param now_pos チェックする座標
 * @retval GMEVENT NULL=接続なし
 */
//--------------------------------------------------------------
static GMEVENT * fldmap_Event_CheckConnect(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const VecFx32 *now_pos )
{
	const CONNECT_DATA * cnct;
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
	int idx = EVENTDATA_SearchConnectIDByPos(evdata, now_pos);
	
	if( idx == EXIT_ID_NONE ){
		return NULL;
	}

	//マップ遷移発生の場合、出入口を記憶しておく
	{
		LOCATION ent_loc;
		LOCATION_Set(
				&ent_loc, FIELDMAP_GetZoneID(fieldWork), idx, 0,
				now_pos->x, now_pos->y, now_pos->z);
		GAMEDATA_SetEntranceLocation(gamedata, &ent_loc);
	}

	cnct = EVENTDATA_GetConnectByID(evdata, idx);
	
	if (CONNECTDATA_IsSpecialExit(cnct)) {
		//特殊接続先が指定されている場合、記憶しておいた場所に飛ぶ
		const LOCATION * sp = GAMEDATA_GetSpecialLocation(gamedata);
		return EVENT_ChangeMap(gsys, fieldWork, sp);
	}else{
		LOCATION loc_req;
		CONNECTDATA_SetNextLocation(cnct, &loc_req);
		return EVENT_ChangeMap(gsys, fieldWork, &loc_req);
	}
}

//--------------------------------------------------------------
/**
 * イベント キー入力イベント起動チェック
 * @param gsys GAMESYS_WORK
 * @param	fieldWork FIELDMAP_WORK
 * @retval GMEVENT NULL イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * fldmap_Event_CheckPushConnect(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
	VecFx32 now_pos = *FIELDMAP_GetNowPos(fieldWork);
	
	switch( GFL_UI_KEY_GetCont() ) {
	case PAD_KEY_UP:		now_pos.z -= FX32_ONE * 16; break;
	case PAD_KEY_DOWN:	now_pos.z += FX32_ONE * 16; break;
	case PAD_KEY_LEFT:	now_pos.x -= FX32_ONE * 16; break;
	case PAD_KEY_RIGHT:	now_pos.x += FX32_ONE * 16; break;
	default: return NULL;
	}
	
	{	
		u32 attr = 0;
		FLDMAPPER_GRIDINFO gridInfo;
		FLDMAPPER *g3Dmapper = FIELDMAP_GetFieldG3Dmapper(fieldWork);
		
		if( FLDMAPPER_GetGridInfo(g3Dmapper,&now_pos,&gridInfo) == TRUE ){
			attr = gridInfo.gridData[0].attr;
		}

		if( attr == 0 ){
			return NULL;
		}
	}
	
	return fldmap_Event_CheckConnect(gsys, fieldWork, &now_pos);
}

//--------------------------------------------------------------
/**
 * サブスクリーンからのイベント起動チェック
 * @param gsys GAMESYS_WORK
 * @param	fieldWork FIELDMAP_WORK
 * @retval GMEVENT NULL イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * fldmap_Event_Check_SubScreen(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  GMEVENT* event=NULL;
  
  switch(FIELD_SUBSCREEN_GetAction(FIELDMAP_GetFieldSubscreenWork(fieldWork))){
  case FIELD_SUBSCREEN_ACTION_DEBUGIRC:
		{
			GAME_COMM_SYS_PTR gcsp = GAMESYSTEM_GetGameCommSysPtr(gsys);
			GAME_COMM_NO no = GameCommSys_BootCheck(gcsp);
			if(GAME_COMM_NO_FIELD_BEACON_SEARCH == no){
				GameCommSys_ExitReq(gcsp);
			}
			if((GAME_COMM_NO_FIELD_BEACON_SEARCH == no) || (GAME_COMM_NO_NULL == no)){
				event = EVENT_IrcBattle(gsys, fieldWork, NULL, TRUE);
			}
		}
    break;
#if PM_DEBUG
  case FIELD_SUBSCREEN_ACTION_DEBUG_PALACEJUMP:
		event = DEBUG_PalaceJamp(fieldWork , gsys, FIELDMAP_GetFieldPlayer(fieldWork));
		break;
#endif
	}
	if(event)
	{
		FIELD_SUBSCREEN_GrantPermission(FIELDMAP_GetFieldSubscreenWork(fieldWork));
	}
	else
	{
		FIELD_SUBSCREEN_ResetAction(FIELDMAP_GetFieldSubscreenWork(fieldWork));
	}
  return event;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * checkRailExit(GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork)
{
  int idx;
  VecFx32 pos;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
  int * firstID = FIELDMAP_GetFirstConnectID(fieldWork);

  if (ZONEDATA_DEBUG_IsRailMap(FIELDMAP_GetZoneID(fieldWork)) == FALSE) return NULL;
  FIELD_RAIL_MAN_GetPos( FIELDMAP_GetFieldRailMan(fieldWork) , &pos );
  idx = EVENTDATA_SearchConnectIDBySphere(evdata, &pos);
  if (*firstID == idx) return NULL;
  if (idx == EXIT_ID_NONE)
  {
    *firstID = idx;
    return NULL;
  }

  {
		LOCATION loc_req;
    const CONNECT_DATA * cnct;
    cnct = EVENTDATA_GetConnectByID(evdata, idx);
	
		CONNECTDATA_SetNextLocation(cnct, &loc_req);
		return EVENT_ChangeMap(gsys, fieldWork, &loc_req);
  }
#if 0
	//マップ遷移発生の場合、出入口を記憶しておく
	{
		LOCATION ent_loc;
		LOCATION_Set(
				&ent_loc, FIELDMAP_GetZoneID(fieldWork), idx, 0,
				now_pos->x, now_pos->y, now_pos->z);
		GAMEDATA_SetEntranceLocation(gamedata, &ent_loc);
	}
#endif

#if 0
	if (CONNECTDATA_IsSpecialExit(cnct)) {
		//特殊接続先が指定されている場合、記憶しておいた場所に飛ぶ
		const LOCATION * sp = GAMEDATA_GetSpecialLocation(gamedata);
		return EVENT_ChangeMap(gsys, fieldWork, sp);
	}else{
		LOCATION loc_req;
		CONNECTDATA_SetNextLocation(cnct, &loc_req);
		return EVENT_ChangeMap(gsys, fieldWork, &loc_req);
	}
#endif

}

extern u16 EVENTDATA_CheckPosEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const VecFx32 *pos );

extern u16 EVENTDATA_CheckTalkBGEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const VecFx32 *pos, u16 talk_dir );

//--------------------------------------------------------------
/**
 * イベント　イベント起動チェック
 * @param	gsys GAMESYS_WORK
 * @param work FIELDMAP_WORK
 * @retval GMEVENT NULL=イベント無し
 */
//--------------------------------------------------------------
GMEVENT * FIELD_EVENT_CheckNormal( GAMESYS_WORK *gsys, void *work )
{
	enum{
		resetCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START,
		chgCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_SELECT
	};
	
  int trg,cont;
	GMEVENT *event;
	PLAYER_MOVE_STATE state;
  PLAYER_MOVE_VALUE value;
	FIELDMAP_WORK *fieldWork = work;
  BOOL isGridMap = FALSE;
  
  //グリッドマップかどうかの判定フラグセット
  {
    const DEPEND_FUNCTIONS *func_tbl = FIELDMAP_GetDependFunctions(fieldWork);
    if( func_tbl->type == FLDMAP_CTRLTYPE_GRID )
    {
      isGridMap = TRUE;
    }
  }

  //キー情報を取得
  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();

  //自機動作ステータス更新
  FIELD_PLAYER_UpdateMoveStatus( FIELDMAP_GetFieldPlayer(fieldWork) );
  
  //自機の動作状態を取得
  value = FIELD_PLAYER_GetMoveValue( FIELDMAP_GetFieldPlayer(fieldWork) );
  state = FIELD_PLAYER_GetMoveState( FIELDMAP_GetFieldPlayer(fieldWork) );
  
  //デバッグ用チェック
#ifdef  PM_DEBUG
	//ソフトリセットチェック
	if( (cont&resetCont) == resetCont ){
		return DEBUG_EVENT_GameEnd(gsys, fieldWork);
	}
	
	//マップ変更チェック
	if( (cont&chgCont) == chgCont ){
		return DEBUG_EVENT_ChangeToNextMap(gsys, fieldWork);
	}
	
	//デバッグメニュー起動チェック
  if( (trg & PAD_BUTTON_X) && (cont & PAD_BUTTON_R) )
  {
		return DEBUG_EVENT_DebugMenu(gsys, fieldWork, 
				FIELDMAP_GetHeapID(fieldWork), ZONEDATA_GetMapRscID(FIELDMAP_GetZoneID(fieldWork)));
	}
#endif //debug
	
  //戦闘移行チェック
  if( isGridMap ){
    #ifdef PM_DEBUG
    if( (cont & PAD_BUTTON_R) == 0 ){
      if( state == PLAYER_MOVE_STATE_END ){
        if( FIELD_ENCOUNT_CheckEncount(FIELDMAP_GetEncount(fieldWork)) == TRUE ){
          return EVENT_Battle( gsys, fieldWork );
        }
      }
    }
    #else
    if( state == PLAYER_MOVE_STATE_END ){
			if(!GFL_NET_IsInit())
			{
				if( FIELD_ENCOUNT_CheckEncount(FIELDMAP_GetEncount(fieldWork)) == TRUE ){
					return EVENT_Battle( gsys, fieldWork );
				}
			}
    }
    #endif
  }
  

  if( state == PLAYER_MOVE_STATE_END )
  { //座標接続チェック
    event = fldmap_Event_CheckConnect(gsys, fieldWork, FIELDMAP_GetNowPos(fieldWork));
    if( event != NULL ){
      return event;
    }
  }

  //看板イベントチェック
  if( isGridMap ){
    if( state == PLAYER_MOVE_STATE_END ){
      u16 id;
      VecFx32 pos;
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
      EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
      FLDMMDL *fmmdl = FIELD_PLAYER_GetFldMMdl( FIELDMAP_GetFieldPlayer(fieldWork) );
      u16 dir = FLDMMDL_GetDirDisp( fmmdl );
    
      FIELD_PLAYER_GetPos( FIELDMAP_GetFieldPlayer(fieldWork), &pos );
      FLDMMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
    
      {
        //OBJ看板チェック
      }
      
      id = EVENTDATA_CheckTalkBoardEvent( evdata, evwork, &pos, dir );
      
      if( id != EVENTDATA_ID_NONE ){
        SCRIPT_FLDPARAM fparam;
        fparam.msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
        event = SCRIPT_SetScript(
           gsys, id, NULL, FIELDMAP_GetHeapID(fieldWork), &fparam );
      }
    }
  }
  

  //座標イベントチェック
  {
    VecFx32 pos;
    u16 id,param;
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
    EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
    FIELD_PLAYER_GetPos( FIELDMAP_GetFieldPlayer(fieldWork), &pos );
    
    id = EVENTDATA_CheckPosEvent( evdata, evwork, &pos );
    
    if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
      SCRIPT_FLDPARAM fparam;
      fparam.msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
      event = SCRIPT_SetScript(
         gsys, id, NULL, FIELDMAP_GetHeapID(fieldWork), &fparam );
      return event;
    }
  }

	
  //レールの場合のマップ遷移チェック
  event = checkRailExit(gsys, fieldWork);
  if( event != NULL ){
    return event;
  }

	//キー入力接続チェック
	event = fldmap_Event_CheckPushConnect(gsys, fieldWork);
	if( event != NULL ){
		return event;
	}
	
	//メニュー起動チェック
	if( trg == PAD_BUTTON_X ){
		if(WIPE_SYS_EndCheck()){
  			return EVENT_FieldMapMenu( gsys, fieldWork, FIELDMAP_GetHeapID(fieldWork) );
		}
	}

	//サブスクリーンからのイベント起動チェック
	event = fldmap_Event_Check_SubScreen(gsys, fieldWork);
	if( event != NULL )
  {
		return event;
	}
  
	///通信用会話処理(仮
  {
    FIELD_COMM_MAIN * commSys = FIELDMAP_GetCommSys(fieldWork);
    //話しかける側
    if( trg == PAD_BUTTON_A ){
      if( FIELD_COMM_MAIN_CanTalk( commSys ) == TRUE ){
        return FIELD_COMM_EVENT_StartTalk(
            gsys , fieldWork , commSys );
      }
    }

    //話しかけられる側(中で一緒に話せる状態かのチェックもしてしまう
    if( FIELD_COMM_MAIN_CheckReserveTalk( commSys ) == TRUE ){
      return FIELD_COMM_EVENT_StartTalkPartner(
          gsys , fieldWork , commSys );
    }
  }
	
	//フィールド話し掛けチェック
	if(	isGridMap )
	{
		if( trg == PAD_BUTTON_A )
		{
      { //OBJ話し掛け
			  int gx,gy,gz;
			  FLDMMDL *fmmdl_talk;
			  FIELD_PLAYER_GetFrontGridPos(
            FIELDMAP_GetFieldPlayer(fieldWork), &gx, &gy, &gz );
	  		fmmdl_talk = FLDMMDLSYS_SearchGridPos(
	  				FIELDMAP_GetFldMMdlSys(fieldWork), gx, gz, FALSE );

		  	if( fmmdl_talk != NULL )
	  		{
	  			u32 scr_id = FLDMMDL_GetEventID( fmmdl_talk );
  				FLDMMDL *fmmdl_player = FIELD_PLAYER_GetFldMMdl(
  						FIELDMAP_GetFieldPlayer(fieldWork) );
          FIELD_PLAYER_GRID_ForceStop( FIELDMAP_GetFieldPlayer(fieldWork) );
  				return EVENT_FieldTalk( gsys, fieldWork,
  					scr_id, fmmdl_player, fmmdl_talk, FIELDMAP_GetHeapID(fieldWork) );
  			}
	  	}
      
      { //BG話し掛け
        u16 id;
        VecFx32 pos;
        GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
        EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
        EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
        FLDMMDL *fmmdl = FIELD_PLAYER_GetFldMMdl( FIELDMAP_GetFieldPlayer(fieldWork) );
        u16 dir = FLDMMDL_GetDirDisp( fmmdl );
        
        FIELD_PLAYER_GetPos( FIELDMAP_GetFieldPlayer(fieldWork), &pos );
        FLDMMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
        id = EVENTDATA_CheckTalkBGEvent( evdata, evwork, &pos, dir );
        
        if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
          SCRIPT_FLDPARAM fparam;
          fparam.msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
          event = SCRIPT_SetScript(
             gsys, id, NULL, FIELDMAP_GetHeapID(fieldWork), &fparam );
          return event;
        }
      }
    }
	}
	
	//デバッグ：パレスで木に触れたらワープ
  {
    GMEVENT *ev;
    FIELD_COMM_MAIN * commSys = FIELDMAP_GetCommSys(fieldWork);
    ev =  DEBUG_PalaceTreeMapWarp(fieldWork, gsys, FIELDMAP_GetFieldPlayer(fieldWork), commSys);
    if(ev != NULL){
      return ev;
    }
	}
	return NULL;
}

//--------------------------------------------------------------
/**
 * イベント起動チェック　決定ボタン入力時に発生するイベントチェック
 * @param fieldWork FIELDMAP_WORK
 * @param event 発生イベントプロセス格納先
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval BOOL TRUE=イベント起動
 */
//--------------------------------------------------------------
static BOOL event_CheckEventDecideButton( FIELDMAP_WORK *fieldWork,
    GMEVENT **event, const int key_trg, const int key_cont )
{
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント起動チェック　自機動作終了時に発生するイベントチェック。
 * 移動終了、振り向き、壁衝突動作などで発生。
 * @param fieldWork FIELDMAP_WORK
 * @param event 発生イベントプロセス格納先
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval BOOL TRUE=イベント起動
 */
//--------------------------------------------------------------
static BOOL event_CheckEventMoveEnd( FIELDMAP_WORK *fieldWork,
    GMEVENT **event, const int key_trg, const int key_cont )
{
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント起動チェック　一歩移動終了時に発生するイベントチェック
 * @param fieldWork FIELDMAP_WORK
 * @param event 発生イベントプロセス格納先
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval BOOL TRUE=イベント起動
 */
//--------------------------------------------------------------
static BOOL event_CheckEventOneStepMoveEnd( FIELDMAP_WORK *fieldWork,
    GMEVENT **event, const int key_trg, const int key_cont )
{
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント起動チェック　十字キー入力中に発生するイベントチェック
  * @param fieldWork FIELDMAP_WORK
 * @param event 発生イベントプロセス格納先
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval BOOL TRUE=イベント起動
 */
//--------------------------------------------------------------
static BOOL event_CheckEventPushKey( FIELDMAP_WORK *fieldWork,
    GMEVENT **event, const int key_trg, const int key_cont )
{
  return FALSE;
}


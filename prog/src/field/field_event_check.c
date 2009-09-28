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
#include "field_comm/field_comm_event.h"   //FIELD_COMM_EVENT_～
#include "field_comm/intrude_field.h"
#include "event_debug_menu.h"     //DEBUG_EVENT_DebugMenu
#include "event_battle.h"         //EVENT_Battle
#include "event_fieldtalk.h"      //EVENT_FieldTalk
#include "event_fieldmap_menu.h"  //EVENT_FieldMapMenu
#include "rail_editor.h"
#include "script.h"     //SCRIPT_SetEventScript
#include "net_app/union/union_event_check.h"

#include "system/main.h"    //HEAPID_FIELDMAP
#include "isdbglib.h"

#include "field_encount.h"      //FIELD_ENCOUNT_CheckEncount

#include "fieldmap_ctrl_grid.h"
#include "fieldmap_ctrl_nogrid_work.h"
#include "field_player_grid.h"

#include "map_attr.h"

#include "event_trainer_eye.h"
#include "fieldmap/zone_id.h"
#include "net_app/union/union_main.h"
#include "event_comm_error.h"

#include "../../../resource/fldmapdata/script/bg_attr_def.h"  //SCRID_BG_MSG_～

#include "field_gimmick.h"   //for FLDGMK_GimmickCodeCheck
#include "field_gimmick_def.h"  //for FLD_GIMMICK_GYM_ELEC
#include "gym_elec.h"     //for GYM_ELEC_CreateMoveEvt

//======================================================================
//======================================================================

extern u16 EVENTDATA_CheckPosEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const VecFx32 *pos );

extern u16 EVENTDATA_CheckTalkBGEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const VecFx32 *pos, u16 talk_dir );

//======================================================================
//======================================================================
#define FRONT_TALKOBJ_HEIGHT_DIFF (FX32_CONST(16)) //話しかけ可能なOBJのY値差分

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  HEAPID heapID;              ///<ヒープID
  GAMESYS_WORK * gsys;        ///<ゲームシステムへのポインタ
  GAMEDATA * gamedata;        ///<ゲームデータへのポインタ
  EVENTDATA_SYSTEM *evdata;   ///<参照しているイベントデータ
  BOOL isGridMap;             ///<グリッドマップかどうか？のフラグ
  u16 map_id;                 ///<現在のマップID
  //↑変化がないパラメータ　

  //↓毎回変化する可能性があるパラメータ
  FIELD_PLAYER * field_player;
	PLAYER_MOVE_STATE player_state;
  PLAYER_MOVE_VALUE player_value;
  u16 player_dir;                   ///<自機の向き
  int key_trg;                ///<キー情報（トリガー）
  int key_cont;               ///<キー情報（）
  const VecFx32 * now_pos;        ///<現在のマップ位置
  u32 mapattr;                  ///<足元のアトリビュート情報

  BOOL talkRequest; ///<話しかけキー操作があったかどうか
  BOOL menuRequest; ///<メニューオープン操作があったかどうか
  BOOL moveRequest; ///<一歩移動終了タイミングかどうか
  BOOL stepRequest; ///<振り向きor一歩移動終了タイミングかどうか
  BOOL pushRequest; ///<押し込み操作があったかどうか
  BOOL convRequest; ///<便利ボタン操作があったかどうか

  BOOL debugRequest;  ///<デバッグ操作があったかどうか
}EV_REQUEST;

//======================================================================
//======================================================================
//event
static GMEVENT * checkExit(EV_REQUEST * req,
    FIELDMAP_WORK *fieldWork, const VecFx32 *now_pos );
static GMEVENT * checkPushExit(EV_REQUEST * req,
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static GMEVENT * checkRailExit(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork);
static GMEVENT * checkSubScreenEvent(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );

static void setupRequest(EV_REQUEST * req, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork);

static void setFrontPos(const EV_REQUEST * req, VecFx32 * pos);
static BOOL checkDirection(u16 player_dir, u16 exit_dir);
static int getConnectID(const EV_REQUEST * req, const VecFx32 * now_pos);
static void rememberExitInfo(EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx, const VecFx32 * pos);
static GMEVENT * getChangeMapEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx);
static GMEVENT * DEBUG_checkKeyEvent(EV_REQUEST * req, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork);

static GMEVENT * checkEvent_ConvenienceButton( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );

static GMEVENT * checkEvent_PlayerNaminoriStart( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );

static GMEVENT * checkEvent_PlayerNaminoriEnd( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );

static MMDL * getFrontTalkOBJ( EV_REQUEST *req, FIELDMAP_WORK *fieldMap );

static MMDL * getRailFrontTalkOBJ( EV_REQUEST *req, FIELDMAP_WORK *fieldMap );

static u16 checkTalkAttrEvent( EV_REQUEST *req, FIELDMAP_WORK *fieldMap);

//======================================================================
//
//
//    イベント起動チェック本体
//
//
//======================================================================

//--------------------------------------------------------------
/**
 * イベント　イベント起動チェック
 * @param	gsys GAMESYS_WORK
 * @param work FIELDMAP_WORK
 * @retval GMEVENT NULL=イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * FIELD_EVENT_CheckNormal( GAMESYS_WORK *gsys, void *work )
{
	
  EV_REQUEST req;
	GMEVENT *event;
	FIELDMAP_WORK *fieldWork = work;
  
  setupRequest( &req, gsys, fieldWork );

  
  //デバッグ用チェック
#ifdef  PM_DEBUG
  event = DEBUG_checkKeyEvent( &req, gsys, fieldWork );
  if (event != NULL) {
    return event;
  }
#endif //debug
	
//☆☆☆特殊スクリプト起動チェックがここに入る
  event = SCRIPT_SearchSceneScript( gsys, req.heapID );
  if (event) return event;


//☆☆☆トレーナー視線チェックがここに入る
  if( !(req.debugRequest) ){
    event = EVENT_CheckTrainerEye( fieldWork, FALSE );
    if( event != NULL ){
      return event;
    }
  }

//☆☆☆一歩移動チェックがここから
  //座標イベントチェック
  if( req.moveRequest )
  {
    VecFx32 pos;
    u16 id;
    EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
    FIELD_PLAYER_GetPos( req.field_player, &pos );
    
    id = EVENTDATA_CheckPosEvent( req.evdata, evwork, &pos );
    
    if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
      event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
      return event;
    }

   //座標接続チェック
    event = checkExit(&req, fieldWork, req.now_pos);
    if( event != NULL ){
      return event;
    }
  }


	
  //レールの場合のマップ遷移チェック
  if (FIELDMAP_GetMapControlType(fieldWork) == FLDMAP_CTRLTYPE_NOGRID)
  {
    event = checkRailExit(&req, gsys, fieldWork);
    if( event != NULL ){
      return event;
    }
  }
//☆☆☆ステップチェック（一歩移動or振り向き）がここから
  //戦闘移行チェック
  if( req.isGridMap ){
    FIELD_ENCOUNT * encount = FIELDMAP_GetEncount(fieldWork);
    #ifdef PM_DEBUG
    if( !(req.debugRequest) ){
      if( req.stepRequest ){
        if( FIELD_ENCOUNT_CheckEncount(encount) == TRUE ){
          return EVENT_Battle( gsys, fieldWork );
        }
      }
    }
    #else
    if( req.stepRequest ){
			if(!GFL_NET_IsInit())
			{
				if( FIELD_ENCOUNT_CheckEncount(encount) == TRUE ){
					return EVENT_Battle( gsys, fieldWork );
				}
			}
    }
    #endif
  }
  

  //看板イベントチェック
  if( req.isGridMap ){
    if( req.stepRequest ){
      u16 id;
      VecFx32 pos;
      EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
      MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req.field_player );
      u16 dir = MMDL_GetDirDisp( fmmdl );
    
      FIELD_PLAYER_GetPos( req.field_player, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
    
      {
        //OBJ看板チェック
      }
      
      id = EVENTDATA_CheckTalkBoardEvent( req.evdata, evwork, &pos, dir );
      
      if( id != EVENTDATA_ID_NONE ){
        event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
      }
    }
  }
  
//☆☆☆自機状態イベントチェックがここから
    /* 今はない */

//☆☆☆会話チェック

	///通信用会話処理(仮
#if 0 //※check　侵入通信変更の為、暫定Fix
  {
    FIELD_COMM_MAIN * commSys = FIELDMAP_GetCommSys(fieldWork);
    
    if(commSys != NULL){
      //話しかける側
      if( req.talkRequest ){
        if( FIELD_COMM_MAIN_CanTalk( commSys ) == TRUE ){
          return FIELD_COMM_EVENT_StartTalk( gsys , fieldWork , commSys );
        }
      }

      //話しかけられる側(中で一緒に話せる状態かのチェックもしてしまう
      if( FIELD_COMM_MAIN_CheckReserveTalk( commSys ) == TRUE ){
        return FIELD_COMM_EVENT_StartTalkPartner( gsys , fieldWork , commSys );
      }
    }
  }
#endif

	//フィールド話し掛けチェック
	if(	req.isGridMap )
	{
		if( req.talkRequest )
		{
      { //OBJ話し掛け
        MMDL *fmmdl_talk = getFrontTalkOBJ( &req, fieldWork );
		  	if( fmmdl_talk != NULL )
	  		{
	  			u32 scr_id = MMDL_GetEventID( fmmdl_talk );
  				MMDL *fmmdl_player = FIELD_PLAYER_GetMMdl( req.field_player );
          FIELD_PLAYER_GRID_ForceStop( req.field_player );
  				return EVENT_FieldTalk( gsys, fieldWork,
  					scr_id, fmmdl_player, fmmdl_talk, req.heapID );
  			}
	  	}
      
      { //BG話し掛け
        u16 id;
        VecFx32 pos;
        EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
        MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req.field_player );
        u16 dir = MMDL_GetDirDisp( fmmdl );
        
        FIELD_PLAYER_GetPos( req.field_player, &pos );
        MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
        id = EVENTDATA_CheckTalkBGEvent( req.evdata, evwork, &pos, dir );
        
        if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
          event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
          return event;
        }
      }

      { //BG Attribute 話しかけ
        u16 id = checkTalkAttrEvent( &req, fieldWork );
        if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
          event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
          return event;
        }

      }
    }
  }


  { //波乗りテスト

    if( req.player_state == PLAYER_MOVE_STATE_END ||
        req.player_state == PLAYER_MOVE_STATE_OFF )
    {
#if 0
      if( req.key_trg & PAD_BUTTON_SELECT ){
        event = checkEvent_PlayerNaminoriStart( &req, gsys, fieldWork );
        if( event != NULL ){
          return event;
       }
      }
#endif         
      #ifdef PM_DEBUG
      if( !(req.debugRequest) ){
        event = checkEvent_PlayerNaminoriEnd( &req, gsys, fieldWork );
        if( event != NULL ){
         return event;
       }
      }
      #else
      event = checkEvent_PlayerNaminoriEnd( &req, gsys, fieldWork );
      if( event != NULL ){
        return event;
      }
      #endif
    }
  }
  
//☆☆☆押し込み操作チェック（マットでのマップ遷移など）
	//キー入力接続チェック
  if (req.pushRequest) {
    event = checkPushExit(&req, gsys, fieldWork);
    if( event != NULL ){
      return event;
    }
  }
  
//☆☆☆自機位置に関係ないキー入力イベントチェック
  //便利ボタンチェック
  if( req.convRequest ){
    if( req.isGridMap ){
      event = checkEvent_ConvenienceButton( &req, gsys, fieldWork );
    
      if( event != NULL ){
        return event;
      }
    }
  }
  
  //通信エラー画面呼び出しチェック(※メニュー起動チェックの真上に配置する事！)
  if( GAMESYSTEM_GetFieldCommErrorReq(gsys) == TRUE ){
		if(WIPE_SYS_EndCheck()){
      return EVENT_FieldCommErrorProc(gsys, fieldWork);
    }
  }
  
	//メニュー起動チェック
	if( req.menuRequest ){
		if(WIPE_SYS_EndCheck()){
  			return EVENT_FieldMapMenu( gsys, fieldWork, req.heapID );
		}
	}

	//サブスクリーンからのイベント起動チェック
	event = checkSubScreenEvent(gsys, fieldWork);
	if( event != NULL )
  {
		return event;
	}
  
	
	//デバッグ：パレスで木に触れたらワープ
  {
    GMEVENT *ev;
    INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(GAMESYSTEM_GetGameCommSysPtr(gsys));
    ev =  DEBUG_IntrudeTreeMapWarp(fieldWork, gsys, req.field_player, intcomm);
    if(ev != NULL){
      return ev;
    }
	}
	return NULL;
}


//--------------------------------------------------------------
/**
 * イベント起動チェック と, イベント生成時の処理
 * @param	gsys GAMESYS_WORK
 * @param work FIELDMAP_WORK
 * @retval GMEVENT NULL=イベント無し
 */
//--------------------------------------------------------------
GMEVENT * FIELD_EVENT_CheckNormal_Wrap( GAMESYS_WORK *gsys, void *work )
{
	GMEVENT *event;
	FIELDMAP_WORK *fieldmap_work;
	FIELD_PLACE_NAME *place_name_sys;

	// イベント起動チェック
	event = FIELD_EVENT_CheckNormal( gsys, work );

	// イベント生成時の処理
	if( event != NULL )
	{
		// 地名表示ウィンドウを消去
		fieldmap_work   = (FIELDMAP_WORK*)GAMESYSTEM_GetFieldMapWork( gsys );
		place_name_sys  = FIELDMAP_GetPlaceNameSys( fieldmap_work );
		FIELD_PLACE_NAME_Hide( place_name_sys );
	}

	return event;
}

//==================================================================
/**
 * イベント起動チェック：ユニオンorコロシアム
 *
 * @param   gsys		
 * @param   work		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * FIELD_EVENT_CheckUnion( GAMESYS_WORK *gsys, void *work )
{
  EV_REQUEST req;
	GMEVENT *event;
	FIELDMAP_WORK *fieldWork = work;
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  UNION_SYSTEM_PTR unisys = GameCommSys_GetAppWork(game_comm);
  
  setupRequest( &req, gsys, fieldWork );

  if(unisys == NULL){
    //ユニオン終了
    VecFx32 pos;
    pos.x = 72 << FX32_SHIFT;
    pos.y = 48;
    pos.z = 88 << FX32_SHIFT;
    return DEBUG_EVENT_ChangeMapPos(gsys, fieldWork, ZONE_ID_T02PC0101, &pos, 0);
  }
  
  if(UnionMain_GetFinishReq(unisys) == TRUE){
    if(UnionMain_GetFinishExe(unisys) == FALSE){
      //ユニオン終了処理開始
      GameCommSys_ExitReq(game_comm);
    }
    return NULL;
  }
  
  //デバッグ用チェック
#ifdef  PM_DEBUG
  event = DEBUG_checkKeyEvent( &req, gsys, fieldWork );
  if (event != NULL) {
    return event;
  }
#endif //debug
  
//☆☆☆一歩移動チェックがここから
  //座標イベントチェック
  if( req.moveRequest )
  {
    VecFx32 pos;
    u16 id;
    EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
    FIELD_PLAYER_GetPos( req.field_player, &pos );
    
    id = EVENTDATA_CheckPosEvent( req.evdata, evwork, &pos );
    
    if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
      event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
      return event;
    }

#if 0
   //座標接続チェック
    event = checkExit(&req, fieldWork, req.now_pos);
    if( event != NULL ){
      return event;
    }
#endif
  }

//☆☆☆自機状態イベントチェックがここから
    /* 今はない */

//☆☆☆会話チェック

	///通信用会話処理(仮
  {
    //通信イベント発生チェック
    event = UnionEvent_CommCheck( gsys, fieldWork );
    if(event != NULL){
      return event;
    }
    
    //通信話しかけイベント発生チェック
    if(req.talkRequest){
      event = UnionEvent_TalkCheck( gsys, fieldWork );
      if(event != NULL){
        return event;
      }
    }
  }

  //通信エラー画面呼び出しチェック(※メニュー起動チェックの真上に配置する事！)
  if( GAMESYSTEM_GetFieldCommErrorReq(gsys) == TRUE ){
		if(WIPE_SYS_EndCheck()){
      return EVENT_FieldCommErrorProc(gsys, fieldWork);
    }
  }

	//メニュー起動チェック
	if( req.menuRequest ){
		if(WIPE_SYS_EndCheck()){
  			return EVENT_UnionMapMenu( gsys, fieldWork, req.heapID );
		}
	}

	return NULL;
}


//----------------------------------------------------------------------------
/**
 * イベント起動チェック：ノーグリッドマップ
 *
 *	@param	gsys
 *	@param	work 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
GMEVENT * FIELD_EVENT_CheckNoGrid( GAMESYS_WORK *gsys, void *work )
{
  EV_REQUEST req;
	GMEVENT *event;
	FIELDMAP_WORK *fieldWork = work;
  
  setupRequest( &req, gsys, fieldWork );

  
  //デバッグ用チェック
#ifdef  PM_DEBUG
  event = DEBUG_checkKeyEvent( &req, gsys, fieldWork );
  if (event != NULL) {
    return event;
  }
#endif //debug
	
//☆☆☆特殊スクリプト起動チェックがここに入る
    /* 今はない */

//☆☆☆トレーナー視線チェックがここに入る
  if( !(req.debugRequest) ){
/*
    event = EVENT_CheckTrainerEye( fieldWork, FALSE );
    if( event != NULL ){
      return event;
    }
//*/
  }

//☆☆☆一歩移動チェックがここから
  //座標イベントチェック
  if( req.moveRequest )
  {
    /*
    VecFx32 pos;
    u16 id;
    EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
    FIELD_PLAYER_GetPos( req.field_player, &pos );
    
    id = EVENTDATA_CheckPosEvent( req.evdata, evwork, &pos );
    
    if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
      event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
      return event;
    }
    //*/

    //座標接続チェック
    event = checkRailExit(&req, gsys, fieldWork);
    if( event != NULL ){
      return event;
    }
  }


	
//☆☆☆ステップチェック（一歩移動or振り向き）がここから
  //戦闘移行チェック
  {
    FIELD_ENCOUNT * encount = FIELDMAP_GetEncount(fieldWork);
    #ifdef PM_DEBUG
    if( !(req.debugRequest) ){
      if( req.stepRequest ){
        if( FIELD_ENCOUNT_CheckEncount(encount) == TRUE ){
          return EVENT_Battle( gsys, fieldWork );
        }
      }
    }
    #else
    if( req.stepRequest ){
			if(!GFL_NET_IsInit())
			{
				if( FIELD_ENCOUNT_CheckEncount(encount) == TRUE ){
					return EVENT_Battle( gsys, fieldWork );
				}
			}
    }
    #endif
  }
  

  //看板イベントチェック
  if( req.stepRequest ){
    /*
    u16 id;
    VecFx32 pos;
    EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req.field_player );
    u16 dir = MMDL_GetDirDisp( fmmdl );
  
    FIELD_PLAYER_GetPos( req.field_player, &pos );
    MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  
    {
      //OBJ看板チェック
    }
    
    id = EVENTDATA_CheckTalkBoardEvent( req.evdata, evwork, &pos, dir );
    
    if( id != EVENTDATA_ID_NONE ){
      event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
    }
    //*/
  }
  
//☆☆☆自機状態イベントチェックがここから
    /* 今はない */

//☆☆☆会話チェック

	///通信用会話処理(仮
#if 0 //※check　侵入通信変更の為、暫定Fix
  {
    FIELD_COMM_MAIN * commSys = FIELDMAP_GetCommSys(fieldWork);
    
    if(commSys != NULL){
      //話しかける側
      if( req.talkRequest ){
        if( FIELD_COMM_MAIN_CanTalk( commSys ) == TRUE ){
          return FIELD_COMM_EVENT_StartTalk( gsys , fieldWork , commSys );
        }
      }

      //話しかけられる側(中で一緒に話せる状態かのチェックもしてしまう
      if( FIELD_COMM_MAIN_CheckReserveTalk( commSys ) == TRUE ){
        return FIELD_COMM_EVENT_StartTalkPartner( gsys , fieldWork , commSys );
      }
    }
  }
#endif

	//フィールド話し掛けチェック
  if( req.talkRequest )
  {
    { //OBJ話し掛け
      MMDL *fmmdl_talk = getRailFrontTalkOBJ( &req, fieldWork );
      if( fmmdl_talk != NULL )
      {
//        u32 scr_id = MMDL_GetEventID( fmmdl_talk );
//        MMDL *fmmdl_player = FIELD_PLAYER_GetMMdl( req.field_player );
//        FIELD_PLAYER_GRID_ForceStop( req.field_player );
//        return EVENT_FieldTalk( gsys, fieldWork,
//          scr_id, fmmdl_player, fmmdl_talk, req.heapID );

        if(WIPE_SYS_EndCheck()){
            return EVENT_FieldMapMenu( gsys, fieldWork, req.heapID );
        }
      }
    }
    
    /*
    { //BG話し掛け
      u16 id;
      VecFx32 pos;
      EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
      MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req.field_player );
      u16 dir = MMDL_GetDirDisp( fmmdl );
      
      FIELD_PLAYER_GetPos( req.field_player, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      id = EVENTDATA_CheckTalkBGEvent( req.evdata, evwork, &pos, dir );
      
      if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
        event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
        return event;
      }
    }

    { //BG Attribute 話しかけ
      u16 id = checkTalkAttrEvent( &req, fieldWork );
      if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
        event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
        return event;
      }

    }
    //*/
  }


#if 0
  { //波乗りテスト

    if( req.player_state == PLAYER_MOVE_STATE_END ||
        req.player_state == PLAYER_MOVE_STATE_OFF )
    {
#if 0
      if( req.key_trg & PAD_BUTTON_SELECT ){
        event = checkEvent_PlayerNaminoriStart( &req, gsys, fieldWork );
        if( event != NULL ){
          return event;
       }
      }
#endif         
      #ifdef PM_DEBUG
      if( !(req.debugRequest) ){
        event = checkEvent_PlayerNaminoriEnd( &req, gsys, fieldWork );
        if( event != NULL ){
         return event;
       }
      }
      #else
      event = checkEvent_PlayerNaminoriEnd( &req, gsys, fieldWork );
      if( event != NULL ){
        return event;
      }
      #endif
    }
  }
#endif
  
//☆☆☆押し込み操作チェック（マットでのマップ遷移など）
	//キー入力接続チェック
  if (req.pushRequest) {
    event = checkRailExit(&req, gsys, fieldWork);
    if( event != NULL ){
      return event;
    }
  }
  
//☆☆☆自機位置に関係ないキー入力イベントチェック
  //便利ボタンチェック
  if( req.convRequest ){
    event = checkEvent_ConvenienceButton( &req, gsys, fieldWork );
    if( event != NULL ){
      return event;
    }
  }
  
  //通信エラー画面呼び出しチェック(※メニュー起動チェックの真上に配置する事！)
  if( GAMESYSTEM_GetFieldCommErrorReq(gsys) == TRUE ){
		if(WIPE_SYS_EndCheck()){
      return EVENT_FieldCommErrorProc(gsys, fieldWork);
    }
  }

	//メニュー起動チェック
	if( req.menuRequest ){
		if(WIPE_SYS_EndCheck()){
  			return EVENT_FieldMapMenu( gsys, fieldWork, req.heapID );
		}
	}

	//サブスクリーンからのイベント起動チェック
	event = checkSubScreenEvent(gsys, fieldWork);
	if( event != NULL )
  {
		return event;
	}
  
	
	//デバッグ：パレスで木に触れたらワープ
  {
    GMEVENT *ev;
    INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(GAMESYSTEM_GetGameCommSysPtr(gsys));
    ev =  DEBUG_IntrudeTreeMapWarp(fieldWork, gsys, req.field_player, intcomm);
    if(ev != NULL){
      return ev;
    }
	}
	return NULL;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 判定用パラメータの生成
 *
 *
 * 現状チェックごとに生成しているが実はほとんどの値は初期化時に一度だけでもいいのかも。
 */
//--------------------------------------------------------------
static void setupRequest(EV_REQUEST * req, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork)
{
  req->heapID = FIELDMAP_GetHeapID(fieldWork);
  req->gsys = gsys;
  req->gamedata = GAMESYSTEM_GetGameData(gsys);
	req->evdata = GAMEDATA_GetEventData(req->gamedata);
  req->map_id = FIELDMAP_GetZoneID(fieldWork);
  { //グリッドマップかどうかの判定フラグセット
    const DEPEND_FUNCTIONS *func_tbl = FIELDMAP_GetDependFunctions(fieldWork);
    req->isGridMap = ( func_tbl->type == FLDMAP_CTRLTYPE_GRID );
  }

  req->key_trg = GFL_UI_KEY_GetTrg();
  req->key_cont = GFL_UI_KEY_GetCont();

  req->field_player = FIELDMAP_GetFieldPlayer( fieldWork );
  //自機動作ステータス更新
  FIELD_PLAYER_UpdateMoveStatus( req->field_player );
  //自機の動作状態を取得
  req->player_value = FIELD_PLAYER_GetMoveValue( req->field_player );
  req->player_state = FIELD_PLAYER_GetMoveState( req->field_player );
  {
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req->field_player );
    req->player_dir = MMDL_GetDirDisp( fmmdl );
  }

#if 0
  req->now_pos = FIELDMAP_GetNowPos( fieldWork );
#else
  {
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( req->field_player );
    req->now_pos = MMDL_GetVectorPosAddress( mmdl );
  }
#endif
  
  {
		FLDMAPPER *g3Dmapper = FIELDMAP_GetFieldG3Dmapper(fieldWork);
    req->mapattr = MAPATTR_GetAttribute(g3Dmapper, req->now_pos);
  }
  req->talkRequest = ((req->key_trg & PAD_BUTTON_A) != 0);

  req->menuRequest = ((req->key_trg & PAD_BUTTON_X) != 0);

  //本当はvalueも見るが、今は暫定
  req->moveRequest = ((req->player_state == PLAYER_MOVE_STATE_END));

  req->stepRequest = ((req->player_state == PLAYER_MOVE_STATE_END));

  if ( ( (req->player_dir == DIR_UP) && (req->key_cont & PAD_KEY_UP) )
      || ( (req->player_dir == DIR_DOWN) && (req->key_cont & PAD_KEY_DOWN) )
      || ( (req->player_dir == DIR_LEFT) && (req->key_cont & PAD_KEY_LEFT) )
      || ( (req->player_dir == DIR_RIGHT) && (req->key_cont & PAD_KEY_RIGHT) )
     )
  {
    req->pushRequest = TRUE;
  }
  else 
  {
    req->pushRequest = FALSE;
  }

  if( req->key_trg & PAD_BUTTON_Y ){
    req->convRequest = TRUE;
  }else{
    req->convRequest = FALSE;
  }

  req->debugRequest = ( (req->key_cont & PAD_BUTTON_R) != 0);
  if (req->debugRequest)
  {
    req->talkRequest = FALSE;
    //req->menuRequest = FALSE;
    req->stepRequest = FALSE;
    req->moveRequest = FALSE;
    req->pushRequest = FALSE;
    req->convRequest = FALSE;
  }
    
}

//======================================================================
//
//
//    イベント起動チェック：デバッグ関連
//
//
//======================================================================
#ifdef  PM_DEBUG
//--------------------------------------------------------------
/**
 * @brief デバッグ用イベント起動チェック（主にキー入力による）
 */
//--------------------------------------------------------------
static GMEVENT * DEBUG_checkKeyEvent(EV_REQUEST * req, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork)
{
	enum{
		resetCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START,
		chgCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_SELECT,

		railCont = PAD_BUTTON_R | PAD_BUTTON_B | PAD_BUTTON_A,
	};

	//ソフトリセットチェック
	if( (req->key_cont & resetCont) == resetCont ){
		return DEBUG_EVENT_GameEnd(gsys, fieldWork);
	}
	
	//マップ変更チェック
	if( (req->key_cont & chgCont) == chgCont ){
    ISDPrintSetBlockingMode(1);
    GFL_HEAP_DEBUG_PrintSystemInfo();
    GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_PROC );
    GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_FIELDMAP );
    ISDPrintSetBlockingMode(0);
		//return DEBUG_EVENT_ChangeToNextMap(gsys, fieldWork);
	}
	
	//デバッグメニュー起動チェック
  if( req->menuRequest && req->debugRequest )
  {
		return DEBUG_EVENT_DebugMenu(gsys, fieldWork, 
				req->heapID, ZONEDATA_GetMapRscID(req->map_id));
	}

#if defined(DEBUG_ONLY_FOR_tomoya_takahashi) | defined(DEBUG_ONLY_FOR_nakatsui)
	// レールエディタ起動
	if( (req->key_cont & railCont) == railCont )
	{
		return DEBUG_EVENT_RailEditor( gsys, fieldWork );
	}
#endif
	
  return NULL;
}
#endif

//======================================================================
//
//
//    イベント起動チェック：マップ遷移関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * イベント マップ接続チェック
 *
 * @param req   イベントチェック用ワーク
 * @param fieldWork FIELDMAP_WORK
 * @param now_pos チェックする座標
 * @return GMEVENT  発生したイベントデータへのポインタ。 NULLの場合、接続なし
 */
//--------------------------------------------------------------
static GMEVENT * checkExit(EV_REQUEST * req,
    FIELDMAP_WORK *fieldWork, const VecFx32 *now_pos )
{
  int idx = getConnectID(req, now_pos);
  if (idx == EXIT_ID_NONE) {
    return NULL;
  }
  {
    const CONNECT_DATA * cnct;
    cnct = EVENTDATA_GetConnectByID(req->evdata, idx);
    if (CONNECTDATA_GetExitType(cnct) != EXIT_TYPE_NONE)
    {
      return NULL;
    }
  }

	//マップ遷移発生の場合、出入口を記憶しておく
  rememberExitInfo(req, fieldWork, idx, now_pos);
  return getChangeMapEvent(req, fieldWork, idx);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * getChangeMapEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx)
{
	const CONNECT_DATA * cnct;
  cnct = EVENTDATA_GetConnectByID(req->evdata, idx);

  return EVENT_ChangeMapByConnect(req->gsys, fieldWork, cnct);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void rememberExitInfo(EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx, const VecFx32 * pos)
{
	//マップ遷移発生の場合、出入口を記憶しておく
  LOCATION ent_loc;
  LOCATION_Set( &ent_loc, FIELDMAP_GetZoneID(fieldWork), idx, 0,
      pos->x, pos->y, pos->z);
  GAMEDATA_SetEntranceLocation(req->gamedata, &ent_loc);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static int getConnectID(const EV_REQUEST * req, const VecFx32 * now_pos)
{
	const CONNECT_DATA * cnct;
	int idx = EVENTDATA_SearchConnectIDByPos(req->evdata, now_pos);
	
	if( idx == EXIT_ID_NONE ){
		return EXIT_ID_NONE;
	}
	cnct = EVENTDATA_GetConnectByID(req->evdata, idx);
  if (CONNECTDATA_GetExitType(cnct) != EXIT_TYPE_NONE)
  {
    EXIT_DIR dir = CONNECTDATA_GetExitDir(cnct);
    if (checkDirection(req->player_dir, dir) == FALSE)
    {
      return EXIT_ID_NONE;
    }
  }
  return idx;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void setFrontPos(const EV_REQUEST * req, VecFx32 * pos)
{
  enum { GRIDSIZE = 16 * FX32_ONE };
	*pos = *req->now_pos;
	
	switch( req->player_dir) {
	case DIR_UP:		pos->z -= GRIDSIZE; break;
	case DIR_DOWN:	pos->z += GRIDSIZE; break;
	case DIR_LEFT:	pos->x -= GRIDSIZE; break;
	case DIR_RIGHT:	pos->x += GRIDSIZE; break;
	default:
                  GF_ASSERT(0);
	}
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL checkDirection(u16 player_dir, u16 exit_dir)
{
  if (
      (player_dir == DIR_UP && exit_dir == EXIT_DIR_DOWN) ||
      (player_dir == DIR_DOWN && exit_dir == EXIT_DIR_UP) ||
      (player_dir == DIR_LEFT && exit_dir == EXIT_DIR_RIGHT) ||
      (player_dir == DIR_RIGHT && exit_dir == EXIT_DIR_LEFT))
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント キー入力マップ遷移起動チェック
 *
 * @param req   イベントチェック用ワーク
 * @param gsys GAMESYS_WORK
 * @param	fieldWork FIELDMAP_WORK
 * @retval GMEVENT NULL イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * checkPushExit(EV_REQUEST * req,
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
	VecFx32 front_pos;
  int idx;
	
  //目の前が通行不可でない場合、チェックしない
  setFrontPos(req, &front_pos);
#if 1
  {
		FLDMAPPER *g3Dmapper = FIELDMAP_GetFieldG3Dmapper(fieldWork);
    MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( MAPATTR_GetAttribute(g3Dmapper, &front_pos) );
    //MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( MAPATTR_GetAttribute(g3Dmapper, &now_pos) );
    if (!(flag & MAPATTR_FLAGBIT_HITCH))
    {
      return NULL;
    }
  }
#endif

  //足元チェック（マット）
  idx = getConnectID(req, req->now_pos);
  if (idx != EXIT_ID_NONE)
  {
    const CONNECT_DATA * cnct = EVENTDATA_GetConnectByID(req->evdata, idx);
    if (CONNECTDATA_GetExitType(cnct) == EXIT_TYPE_MAT)
    {
      rememberExitInfo(req, fieldWork, idx, req->now_pos);
      return getChangeMapEvent(req, fieldWork, idx);
    }
  }
	
  //目の前チェック（階段、壁、ドア）
  idx = getConnectID(req, &front_pos);
  if (idx == EXIT_ID_NONE) {
    //壁の場合、電気ジムかを調べる
    //ギミックが割り当てられているかを調べて、ジムソースのオーバレイがなされていることを確認する
    if ( FLDGMK_GimmickCodeCheck(fieldWork, FLD_GIMMICK_GYM_ELEC) ){  
      //電気ジム
      return GYM_ELEC_CreateMoveEvt(gsys);
    }
    return NULL;
  }

	//マップ遷移発生の場合、出入口を記憶しておく
  rememberExitInfo(req, fieldWork, idx, &front_pos);
  return getChangeMapEvent(req, fieldWork, idx);
}

//--------------------------------------------------------------
/**
 * @brief ノーグリッドマップ、レールシステム上での出入口判定
 */
//--------------------------------------------------------------
static GMEVENT * checkRailExit(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork)
{
  int idx;
  VecFx32 pos;
  int * firstID = FIELDMAP_GetFirstConnectID(fieldWork);
  FLDNOGRID_MAPPER* nogridMapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
  FIELDMAP_CTRL_NOGRID_WORK* p_mapctrl_work = FIELDMAP_GetMapCtrlWork( fieldWork );
  FIELD_PLAYER_NOGRID* p_nogrid_player = FIELDMAP_CTRL_NOGRID_WORK_GetNogridPlayerWork( p_mapctrl_work );

  FIELD_PLAYER_NOGRID_GetPos( p_nogrid_player , &pos );
  idx = EVENTDATA_SearchConnectIDBySphere(req->evdata, &pos);
  if (*firstID == idx) return NULL;
  if (idx == EXIT_ID_NONE)
  {
    *firstID = idx;
    return NULL;
  }

  return getChangeMapEvent(req, fieldWork, idx);
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * サブスクリーンからのイベント起動チェック
 * @param gsys GAMESYS_WORK
 * @param	fieldWork FIELDMAP_WORK
 * @retval GMEVENT NULL イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * checkSubScreenEvent(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  GMEVENT* event=NULL;
  FIELD_SUBSCREEN_WORK * subscreen;
  subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);

  switch(FIELD_SUBSCREEN_GetAction(subscreen)){
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
		FIELD_SUBSCREEN_GrantPermission(subscreen);
	}
	else
	{
		FIELD_SUBSCREEN_ResetAction(subscreen);
	}
  return event;
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

//======================================================================
//  便利ボタンイベント
//======================================================================
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldWork;
}EVWORK_CONVBTN;

static GMEVENT_RESULT event_ConvenienceButton(
    GMEVENT *event, int *seq, void *wk )
{
  //仮 自転車テスト
  EVWORK_CONVBTN *work = wk;
  FIELD_PLAYER *fld_player;
  FIELDMAP_CTRL_GRID *gridMap;
  FIELD_PLAYER_GRID *gjiki;
  PLAYER_MOVE_FORM form;
  
  gridMap = FIELDMAP_GetMapCtrlWork( work->fieldWork );
  gjiki = FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( gridMap );
  
  fld_player = FIELDMAP_GetFieldPlayer( work->fieldWork );
  form = FIELD_PLAYER_GetMoveForm( fld_player );
  
  switch( form ){
  case PLAYER_MOVE_FORM_NORMAL:
    FIELD_PLAYER_GRID_SetRequest( gjiki, FIELD_PLAYER_GRID_REQBIT_CYCLE );
    break;
  case PLAYER_MOVE_FORM_CYCLE:
    FIELD_PLAYER_GRID_SetRequest( gjiki, FIELD_PLAYER_GRID_REQBIT_NORMAL );
    break;
  default:
    GF_ASSERT( 0 ); //歩行,自転車ではないのにイベントが実行されている
  }
  
  return GMEVENT_RES_FINISH;
} 

static GMEVENT * eventSet_ConvenienceButton( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  GMEVENT *event;
  EVWORK_CONVBTN *work;
  GF_ASSERT( fieldWork != NULL );
  event = GMEVENT_Create(
      gsys, NULL, event_ConvenienceButton, sizeof(EVWORK_CONVBTN) );
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldWork = fieldWork;
  return( event );
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT * checkEvent_ConvenienceButton( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  { //イベントチェック
  }
  
  { //イベント発行
    GMEVENT *event;
    event = eventSet_ConvenienceButton( req, gsys, fieldWork );
    return( event );
  }
}

//======================================================================
//  波乗りイベント
//======================================================================
#include "fldeff_namipoke.h"

typedef struct
{
  int wait;
  u16 dir;
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldWork;
}EVWORK_NAMINORI;

static GMEVENT_RESULT event_NaminoriStart(
    GMEVENT *event, int *seq, void *wk )
{
  EVWORK_NAMINORI *work = wk;

  FIELD_PLAYER *fld_player =
    FIELDMAP_GetFieldPlayer( work->fieldWork );
  FIELDMAP_CTRL_GRID *gridMap =
    FIELDMAP_GetMapCtrlWork( work->fieldWork );
  FIELD_PLAYER_GRID *gjiki =
    FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( gridMap );
  PLAYER_MOVE_FORM form =
    FIELD_PLAYER_GetMoveForm( fld_player );
  MMDL *mmdl =
    FIELD_PLAYER_GetMMdl( fld_player );
  
  FLDEFF_TASK *task;
  
  switch( *seq )
  {
  case 0: //波乗りポケモン出現
    {
      u16 dir;
      s16 gx,gz;
      fx32 height;
      VecFx32 pos;
      dir = MMDL_GetDirDisp( mmdl );
      gx = MMDL_GetGridPosX( mmdl );
      gz = MMDL_GetGridPosZ( mmdl );
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_GetCenterGridPos( gx, gz, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32*2 ); //2マス先
  
      height = 0;
      MMDL_GetMapPosHeight( mmdl, &pos, &height );
      pos.y = height;
     
      {
        FLDEFF_CTRL *fectrl;
        fectrl = FIELDMAP_GetFldEffCtrl( work->fieldWork );
        
        task = FLDEFF_NAMIPOKE_SetMMdl(
            fectrl, dir, &pos, mmdl, FALSE );
        
        FIELD_PLAYER_GRID_SetEffectTaskWork( gjiki, task );
      }
    }
    
    (*seq)++;
    break;
  case 1:
    work->wait++;
    if( work->wait > 15 ){
      (*seq)++;
    }
    break;
  case 2:
    {
      u16 dir = MMDL_GetDirDisp( mmdl );
      u16 ac = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
      MMDL_SetAcmd( mmdl, ac );
    }
    (*seq)++;
    break;
  case 3:
    if( MMDL_CheckEndAcmd(mmdl) == TRUE ){
      MMDL_EndAcmd( mmdl );
      task = FIELD_PLAYER_GRID_GetEffectTaskWork( gjiki );
      FLDEFF_NAMIPOKE_SetJointFlag( task, TRUE );
      FIELD_PLAYER_SetNaminori( gjiki );
      (*seq)++;
    }
    break;
  case 4:
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}

static GMEVENT * eventSet_NaminoriStart( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  GMEVENT *event;
  EVWORK_NAMINORI *work;
  
  event = GMEVENT_Create(
      gsys, NULL, event_NaminoriStart, sizeof(EVWORK_NAMINORI) );
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldWork = fieldWork;
  return( event );
}

void FIELD_EVENT_ChangeNaminoriStart( GMEVENT *event,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  EVWORK_NAMINORI *work;
  GMEVENT_Change( event, event_NaminoriStart, sizeof(EVWORK_NAMINORI) );
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldWork = fieldWork;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT * checkEvent_PlayerNaminoriStart( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  { //イベントチェック
    FIELD_PLAYER *fld_player;
    PLAYER_MOVE_FORM form;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    form = FIELD_PLAYER_GetMoveForm( fld_player );
    if( form == PLAYER_MOVE_FORM_SWIM ){
      return NULL;
    }
  }
  
  { //イベント発行
    GMEVENT *event;
    event = eventSet_NaminoriStart( req, gsys, fieldWork );
    return( event );
  }
}

//======================================================================
//  波乗り上陸イベント
//======================================================================
static GMEVENT_RESULT event_NaminoriEnd(
    GMEVENT *event, int *seq, void *wk )
{
  EVWORK_NAMINORI *work = wk;

  FIELD_PLAYER *fld_player =
    FIELDMAP_GetFieldPlayer( work->fieldWork );
  FIELDMAP_CTRL_GRID *gridMap =
    FIELDMAP_GetMapCtrlWork( work->fieldWork );
  FIELD_PLAYER_GRID *gjiki =
    FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( gridMap );
  PLAYER_MOVE_FORM form =
    FIELD_PLAYER_GetMoveForm( fld_player );
  MMDL *mmdl =
    FIELD_PLAYER_GetMMdl( fld_player );
  
  FLDEFF_TASK *task;
  
  switch( *seq )
  {
  case 0: //入力された方向に向く
    MMDL_SetDirDisp( mmdl, work->dir );
    (*seq)++;
    break;
  case 1: //波乗りポケモン切り離し
    task = FIELD_PLAYER_GRID_GetEffectTaskWork( gjiki );
    FLDEFF_NAMIPOKE_SetJointFlag( task, FALSE );
    FIELD_PLAYER_GRID_SetRequest( gjiki, FIELD_PLAYER_GRID_REQBIT_NORMAL );
    FIELD_PLAYER_GRID_UpdateRequest( gjiki );
    {
      u16 ac = MMDL_ChangeDirAcmdCode( work->dir, AC_JUMP_U_2G_16F );
      MMDL_SetAcmd( mmdl, ac );
    }
    (*seq)++;
    break;
  case 2:
    if( MMDL_CheckEndAcmd(mmdl) == TRUE ){
      FIELD_PLAYER_SetNaminoriEnd( gjiki );
      (*seq)++;
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}

static GMEVENT * eventSet_NaminoriEnd( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u16 dir )
{
  GMEVENT *event;
  EVWORK_NAMINORI *work;
  
  event = GMEVENT_Create(
      gsys, NULL, event_NaminoriEnd, sizeof(EVWORK_NAMINORI) );
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldWork = fieldWork;
  work->dir = dir;
  return( event );
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT * checkEvent_PlayerNaminoriEnd( const EV_REQUEST *req,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
  PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( fld_player );
    
  if( form == PLAYER_MOVE_FORM_SWIM ) //波乗り
  {
    VecFx32 pos;
    MAPATTR attr;
    MAPATTR_FLAG attr_flag;
    FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
    FIELDMAP_CTRL_GRID *gridMap =
      FIELDMAP_GetMapCtrlWork( fieldWork );
    FIELD_PLAYER_GRID *gjiki =
      FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( gridMap );
    u16 dir = FIELD_PLAYER_GRID_GetKeyDir( gjiki, req->key_cont );
    
    if( dir != DIR_NOT )
    {
      FIELD_PLAYER_GetDirPos( fld_player, dir, &pos );
      attr = MAPATTR_GetAttribute( mapper, &pos );
      attr_flag = MAPATTR_GetAttrFlag( attr );
    
      if( (attr_flag&MAPATTR_FLAGBIT_HITCH) == 0 && //進入可能で
          (attr_flag&MAPATTR_FLAGBIT_WATER) == 0 ){ //水以外
        GMEVENT *event;
        event = eventSet_NaminoriEnd( req, gsys, fieldWork, dir );
        return( event );
      }
    }
  }

  return NULL;
}

//======================================================================
//
//======================================================================
//--------------------------------------------------------------
/**
 * 自機前方の会話OBJ取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
static MMDL * getFrontTalkOBJ( EV_REQUEST *req, FIELDMAP_WORK *fieldMap )
{
  MMDL *mmdl,*jiki;
  VecFx32 pos;
  s16 gx,gy,gz;
  FLDMAPPER *mapper;
  MAPATTR attr;
  MAPATTR_VALUE attr_val;

  jiki = FIELD_PLAYER_GetMMdl( req->field_player );
  
  FIELD_PLAYER_GetFrontGridPos( req->field_player, &gx, &gy, &gz );
  MMDL_TOOL_GridPosToVectorPos( gx, gy, gz, &pos );
  
  mapper = FIELDMAP_GetFieldG3Dmapper( fieldMap );
  attr = MAPATTR_GetAttribute( mapper, &pos );
  attr_val = MAPATTR_GetAttrValue( attr );
  
  if( MAPATTR_VALUE_CheckCounter(attr_val) == TRUE ){
    u16 dir = MMDL_GetDirDisp( jiki );
    MMDL_TOOL_AddDirGrid( dir, &gx, &gz, 1 );
  }
  
  mmdl = MMDLSYS_SearchGridPosEx(
      FIELDMAP_GetMMdlSys(fieldMap), gx, gz, MMDL_GetVectorPosY(jiki), FRONT_TALKOBJ_HEIGHT_DIFF, FALSE );
  
  return( mmdl );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールマップでの、目の前のモデルを取得
 *
 *	@param	req
 *	@param	*fieldMap 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static MMDL * getRailFrontTalkOBJ( EV_REQUEST *req, FIELDMAP_WORK *fieldMap )
{
  MMDL *mmdl;
  MMDL* player_mmdl;
  RAIL_LOCATION location;

  player_mmdl = FIELD_PLAYER_GetMMdl( req->field_player );
  if( MMDL_GetRailFrontLocation( player_mmdl, &location ) )
  {
    mmdl = MMDLSYS_SearchRailLocation(
        FIELDMAP_GetMMdlSys(fieldMap), &location, FALSE );
  }
  else
  {
    mmdl = NULL;
  }
  
  return( mmdl );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  BOOL (*attr_check_func)(const MAPATTR_VALUE );
  u16 dir_code;
  u16 script_id;
}MAPATTR_EVENTDATA;

//--------------------------------------------------------------
/**
 * @brief BGアトリビュート話しかけチェック
 */
//--------------------------------------------------------------
static u16 checkTalkAttrEvent( EV_REQUEST *req, FIELDMAP_WORK *fieldMap)
{
  static const MAPATTR_EVENTDATA check_attr_data[] = {
    { MAPATTR_VALUE_CheckPC,          0,  EVENTDATA_ID_NONE },
    { MAPATTR_VALUE_CheckMap,         0,  EVENTDATA_ID_NONE },
    { MAPATTR_VALUE_CheckTV,          0,  EVENTDATA_ID_NONE },
    { MAPATTR_VALUE_CheckBookShelf1,  0,  SCRID_BG_MSG_BOOK1_01 },
    { MAPATTR_VALUE_CheckBookShelf2,  0,  SCRID_BG_MSG_BOOK2_01 },
    { MAPATTR_VALUE_CheckBookShelf3,  0,  SCRID_BG_MSG_BOOKRACK1_01 },
    { MAPATTR_VALUE_CheckBookShelf4,  0,  SCRID_BG_MSG_BOOKRACK2_01 },
    { MAPATTR_VALUE_CheckVase,        0,  SCRID_BG_MSG_SCRAP_01 },
    { MAPATTR_VALUE_CheckDustBox,     0,  SCRID_BG_MSG_SCRAP_01 },
    { MAPATTR_VALUE_CheckShopShelf1,  0,  SCRID_BG_MSG_SHOPRACK1_01 },
    { MAPATTR_VALUE_CheckShopShelf2,  0,  SCRID_BG_MSG_SHOPRACK2_01 },
    { MAPATTR_VALUE_CheckShopShelf3,  0,  SCRID_BG_MSG_SHOPRACK2_01 },
  };
  int i;

  VecFx32 pos;
  s16 gx,gy,gz;
  FLDMAPPER *mapper;
  MAPATTR attr;
  MAPATTR_VALUE attr_val;
  
  FIELD_PLAYER_GetFrontGridPos( req->field_player, &gx, &gy, &gz );
  MMDL_TOOL_GridPosToVectorPos( gx, gy, gz, &pos );
  
  mapper = FIELDMAP_GetFieldG3Dmapper( fieldMap );
  attr = MAPATTR_GetAttribute( mapper, &pos );
  attr_val = MAPATTR_GetAttrValue( attr );
  

  for (i = 0; i < NELEMS(check_attr_data); i++)
  {
    if ( check_attr_data[i].attr_check_func(attr_val) )
    {
      //本当はさらに方向もチェック
      return check_attr_data[i].script_id;
    }
  }
  return EVENTDATA_ID_NONE;
}


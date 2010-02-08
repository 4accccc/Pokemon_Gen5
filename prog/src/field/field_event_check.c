//======================================================================
/**
 * @file  field_event_check.c
 * @brief フィールドでのイベント起動チェック処理群
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
#include "field_comm/intrude_field.h"
#include "event_debug_menu.h"     //DEBUG_EVENT_DebugMenu
#include "event_battle.h"         //EVENT_Battle
#include "event_gsync.h"         //EVENT_GSync
#include "event_cg_wireless.h"         //EVENT_CG_Wireless
#include "event_beacon_detail.h"         //EVENT_BeaconDetail
#include "event_fieldtalk.h"      //EVENT_FieldTalk
#include "event_fieldmap_menu.h"  //EVENT_FieldMapMenu
#include "rail_editor.h"
#include "script.h"     //SCRIPT_SetEventScript
#include "net_app/union/union_event_check.h"
#include "event_comm_talk.h"      //EVENT_CommTalk
#include "event_comm_talked.h"      //EVENT_CommWasTalkedTo
#include "event_rail_slipdown.h"    //EVENT_RailSlipDown
#include "event_intrude_subscreen.h"
#include "event_subscreen.h"      //EVENT_ChangeSubScreen
#include "event_shortcut_menu.h"	//EVENT_ShortCutMenu

#include "system/main.h"    //HEAPID_FIELDMAP
#include "isdbglib.h"

#include "field_encount.h"      //FIELD_ENCOUNT_CheckEncount
#include "effect_encount.h"

#include "fieldmap_ctrl.h"
#include "fieldmap_ctrl_grid.h"
#include "fieldmap_ctrl_nogrid_work.h"
#include "field_player_grid.h"

#include "map_attr.h"

#include "event_trainer_eye.h"
#include "fieldmap/zone_id.h"
#include "net_app/union/union_main.h"
#include "event_comm_error.h"

#include "../../../resource/fldmapdata/script/bg_attr_def.h" //SCRID_BG_MSG_～
#include "../../../resource/fldmapdata/script/hiden_def.h"

#include "field_gimmick.h"   //for FLDGMK_GimmickCodeCheck
#include "field_gimmick_def.h"  //for FLD_GIMMICK_GYM_～
#include "gym_elec.h"     //for GYM_ELEC_CreateMoveEvt
#include "gym_fly.h"     //for GYM_FLY_CreateShotEvt
#include "gym_insect.h"     //for GYM_INSECT_CreatePushWallEvt
#include "field_gimmick_d06.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "waza_tool/wazano_def.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_mission.h"

#include "savedata/encount_sv.h"
#include "../../../resource/fldmapdata/script/eggevent_scr_def.h"   // for SCRID_EGG_BIRTH
#include "../../../resource/fldmapdata/script/pasokon_def.h" // for SCRID_PC
#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLE_EV_SPRAY_EFFECT_END

#include "debug/debug_flg.h" //DEBUG_FLG_～

#include "field/field_const.h"  // for FX32_TO_GRID

#include "event_fourkings.h"  // 

#include "pleasure_boat.h"    //for PL_BOAT_

#include "event_autoway.h" // for EVENT_PlayerMoveOnAutoWay
#include "event_current.h" // for EVENT_PlayerMoveOnCurrent

#include "field_status_local.h" //FIELD_STATUS_GetReservedScript

#include "event_intrude.h"
#include "field_comm/intrude_work.h"
#include "scrcmd.h"
#include "scrcmd_intrude.h"

#ifdef PM_DEBUG
extern BOOL DebugBGInitEnd;    //BG初期化監視フラグ             宣言元　fieldmap.c
extern BOOL MapFadeReqFlg;    //マップフェードリクエストフラグ  宣言元　script.c
#endif


//======================================================================
//======================================================================


//======================================================================
//======================================================================
#define FRONT_TALKOBJ_HEIGHT_DIFF (FX32_CONST(16)) //話しかけ可能なOBJのY値差分

//--------------------------------------------------------------
/**
 * @brief 判定チェック用パラメータ
 */
//--------------------------------------------------------------
typedef struct {
  HEAPID heapID;              ///<ヒープID
  GAMESYS_WORK * gsys;        ///<ゲームシステムへのポインタ
  GAMEDATA * gamedata;        ///<ゲームデータへのポインタ
  EVENTDATA_SYSTEM *evdata;   ///<参照しているイベントデータ
  u16 map_id;                 ///<現在のマップID
  //↑変化がないパラメータ　

  //↓毎回変化する可能性があるパラメータ
  FIELDMAP_WORK * fieldWork;
  FIELD_PLAYER * field_player;
	PLAYER_MOVE_STATE player_state;
  PLAYER_MOVE_VALUE player_value;
  u16 player_dir;                   ///<自機の向き
  u16 reserved_scr_id;        ///<予約スクリプトのID
  int key_trg;                ///<キー情報（トリガー）
  int key_cont;               ///<キー情報（）
  const VecFx32 * now_pos;        ///<現在のプレイヤー位置
  u32 mapattr;                  ///<足元のアトリビュート情報

#if 0
  BOOL talkRequest; ///<話しかけキー操作があったかどうか
  BOOL menuRequest; ///<メニューオープン操作があったかどうか
  BOOL moveRequest; ///<一歩移動終了タイミングかどうか
  BOOL stepRequest; ///<振り向きor一歩移動終了タイミングかどうか
  BOOL dirPosRequest; ///<方向つきPOSイベントのチェックをするタイミングか？
  BOOL pushRequest; ///<押し込み操作があったかどうか
  BOOL convRequest; ///<便利ボタン操作があったかどうか

  BOOL debugRequest;  ///<デバッグ操作があったかどうか
#else
  u32 talkRequest:1; ///<話しかけキー操作があったかどうか
  u32 menuRequest:1; ///<メニューオープン操作があったかどうか
  u32 moveRequest:1; ///<一歩移動終了タイミングかどうか
  u32 stepRequest:1; ///<振り向きor一歩移動終了タイミングかどうか
  u32 dirPosRequest:1; ///<方向つきPOSイベントのチェックをするタイミングか？
  u32 pushRequest:1; ///<押し込み操作があったかどうか
  u32 convRequest:1; ///<便利ボタン操作があったかどうか
  u32 subscreenRequest:1; ///<サブスクリーンイベントを起動してよいタイミングか？
  u32 debugRequest:1;  ///<デバッグ操作があったかどうか


#endif
}EV_REQUEST;

//======================================================================
//======================================================================
//--------------------------------------------------------------
//
//--------------------------------------------------------------
static void setupRequest(EV_REQUEST * req, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork);

//--------------------------------------------------------------
//  個別イベントチェック
//--------------------------------------------------------------
static GMEVENT * checkMoveEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork);
static GMEVENT* CheckSodateya( FIELDMAP_WORK * fieldWork, GAMESYS_WORK* gsys, GAMEDATA* gdata );
static GMEVENT* CheckSpray( FIELDMAP_WORK * fieldWork, GAMESYS_WORK* gsys, GAMEDATA* gdata );
static GMEVENT* CheckEffectEncount( FIELDMAP_WORK * fieldWork, GAMESYS_WORK* gsys, GAMEDATA* gdata );
static void updatePartyEgg( POKEPARTY* party );
static BOOL checkPartyEgg( POKEPARTY* party );

static GMEVENT * checkExit(EV_REQUEST * req,
    FIELDMAP_WORK *fieldWork, const VecFx32 *now_pos );
static GMEVENT * checkAttribute(EV_REQUEST * req, FIELDMAP_WORK *fieldWork );
static GMEVENT * checkPushExit(EV_REQUEST * req,
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static GMEVENT * checkPushGimmick(const EV_REQUEST * req,
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static GMEVENT * checkIntrudeSubScreenEvent(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
static GMEVENT * checkSubScreenEvent(
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static GMEVENT * checkNormalEncountEvent( const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );


static void setFrontPos(const EV_REQUEST * req, VecFx32 * pos);
static BOOL checkConnectExitDir(const CONNECT_DATA* cnct, u16 player_dir);
static BOOL checkConnectIsFreeExit( const CONNECT_DATA * cnct );
static int getConnectID(const EV_REQUEST * req, const VecFx32 * now_pos);
static void rememberExitInfo(EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx, const VecFx32 * pos);
static GMEVENT * getChangeMapEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx,
    const VecFx32 * pos);
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

static int checkPokeWazaGroup( GAMEDATA *gdata, u32 waza_no );

static GMEVENT* checkSpecialEvent( EV_REQUEST * req );

static GMEVENT* checkPosEvent( EV_REQUEST* req );
static GMEVENT* checkPosEvent_core( EV_REQUEST * req, u16 dir );
static GMEVENT* checkPosEvent_OnlyDirection( EV_REQUEST * req );
static GMEVENT* checkPosEvent_prefetchDirection( EV_REQUEST * req );
static GMEVENT* checkPosEvent_sandstream( EV_REQUEST* req );

//--------------------------------------------------------------
//レール用
//--------------------------------------------------------------
static GMEVENT * eventCheckNoGrid( GAMESYS_WORK *gsys, void *work );

static GMEVENT * checkRailExit(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork);
static GMEVENT * checkRailPushExit(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork);
static GMEVENT * checkRailSlipDown(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork);
static void rememberExitRailInfo(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx, const RAIL_LOCATION* loc);
static GMEVENT * getRailChangeMapEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx,
    const RAIL_LOCATION* loc);
static BOOL checkRailFrontMove( const FIELD_PLAYER* cp_player );


//======================================================================
//
//
//    イベント起動チェック本体
//
//
//======================================================================

//--------------------------------------------------------------
/**
 * @brief　イベント起動チェック：通常用
 * @param	gsys GAMESYS_WORK
 * @param work FIELDMAP_WORK
 * @retval GMEVENT NULL=イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * FIELD_EVENT_CheckNormal(
    GAMESYS_WORK *gsys, void *work, BOOL *eff_delete_flag, BOOL *menu_open_flag )
{
	
  EV_REQUEST req;
	GMEVENT *event;
	FIELDMAP_WORK *fieldWork = work;

  *eff_delete_flag = FALSE;  //まずはリクエストなし
  *menu_open_flag = FALSE;

  //リクエスト更新
  setupRequest( &req, gsys, fieldWork );

//☆☆☆デバッグ用チェックを最優先とする
#ifdef  PM_DEBUG
  event = DEBUG_checkKeyEvent( &req, gsys, fieldWork );
  if (event != NULL) {
    return event;
  }
#endif //PM_DEBUG
	
//☆☆☆特殊スクリプト起動チェックがここに入る
  if (DEBUG_FLG_GetFlg(DEBUG_FLG_DisableEvents) == FALSE) {
    event = checkSpecialEvent( &req );
    if ( event )
    {
      *eff_delete_flag = TRUE;  //エフェクトエンカウント消去リクエスト
      return event;
    }
  }

#ifdef PM_DEBUG
  GF_ASSERT_MSG(DebugBGInitEnd,"ERROR:BG NOT INITIALIZE");    //BG初期化監視フラグ
  GF_ASSERT_MSG(!MapFadeReqFlg,"ERROR:CALLED MAP FADE REQ");   //マップフェードリクエストフラグ
#endif  

  //遊覧船時間監視
  {
    PL_BOAT_WORK_PTR *wk_ptr = GAMEDATA_GetPlBoatWorkPtr(req.gamedata);
    event = PL_BOAT_CheckEvt(gsys, *wk_ptr);
    if (event != NULL) return event;
  }


//☆☆☆トレーナー視線チェックがここに入る
  if( !(req.debugRequest) && DEBUG_FLG_GetFlg(DEBUG_FLG_DisableTrainerEye) == FALSE ){
    BOOL flag = FIELD_EVENT_Check2vs2Battle( gsys );
    event = EVENT_CheckTrainerEye( fieldWork, flag );
    if( event != NULL ){
      *eff_delete_flag = TRUE;  //エフェクトエンカウント消去リクエスト
      return event;
    }
  }

//☆☆☆一歩移動チェックがここから
  //座標イベントチェック
  if( req.moveRequest )
  {
    // POSイベントチェック
    event = checkPosEvent( &req );
    if( event != NULL ){
      *eff_delete_flag = TRUE;  //エフェクトエンカウント消去リクエスト
      return event;
    } 
    //座標接続チェック
    event = checkExit(&req, fieldWork, req.now_pos);
    if( event != NULL ){
      return event;
    }
    // アトリビュートチェック
    event = checkAttribute( &req, fieldWork );
    if( event != NULL ) {
      return event;
    }
    //汎用一歩移動イベントチェック群
    event = checkMoveEvent(&req, fieldWork);
    if ( event != NULL) {
      return event;
    }
  }

//☆☆☆ステップチェック（一歩移動or振り向き）がここから
  if (req.stepRequest )
  {
    //POSイベント：向き指定ありをチェック
    event = checkPosEvent_OnlyDirection( &req );
    if (event)
    {
      return event;
    }
  }
  //POSイベント：方向指定ありチェックその２
  //if ( req->player_state != PLAYER_MOVE_STATE_END && req->player_value == PLAYER_MOVE_VALUE_TURN )
  {
    event = checkPosEvent_prefetchDirection( &req );
    if (event)
    {
      return event;
    }
  }
  
  //戦闘移行チェック
  { //ノーマルエンカウントイベント起動チェック
    GMEVENT* enc_event = checkNormalEncountEvent( &req, gsys, fieldWork );
    if(enc_event != NULL){
      return enc_event;
    }
  }


  //看板イベントチェック
  /*
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
      return event;
    }
  }
  */
  
//☆☆☆自機状態イベントチェック
  if( !(req.debugRequest) ){
    PLAYER_EVENTBIT evbit = PLAYER_EVENTBIT_NON;
     EVENTWORK *ev = GAMEDATA_GetEventWork( req.gamedata );
    
    if( EVENTWORK_CheckEventFlag(ev,SYS_FLAG_KAIRIKI) == TRUE ){
      evbit |= PLAYER_EVENTBIT_KAIRIKI;
    }
    
    event = FIELD_PLAYER_CheckMoveEvent(
        req.field_player, req.key_trg, req.key_cont, evbit );
    
    if( event != NULL ){
      return event;
    }
  }
  
//☆☆☆会話チェック

	///通信用会話処理(仮
  {
    GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
    INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(game_comm);
    MMDL *fmmdl_player = FIELD_PLAYER_GetMMdl( req.field_player );
    u32 talk_netid;
    
    if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION && intcomm != NULL){
      //話しかけられていないかチェック
      if(IntrudeField_CheckTalkedTo(intcomm, &talk_netid) == TRUE){
        FIELD_PLAYER_GRID_ForceStop( req.field_player );
        return EVENT_CommWasTalkedTo(gsys, fieldWork, intcomm, fmmdl_player, talk_netid, req.heapID);
      }
      //ミッション結果を受信していないかチェック
      if(MISSION_CheckRecvResult(&intcomm->mission) == TRUE){
        FIELD_PLAYER_GRID_ForceStop( req.field_player );
        return EVENT_CommMissionResult(gsys, fieldWork, intcomm, fmmdl_player, req.heapID);
      }
      //話しかける
      if( req.talkRequest ){
        if(IntrudeField_CheckTalk(intcomm, req.field_player, &talk_netid) == TRUE){
          FIELD_PLAYER_GRID_ForceStop( req.field_player );
          return EVENT_CommTalk(gsys, fieldWork, intcomm, fmmdl_player, talk_netid, req.heapID);
        }
      }
    }
  }

	//フィールド話し掛けチェック
  if( req.talkRequest )
  {
    { //OBJ話し掛け
      MMDL *fmmdl_talk = getFrontTalkOBJ( &req, fieldWork );
      if( fmmdl_talk != NULL )
      {
        u32 scr_id = MMDL_GetEventID( fmmdl_talk );
        if ( SCRIPT_IsValidScriptID( scr_id ) == TRUE )
        {
          MMDL *fmmdl_player = FIELD_PLAYER_GetMMdl( req.field_player );
          FIELD_PLAYER_GRID_ForceStop( req.field_player );
          *eff_delete_flag = TRUE;  //エフェクトエンカウント消去リクエスト
          return EVENT_FieldTalk( gsys, fieldWork,
            scr_id, fmmdl_player, fmmdl_talk, req.heapID );
        }
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


  { //波乗りテスト

    if( req.player_state == PLAYER_MOVE_STATE_END ||
        req.player_state == PLAYER_MOVE_STATE_OFF )
    {
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
    event = checkPushGimmick(&req, gsys, fieldWork);
    if( event != NULL ){
      return event;
    }
  }
  
//☆☆☆自機位置に関係ないキー入力イベントチェック
  //便利ボタンチェック
  if( req.convRequest ){
		if(WIPE_SYS_EndCheck()){
			if( EVENT_ShortCutMenu_IsOpen(gsys) )
			{	
				event = EVENT_ShortCutMenu( gsys, fieldWork, req.heapID );
				if( event != NULL ){
					return event;
				}
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
      *menu_open_flag = TRUE;
  	  return EVENT_FieldMapMenu( gsys, fieldWork, req.heapID );
		}
	}
	
	//侵入によるサブスクリーン切り替えイベント起動チェック
	if(WIPE_SYS_EndCheck()){
    event = checkIntrudeSubScreenEvent(gsys, fieldWork);
    if(event != NULL){
      return event;
    }
  }
	
	//新サブスクリーンからのイベント起動チェック
	if(WIPE_SYS_EndCheck()){
    event = FIELD_SUBSCREEN_EventCheck( FIELDMAP_GetFieldSubscreenWork(fieldWork), req.subscreenRequest );
    if(event != NULL){
      return event;
    }
  }
	//旧サブスクリーンからのイベント起動チェック
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
	FIELD_PLACE_NAME *place_name_sys;
	FIELDMAP_WORK * fieldmap_work = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_ENCOUNT* encount = FIELDMAP_GetEncount(fieldmap_work); 
  BOOL eff_delete_flag,menu_open_flag;

  //エフェクトエンカウントの　OBJとの接触によるエフェクト破棄チェック
  EFFECT_ENC_CheckObjHit( encount );

	// イベント起動チェック
	event = FIELD_EVENT_CheckNormal( gsys, work, &eff_delete_flag, &menu_open_flag );

	// イベント生成時の処理
	if( event != NULL )
	{
    if (eff_delete_flag)
    {
      EFFECT_ENC_EffectDelete( encount );
    }
		// 地名表示ウィンドウを消去
		place_name_sys  = FIELDMAP_GetPlaceNameSys( fieldmap_work );
		FIELD_PLACE_NAME_Hide( place_name_sys );
    
    // 自機が特殊表示の場合は元に戻す
    FIELD_PLAYER_CheckSpecialDrawForm(
        FIELDMAP_GetFieldPlayer(fieldmap_work), menu_open_flag );
	}

	return event;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * イベント起動チェック：ユニオンorコロシアム
 *
 * @param   gsys		
 * @param   work		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
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
  #if 1   //※check　コロシアムマップをチェックする為、ユニオン限定にしておく
          //         コロシアム込みでエラー処理が確定したら修正を行う
    if(ZONEDATA_IsUnionRoom(FIELDMAP_GetZoneID(fieldWork)) == TRUE){
      return EVENT_ChangeMapFromUnion( gsys, fieldWork );
    }
    return NULL;
  #else
    return EVENT_ChangeMapFromUnion( gsys, fieldWork );
  #endif
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
  
//☆☆☆特殊スクリプト起動チェックがここに入る

//☆☆☆一歩移動チェックがここから
  //座標イベントチェック
  if( req.moveRequest )
  {
    GMEVENT * event = checkPosEvent_core( &req, DIR_NOT );
    if ( event != NULL )
    {
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

  //++++ ここから下は自機が完全に動作フリーな状態でなければチェックしない ++++
	if(UnionMain_CheckPlayerFreeMode(gsys) == FALSE){
    return NULL;
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


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * イベント起動チェック：ノーグリッドマップ
 *
 *	@param	gsys
 *	@param	work 
 *
 *	@return
 */
//--------------------------------------------------------------
GMEVENT * FIELD_EVENT_CheckNoGrid( GAMESYS_WORK *gsys, void *work )
{
	GMEVENT *event;
	FIELDMAP_WORK *fieldmap_work;
	FIELD_PLACE_NAME *place_name_sys;

	// イベント起動チェック
	event = eventCheckNoGrid( gsys, work );

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

//--------------------------------------------------------------
/**
 *	@brief  ノーグリッドマップ、　イベントチェック
 */
//--------------------------------------------------------------
static GMEVENT * eventCheckNoGrid( GAMESYS_WORK *gsys, void *work )
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
  if (DEBUG_FLG_GetFlg(DEBUG_FLG_DisableEvents) == FALSE) {
    event = checkSpecialEvent( &req );
    if ( event )
    {
      return event;
    }
  }


#ifdef PM_DEBUG
  GF_ASSERT_MSG(DebugBGInitEnd,"ERROR:BG NOT INITIALIZE");    //BG初期化監視フラグ
  GF_ASSERT_MSG(!MapFadeReqFlg,"ERROR:CALLED MAP FADE REQ");   //マップフェードリクエストフラグ
#endif  

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
    //*
    RAIL_LOCATION pos;
    const MMDL* player = FIELD_PLAYER_GetMMdl( req.field_player );
    u16 id;
    EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
    MMDL_GetRailLocation( player, &pos );
    
    id = EVENTDATA_CheckPosEventRailLocation( req.evdata, evwork, &pos );
    
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

    //汎用一歩移動イベントチェック群
    event = checkMoveEvent(&req, fieldWork);
    if ( event != NULL) {
      return event;
    }
  }


	
//☆☆☆ステップチェック（一歩移動or振り向き）がここから
  //戦闘移行チェック
  {
    GMEVENT* enc_event = checkNormalEncountEvent( &req, gsys, fieldWork );
    if(enc_event != NULL){
      return enc_event;
    }
  }
  

  //看板イベントチェック
  if( req.stepRequest ){
    //*
    u16 id;
    RAIL_LOCATION pos;
    EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
    const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req.field_player );
    u16 dir = MMDL_GetDirDisp( fmmdl );
  
    // 1歩前のレールロケーション取得
    MMDL_GetRailFrontLocation( fmmdl, &pos );
  
    {
      //OBJ看板チェック
    }
    
    id = EVENTDATA_CheckTalkBoardEventRailLocation( req.evdata, evwork, &pos, dir );
    
    if( id != EVENTDATA_ID_NONE ){
      event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
    }
    //*/
  }
  
//☆☆☆自機状態イベントチェックがここから
    /* 今はない */

//☆☆☆会話チェック
	//フィールド話し掛けチェック
  if( req.talkRequest )
  {
    { //OBJ話し掛け
      MMDL *fmmdl_talk = getRailFrontTalkOBJ( &req, fieldWork );
      if( fmmdl_talk != NULL )
      {
        u32 scr_id = MMDL_GetEventID( fmmdl_talk );
        if ( SCRIPT_IsValidScriptID( scr_id ) == TRUE )
        {
          FIELD_PLAYER_NOGRID* player_nogrid = FIELDMAP_GetPlayerNoGrid( fieldWork );
          MMDL *fmmdl_player = FIELD_PLAYER_GetMMdl( req.field_player );
          
          FIELD_PLAYER_NOGRID_ForceStop( player_nogrid );
          return EVENT_FieldTalk( gsys, fieldWork,
            scr_id, fmmdl_player, fmmdl_talk, req.heapID );
        }
      }
    }
    
    { //BG話し掛け
      u16 id;
      RAIL_LOCATION pos;
      EVENTWORK *evwork = GAMEDATA_GetEventWork( req.gamedata );
      MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req.field_player );
      u16 dir = MMDL_GetDirDisp( fmmdl );
      
      MMDL_GetRailFrontLocation( fmmdl, &pos );
      id = EVENTDATA_CheckTalkBGEventRailLocation( req.evdata, evwork, &pos, dir );
      
      if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
        event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
        return event;
      }
    }
    /*
    { //BG Attribute 話しかけ
      u16 id = checkTalkAttrEvent( &req, fieldWork );
      if( id != EVENTDATA_ID_NONE ){ //座標イベント起動
        event = SCRIPT_SetEventScript( gsys, id, NULL, req.heapID );
        return event;
      }

    }
    //*/
  }


//☆☆☆押し込み操作チェック（マットでのマップ遷移など）
	//キー入力接続チェック
  if (req.pushRequest) {
    event = checkRailPushExit(&req, gsys, fieldWork);
    if( event != NULL ){
      return event;
    }

    event = checkRailSlipDown(&req, gsys, fieldWork);
    if( event != NULL ){
      return event;
    }
  }
  
//☆☆☆自機位置に関係ないキー入力イベントチェック
  //便利ボタンチェック
  if( req.convRequest ){
		if(WIPE_SYS_EndCheck()){
			if( EVENT_ShortCutMenu_IsOpen(gsys) )
			{	
				event = EVENT_ShortCutMenu( gsys, fieldWork, req.heapID );
				if( event != NULL ){
					return event;
				}
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

	//侵入によるサブスクリーン切り替えイベント起動チェック
	if(WIPE_SYS_EndCheck()){
    event = checkIntrudeSubScreenEvent(gsys, fieldWork);
    if(event != NULL){
      return event;
    }
  }

	//サブスクリーンからのイベント起動チェック
	event = checkSubScreenEvent(gsys, fieldWork);
	if( event != NULL )
  {
		return event;
	}
  
  /*	
	//デバッグ：パレスで木に触れたらワープ
  {
    GMEVENT *ev;
    INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(GAMESYSTEM_GetGameCommSysPtr(gsys));
    ev =  DEBUG_IntrudeTreeMapWarp(fieldWork, gsys, req.field_player, intcomm);
    if(ev != NULL){
      return ev;
    }
	}
  //*/
	return NULL;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 *	@brief  イベント起動チェック：ハイブリッドマップ
 */
//--------------------------------------------------------------
GMEVENT * FIELD_EVENT_CheckHybrid( GAMESYS_WORK *gsys, void *work )
{
	FIELDMAP_WORK *fieldWork = work;
  u32 base_system;
  GMEVENT * event;
  
  // 現在のBaseシステムを取得 
  base_system = FIELDMAP_GetBaseSystemType( fieldWork );
  
  if( base_system == FLDMAP_BASESYS_GRID )
  {
    event = FIELD_EVENT_CheckNormal_Wrap( gsys, work );
  }
  else
  {
    event = FIELD_EVENT_CheckNoGrid( gsys, work );
  }

  return event;
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 判定用パラメータの生成
 *
 *
 * @note
 * 現状チェックごとに生成しているが実はほとんどの値は初期化時に一度だけでもいいのかも。
 */
//--------------------------------------------------------------
static void setupRequest(EV_REQUEST * req, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork)
{
  GFL_STD_MemClear( req, sizeof(EV_REQUEST) );
  req->heapID = FIELDMAP_GetHeapID(fieldWork);
  req->gsys = gsys;
  req->gamedata = GAMESYSTEM_GetGameData(gsys);
	req->evdata = GAMEDATA_GetEventData(req->gamedata);
  req->map_id = FIELDMAP_GetZoneID(fieldWork);

  req->key_trg = GFL_UI_KEY_GetTrg();
  req->key_cont = GFL_UI_KEY_GetCont();

  req->fieldWork = fieldWork;
  req->field_player = FIELDMAP_GetFieldPlayer( fieldWork );
  //自機動作ステータス更新
  FIELD_PLAYER_UpdateMoveStatus( req->field_player );
  //自機の動作状態を取得
  req->player_value = FIELD_PLAYER_GetMoveValue( req->field_player );
  req->player_state = FIELD_PLAYER_GetMoveState( req->field_player );
  {
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( req->field_player );
    req->player_dir = MMDL_GetDirDisp( fmmdl );
    req->now_pos = MMDL_GetVectorPosAddress( fmmdl );
  }
  req->reserved_scr_id = FIELD_STATUS_GetReserveScript( GAMEDATA_GetFieldStatus(req->gamedata) );
  
  // アトリビュート取得
  req->mapattr = FIELD_PLAYER_GetMapAttr( req->field_player );

  if (req->player_state == PLAYER_MOVE_STATE_END || req->player_state == PLAYER_MOVE_STATE_OFF )
  {
    req->talkRequest = ((req->key_trg & PAD_BUTTON_A) != 0);

    req->menuRequest = ((req->key_trg & PAD_BUTTON_X) != 0);

    //作成中だと止まったりするので、一時的に入力不可にします
    req->convRequest = ((req->key_trg & PAD_BUTTON_Y) != 0);

    {
      //サブスクリーンイベント起動許可チェック
      u8 input = 0;

      input += req->talkRequest;
      input += req->menuRequest;
      input += req->convRequest;
      if( input == 0 ){
        req->subscreenRequest = TRUE;
      }
    }
  }
  
  req->moveRequest = 0;

  if( req->player_value == PLAYER_MOVE_VALUE_WALK ){
    req->moveRequest = ((req->player_state == PLAYER_MOVE_STATE_END));
  }

  req->stepRequest = ((req->player_state == PLAYER_MOVE_STATE_END));

  if( req->moveRequest ){
    IWASAWA_Printf( "Req->MoveRequst\n");
  }else if( req->stepRequest ) {
    IWASAWA_Printf( "Req->StepRequst\n");
  }
  req->dirPosRequest = FALSE;
  if ( req->player_state != PLAYER_MOVE_STATE_END && req->player_value == PLAYER_MOVE_VALUE_TURN )
  {
    req->dirPosRequest = TRUE;
  } 
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
    
  if (req->key_cont & PAD_BUTTON_L) {
    TAMADA_Printf("KEY:%04x PSTATE:%d PVALUE:%d DIR:%d\n",
        req->key_cont & 0xff, req->player_state, req->player_value, req->player_dir );
  }
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 特殊スクリプト起動チェック
 *
 * @param req EV_REQUEST
 *
 * @return 起動したPOSイベント
 */
//--------------------------------------------------------------
static GMEVENT * checkSpecialEvent( EV_REQUEST * req )
{
  GMEVENT * event;
  if (req->reserved_scr_id != SCRID_NULL)
  {
    FIELD_STATUS_SetReserveScript( GAMEDATA_GetFieldStatus(req->gamedata), SCRID_NULL );
    event = SCRIPT_SetEventScript( req->gsys, req->reserved_scr_id, NULL, req->heapID );
    return event;
  }
  event = SCRIPT_SearchSceneScript( req->gsys, req->heapID );
  if (event){
    return event;
  }
  return NULL;
}

//======================================================================
//
//
//  イベント起動チェック：POSイベント関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief POSイベントチェック
 *
 * @param req EV_REQUEST
 *
 * @return 起動したPOSイベント
 */
//--------------------------------------------------------------
static GMEVENT* checkPosEvent( EV_REQUEST* req )
{
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( req->gsys );
  FIELD_PLAYER*    player = FIELDMAP_GetFieldPlayer( fieldmap );
  MAPATTR_VALUE attrval;
  VecFx32 pos;
  FIELD_PLAYER_GetPos( player, &pos );  // 自機座標を取得

  // アトリビュートをチェック
  { 
    FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
    MAPATTR attr = MAPATTR_GetAttribute( mapper, &pos );
    attrval = MAPATTR_GetAttrValue( attr );
  }

  // 流砂アトリビュートにいる場合はイレギュラーなPOSイベント判定を行う
  if( MAPATTR_VALUE_CheckSandStream(attrval) == TRUE )
  { 
    // 流砂POSイベントチェック
    GMEVENT* event = checkPosEvent_sandstream( req );
    if( event != NULL )
    { // 流砂POSイベント起動
      return event;
    }
    return NULL;
  }
  // 通常POSイベントチェック
  return checkPosEvent_core( req, DIR_NOT );
}
//--------------------------------------------------------------
/**
 * @brief POSイベントチェック
 * @param req EV_REQUEST
 * @param dir チェックする向きの指定
 * @return 起動したPOSイベント
 */
//--------------------------------------------------------------
static GMEVENT* checkPosEvent_core( EV_REQUEST * req, u16 dir )
{
  u16 id;
  VecFx32 pos;
  MAPATTR_VALUE attrval;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( req->gsys );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( req->gamedata ); 
  FIELD_PLAYER_GetPos( req->field_player, &pos );

  // アトリビュートをチェック
  { 
    FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
    MAPATTR attr = MAPATTR_GetAttribute( mapper, &pos );
    attrval = MAPATTR_GetAttrValue( attr );
  }
  // 流砂アトリビュートにいる場合はPOSイベントを無視する
  if( MAPATTR_VALUE_CheckSandStream(attrval) == TRUE ){ return NULL; }

  id = EVENTDATA_CheckPosEvent( req->evdata, evwork, &pos, dir ); 
  if( id != EVENTDATA_ID_NONE ) 
  { //座標イベント起動
    GMEVENT* event = SCRIPT_SetEventScript( req->gsys, id, NULL, req->heapID );
    return event;
  } 
  return NULL;
}

//--------------------------------------------------------------
/**
 * @brief POSイベントチェック：向き指定あり
 * @param req EV_REQUEST
 * @return 起動したPOSイベント
 *
 * @todo
 * 氷状態で強制移動・方向固定のままPOSに踏み込む状況で、キーを
 * 押しっぱなしにしていると発動しないタイミングが発見されている
 * （現状はそのような地形をなくすことで対処した）
 */
//--------------------------------------------------------------
static GMEVENT * checkPosEvent_OnlyDirection( EV_REQUEST * req )
{
  GMEVENT * event;
  FIELD_PLAYER_GRID * fpg;
  u16 next_dir;

  //現在の向きに適合するイベントの有無をチェック
  event = checkPosEvent_core( req, req->player_dir );
  if (event)
  {
    return event;
  }

  //fpg = FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( FIELDMAP_GetMapCtrlWork( req->fieldWork ) );
  fpg = FIELDMAP_GetPlayerGrid( req->fieldWork );
  if ( fpg == NULL )
  {
    return NULL;
  }
  next_dir = FIELD_PLAYER_GRID_GetKeyDir( fpg, req->key_cont );
  if ( next_dir == req->player_dir )
  {
    return NULL;
  }
  //現在の向き意外に、キーの先読みで自機が向く方向もチェック
  event = checkPosEvent_core( req, next_dir );
  if ( event )
  {
    OS_Printf( " dir pos event!!\n" );
    return event;
  }
  return NULL;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * checkPosEvent_prefetchDirection( EV_REQUEST * req )
{
  GMEVENT * event;
  FIELD_PLAYER_GRID * fpg;
  u16 key_dir = DIR_NOT;
  int st = req->player_state;
  int val = req->player_value;
  //if ( req->player_state == PLAYER_MOVE_STATE_END ) return NULL;
  //if ( req->player_value == PLAYER_MOVE_VALUE_WALK ) return NULL;

  if ( req->debugRequest ) return NULL;

  if ( req->key_cont & PAD_KEY_UP    ) {
    key_dir = DIR_UP;
  } else if ( req->key_cont & PAD_KEY_DOWN  ) {
    key_dir = DIR_DOWN;
  } else if ( req->key_cont & PAD_KEY_LEFT  ) {
    key_dir = DIR_LEFT;
  } else if ( req->key_cont & PAD_KEY_RIGHT ) {
    key_dir = DIR_RIGHT;
  }

#if 0
  switch ( val )
  {
  case PLAYER_MOVE_VALUE_STOP:
    GF_ASSERT( st == PLAYER_MOVE_STATE_OFF );
    break;
  case PLAYER_MOVE_VALUE_WALK:
  case PLAYER_MOVE_VALUE_TURN:
    GF_ASSERT( st != PLAYER_MOVE_STATE_OFF );
    break;
  }
#endif
  if ( key_dir == req->player_dir ) return NULL;
  if ( key_dir == DIR_NOT ) return NULL;

  switch ( st )
  {
  //case PLAYER_MOVE_STATE_OFF:
  case PLAYER_MOVE_STATE_START:
  case PLAYER_MOVE_STATE_ON:
    return NULL;
  //case PLAYER_MOVE_STATE_END:
  }

  fpg = FIELDMAP_GetPlayerGrid( req->fieldWork );
  if ( fpg == NULL )
  {
    return NULL;
  }
#if 0
  if ( FIELD_PLAYER_GRID_GetMoveValue( fpg, key_dir ) == PLAYER_MOVE_VALUE_STOP )
  {
    return NULL;
  }
  switch ( val )
  {
  case PLAYER_MOVE_VALUE_STOP:
    if
  case PLAYER_MOVE_VALUE_WALK:
  case PLAYER_MOVE_VALUE_TURN:
  }
#endif
  event = checkPosEvent_core( req, key_dir );
#if 0
  if ( event )
  {
    OS_Printf( " prefetch dir pos event!!\n" );
  } else if ( req->key_cont & PAD_BUTTON_L ) {
    OS_Printf( " prefetch dir pos not!!\n" );
  }
#endif

  return event;
}

//--------------------------------------------------------------
/**
 * @brief 流砂アトリビュートチェック
 *
 * @param req EV_REQUEST
 *
 * @return 足元が流砂アトリビュートだった場合,
 *         以下のどちらかの条件に合致する場合にPOSイベントを検索し, 
 *         実行すべきイベントを返す.
 *
 *         条件1.POSイベントの真上にいる
 *         条件2.周囲2グリッド以内にPOSイベントが存在する and
 *               (自転車に乗っている or 走っている)
 *
 *         実行すべきイベントが無い場合, NULL を返す.
 */
//--------------------------------------------------------------
static GMEVENT* checkPosEvent_sandstream( EV_REQUEST* req )
{
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( req->gsys );
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
  u16 zone_id = FIELDMAP_GetZoneID( fieldmap );
  const POS_EVENT_DATA* pos_event;
  VecFx32 pos;
  FIELD_PLAYER_GetPos( player, &pos );

  // アトリビュートをチェック
  {
		FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
    MAPATTR attr = MAPATTR_GetAttribute( mapper, &pos );
    MAPATTR_VALUE attrval = MAPATTR_GetAttrValue( attr );
    if( MAPATTR_VALUE_CheckSandStream(attrval) != TRUE ) return NULL; // 流砂アトリビュートじゃない
  }

  // 流砂POSイベントを取得
  {
    EVENTWORK *evwork = GAMEDATA_GetEventWork( req->gamedata );
    pos_event = EVENTDATA_GetPosEvent_XZ( req->evdata, evwork, &pos, DIR_NOT );
    if( !pos_event ) return NULL;   // POSイベントが無い
    if( pos_event->pos_type != EVENTDATA_POSTYPE_GRID ) return NULL; // グリッド以外のPOSイベント発見
  }

  // 流砂の中心にいるかどうかチェック
  {
    const POS_EVENT_DATA_GPOS* pos_event_gpos;
    int centerGX, centerGZ;
    pos_event_gpos = (const POS_EVENT_DATA_GPOS*)pos_event->pos_buf;
    centerGX = pos_event_gpos->gx + pos_event_gpos->sx/2;
    centerGZ = pos_event_gpos->gz + pos_event_gpos->sz/2;
    if( (centerGX == FX32_TO_GRID(pos.x)) &&
        (centerGZ == FX32_TO_GRID(pos.z)) )
    { // 中心にいたらPOSイベント起動
      GMEVENT* event = SCRIPT_SetEventScript( req->gsys, pos_event->id, NULL, req->heapID );
        return event;
    }
  }

  // 主人公の状態チェック
  {
    PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( player );
    MMDL* mmdl = FIELD_PLAYER_GetMMdl( player );
    u16  acmd = MMDL_GetAcmdCode( mmdl );
    if( (form != PLAYER_MOVE_FORM_NORMAL) ||
        (acmd == AC_DASH_U_4F) ||
        (acmd == AC_DASH_D_4F) ||
        (acmd == AC_DASH_L_4F) ||
        (acmd == AC_DASH_R_4F) )
    { // 歩き以外ならPOSイベント起動
      GMEVENT* event = SCRIPT_SetEventScript( req->gsys, pos_event->id, NULL, req->heapID );
      return event;
    }
  }

  return NULL;
}

//======================================================================
//
//
//    イベント起動チェック：一歩移動関連
//
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * checkMoveEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork)
{
  HEAPID     heap_id = FIELDMAP_GetHeapID( fieldWork );
  GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( fieldWork ); 
  GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );
  GMEVENT*     event = NULL;

  //エンカウントエフェクト起動監視
  event = CheckEffectEncount( fieldWork, gsys, gdata );
  if( event != NULL) return event;

  //育て屋チェック
  event  = CheckSodateya( fieldWork, gsys, gdata );
  if( event != NULL) return event;

  //虫除けスプレーチェック
  event = CheckSpray( fieldWork, gsys, gdata );
  if( event != NULL) return event;

  return NULL;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちタマゴの孵化カウント更新
 * @param party 更新対象のポケパーティー
 */
//--------------------------------------------------------------
static void updatePartyEgg( POKEPARTY* party )
{
  int i;
  int poke_count = PokeParty_GetPokeCount( party );

  for( i=0; i<poke_count; i++ )
  {
    POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, i );
    u32      tamago_flag = PP_Get( param, ID_PARA_tamago_flag, NULL );
    u32           friend = PP_Get( param, ID_PARA_friend, NULL ); // なつき度(タマゴの場合は孵化までの残り歩数)

    if( tamago_flag == TRUE ) // タマゴなら
    { 
      // 残り歩数をデクリメント
      if( 0 < friend ) PP_Put( param, ID_PARA_friend, --friend );
    }
  }
}
//--------------------------------------------------------------
/**
 * @brief 手持ちタマゴの孵化チェック
 * @param party チェック対象のポケパーティー
 * @return 孵化する卵がある場合TRUE
 *
 * ※タマゴの間は, なつき度を孵化歩数カウンタとして利用する.
 */
//--------------------------------------------------------------
static BOOL checkPartyEgg( POKEPARTY* party )
{
  int i;
  int poke_count = PokeParty_GetPokeCount( party );

  for( i=0; i<poke_count; i++ )
  {
    POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, i );
    u32      tamago_flag = PP_Get( param, ID_PARA_tamago_flag, NULL );
    u32           friend = PP_Get( param, ID_PARA_friend, NULL ); 

    if( (tamago_flag == TRUE) && (friend == 0) ) 
    { // カウンタ0のタマゴを発見
      return TRUE;
    }
  } 
  return FALSE;
}

/*
 *  @brief  育て屋イベントチェック
 */
static GMEVENT* CheckSodateya( FIELDMAP_WORK * fieldWork, GAMESYS_WORK* gsys, GAMEDATA* gdata )
{
  HEAPID       heap_id = FIELDMAP_GetHeapID( fieldWork );
  POKEPARTY*   party = GAMEDATA_GetMyPokemon( gdata );
  SODATEYA*    sodateya = FIELDMAP_GetSodateya( fieldWork );

  // 育て屋: 経験値加算, 子作り判定など1歩分の処理
  SODATEYA_BreedPokemon( sodateya ); 

  // 手持ちタマゴ: 孵化カウンタ更新
  updatePartyEgg( party );

  // 手持ちタマゴ: 孵化チェック
  if( checkPartyEgg( party ) )
  {
    return SCRIPT_SetEventScript( gsys, SCRID_EGG_BIRTH, NULL, heap_id );
  }
  return NULL;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * 1歩ごとのスプレー効果減算
 */
//--------------------------------------------------------------
static GMEVENT* CheckSpray( FIELDMAP_WORK * fieldWork, GAMESYS_WORK* gsys, GAMEDATA* gdata )
{
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork(gdata);
	u8 end_f;

	//セーブデータ取得
	end_f = EncDataSave_DecSprayCnt(EncDataSave_GetSaveDataPtr(save));
  //減算した結果0になったら効果切れメッセージ
	if( end_f ){
		OS_Printf("スプレー効果切れ\n");
     return SCRIPT_SetEventScript( gsys, SCRID_FLD_EV_SPRAY_EFFECT_END, NULL, FIELDMAP_GetHeapID( fieldWork ));
	}
	return NULL;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * 1歩ごとのエンカウントエフェクト起動チェック
 */
//--------------------------------------------------------------
static GMEVENT* CheckEffectEncount( FIELDMAP_WORK * fieldWork, GAMESYS_WORK* gsys, GAMEDATA* gdata )
{
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork(gdata);
  FIELD_ENCOUNT * encount = FIELDMAP_GetEncount(fieldWork);
  GMEVENT* event = NULL;
  
  //エフェクトエンカウント接触チェック
  event = EFFECT_ENC_CheckEventApproch( encount );
  if( event != NULL){
    return event;
  }
  //エフェクトエンカウント起動チェック
  EFFECT_ENC_CheckEffectEncountStart( encount );
	return NULL;
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

		fourkingsCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_A,
	};

	//ソフトリセットチェック
	if( (req->key_cont & resetCont) == resetCont ){
#if 0
		return DEBUG_EVENT_GameEnd(gsys, fieldWork);
#endif
	}
	
	//マップ変更チェック
	if( (req->key_cont & chgCont) == chgCont ){
#if 0
    ISDPrintSetBlockingMode(1);
    GFL_HEAP_DEBUG_PrintSystemInfo();
    GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_PROC );
    GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_FIELDMAP );
    ISDPrintSetBlockingMode(0);
#endif
		//return DEBUG_EVENT_ChangeToNextMap(gsys, fieldWork);
	}
	
	//デバッグメニュー起動チェック
  if( req->menuRequest && req->debugRequest )
  {
		return DEBUG_EVENT_DebugMenu(gsys, fieldWork, 
				req->heapID, ZONEDATA_GetMapRscID(req->map_id));
	}

#if defined(DEBUG_ONLY_FOR_tomoya_takahashi) | defined(DEBUG_ONLY_FOR_nakatsui) | defined(DEBUG_ONLY_FOR_takahashi) | defined(DEBUG_ONLY_FOR_hozumi_yukiko) | defined(DEBUG_ONLY_FOR_iwao_kazumasa) | defined(DEBUG_ONLY_FOR_murakami_naoto) | defined(DEBUG_ONLY_FOR_nakamura_akira)
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
    if ( CONNECTDATA_IsClosedExit(cnct) == TRUE || checkConnectIsFreeExit( cnct ) == FALSE )
    {
      return NULL;
    }
  }

	//マップ遷移発生の場合、出入口を記憶しておく
  rememberExitInfo(req, fieldWork, idx, now_pos);
  return getChangeMapEvent(req, fieldWork, idx, now_pos);
}

//--------------------------------------------------------------
/**
 * イベント アトリビュートチェック
 *
 * @param req   イベントチェック用ワーク
 * @param fieldWork FIELDMAP_WORK
 * @return GMEVENT  発生したイベントデータへのポインタ。 NULLの場合、イベントなし
 */
//--------------------------------------------------------------
static GMEVENT * checkAttribute(EV_REQUEST * req, FIELDMAP_WORK *fieldWork )
{
  VecFx32 pos;
  MAPATTR attr;
  MAPATTR_VALUE attrval;
  FLDMAPPER* mapper;

  // 足元のアトリビュートを取得
  FIELD_PLAYER_GetPos( req->field_player, &pos );
  mapper  = FIELDMAP_GetFieldG3Dmapper( fieldWork );
  attr    = MAPATTR_GetAttribute( mapper, &pos );
  attrval = MAPATTR_GetAttrValue( attr );

  if( CheckAttributeIsAutoWay(attrval) )
  { // 移動床
    return EVENT_PlayerMoveOnAutoWay( NULL, req->gsys, fieldWork );
  }
  else if( CheckAttributeIsCurrent(attrval) )
  { // 水流
    return EVENT_PlayerMoveOnCurrent( NULL, req->gsys, fieldWork );
  }
  return NULL;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * getChangeMapEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx,
    const VecFx32 * pos)
{
  LOC_EXIT_OFS exit_ofs;
	const CONNECT_DATA * cnct;
  cnct = EVENTDATA_GetConnectByID(req->evdata, idx);
  if (pos != NULL) {
    exit_ofs = CONNECTDATA_GetExitOfs(cnct, pos);
  } else {
    exit_ofs = LOCATION_DEFAULT_EXIT_OFS;
  }

  return EVENT_ChangeMapByConnect(req->gsys, fieldWork, cnct, exit_ofs);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void rememberExitInfo(EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx, const VecFx32 * pos)
{
	//マップ遷移発生の場合、出入口を記憶しておく
  LOCATION ent_loc;
  LOCATION_Set( &ent_loc, FIELDMAP_GetZoneID(fieldWork), idx, 0, LOCATION_DEFAULT_EXIT_OFS,
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
  if ( checkConnectIsFreeExit( cnct ) == FALSE )
  {
    if( checkConnectExitDir( cnct, req->player_dir ) == FALSE )
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
/**
 * @brief 踏むだけ出入口かどうかのチェック
 * @param cnct
 * @retval  TRUE  踏むだけ
 * @retval  FALSE それ以外（方向や当たり判定を見る）
 */
//--------------------------------------------------------------
static BOOL checkConnectIsFreeExit( const CONNECT_DATA * cnct )
{
  EXIT_TYPE exit_type = CONNECTDATA_GetExitType( cnct );
  return ( exit_type == EXIT_TYPE_NONE
      || exit_type == EXIT_TYPE_WARP
      || exit_type == EXIT_TYPE_INTRUDE );
}

//--------------------------------------------------------------
/**
 * @brief 出入口の持つ方向指定チェック
 * @param cnct
 * @param player_dir
 * @retval  TRUE  適合している
 * @retval  FALSE 適合していない
 */
//--------------------------------------------------------------
static BOOL checkConnectExitDir(const CONNECT_DATA* cnct, u16 player_dir)
{
  EXIT_DIR exit_dir;

  exit_dir = CONNECTDATA_GetExitDir( cnct );
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
  {
		FLDMAPPER *g3Dmapper = FIELDMAP_GetFieldG3Dmapper(fieldWork);
    MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( MAPATTR_GetAttribute(g3Dmapper, &front_pos) );
    if (!(flag & MAPATTR_FLAGBIT_HITCH))
    {
      return NULL;
    }
  }


  //足元チェック（マット）
  idx = getConnectID(req, req->now_pos);
  if (idx != EXIT_ID_NONE)
  {
    const CONNECT_DATA * cnct = EVENTDATA_GetConnectByID(req->evdata, idx);
    if ( CONNECTDATA_IsClosedExit(cnct) == FALSE && CONNECTDATA_GetExitType(cnct) == EXIT_TYPE_MAT )
    {
      rememberExitInfo(req, fieldWork, idx, req->now_pos);
      return getChangeMapEvent(req, fieldWork, idx, req->now_pos);
    }
  }
	
  //目の前チェック（階段、壁、ドア）
  idx = getConnectID(req, &front_pos);
  if (idx != EXIT_ID_NONE) 
  {
    const CONNECT_DATA * cnct = EVENTDATA_GetConnectByID(req->evdata, idx);
    if ( CONNECTDATA_IsClosedExit(cnct) == FALSE )
    {
      //マップ遷移発生の場合、出入口を記憶しておく
      rememberExitInfo(req, fieldWork, idx, &front_pos);
      return getChangeMapEvent(req, fieldWork, idx, &front_pos);
    }
  }


  // 無効イベントだった
  return NULL;
}

//--------------------------------------------------------------
/**
 * イベント キー入力ギミック起動チェック
 *
 * @param req   イベントチェック用ワーク
 * @param gsys GAMESYS_WORK
 * @param	fieldWork FIELDMAP_WORK
 * @retval GMEVENT NULL イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * checkPushGimmick(const EV_REQUEST * req,
		GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
	VecFx32 front_pos;
  int idx;

  setFrontPos(req, &front_pos);

  //ギミックが割り当てられているかを調べて、ジムソースのオーバレイがなされていることを確認する
  if ( FLDGMK_GimmickCodeCheck(fieldWork, FLD_GIMMICK_GYM_INSECT) ){
    //虫ジム
    int x,z;
    x =front_pos.x / FIELD_CONST_GRID_FX32_SIZE;
    z =front_pos.z / FIELD_CONST_GRID_FX32_SIZE;
    return GYM_INSECT_CreatePushWallEvt(gsys, x, z);
  }
  
  //D06電気洞窟
  if( FLDGMK_GimmickCodeCheck(fieldWork, FLD_GIMMICK_D06R0101) ||
      FLDGMK_GimmickCodeCheck(fieldWork, FLD_GIMMICK_D06R0201) ||
      FLDGMK_GimmickCodeCheck(fieldWork, FLD_GIMMICK_D06R0301) ){
    return( D06_GIMMICK_CheckPushEvent(fieldWork,req->player_dir) );
  }
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

  //目の前チェック
  idx = getConnectID(req, &front_pos);
  if (idx == EXIT_ID_NONE) {
    //壁の場合、電気ジムかを調べる
    //ギミックが割り当てられているかを調べて、ジムソースのオーバレイがなされていることを確認する
    if ( FLDGMK_GimmickCodeCheck(fieldWork, FLD_GIMMICK_GYM_ELEC) ){  
      //電気ジム
      return GYM_ELEC_CreateMoveEvt(gsys);
    }else if( FLDGMK_GimmickCodeCheck(fieldWork, FLD_GIMMICK_GYM_FLY) ){
      //飛行ジム
      return GYM_FLY_CreateShotEvt(gsys);
    }
    return NULL;
  }

  return NULL;
}



//--------------------------------------------------------------
/**
 * @brief ノーグリッドマップ、レールシステム上での出入口判定
 */
//--------------------------------------------------------------
static GMEVENT * checkRailExit(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork)
{
  int idx;
  RAIL_LOCATION pos;
  int * firstID = FIELDMAP_GetFirstConnectID(fieldWork);
  FLDNOGRID_MAPPER* nogridMapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
  FIELD_PLAYER_NOGRID* p_nogrid_player = FIELDMAP_GetPlayerNoGrid( fieldWork );
  const CONNECT_DATA* cnct;

  // @TODO ３D座標のイベントとレール座標のイベントを併用しているため複雑
  {
    VecFx32 pos_3d;
    FIELD_PLAYER_NOGRID_GetPos( p_nogrid_player , &pos_3d );
    idx = EVENTDATA_SearchConnectIDBySphere(req->evdata, &pos_3d);
  }
  if( idx == EXIT_ID_NONE )
  {
    FIELD_PLAYER_NOGRID_GetLocation( p_nogrid_player , &pos );
    idx = EVENTDATA_SearchConnectIDByRailLocation(req->evdata, &pos);
  }

  if (*firstID == idx) return NULL;
  if (idx == EXIT_ID_NONE){
    *firstID = idx;
    return NULL;
  }

  // 乗っただけで起動するのかチェック
  cnct = EVENTDATA_GetConnectByID( req->evdata, idx );
  if( CONNECTDATA_IsClosedExit(cnct) == TRUE || checkConnectIsFreeExit( cnct ) == FALSE  )
  {
    return NULL;
  }
  
  rememberExitRailInfo( req, fieldWork, idx, &pos );
  return getRailChangeMapEvent(req, fieldWork, idx, &pos);
}

//--------------------------------------------------------------
/**
 *	@brief  ノーグリッドマップ　レールシステム上での押し込み出入り口判定
 */
//--------------------------------------------------------------
static GMEVENT * checkRailPushExit(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork)
{
  int idx;
  RAIL_LOCATION pos, front_pos;
  BOOL result;
  const FIELD_PLAYER* cp_player = FIELDMAP_GetFieldPlayer( fieldWork );
  const MMDL* cp_mmdl = FIELD_PLAYER_GetMMdl( cp_player );
  const CONNECT_DATA* cnct;

  MMDL_GetRailLocation( cp_mmdl, &pos );
  result = MMDL_GetRailFrontLocation( cp_mmdl, &front_pos );

  // 目の前が交通不可能出ない場合にはチェックしない
  if( checkRailFrontMove( cp_player ) )
  {
    return NULL;
  }
   
  // その場チェック　＝　マット
  idx = EVENTDATA_SearchConnectIDByRailLocation(req->evdata, &pos);

  if (idx != EXIT_ID_NONE){
    cnct = EVENTDATA_GetConnectByID( req->evdata, idx );

    if( CONNECTDATA_IsClosedExit(cnct) == FALSE && checkConnectExitDir( cnct, req->player_dir ) )
    {
      if( CONNECTDATA_GetExitType( cnct ) == EXIT_TYPE_MAT)
      {
        // GO
        rememberExitRailInfo( req, fieldWork, idx, &pos );
        return getRailChangeMapEvent(req, fieldWork, idx, &pos);
      }
    }
  }

  // 目の前チェック = 階段、壁、ドア
  idx = EVENTDATA_SearchConnectIDByRailLocation(req->evdata, &front_pos);
  if(idx == EXIT_ID_NONE)
  {
    return NULL;
  }

  
  cnct = EVENTDATA_GetConnectByID( req->evdata, idx );

  if( CONNECTDATA_IsClosedExit(cnct) == FALSE && checkConnectExitDir( cnct, req->player_dir ) )
  {
    rememberExitRailInfo( req, fieldWork, idx, &pos );  // front_posは移動不可なので、posにする
    return getRailChangeMapEvent(req, fieldWork, idx, &pos);
  }

  return NULL;
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * 侵入によるサブスクリーン切り替えイベント起動チェック
 * @param   gsys		
 * @param   fieldWork		
 * @retval  GMEVENT *		NULL=イベントなし
 */
//--------------------------------------------------------------
static GMEVENT * checkIntrudeSubScreenEvent(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  FIELD_SUBSCREEN_WORK * subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
  FIELD_SUBSCREEN_MODE subscreen_mode = Intrude_SUBSCREEN_Watch(game_comm, subscreen);
  GMEVENT* event = NULL;
  
  if(subscreen_mode != FIELD_SUBSCREEN_MODE_MAX){
    event = EVENT_ChangeSubScreen(gsys, fieldWork, subscreen_mode);
  }
  return event;
}

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
  case FIELD_SUBSCREEN_ACTION_IRC:
		{
			GAME_COMM_SYS_PTR gcsp = GAMESYSTEM_GetGameCommSysPtr(gsys);
			GAME_COMM_NO no = GameCommSys_BootCheck(gcsp);
			if(GAME_COMM_NO_FIELD_BEACON_SEARCH == no || GAME_COMM_NO_DEBUG_SCANONLY == no){
				GameCommSys_ExitReq(gcsp);
			}
			if((GAME_COMM_NO_FIELD_BEACON_SEARCH == no) || (GAME_COMM_NO_NULL == no)||
         (GAME_COMM_NO_DEBUG_SCANONLY == no)){
				event = EVENT_IrcBattle(gsys, fieldWork, NULL, TRUE);
			}
		}
    break;
  case FIELD_SUBSCREEN_ACTION_GSYNC:
    event = EVENT_GSync(gsys, fieldWork, NULL, TRUE);
    break;
  case FIELD_SUBSCREEN_ACTION_WIRELESS:
    event = EVENT_CG_Wireless(gsys, fieldWork, NULL, TRUE);
    break;
  case FIELD_SUBSCREEN_ACTION_PALACE_WARP:
    event = EVENT_IntrudeTownWarp(gsys, fieldWork, PALACE_TOWN_DATA_PALACE);
    break;
  case FIELD_SUBSCREEN_ACTION_INTRUDE_TOWN_WARP:
    {
      GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
      event = EVENT_IntrudeTownWarp(gsys, fieldWork, Intrude_GetWarpTown(game_comm));
    }
    break;
  case FIELD_SUBSCREEN_ACTION_INTRUDE_PLAYER_WARP:
    {
      GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
      event = EVENT_IntrudePlayerWarp(gsys, fieldWork, Intrude_GetWarpPlayerNetID(game_comm));
    }
    break;
  case FIELD_SUBSCREEN_ACTION_INTRUDE_MISSION_ENTRY:
    event = EVENT_Intrude_MissionStartWait(gsys);
    break;
  
  case FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW:
    event = EVENT_ChangeSubScreen(gsys, fieldWork, FIELD_SUBSCREEN_BEACON_VIEW);
    break;
  case FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_CGEAR:
    event = EVENT_ChangeSubScreen(gsys, fieldWork, FIELD_SUBSCREEN_NORMAL);
    break;
  case FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_INTRUDE:
    event = EVENT_ChangeSubScreen(gsys, fieldWork, FIELD_SUBSCREEN_INTRUDE);
    break;
  case FIELD_SUBSCREEN_ACTION_BEACON_DETAIL:
    event = EVENT_BeaconDetail( gsys, fieldWork );
    break;
    
#if 0
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
 * フィールドノーマルエンカウントイベント起動チェック
 * @param gsys GAMESYS_WORK
 * @param	fieldWork FIELDMAP_WORK
 * @retval GMEVENT NULL イベント無し
 */
//--------------------------------------------------------------
static GMEVENT * checkNormalEncountEvent( const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  FIELD_ENCOUNT * encount = FIELDMAP_GetEncount(fieldWork);
  ENCOUNT_TYPE enc_type;
  
#ifdef PM_DEBUG
  if( req->debugRequest ){
    return NULL;
  }
#endif
  if( !(req->stepRequest) ){
    return NULL;
  }
#ifndef PM_DEBUG
  if(GFL_NET_IsInit()){
    return NULL;
  }
#endif
  
  // WFBC
  if( ZONEDATA_IsWfbc( req->map_id ) )
  {
    FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
    FIELD_WFBC * p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
    return (GMEVENT*)FIELD_ENCOUNT_CheckWfbcEncount(encount, p_wfbc );
  }
  
  enc_type = ENC_TYPE_NORMAL; //(ZONEDATA_IsBingo( req->map_id ) == TRUE) ? ENC_TYPE_BINGO : ENC_TYPE_NORMAL;
  return (GMEVENT*)FIELD_ENCOUNT_CheckEncount(encount, enc_type);
}

//--------------------------------------------------------------
/**
 *	@brief  レールずり落ちチェック
 */
//--------------------------------------------------------------
static GMEVENT * checkRailSlipDown(const EV_REQUEST * req, GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldWork)
{
  const FIELD_PLAYER* cp_player = FIELDMAP_GetFieldPlayer( fieldWork );

  // 目の前が交通不可能出ない場合にはチェックしない
  if( checkRailFrontMove( cp_player ) )
  {
    return NULL;
  }

  // ずり落ち処理
  if( RAIL_ATTR_VALUE_CheckSlipDown( MAPATTR_GetAttrValue(req->mapattr) ) )
  {
    // プレイヤーは下をみているか？
    if( req->player_dir == DIR_DOWN )
    {
      // ずり落ち開始
      return EVENT_RailSlipDown( gsys, fieldWork );
    }
  }

  return NULL;
}

//--------------------------------------------------------------
/**
 *	@brief  レールマップの出入り口情報を保存
 */
//--------------------------------------------------------------
static void rememberExitRailInfo(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx, const RAIL_LOCATION* loc)
{
	//マップ遷移発生の場合、出入口を記憶しておく
  LOCATION ent_loc;
  LOCATION_SetRail( &ent_loc, FIELDMAP_GetZoneID(fieldWork), idx, 0, LOCATION_DEFAULT_EXIT_OFS,
      loc->rail_index, loc->line_grid, loc->width_grid);
  GAMEDATA_SetEntranceLocation(req->gamedata, &ent_loc);
}

//--------------------------------------------------------------
/**
 *	@brief  レールのマップ変更イベントを取得する
 */
//--------------------------------------------------------------
static GMEVENT * getRailChangeMapEvent(const EV_REQUEST * req, FIELDMAP_WORK * fieldWork, int idx,
    const RAIL_LOCATION* loc)
{
  LOC_EXIT_OFS exit_ofs;
	const CONNECT_DATA * cnct;
  cnct = EVENTDATA_GetConnectByID(req->evdata, idx);
  exit_ofs = CONNECTDATA_GetRailExitOfs( cnct, loc );
  return EVENT_ChangeMapByConnect(req->gsys, fieldWork, cnct, exit_ofs);
}

//--------------------------------------------------------------
/**
 *	@brief  レールマップ目の前に移動してるかチェック
 *
 *	@retval TRUE    している
 *	@retval FALSE   していない
 */
//--------------------------------------------------------------
static BOOL checkRailFrontMove( const FIELD_PLAYER* cp_player )
{
  RAIL_LOCATION now;
  RAIL_LOCATION old;
  const MMDL* cp_mmdl = FIELD_PLAYER_GetMMdl( cp_player );

  
  MMDL_GetRailLocation( cp_mmdl, &now );
  MMDL_GetOldRailLocation( cp_mmdl, &old );

  if( GFL_STD_MemComp( &now, &old, sizeof(RAIL_LOCATION) ) == 0 )
  {
    return FALSE;
  }
  return TRUE;
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

//======================================================================
//  波乗りイベント
//======================================================================
#include "fldeff_namipoke.h"

typedef struct
{
  int wait;
  u16 dir;
  u16 attr_kishi_flag;
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldWork;
}EVWORK_NAMINORI;

static GMEVENT_RESULT event_NaminoriStart(
    GMEVENT *event, int *seq, void *wk )
{
  EVWORK_NAMINORI *work = wk;

  FIELD_PLAYER *fld_player =
    FIELDMAP_GetFieldPlayer( work->fieldWork );
  FIELD_PLAYER_GRID *gjiki =
    FIELDMAP_GetPlayerGrid( work->fieldWork );
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
      
      {
        fx32 range = GRID_FX32; //１マス先
        
        if( work->attr_kishi_flag == TRUE ){
          range <<= 1; //２マス先
        }
      
        MMDL_TOOL_AddDirVector( dir, &pos, range );
      }
      
      height = 0;
      MMDL_GetMapPosHeight( mmdl, &pos, &height );
      pos.y = height;
     
      {
        FLDEFF_CTRL *fectrl;
        fectrl = FIELDMAP_GetFldEffCtrl( work->fieldWork );
        
        task = FLDEFF_NAMIPOKE_SetMMdl(
            fectrl, dir, &pos, mmdl, NAMIPOKE_JOINT_OFF );
        
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
      u16 ac = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_1G_8F );
      
      if( work->attr_kishi_flag == TRUE ){
        ac = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
      }
      
      MMDL_SetAcmd( mmdl, ac );
    }
    (*seq)++;
    break;
  case 3:
    if( MMDL_CheckEndAcmd(mmdl) == TRUE ){
      MMDL_EndAcmd( mmdl );
      task = FIELD_PLAYER_GRID_GetEffectTaskWork( gjiki );
      FLDEFF_NAMIPOKE_SetJointFlag( task, NAMIPOKE_JOINT_ON );
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
  FIELD_PLAYER_GRID *gjiki =
    FIELDMAP_GetPlayerGrid( work->fieldWork );
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
    FLDEFF_NAMIPOKE_SetJointFlag( task, NAMIPOKE_JOINT_OFF );
    FIELD_PLAYER_GRID_SetRequest( gjiki, FIELD_PLAYER_REQBIT_NORMAL );
    FIELD_PLAYER_GRID_UpdateRequest( gjiki );
    {
      u16 ac = MMDL_ChangeDirAcmdCode( work->dir, AC_JUMP_U_1G_8F );
      if( work->attr_kishi_flag == TRUE ){
        ac = MMDL_ChangeDirAcmdCode( work->dir, AC_JUMP_U_2G_16F );
      }
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
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
    u16 dir, BOOL attr_kishi_flag )
{
  GMEVENT *event;
  EVWORK_NAMINORI *work;
  
  event = GMEVENT_Create(
      gsys, NULL, event_NaminoriEnd, sizeof(EVWORK_NAMINORI) );
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldWork = fieldWork;
  work->dir = dir;
  work->attr_kishi_flag = attr_kishi_flag;
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
    BOOL kishi;
    VecFx32 pos;
    MAPATTR attr;
    MAPATTR_FLAG attr_flag;
    MAPATTR_VALUE attr_value;
    FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
    FIELD_PLAYER_GRID *gjiki =
      FIELDMAP_GetPlayerGrid( fieldWork );
    u16 dir = FIELD_PLAYER_GRID_GetKeyDir( gjiki, req->key_cont );
    
    if( dir != DIR_NOT )
    {
      FIELD_PLAYER_GetDirPos( fld_player, dir, &pos );
      attr = MAPATTR_GetAttribute( mapper, &pos );
      attr_flag = MAPATTR_GetAttrFlag( attr );
      attr_value = MAPATTR_GetAttrValue( attr );
      
      kishi = FALSE;

      if( MAPATTR_VALUE_CheckShore(attr_value) == TRUE ){ //岸
        MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 ); //もう一歩先
        attr = MAPATTR_GetAttribute( mapper, &pos );
        attr_flag = MAPATTR_GetAttrFlag( attr );
        kishi = TRUE;
      }
      
      if( (attr_flag&MAPATTR_FLAGBIT_HITCH) == 0 && //進入可能で
          (attr_flag&MAPATTR_FLAGBIT_WATER) == 0 ){ //水以外
        GMEVENT *event;
        event = eventSet_NaminoriEnd( req, gsys, fieldWork, dir, kishi );
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


//--------------------------------------------------------------
/**
 *	@brief  レールマップでの、目の前のモデルを取得
 *
 *	@param	req
 *	@param	*fieldMap 
 *
 *	@return
 */
//--------------------------------------------------------------
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
    { MAPATTR_VALUE_CheckPC,          0,  SCRID_PC },
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
    { MAPATTR_VALUE_CheckWaterFall,   0,  SCRID_HIDEN_TAKINOBORI },
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
  
  //波乗りアトリビュート話し掛けチェック

  if( FIELD_PLAYER_GetMoveForm(req->field_player) != PLAYER_MOVE_FORM_SWIM )
  {
    MAPATTR_FLAG attr_flag = MAPATTR_GetAttrFlag( attr );
    
    if( MAPATTR_VALUE_CheckShore(attr_val) == TRUE ||
        (MAPATTR_GetHitchFlag(attr) == FALSE &&
         (attr_flag&MAPATTR_FLAGBIT_WATER) ) )
    {
      //本来はバッジチェックも入る。
      if( checkPokeWazaGroup(req->gamedata,WAZANO_NAMINORI) != 6 ){
        return SCRID_HIDEN_NAMINORI;
      }
    }
  }
  
  return EVENTDATA_ID_NONE;
}

//--------------------------------------------------------------
/**
 * 手持ちポケモン技チェック
 * @param gdata GAMEDATA
 * @param waza_no チェックする技ナンバー
 * @retval int 6=技無し
 */
//--------------------------------------------------------------
static int checkPokeWazaGroup( GAMEDATA *gdata, u32 waza_no )
{
  int i,max;
  POKEMON_PARAM *pp;
  POKEPARTY *party = GAMEDATA_GetMyPokemon( gdata );
  
  max = PokeParty_GetPokeCount( party );
  
  for( i = 0; i < max; i++ ){
    pp = PokeParty_GetMemberPointer( party, i );
    
    //たまごチェック
    if( PP_Get(pp,ID_PARA_tamago_flag,NULL) != 0 ){
      continue;
    }
    
    //技リストからチェック
    if( PP_Get(pp,ID_PARA_waza1,NULL) == waza_no ||
        PP_Get(pp,ID_PARA_waza2,NULL) == waza_no ||
        PP_Get(pp,ID_PARA_waza3,NULL) == waza_no ||
        PP_Get(pp,ID_PARA_waza4,NULL) == waza_no ){
      return i;
    }
  }

#ifdef DEBUG_ONLY_FOR_kagaya  //test
  return 0;
#endif
   
  return 6;
}


//--------------------------------------------------------------
/**
 * 2vs2対戦が可能かどうかチェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL FIELD_EVENT_Check2vs2Battle( GAMESYS_WORK *gsys )
{
  int i,max,count;
  POKEMON_PARAM *pp;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY *party = GAMEDATA_GetMyPokemon( gdata );
  
  max = PokeParty_GetPokeCount( party );
  
  for( i = 0, count = 0; i < max; i++ ){
    pp = PokeParty_GetMemberPointer( party, i );
    
    //たまごチェック
    if( PP_Get(pp,ID_PARA_tamago_flag,NULL) != 0 ){
      continue;
    }
    
    //HPチェック
    if( PP_Get(pp,ID_PARA_hp,NULL) == 0 ){
      continue;
    }

    count++;
  }
  
  if( count < 2 ){
    return FALSE;
  }
  
  return( TRUE );
}


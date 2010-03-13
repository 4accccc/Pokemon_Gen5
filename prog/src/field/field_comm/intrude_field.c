//==============================================================================
/**
 * @file    intrude_field.c
 * @brief   侵入でフィールド時に行うメイン：フィールドオーバーレイに配置
 * @author  matsuda
 * @date    2009.09.03(木)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "infowin/infowin.h"
#include "gamesystem/gamesystem.h"
#include "field/field_player.h"
#include "test/performance.h"
#include "system/main.h"
#include "field/field_comm_actor.h"
#include "gamesystem/game_comm.h"
#include "fieldmap/zone_id.h"
#include "field/event_mapchange.h"
#include "intrude_types.h"
#include "intrude_field.h"
#include "field/intrude_comm.h"
#include "intrude_comm_command.h"
#include "system/net_err.h"
#include "field/zonedata.h"
#include "field/field_g3d_mapper.h"
#include "intrude_main.h"
#include "intrude_mission.h"
#include "field/intrude_secret_item.h"
#include "field/event_mission_secretitem_ex.h"

#include "message.naix"
#include "msg/msg_d_field.h"
#include "msg/msg_invasion.h"
#include "fieldmap/map_matrix.naix"  //MATRIX_ID

#include "intrude_minimono.h"   //DEBUG_INTRUDE_Pokemon_Add


//==============================================================================
//  構造体定義
//==============================================================================
///デバッグ用
typedef struct{
  VecFx32 pos;
  INTRUDE_COMM_SYS_PTR intcomm;
  FIELDMAP_WORK *fieldmap;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  ZONEID zoneID;
  u16 wait;
  u8 padding[2];
}DEBUG_SIOEND_WARP;  //DEBUG_SIOEND_MAPCHANGE;

///デバッグ用
typedef struct{
  INTRUDE_COMM_SYS_PTR intcomm;
  FIELDMAP_WORK *fieldmap;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  u16 wait;
  u8 padding[2];
}DEBUG_SIOEND_CHILD;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT * DEBUG_EVENT_ChangeMapPosCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		u16 zone_id, const VecFx32 * pos, INTRUDE_COMM_SYS_PTR intcomm );
static GMEVENT_RESULT DebugEVENT_MapChangeCommEnd(GMEVENT * event, int *seq, void*work);
static GMEVENT * DEBUG_EVENT_ChildCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, INTRUDE_COMM_SYS_PTR intcomm);
static GMEVENT_RESULT DebugEVENT_ChildCommEnd(GMEVENT * event, int *seq, void*work);
static void _PalaceMapCommBootCheck(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, GAME_COMM_SYS_PTR game_comm, FIELD_PLAYER *pcActor);
static void _PalaceFieldPlayerWarp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor, INTRUDE_COMM_SYS_PTR intcomm);



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
// フィールド通信システム更新
//  @param  FIELDMAP_WORK フィールドワーク
//  @param  GAMESYS_WORK  ゲームシステムワーク(PLAYER_WORK取得用
//  @param  PC_ACTCONT    プレイヤーアクター(プレイヤー数値取得用
//  自分のキャラの数値を取得して通信用に保存
//  他キャラの情報を取得し、通信から設定
//--------------------------------------------------------------
void IntrudeField_UpdateCommSystem( FIELDMAP_WORK *fieldWork ,
        GAMESYS_WORK *gameSys , FIELD_PLAYER *pcActor )
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gameSys);
  INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(game_comm);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gameSys);
  int i, my_net_id;
  BOOL update_ret;

  //パレス通信自動起動ON/OFF処理
  _PalaceMapCommBootCheck(fieldWork, gameSys, game_comm, pcActor);

  if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION || intcomm == NULL){
    return;
  }

  if(intcomm->palace_in == FALSE){
    PLAYER_WORK *my_player = GAMEDATA_GetMyPlayerWork(gamedata);
    if(ZONEDATA_IsPalace( PLAYERWORK_getZoneID( my_player ) ) == TRUE){
      intcomm->palace_in = TRUE;
    }
  }
  IntrudeField_ConnectMap(fieldWork, gameSys, intcomm);

  if(intcomm->comm_status != INTRUDE_COMM_STATUS_UPDATE){
    return;
  }

  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);

  if(Intrude_SetSendStatus(intcomm) == TRUE){
    intcomm->send_status = TRUE;
  }

  CommPlayer_Update(intcomm->cps);
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(i == my_net_id){
      continue;
    }
    if(intcomm->recv_profile & (1 << i)){
      if(CommPlayer_CheckOcc(intcomm->cps, i) == FALSE){
        MYSTATUS *myst = GAMEDATA_GetMyStatusPlayer(gamedata, i);
        u16 obj_code = Intrude_GetObjCode(&intcomm->intrude_status[i], 
          GAMEDATA_GetMyStatusPlayer(gamedata, i));
        CommPlayer_Add(intcomm->cps, i, obj_code, &intcomm->intrude_status[i].player_pack);
      }
      else{
        CommPlayer_SetParam(intcomm->cps, i, &intcomm->intrude_status[i].player_pack);
        //バトル or ビンゴバトル中なら「！」を表示
        if(intcomm->intrude_status[i].action_status == INTRUDE_ACTION_BATTLE
            || intcomm->intrude_status[i].action_status == INTRUDE_ACTION_BINGO_BATTLE){
          CommPlayer_SetGyoeTask(intcomm->cps, i);
        }
      }
    }
  }
  
  _PalaceFieldPlayerWarp(fieldWork, gameSys, pcActor, intcomm);
  
  //※check　デバッグ
  if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT){
    DEBUG_INTRUDE_Pokemon_Add(fieldWork);
  }
}

//==================================================================
/**
 * 自分の正面座標に対しての通信プレイヤー話しかけチェック
 *
 * @param   intcomm		
 * @param   fld_player		
 * @param   hit_netid		  ヒットした場合、対象プレイヤーのNetIDが代入される
 *
 * @retval  BOOL		TRUE:通信プレイヤーヒット。　FALSE:相手が居ない
 */
//==================================================================
BOOL IntrudeField_CheckTalk(INTRUDE_COMM_SYS_PTR intcomm, const FIELD_PLAYER *fld_player, u32 *hit_netid)
{
  s16 check_gx, check_gy, check_gz;
  u32 out_index;
  
  if(intcomm == NULL || intcomm->cps == NULL){
    return FALSE;
  }
  
  FIELD_PLAYER_GetFrontGridPos(fld_player, &check_gx, &check_gy, &check_gz);
  if(CommPlayer_SearchGridPos(intcomm->cps, check_gx, check_gz, &out_index) == TRUE){
    OS_TPrintf("Talkターゲット発見! net_id = %d, gx=%d, gz=%d\n", 
      out_index, check_gx, check_gz);
    *hit_netid = out_index;
    return TRUE;
  }
  
  return FALSE;
}

//==================================================================
/**
 * 通信プレイヤーから話しかけられていないかチェック
 *
 * @param   intcomm		
 * @param   fld_player		
 * @param   hit_netid		  話しかけられている場合、対象プレイヤーのNetIDが代入される
 *
 * @retval  BOOL		TRUE:通信プレイヤーから話しかけられている　FALSE:話しかけられていない
 */
//==================================================================
BOOL IntrudeField_CheckTalkedTo(INTRUDE_COMM_SYS_PTR intcomm, u32 *hit_netid)
{
  s16 check_gx, check_gy, check_gz;
  u32 out_index;
  
  if(intcomm == NULL || intcomm->cps == NULL){
    return FALSE;
  }
  
  //自分から話しかけていないのに値がNULL以外なのは話しかけられたから
  if(intcomm->talk.talk_netid != INTRUDE_NETID_NULL){
    *hit_netid = intcomm->talk.talk_netid;
    return TRUE;
  }
  
  return FALSE;
}

//==============================================================================
//
//  橋の特定位置まで来て、誰とも通信していないなら注意メッセージを出して自機を戻す
//
//==============================================================================
typedef struct{
  FIELDMAP_WORK *fieldWork;
  MMDL *player_mmdl;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  BOOL left_right;
}DEBUG_PALACE_NGWIN;

static GMEVENT_RESULT DebugPalaceNGWinEvent( GMEVENT *event, int *seq, void *wk );

static GMEVENT * EVENT_DebugPalaceNGWin( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_PLAYER *fld_player, BOOL left_right )
{
  DEBUG_PALACE_NGWIN *ngwin;
  GMEVENT * event;
  
  event = GMEVENT_Create(gsys, NULL, DebugPalaceNGWinEvent, sizeof(DEBUG_PALACE_NGWIN));
  
  ngwin = GMEVENT_GetEventWork(event);
  GFL_STD_MemClear( ngwin, sizeof(DEBUG_PALACE_NGWIN) );
  
  ngwin->fieldWork = fieldWork;
  ngwin->player_mmdl = FIELD_PLAYER_GetMMdl(fld_player);
  ngwin->left_right = left_right;
  
  return event;
}

static GMEVENT_RESULT DebugPalaceNGWinEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_PALACE_NGWIN *ngwin = wk;
  
  switch(*seq){
  case 0:
    {
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(ngwin->fieldWork);
      ngwin->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      ngwin->msgWin = FLDMSGWIN_AddTalkWin( msgBG, ngwin->msgData );
      FLDMSGWIN_Print( ngwin->msgWin, 0, 0, msg_invasion_test09_00 );
      GXS_SetMasterBrightness(-16);
      (*seq)++;
    }
    break;
  case 1:
    if( FLDMSGWIN_CheckPrintTrans(ngwin->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 2:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        (*seq)++;
      }
    }
    break;
  case 3:
    FLDMSGWIN_Delete( ngwin->msgWin );
    GFL_MSG_Delete( ngwin->msgData );
    GXS_SetMasterBrightness(0);
    (*seq)++;
    break;
  case 4:
    if(MMDL_CheckPossibleAcmd(ngwin->player_mmdl) == TRUE){
      u16 code = (ngwin->left_right == 0) ? AC_WALK_R_16F : AC_WALK_L_16F;
      MMDL_SetAcmd(ngwin->player_mmdl, code);
      (*seq)++;
    }
    break;
  case 5:
    if(MMDL_CheckEndAcmd(ngwin->player_mmdl) == TRUE){
      MMDL_EndAcmd(ngwin->player_mmdl);
      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}


//==================================================================
/**
 *   木に重なったらT1へワープ
 *
 * @param   fieldWork		
 * @param   gameSys		
 * @param   pcActor		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * DEBUG_IntrudeTreeMapWarp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor, INTRUDE_COMM_SYS_PTR intcomm)
{
  PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gameSys );
  ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
  VecFx32 pos;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gameSys);
  int i;
  
  if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION || intcomm == NULL){
//    return NULL;
  }

  if(intcomm != NULL && intcomm->comm_status == INTRUDE_COMM_STATUS_UPDATE){
    if(intcomm->exit_recv == TRUE){
      return DEBUG_EVENT_ChildCommEnd(gameSys, fieldWork, intcomm);
    }
  }
  
  if(ZONEDATA_IsBingo(zone_id) == TRUE){  //ビンゴマップから脱出監視
    FIELD_PLAYER_GetPos( pcActor, &pos );
    if(pos.z >= FX32_CONST(680)){
    	pos.x = PALACE_MAP_LEN/2;
    	pos.y = 0;
    	pos.z = 408 << FX32_SHIFT;
      return EVENT_ChangeMapPos(gameSys, fieldWork, ZONE_ID_PALACE01, &pos, 0, FALSE);
    }
    return NULL;
  }
  
  if(ZONEDATA_IsPalace(zone_id) == FALSE){
    return NULL;
  }
  
  FIELD_PLAYER_GetPos( pcActor, &pos );
  //T01へワープ
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(pos.x >= FX32_CONST(504) + PALACE_MAP_LEN*i && pos.x <= FX32_CONST(504) + PALACE_MAP_LEN*i
        && pos.z >= FX32_CONST(712) && pos.z <= FX32_CONST(712)){
      pos.x = 12536 << FX32_SHIFT;
      pos.y = 0;
      pos.z = 12120 << FX32_SHIFT;
      
      if(intcomm != NULL 
//          && GFL_NET_GetConnectNum() > 1 
          && GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gameSys)) == GAME_COMM_NO_INVASION
//          && GFL_NET_IsParentMachine() == TRUE
          && intcomm->intrude_status_mine.palace_area == GFL_NET_SystemGetCurrentID()){
        //親が通信状態で自分の街に入る場合は切断
        return DEBUG_EVENT_ChangeMapPosCommEnd(gameSys, fieldWork, ZONE_ID_T01, &pos, intcomm);
      }
      return EVENT_ChangeMapFromPalace(gameSys);
      //return EVENT_ChangeMapPos(gameSys, fieldWork, ZONE_ID_T01, &pos, 0, FALSE);
    }
  }
  //ビンゴマップへワープ
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(pos.x >= FX32_CONST(984) + PALACE_MAP_LEN*i
        && pos.x <= FX32_CONST(1064) + PALACE_MAP_LEN*i 
        && pos.z >= FX32_CONST(184) && pos.z <= FX32_CONST(200)){
      pos.x = 472 << FX32_SHIFT;
      pos.y = 0;
      pos.z = 504 << FX32_SHIFT;
      return EVENT_ChangeMapPos(gameSys, fieldWork, ZONE_ID_PALACE02, &pos, 0, FALSE);
    }
  }
  
  {//自機の座標を監視し、誰とも通信していないのにパレスの橋の一定位置まで来たら
   //注意メッセージを出して引き返させる
    VecFx32 pos;
    BOOL left_right;
    
    FIELD_PLAYER_GetPos( pcActor, &pos );
    pos.x >>= FX32_SHIFT;
    pos.z >>= FX32_SHIFT;
    if(GFL_NET_GetConnectNum() <= 1 && ((pos.x <= 136 && pos.x >= 136-16) || (pos.x >= 872 && pos.x <= 872+16)) && (pos.z >= 472 && pos.z <= 536)){
      left_right = pos.x <= (PALACE_MAP_LEN/2 >> FX32_SHIFT) ? 0 : 1;
      return EVENT_DebugPalaceNGWin( gameSys, fieldWork, pcActor, left_right );
    }
  }
  
  return NULL;
}


//==================================================================
/**
 * ※デバッグ用　通信を終了させてからマップ移動
 *
 * @param   gsys		
 * @param   fieldmap		
 * @param   zone_id		
 * @param   pos		
 * @param   dir		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
static GMEVENT * DEBUG_EVENT_ChangeMapPosCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		u16 zone_id, const VecFx32 * pos, INTRUDE_COMM_SYS_PTR intcomm )
{
  DEBUG_SIOEND_WARP *dsw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, DebugEVENT_MapChangeCommEnd, sizeof(DEBUG_SIOEND_WARP));
	dsw = GMEVENT_GetEventWork(event);
	GFL_STD_MemClear(dsw, sizeof(DEBUG_SIOEND_WARP));
	dsw->fieldmap = fieldmap;
	dsw->zoneID = zone_id;
	dsw->pos = *pos;
	dsw->intcomm = intcomm;
	
	OS_TPrintf("親機：通信切断イベント起動\n");
	return event;
}

static GMEVENT_RESULT DebugEVENT_MapChangeCommEnd(GMEVENT * event, int *seq, void*work)
{
	DEBUG_SIOEND_WARP * dsw = work;
	GAMESYS_WORK  * gsys = GMEVENT_GetGameSysWork(event);
	FIELDMAP_WORK * fieldmap = dsw->fieldmap;
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	
	switch (*seq) {
	case 0:
    GameCommSys_ExitReq(game_comm); //通信終了リクエスト
	  {
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(dsw->fieldmap);
      dsw->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      dsw->msgWin = FLDMSGWIN_AddTalkWin( msgBG, dsw->msgData );
      FLDMSGWIN_Print( dsw->msgWin, 0, 0, msg_invasion_test06_01 );
      GXS_SetMasterBrightness(-16);
      (*seq)++;
    }
    break;
  case 1:
    if( FLDMSGWIN_CheckPrintTrans(dsw->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 2:
    dsw->wait++;
    if(dsw->wait > 60){
      if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_NULL){
        GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
        int i;
        PLAYER_WORK *plwork;
        VecFx32 pos = {0,0,0};
        
        for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
          plwork = GAMEDATA_GetPlayerWork( gamedata , i );
          PLAYERWORK_setPosition(plwork, &pos);
        }
        (*seq)++;
      }
    }
    break;
  case 3:
    FLDMSGWIN_Delete( dsw->msgWin );
    GFL_MSG_Delete( dsw->msgData );
    GXS_SetMasterBrightness(0);
    (*seq)++;
    break;
	case 4:
  	GMEVENT_CallEvent(
  	  //event, EVENT_ChangeMapPos(gsys, dsw->fieldmap, dsw->zoneID, &dsw->pos, 0, FALSE));
      event, EVENT_ChangeMapFromPalace(gsys) );
    (*seq)++;
    break;
	default:
		return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ※デバッグ用　子機の通信終了
 *
 * @param   gsys		
 * @param   fieldmap		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * DEBUG_EVENT_ChildCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, INTRUDE_COMM_SYS_PTR intcomm)
{
  DEBUG_SIOEND_CHILD *dsc;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, DebugEVENT_ChildCommEnd, sizeof(DEBUG_SIOEND_CHILD));
	dsc = GMEVENT_GetEventWork(event);
	GFL_STD_MemClear(dsc, sizeof(DEBUG_SIOEND_CHILD));
	dsc->fieldmap = fieldmap;
	dsc->intcomm = intcomm;
	
	OS_TPrintf("子機：通信切断イベント起動\n");
	return event;
}

static GMEVENT_RESULT DebugEVENT_ChildCommEnd(GMEVENT * event, int *seq, void*work)
{
  DEBUG_SIOEND_CHILD *dsc = work;
	GAMESYS_WORK  * gsys = GMEVENT_GetGameSysWork(event);
	FIELDMAP_WORK * fieldmap = dsc->fieldmap;
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	
	switch (*seq) {
	case 0:
    GameCommSys_ExitReq(game_comm); //通信終了リクエスト
	  {
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(dsc->fieldmap);
      dsc->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      dsc->msgWin = FLDMSGWIN_AddTalkWin( msgBG, dsc->msgData );
      FLDMSGWIN_Print( dsc->msgWin, 0, 0, msg_invasion_test06_01 );
      GXS_SetMasterBrightness(-16);
      (*seq)++;
    }
    break;
  case 1:
    if( FLDMSGWIN_CheckPrintTrans(dsc->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 2:
    dsc->wait++;
    if(dsc->wait > 60){
      if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_NULL){
        GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
        int i;
        PLAYER_WORK *plwork;
        VecFx32 pos = {0,0,0};
        
        for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
          plwork = GAMEDATA_GetPlayerWork( gamedata , i );
          PLAYERWORK_setPosition(plwork, &pos);
        }
        (*seq)++;
      }
    }
    break;
  case 3:
    FLDMSGWIN_Delete( dsc->msgWin );
    GFL_MSG_Delete( dsc->msgData );
    GXS_SetMasterBrightness(0);
    (*seq)++;
    break;
	default:
		return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自分のパレス島の中にいるか、橋側にいるかを判定
 *
 * @param   player_pos		
 *
 * @retval  BOOL		    TRUE:橋　FALSE:パレス島
 */
//--------------------------------------------------------------
static BOOL _PlayerPosCheck_PalaceBridge(const VecFx32 *player_pos)
{
  if(player_pos->x >= 0x348000 || player_pos->x <= 0xb8000){
    return TRUE;  //橋の上
  }
  return FALSE;   //自分のパレス島の中
}

//--------------------------------------------------------------
/**
 * パレス通信自動起動ON/OFF処理
 *
 * @param   fieldWork		
 * @param   gameSys		
 * @param   game_comm		
 * @param   pcActor		
 */
//--------------------------------------------------------------
static void _PalaceMapCommBootCheck(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, GAME_COMM_SYS_PTR game_comm, FIELD_PLAYER *pcActor)
{
  ZONEID zone_id;
  GAME_COMM_NO comm_no;

  VecFx32 player_pos;

  if(GameCommSys_CheckSystemWaiting(game_comm) == TRUE){
    return;
  }
  
  zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(GAMESYSTEM_GetGameData(gameSys)));
  if(ZONEDATA_IsPalace(zone_id) == FALSE){
    return;
  }
  
  FIELD_PLAYER_GetPos( pcActor, &player_pos );
  comm_no = GameCommSys_BootCheck(game_comm);
  switch(comm_no){
  case GAME_COMM_NO_NULL:
    if(_PlayerPosCheck_PalaceBridge(&player_pos) == TRUE){
      FIELD_INVALID_PARENT_WORK *invalid_parent;
      
      invalid_parent = GFL_HEAP_AllocClearMemory(
          GFL_HEAP_LOWID(HEAPID_PROC), sizeof(FIELD_INVALID_PARENT_WORK));
      invalid_parent->my_invasion = TRUE;
      invalid_parent->gsys = gameSys;
      invalid_parent->game_comm = GAMESYSTEM_GetGameCommSysPtr(gameSys);
      GameCommSys_Boot(game_comm, GAME_COMM_NO_INVASION, invalid_parent);

      OS_TPrintf("パレス通信起動：親\n");
    }
    break;
  case GAME_COMM_NO_INVASION:
    //自分のパレス島にいて接続している相手もいない場合は通信終了
    if(_PlayerPosCheck_PalaceBridge(&player_pos) == FALSE
        && GFL_NET_SystemGetConnectNum() <= 1){
      if(GAMESYSTEM_GetAlwaysNetFlag(gameSys) == TRUE){
        GameCommSys_ChangeReq(game_comm, GAME_COMM_NO_FIELD_BEACON_SEARCH, gameSys);
        OS_TPrintf("パレス通信終了：ビーコンサーチへ\n");
      }
      else{
        GameCommSys_ExitReq(game_comm);
        OS_TPrintf("パレス通信終了：通信END\n");
      }
    }
    break;
  case GAME_COMM_NO_FIELD_BEACON_SEARCH:
    //自分のパレス島にいて橋の上にいるならビーコンサーチをOFF
    if(_PlayerPosCheck_PalaceBridge(&player_pos) == TRUE){
      GameCommSys_ExitReq(game_comm);
      OS_TPrintf("ビーコンサーチ終了\n");
    }
    break;
  }
}

//==================================================================
/**
 *   デバッグ用 パレスに飛ぶイベントを作成する
 *  @param   fieldWork		
 *  @param   gameSys		
 *  @param   pcActor		
 *  @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * DEBUG_PalaceJamp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor)
{
  PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gameSys );
  ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
  VecFx32 pos;
  ZONEID jump_zone;
  
  if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
  	pos.x = 184 << FX32_SHIFT;
  	pos.y = 0;
  	pos.z = 184 << FX32_SHIFT;
  	jump_zone = ZONE_ID_UNION;
  }
  else{
  	pos.x = PALACE_MAP_LEN/2;
  	pos.y = 0;
  	pos.z = 408 << FX32_SHIFT;
  	jump_zone = ZONE_ID_PALACE01;
  }
  return EVENT_ChangeMapPos(gameSys, fieldWork, jump_zone, &pos, 0, FALSE);
}

//--------------------------------------------------------------
/**
 * パレスマップの端に来たらマップループ処理を行う
 *
 * @param   fieldWork		
 * @param   gameSys		
 * @param   pcActor		
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void _PalaceFieldPlayerWarp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor, INTRUDE_COMM_SYS_PTR intcomm)
{
  PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gameSys );
  ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
  u16 player_dir = PLAYERWORK_getDirection_Type(plWork);
  VecFx32 pos, new_pos;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gameSys);
  int i, now_area, new_area;
  BOOL warp = FALSE;
  fx32 left_end, right_end;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gameSys);
  
#if 1
  if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION || intcomm == NULL){
    return;
  }
#endif

  if(ZONEDATA_IsPalace(zone_id) == FALSE){
    return;
  }
  
  if(intcomm->member_num < 2){
    return;
  }
  
  FIELD_PLAYER_GetPos( pcActor, &pos );
  new_pos = pos;
  
  now_area = intcomm->intrude_status_mine.palace_area;
  new_area = now_area;
  
  left_end = 0;
  right_end = PALACE_MAP_LEN * intcomm->member_num;
  
  //マップループチェック
  if(pos.x < left_end){// && player_dir == DIR_LEFT){
    new_pos.x = right_end - (left_end - pos.x);
    warp = TRUE;
    OS_TPrintf("left warp now_pos.x = %d, new_pos.x = %d, left_end=%d, right_end=%d\n", pos.x>>FX32_SHIFT, new_pos.x>>FX32_SHIFT, left_end>>FX32_SHIFT, right_end>>FX32_SHIFT);
  }
  else if(pos.x > right_end){// && player_dir == DIR_RIGHT){
    new_pos.x = left_end + (pos.x - right_end);
    warp = TRUE;
    OS_TPrintf("right warp now_pos.x = %d, new_pos.x = %d, left_end=%d, right_end=%d\n", pos.x>>FX32_SHIFT, new_pos.x>>FX32_SHIFT, left_end>>FX32_SHIFT, right_end>>FX32_SHIFT);
  }
  
  //現在の座標からパレスエリアを判定
  new_area = pos.x / PALACE_MAP_LEN;

  if(warp == TRUE){
    FIELD_PLAYER_SetPos( pcActor, &new_pos );
    Intrude_PlayerPosRecalc(intcomm, new_pos.x);
  }
  
  new_area = new_area + GFL_NET_SystemGetCurrentID();
  if(new_area >= intcomm->member_num){
    new_area -= intcomm->member_num;
  }
  if(now_area != new_area){
    OS_TPrintf("new_palace_area = %d new_pos_x = %x\n", new_area, new_pos.x);
    intcomm->intrude_status_mine.palace_area = new_area;
    intcomm->send_status = TRUE;
  #if 0 //パレス島に来ただけでフラグONにするようにした 2010.03.10(水)
    if(GAMEDATA_GetIntrudeMyID(gamedata) == new_area){
      GAMEDATA_SetIntrudeReverseArea(gamedata, FALSE);
    }
    else{
      GAMEDATA_SetIntrudeReverseArea(gamedata, TRUE);
    }
  #endif

    // WFBCCOMMに新通信先を通知
    FIELD_WFBC_COMM_DATA_SetIntrudeNetID( &intcomm->wfbc_comm_data, new_area );
  }
}

//==================================================================
/**
 * 端分のマップワープ場所を連結
 *
 * @param   fieldWork		
 * @param   gameSys		
 * 
 * wb -- col wb wb wb -
 */
//==================================================================
void IntrudeField_ConnectMapInit(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, INTRUDE_COMM_SYS_PTR intcomm)
{
#if 0
  MAP_MATRIX *mmatrix = MAP_MATRIX_Create( HEAPID_WORLD );
  
  MAP_MATRIX_Init(mmatrix, NARC_map_matrix_palace01_mat_bin, ZONE_ID_PALACE01);
  
  OS_TPrintf("--- Map連結 default ----\n");
  FLDMAPPER_Connect( FIELDMAP_GetFieldG3Dmapper( fieldWork ), mmatrix );

  MAP_MATRIX_Delete( mmatrix );
#else
  intcomm->connect_map_count = 0;
#endif
}

//==================================================================
/**
 * プロフィール受信人数分マップを連結
 *
 * @param   fieldWork		
 * @param   gameSys		
 * @param   intcomm		
 * 
 * wb -- col wb wb wb -
 */
//==================================================================
void IntrudeField_ConnectMap(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, INTRUDE_COMM_SYS_PTR intcomm)
{
  int use_num;
  
  if(intcomm == NULL || fieldWork == NULL || ZONEDATA_IsPalace(FIELDMAP_GetZoneID(fieldWork)) == FALSE){
    return;
  }
  
#if 0 //ループマップなのに1つも連結されていないのはチラつきの原因っぽいので
      //最低でも一つは連結しておく 2010.01.23(土)
  use_num = intcomm->member_num - 1;  // -1 = 自分の分は引く
#else
  use_num = intcomm->member_num - 1;
  if(use_num <= 0){
    use_num = 1;
  }
#endif
  if(intcomm->connect_map_count < use_num){
    MAP_MATRIX *mmatrix = MAP_MATRIX_Create( HEAPID_FIELDMAP );
    MAP_MATRIX_Init(mmatrix, NARC_map_matrix_palace02_mat_bin, ZONE_ID_PALACE01);

    do{
      OS_TPrintf("--- Map連結 %d ----\n", intcomm->connect_map_count + 1);
      FLDMAPPER_Connect( fieldWork, FIELDMAP_GetFieldG3Dmapper( fieldWork ), mmatrix );
      intcomm->connect_map_count++;
    }while(intcomm->connect_map_count < use_num);

    MAP_MATRIX_Delete( mmatrix );
  }
}

//==================================================================
/**
 * 自機を変装させる(OBJコードの変更)
 *
 * @param   intcomm		
 * @param   gsys		
 * @param   disguise_code      変装後のOBJCODE 
 *                  (DISGUISE_NO_NULLの場合は通常の姿、DISGUISE_NO_NORMALの場合はパレス時の標準姿)
 * @param   disguise_type      変装後のタイプ(TALK_TYPE_xxx)
 * @param   disguise_sex       変装後の性別
 */
//==================================================================
void IntrudeField_PlayerDisguise(INTRUDE_COMM_SYS_PTR intcomm, GAMESYS_WORK *gsys, u32 disguise_code, u8 disguise_type, u8 disguise_sex)
{
  FIELDMAP_WORK *fieldWork;
  FIELD_PLAYER *fld_player;
  int obj_code;
  
  if(GAMESYSTEM_CheckFieldMapWork( gsys ) == FALSE){
    return;
  }
  
  fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  intcomm->intrude_status_mine.disguise_no = disguise_code;
  intcomm->intrude_status_mine.disguise_type = disguise_type;
  intcomm->intrude_status_mine.disguise_sex = disguise_sex;
  intcomm->send_status = TRUE;

  obj_code = Intrude_GetObjCode(&intcomm->intrude_status_mine, 
    GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gsys)));
  FIELD_PLAYER_ChangeOBJCode( fld_player, obj_code );
  if(disguise_code == DISGUISE_NO_NULL){
    FIELD_PLAYER_ClearOBJCodeFix( fld_player );
  }
}

//--------------------------------------------------------------
/**
 * 隠しアイテムミッションが発動している時に目的地に到達したかチェック
 *
 * @param   gsys		
 * @param   intcomm		
 * @param   pcActor		
 *
 * @retval  MISSION_DATA *		到達した場合はミッションデータへのポインタ
 * @retval  MISSION_DATA *		到達していない場合はNULL
 */
//--------------------------------------------------------------
GMEVENT * IntrudeField_CheckSecretItemEvent(GAMESYS_WORK *gsys, INTRUDE_COMM_SYS_PTR intcomm, const FIELD_PLAYER *pcActor)
{
  if(MISSION_GetMissionEntry(&intcomm->mission) == TRUE
      && MISSION_GetMissionComplete(&intcomm->mission) == FALSE){
    const MISSION_DATA *mdata = MISSION_GetRecvData(&intcomm->mission);
    if(mdata != NULL && mdata->cdata.type == MISSION_TYPE_ITEM 
        && intcomm->intrude_status_mine.palace_area == mdata->target_info.net_id){
      const MISSION_TYPEDATA_ITEM *d_item = (void*)mdata->cdata.data;
      PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gsys );
      ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
    	const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( pcActor );
    	MMDL_GRIDPOS grid_pos;
    	
      MMDL_GetGridPos( fmmdl, &grid_pos );
      if(IntrudeSecretItemPosDataTbl[d_item->secret_pos_tblno].zone_id == zone_id
          && IntrudeSecretItemPosDataTbl[d_item->secret_pos_tblno].grid_x == grid_pos.gx
          && IntrudeSecretItemPosDataTbl[d_item->secret_pos_tblno].grid_y == grid_pos.gy
          && IntrudeSecretItemPosDataTbl[d_item->secret_pos_tblno].grid_z == grid_pos.gz){
        return EVENT_Intrude_SecretItemArrivalEvent(gsys, intcomm, mdata);
      }
    }
  }
  
  return NULL;
}

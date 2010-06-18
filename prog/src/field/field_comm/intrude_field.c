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
#include "intrude_work.h"
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
#include "msg/msg_invasion.h"
#include "fieldmap/map_matrix.naix"  //MATRIX_ID

#include "field/event_intrude.h"
#include "savedata/symbol_save.h"
#include "field/event_symbol.h"

#include "field/fldmmdl.h"
#include "../../../resource/fldmapdata/script/palace01_def.h"  //SCRID_～
#include "field/script_def.h"
#include "field/scrcmd.h"
#include "field/fieldmap_call.h"  //FIELDMAP_IsReady



//==============================================================================
//  定数定義
//==============================================================================
///パレス間移動が出来なかった場合のNG種類
typedef enum{
  PALACE_NG_TYPE_NOT_CONNECT,   //誰とも繋がっていない
  PALACE_NG_TYPE_MISSION_PLAY,  //ミッション中
}PALACE_NG_TYPE;


//==============================================================================
//  構造体定義
//==============================================================================
///通信終了
typedef struct{
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  FLDMSGWIN_STREAM *msgStream;
  FLDMENUFUNC *menufunc;
	INTRUDE_EVENT_DISGUISE_WORK iedw;
  u16 wait;
  u8 padding[2];
}EVENT_SIOEND_WARP;

///子機の通信終了
typedef struct{
  FIELDMAP_WORK *fieldmap;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  u16 wait;
  u8 padding[2];
}EVENT_SIOEND_CHILD;

///NGメッセージイベント用ワーク
typedef struct{
  FIELDMAP_WORK *fieldWork;
  MMDL *player_mmdl;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  PALACE_NG_TYPE ng_type;
  u16 key_dir;
  u16 player_dir;
}EVENT_PALACE_NGWIN;

///バリアーイベント用ワーク
typedef struct{
  FIELDMAP_WORK *fieldWork;
  MMDL *player_mmdl;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  WORDSET *wordset;
  u16 dir;
  u8 no_change;
  u8 padding;
  INTRUDE_EVENT_DISGUISE_WORK ev_diswork;
}EVENT_PALACE_BARRIER;

///ミッションターゲットにされたので表フィールドへ強制ワープ
typedef struct{
  GFL_MSGDATA *msgData;
  FLDMSGWIN_STREAM *msgStream;
	INTRUDE_EVENT_DISGUISE_WORK iedw;
}EVENT_MISSIONTARGET_WARP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT * EVENT_ChangeMapPosCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, INTRUDE_COMM_SYS_PTR intcomm );
static GMEVENT_RESULT EVENT_MapChangeCommEnd(GMEVENT * event, int *seq, void*work);
static GMEVENT * EVENT_ChildCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, INTRUDE_COMM_SYS_PTR intcomm);
static GMEVENT_RESULT EventChildCommEnd(GMEVENT * event, int *seq, void*work);
static void _PalaceMapCommBootCheck(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, GAME_COMM_SYS_PTR game_comm, FIELD_PLAYER *pcActor);
static void _PalaceFieldPlayerWarp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor, INTRUDE_COMM_SYS_PTR intcomm);
static GMEVENT * EVENT_PalaceNGWin( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_PLAYER *fld_player, u16 player_dir, u16 key_dir, PALACE_NG_TYPE ng_type);
static GMEVENT_RESULT _EventPalaceNGWinEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT * EVENT_PalaceBarrierMove( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_PLAYER *fld_player, u16 dir );
static GMEVENT_RESULT _EventPalaceBarrierMove( GMEVENT *event, int *seq, void *wk );
static void _ExitCallback_toInvasion(void *pWork, BOOL exit_pileup);
static void _InvasionCommBoot(GAMESYS_WORK *gsys);
static GMEVENT_RESULT EventMissionTargetWarp(GMEVENT * event, int *seq, void*work);


//==============================================================================
//  データ
//==============================================================================
///パレスへワープしてきたときの出現座標
static const VecFx32 PalaceWarpPos = {
  PALACE_MAP_LEN/2 - 1*FX32_ONE,
  32*FX32_ONE,
  488 * FX32_ONE,
};

//--------------------------------------------------------------
//  外部データ
//--------------------------------------------------------------
#include "palace_zone_setting.cdat"


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

  if(GAMEDATA_GetIsSave(gamedata) == TRUE){
    return;
  }

#if 0
  //デバッグ
  if(ZONEDATA_IsBingo(FIELDMAP_GetZoneID(fieldWork)) && (GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT)){
    DEBUG_INTRUDE_Pokemon_Add(fieldWork);
  }
#endif

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

  if(intcomm->comm_status != INTRUDE_COMM_STATUS_UPDATE || NetErr_App_CheckError()){
    return;
  }

  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);

  Intrude_UpdatePlayerStatusAll(intcomm);
  CommPlayer_Update(gameSys, intcomm->cps);
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
        if(intcomm->intrude_status[i].player_pack.vanish == FALSE
            && (intcomm->intrude_status[i].action_status == INTRUDE_ACTION_BATTLE
            || intcomm->intrude_status[i].action_status == INTRUDE_ACTION_BINGO_BATTLE)){
          CommPlayer_SetGyoeTask(intcomm->cps, i);
        }
      }
    }
  }
  
  _PalaceFieldPlayerWarp(fieldWork, gameSys, pcActor, intcomm);
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
  PLAYER_MOVE_FORM move_form;
  
  if(intcomm == NULL || intcomm->cps == NULL){
    return FALSE;
  }
  
  move_form = FIELD_PLAYER_GetMoveForm(fld_player);
  switch(move_form){
  case PLAYER_MOVE_FORM_NORMAL:
  case PLAYER_MOVE_FORM_CYCLE:
    break;
  default:
    return FALSE; //話しかけができないフォルム(通信相手の画面では非表示)
  }
  
  FIELD_PLAYER_GetFrontGridPos(fld_player, &check_gx, &check_gy, &check_gz);
  if(CommPlayer_SearchGridPos(intcomm->cps, check_gx, check_gz, &out_index) == TRUE){
    OS_TPrintf("Talkターゲット発見! net_id = %d, gx=%d, gz=%d\n", 
      out_index, check_gx, check_gz);
    if(intcomm->intrude_status[out_index].player_pack.vanish == FALSE){
      *hit_netid = out_index;
      return TRUE;
    }
  }
  
  return FALSE;
}

//==============================================================================
//
//  橋の特定位置まで来て、誰とも通信していないなら注意メッセージを出して自機を戻す
//
//==============================================================================
//--------------------------------------------------------------
/**
 * 橋の特定位置まで来て、誰とも通信していないなら注意メッセージを出して自機を戻す
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   fld_player		
 * @param   dir		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * EVENT_PalaceNGWin( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_PLAYER *fld_player, u16 player_dir, u16 key_dir, PALACE_NG_TYPE ng_type)
{
  EVENT_PALACE_NGWIN *ngwin;
  GMEVENT * event;
  
  event = GMEVENT_Create(gsys, NULL, _EventPalaceNGWinEvent, sizeof(EVENT_PALACE_NGWIN));
  
  ngwin = GMEVENT_GetEventWork(event);
  GFL_STD_MemClear( ngwin, sizeof(EVENT_PALACE_NGWIN) );
  
  ngwin->fieldWork = fieldWork;
  ngwin->player_mmdl = FIELD_PLAYER_GetMMdl(fld_player);
  ngwin->player_dir = player_dir;
  ngwin->key_dir = key_dir;
  ngwin->ng_type = ng_type;
  
  return event;
}

//--------------------------------------------------------------
/**
 * 橋の特定位置まで来て、誰とも通信していないなら注意メッセージを出して自機を戻す
 *
 * @param   event		
 * @param   seq		
 * @param   wk		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _EventPalaceNGWinEvent( GMEVENT *event, int *seq, void *wk )
{
  EVENT_PALACE_NGWIN *ngwin = wk;
  enum{
    _SEQ_DIR_CHECK,
    _SEQ_DIR_WAIT,
    _SEQ_NGWIN,
    _SEQ_NGWIN_STREAM_WAIT,
    _SEQ_NGWIN_KEY_WAIT,
    _SEQ_NGWIN_NEXT,
    _SEQ_NGWIN_NEXT_WAIT,
    _SEQ_NGWIN_NEXT_KEY_WAIT,
    _SEQ_NGWIN_CLOSE,
    _SEQ_BACK_STEP,
    _SEQ_BACK_STEP_WAIT,
  };
  
  switch(*seq){
  case _SEQ_DIR_CHECK:
    if(MMDL_CheckPossibleAcmd(ngwin->player_mmdl) == TRUE){
      u16 code = (ngwin->key_dir == DIR_RIGHT) ? AC_DIR_R : AC_DIR_L;
      if(MMDL_GetDirDisp(ngwin->player_mmdl) != ngwin->key_dir){
        MMDL_SetAcmd(ngwin->player_mmdl, code);
        (*seq)++;
      }
      else{
        *seq = _SEQ_NGWIN;
      }
    }
    break;
  case _SEQ_DIR_WAIT:
    if(MMDL_CheckEndAcmd(ngwin->player_mmdl) == TRUE){
      MMDL_EndAcmd(ngwin->player_mmdl);
      (*seq)++;
    }
    break;
  case _SEQ_NGWIN:
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( ngwin->fieldWork ) );
    {
      u32 msg_id;
      
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(ngwin->fieldWork);
      ngwin->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      ngwin->msgWin = FLDMSGWIN_AddTalkWin( msgBG, ngwin->msgData );
      if(ngwin->ng_type == PALACE_NG_TYPE_NOT_CONNECT){
        msg_id = plc_connect_06;
      }
      else{
        msg_id = plc_connect_03;
      }
      FLDMSGWIN_Print( ngwin->msgWin, 0, 0, msg_id );
      (*seq)++;
    }
    break;
  case _SEQ_NGWIN_STREAM_WAIT:
    if( FLDMSGWIN_CheckPrintTrans(ngwin->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case _SEQ_NGWIN_KEY_WAIT:
    if(ngwin->ng_type == PALACE_NG_TYPE_NOT_CONNECT){
      if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL) ){
        (*seq)++;
      }
    }
    else{
      if( GFL_UI_KEY_GetTrg() & EVENT_WAIT_LAST_KEY ){
        (*seq) = _SEQ_NGWIN_CLOSE;
      }
    }
    break;
  case _SEQ_NGWIN_NEXT:
    FLDMSGWIN_ClearWindow(ngwin->msgWin);
    FLDMSGWIN_Print( ngwin->msgWin, 0, 0, plc_connect_06_02 );
    (*seq)++;
    break;
  case _SEQ_NGWIN_NEXT_WAIT:
    if( FLDMSGWIN_CheckPrintTrans(ngwin->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case _SEQ_NGWIN_NEXT_KEY_WAIT:
    {
      if( GFL_UI_KEY_GetTrg() & EVENT_WAIT_LAST_KEY ){
        (*seq)++;
      }
    }
    break;
  case _SEQ_NGWIN_CLOSE:
    FLDMSGWIN_Delete( ngwin->msgWin );
    GFL_MSG_Delete( ngwin->msgData );
    (*seq)++;
    break;
  case _SEQ_BACK_STEP:
    if(MMDL_CheckPossibleAcmd(ngwin->player_mmdl) == TRUE){
      u16 code = (ngwin->key_dir == DIR_RIGHT) ? AC_WALK_L_16F : AC_WALK_R_16F;
      MMDL_SetAcmd(ngwin->player_mmdl, code);
      (*seq)++;
    }
    break;
  case _SEQ_BACK_STEP_WAIT:
    if(MMDL_CheckEndAcmd(ngwin->player_mmdl) == TRUE){
      MMDL_EndAcmd(ngwin->player_mmdl);
   	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(ngwin->fieldWork));
      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * イベント：バリアー移動
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   fld_player		
 * @param   dir		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * EVENT_PalaceBarrierMove( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_PLAYER *fld_player, u16 dir )
{
  EVENT_PALACE_BARRIER *barrier;
  GMEVENT * event;
  
  event = GMEVENT_Create(gsys, NULL, _EventPalaceBarrierMove, sizeof(EVENT_PALACE_BARRIER));
  
  barrier = GMEVENT_GetEventWork(event);
  GFL_STD_MemClear( barrier, sizeof(EVENT_PALACE_BARRIER) );
  
  barrier->fieldWork = fieldWork;
  barrier->player_mmdl = FIELD_PLAYER_GetMMdl(fld_player);
  barrier->dir = dir;
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント：バリアー移動
 *
 * @param   event		
 * @param   seq		
 * @param   wk		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _EventPalaceBarrierMove( GMEVENT *event, int *seq, void *wk )
{
  EVENT_PALACE_BARRIER *barrier = wk;
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  enum{
    _SEQ_PAUSE,
    _SEQ_INIT,
    _SEQ_INIT_WAIT,
    _SEQ_MSG_INIT,
    _SEQ_MSG_WAIT,
    _SEQ_MSG_LASTKEY_WAIT,
    _SEQ_FINISH,
  };
  
  switch(*seq){
  case _SEQ_PAUSE:
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( barrier->fieldWork ) );
    (*seq)++;
    break;
  case _SEQ_INIT:
    if(MMDL_CheckPossibleAcmd(barrier->player_mmdl) == FALSE){
      break;  
    }

    if(intcomm == NULL){  //変身前にエラーチェック：通信切断状態ならばここで終了
      *seq = _SEQ_FINISH;
      break;
    }

    // ミッション用の見た目
    if(MISSION_GetMissionPlay(&intcomm->mission) == TRUE)
    {  
      barrier->no_change = TRUE;

      // 変身なし演出
      IntrudeEvent_Sub_BarrierDisguiseEffectSetup(
        &barrier->ev_diswork, gsys, barrier->fieldWork, intcomm, DISGUISE_NO_NULL, 0, 0, barrier->dir, FALSE); //<-FALSEで変身なし
    }
    // 自分のエリアへ戻る
    else if( Intrude_CheckNextPalaceAreaMine( game_comm, gamedata, barrier->dir ) == TRUE ){ // 移動先が自分のエリアか？
      
      IntrudeEvent_Sub_BarrierDisguiseEffectSetup(
        &barrier->ev_diswork, gsys, barrier->fieldWork, intcomm, DISGUISE_NO_NULL, 0, 0, barrier->dir, TRUE);
    }
    // 通常進入
    else{

      u16 disguise_code;
      u8 disguise_type, disguise_sex;
      Intrude_GetNormalDisguiseObjCode(
        GAMEDATA_GetMyStatus(gamedata), &disguise_code, &disguise_type, &disguise_sex);

      IntrudeEvent_Sub_BarrierDisguiseEffectSetup(&barrier->ev_diswork, gsys, barrier->fieldWork, intcomm,
        disguise_code, disguise_type, disguise_sex, barrier->dir, TRUE);
    }
    (*seq)++;
    break;

  case _SEQ_INIT_WAIT:
    // BARRIER見た目変更処理
    if(IntrudeEvent_Sub_BarrierDisguiseEffectMain(&barrier->ev_diswork, intcomm) == TRUE){
      (*seq)++;
    }
    break;
  case _SEQ_MSG_INIT:
    if(intcomm == NULL){  //メッセージ前にエラーチェック：通信切断状態ならばここで終了
      *seq = _SEQ_FINISH;
      break;
    }
    
    {
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(barrier->fieldWork);
      barrier->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      barrier->msgWin = FLDMSGWIN_AddTalkWin( msgBG, barrier->msgData );
      barrier->wordset = WORDSET_Create(HEAPID_PROC);
      
      if(intcomm->intrude_status_mine.palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
        if(barrier->no_change == TRUE){
          FLDMSGWIN_Print( barrier->msgWin, 0, 0, plc_connect_03 );
        }
        else{
          FLDMSGWIN_Print( barrier->msgWin, 0, 0, plc_connect_02 );
        }
      }
      else{
        STRBUF *expand_buf = GFL_STR_CreateBuffer(256, HEAPID_FIELDMAP);
        STRBUF *src_buf;
        
        WORDSET_RegisterPlayerName( barrier->wordset, 0, 
          GAMEDATA_GetMyStatusPlayer(gamedata, intcomm->intrude_status_mine.palace_area) );
        
        if(barrier->no_change == TRUE){
          src_buf = GFL_MSG_CreateString( barrier->msgData, plc_connect_04 );
        }
        else{
          src_buf = GFL_MSG_CreateString( barrier->msgData, plc_connect_05 );
        }
        WORDSET_ExpandStr(barrier->wordset, expand_buf, src_buf);
        FLDMSGWIN_PrintStrBuf( barrier->msgWin, 0, 0, expand_buf );
        GFL_STR_DeleteBuffer(src_buf);
        GFL_STR_DeleteBuffer(expand_buf);
      }
      (*seq)++;
    }
    break;
  case _SEQ_MSG_WAIT:
    if( FLDMSGWIN_CheckPrintTrans(barrier->msgWin) == TRUE ){
      FIELD_PLACE_NAME_DisplayForce( 
        FIELDMAP_GetPlaceNameSys(barrier->fieldWork), FIELDMAP_GetZoneID(barrier->fieldWork) );
      (*seq)++;
    } 
    break;
  case _SEQ_MSG_LASTKEY_WAIT:
    {
      if( GFL_UI_KEY_GetTrg() & EVENT_WAIT_LAST_KEY ){
        (*seq)++;
      }
    }
    break;
  case _SEQ_FINISH:
    if(barrier->msgWin != NULL){
      FLDMSGWIN_Delete( barrier->msgWin );
      GFL_MSG_Delete( barrier->msgData );
      WORDSET_Delete(barrier->wordset);
    }
 	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(barrier->fieldWork));
    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}


//==================================================================
/**
 * パレス：座標イベント
 *
 * @param   fieldWork		
 * @param   gameSys		
 * @param   pcActor		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * Intrude_CheckPosEvent(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor)
{
  PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gameSys );
  ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
  VecFx32 pos;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gameSys);
  INTRUDE_COMM_SYS_PTR intcomm;
  int i;
  
  intcomm = Intrude_Check_CommConnect(game_comm);

#if 0
  if(intcomm != NULL && intcomm->comm_status == INTRUDE_COMM_STATUS_UPDATE){
    if(intcomm->exit_recv == TRUE){
      return EVENT_ChildCommEnd(gameSys, fieldWork, intcomm);
    }
  }
#endif

  
  
  //------------- ここから下はパレス時のみの処理 -----------------
  
  if(ZONEDATA_IsPalace(zone_id) == FALSE){
    return NULL;
  }

  FIELD_PLAYER_GetPos( pcActor, &pos );
  
  //表フィールドへワープ
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(pos.x >= FX32_CONST(504) + PALACE_MAP_LEN*i && pos.x <= FX32_CONST(504) + PALACE_MAP_LEN*i
        && pos.z >= FX32_CONST(712) && pos.z <= FX32_CONST(712)){
      return EVENT_ChangeMapPosCommEnd(gameSys, fieldWork, intcomm);
    }
  }
  //ビンゴマップへワープ
  {
    fx32 entrance_x = PALACE_MAP_SYMMAP_ENTRANCE_X;
    fx32 entrance_z = PALACE_MAP_SYMMAP_ENTRANCE_Z;
    for (i = 0; i < FIELD_COMM_MEMBER_MAX; i++, entrance_x += PALACE_MAP_LEN )
    {
      if ( pos.x >= entrance_x && pos.x < entrance_x + PALACE_MAP_SYMMAP_ENTRANCE_SX
          && pos.z == entrance_z )
      {
        //入った位置によって出現先もずらす
        VecFx32 warp_pos = { SYMBOL_MAP_DOWN_ENTRANCE_X, 0, SYMBOL_MAP_DOWN_ENTRANCE_Z };
        warp_pos.x += pos.x - entrance_x;
        return EVENT_SymbolMapWarp(gameSys, TRUE, &warp_pos, DIR_UP, SYMBOL_MAP_ID_ENTRANCE);
      }
    }
  }
  
#if 0
  {//自機の座標を監視し、誰とも通信していないのにパレスの橋の一定位置まで来たら
   //注意メッセージを出して引き返させる
    BOOL left_right;
    
    if(GFL_NET_GetConnectNum() <= 1 && ((pos.x <= 136 && pos.x >= 136-16) || (pos.x >= 872 && pos.x <= 872+16)) && (pos.z >= 472 && pos.z <= 536)){
      left_right = pos.x <= (PALACE_MAP_LEN/2 >> FX32_SHIFT) ? 0 : 1;
      return EVENT_PalaceNGWin( gameSys, fieldWork, pcActor, left_right );
    }
  }
#endif
  return NULL;
}

//==================================================================
/**
 * 侵入：キー入力起動イベント
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   pcActor		
 * @param   now_pos		    現在の座標
 * @param   front_pos		  進行方向の座標
 * @param   player_dir		自機の移動方向
 * @param   next_key_dir  キーの先読み
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * Intrude_CheckPushEvent(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_PLAYER *pcActor, const VecFx32 *now_pos, u16 player_dir, u16 next_key_dir)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  s32 now_gx;
  u16 event_dir = DIR_NOT;
  fx32 calc_x = now_pos->x;
  int member_num = 1;
  fx32 map_offset;
  
  if(intcomm != NULL){
    member_num = intcomm->member_num;
  }
  
  if(calc_x <= 0){
    calc_x = PALACE_MAP_LEN * member_num + calc_x;
  }
  else if(calc_x >= PALACE_MAP_LEN * member_num){
    calc_x -= PALACE_MAP_LEN * member_num;
  }
  
  now_gx = FX32_TO_GRID(calc_x) % PALACE_MAP_LEN_GRID;
  map_offset = calc_x % PALACE_MAP_LEN;
  
  //パレスマップの橋で左側へ行こうとしている && プレイヤーが真逆を向いていない
  if(next_key_dir == DIR_LEFT){
    //プレイヤーが真逆の方を向いている場合は半グリッド内に収まっていないと反応しない
    if(player_dir != DIR_RIGHT || (player_dir == DIR_RIGHT && map_offset <= FIELD_CONST_GRID_HALF_FX32_SIZE)){
      if(now_gx == 0){
        event_dir = DIR_LEFT;
      }
    }
  }
  else if(next_key_dir == DIR_RIGHT){
    //プレイヤーが真逆の方を向いている場合は半グリッド内に収まっていないと反応しない
    if(player_dir != DIR_LEFT || (player_dir == DIR_LEFT && map_offset >= PALACE_MAP_LEN - FIELD_CONST_GRID_HALF_FX32_SIZE)){
      if(now_gx == PALACE_MAP_LEN_GRID - 1){
        event_dir = DIR_RIGHT;
      }
    }
  }
  
  if(event_dir != DIR_NOT){
    if(intcomm == NULL && (NetErr_App_CheckError() || GameCommSys_GetLastStatus(game_comm) != GAME_COMM_LAST_STATUS_NULL)){
      return NULL;  //強制ワープイベントが発動するようにここではNULLを返す
    }
    if(intcomm == NULL || GFL_NET_GetConnectNum() <= 1 || intcomm->member_num < 2){
      return EVENT_PalaceNGWin(gsys, fieldWork, pcActor, player_dir, event_dir, PALACE_NG_TYPE_NOT_CONNECT);
    }
    else if(MISSION_GetMissionPlay(&intcomm->mission) == TRUE){
      return EVENT_PalaceNGWin(gsys, fieldWork, pcActor, player_dir, event_dir, PALACE_NG_TYPE_MISSION_PLAY);
    }
    else{
      return EVENT_PalaceBarrierMove(gsys, fieldWork, pcActor, event_dir);
    }
  }
  return NULL;
}


//==================================================================
/**
 * 通信を終了させてからマップ移動
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
static GMEVENT * EVENT_ChangeMapPosCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, INTRUDE_COMM_SYS_PTR intcomm )
{
  EVENT_SIOEND_WARP *dsw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeCommEnd, sizeof(EVENT_SIOEND_WARP));
	dsw = GMEVENT_GetEventWork(event);
	GFL_STD_MemClear(dsw, sizeof(EVENT_SIOEND_WARP));
	
	OS_TPrintf("親機：通信切断イベント起動\n");
	return event;
}

static GMEVENT_RESULT EVENT_MapChangeCommEnd(GMEVENT * event, int *seq, void*work)
{
	EVENT_SIOEND_WARP * dsw = work;
	GAMESYS_WORK  * gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  enum{
  	SEQ_INIT,
    SEQ_INIT_PRINT_WAIT,
  	SEQ_YESNO_WAIT,
  	SEQ_COMM_EXIT,
    SEQ_EXIT_PRINT_WAIT,
    SEQ_COMM_EXIT_WAIT,
    SEQ_COMM_EXIT_FINISH,
    SEQ_DISGUISE_INIT,
    SEQ_DISGUISE_MAIN,
  	SEQ_MAP_WARP,
  	SEQ_MAP_WARP_FINISH,
  	SEQ_EXIT_CANCEL,
    SEQ_PLAYER_MOVE_UP,
    SEQ_PLAYER_MOVE_UP_WAIT,
    SEQ_FINISH,
  };
	
	switch (*seq) {
	case SEQ_INIT:
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( fieldmap ) );
    
	  {
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldmap);
      dsw->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      dsw->msgStream = FLDMSGWIN_STREAM_AddTalkWin(msgBG, dsw->msgData );

      FLDMSGWIN_STREAM_PrintStart(dsw->msgStream, 0, 0, plc_connect_07);
      (*seq)++;
    }
	  break;
  case SEQ_INIT_PRINT_WAIT:
    if( FLDMSGWIN_STREAM_Print(dsw->msgStream) == TRUE){
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldmap);
      dsw->menufunc = FLDMENUFUNC_AddYesNoMenu(msgBG, FLDMENUFUNC_YESNO_YES);
      (*seq)++;
    }
    break;
	case SEQ_YESNO_WAIT:
	  {
  	  FLDMENUFUNC_YESNO fld_yesno = FLDMENUFUNC_ProcYesNoMenu( dsw->menufunc );
  	  if(fld_yesno == FLDMENUFUNC_YESNO_NULL){
        break;
      }
      
      FLDMENUFUNC_DeleteMenu(dsw->menufunc);
      FLDMSGWIN_STREAM_Delete(dsw->msgStream);
      dsw->menufunc = NULL;
      dsw->msgStream = NULL;
      if(fld_yesno == FLDMENUFUNC_YESNO_YES){
  	    *seq = SEQ_COMM_EXIT;
  	  }
  	  else{
  	    *seq = SEQ_EXIT_CANCEL;
  	  }
  	}
  	break;

	case SEQ_COMM_EXIT:
    if(NetErr_App_CheckError() == FALSE 
        && GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION){
      GameCommSys_ExitReq(game_comm); //通信終了リクエスト
      dsw->msgWin = FLDMSGWIN_AddTalkWin( FIELDMAP_GetFldMsgBG(fieldmap), dsw->msgData );
      FLDMSGWIN_Print( dsw->msgWin, 0, 0, msg_invasion_test06_01 );
    }
    (*seq)++;
    break;
  case SEQ_EXIT_PRINT_WAIT:
    if( dsw->msgWin == NULL || FLDMSGWIN_CheckPrintTrans(dsw->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case SEQ_COMM_EXIT_WAIT:
    dsw->wait++;
    if(dsw->msgWin == NULL || dsw->wait > 60){
      if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION){//INVASIONじゃない場合はFIELD_BEACONの場合があるのでINVASIONじゃないかどうかでチェック
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
  case SEQ_COMM_EXIT_FINISH:
    if(dsw->msgWin != NULL){
      FLDMSGWIN_Delete( dsw->msgWin );
    }
    GFL_MSG_Delete( dsw->msgData );
    (*seq)++;
    break;

  case SEQ_DISGUISE_INIT:  //変装をしていれば元に戻す
    {
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
      
      if(FIELDMAP_IsReady(fieldWork) == TRUE){
        FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        
        if(FIELD_PLAYER_CheckIllegalOBJCode( fld_player ) == FALSE){
          IntrudeEvent_Sub_DisguiseEffectSetup(&dsw->iedw, gsys, fieldWork, DISGUISE_NO_NULL, 0,0);
          (*seq)++;
        }
        else{
          *seq = SEQ_MAP_WARP;
        }
      }
    }
    break;
  case SEQ_DISGUISE_MAIN:
    if(IntrudeEvent_Sub_DisguiseEffectMain(&dsw->iedw, NULL) == TRUE){
      (*seq)++;
    }
    break;

	case SEQ_MAP_WARP:
  	GMEVENT_CallEvent(
  	  //event, EVENT_ChangeMapPos(gsys, dsw->fieldmap, dsw->zoneID, &dsw->pos, 0, FALSE));
      event, EVENT_ChangeMapFromPalace(gsys) );
    (*seq)++;
    break;
	case SEQ_MAP_WARP_FINISH:
    *seq = SEQ_FINISH;
    break;
	
	//ここから下はキャンセルした場合の処理
	case SEQ_EXIT_CANCEL:
    GFL_MSG_Delete( dsw->msgData );
    *seq = SEQ_PLAYER_MOVE_UP;
    break;
  case SEQ_PLAYER_MOVE_UP:
    {
      FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
      MMDL *player_mmdl = FIELD_PLAYER_GetMMdl(player);
      if(MMDL_CheckPossibleAcmd(player_mmdl) == TRUE){
        MMDL_SetAcmd(player_mmdl, AC_WALK_U_16F);
        (*seq)++;
      }
    }
    break;
  case SEQ_PLAYER_MOVE_UP_WAIT:
    {
      FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
      MMDL *player_mmdl = FIELD_PLAYER_GetMMdl(player);
      if(MMDL_EndAcmd(player_mmdl) == TRUE){ //アニメコマンド終了待ち
  	    *seq = SEQ_FINISH;
  	    break;
  	  }
  	}
	  break;
	
	case SEQ_FINISH:
 	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(fieldmap));
 	  return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 子機の通信終了
 *
 * @param   gsys		
 * @param   fieldmap		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static GMEVENT * EVENT_ChildCommEnd(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, INTRUDE_COMM_SYS_PTR intcomm)
{
  EVENT_SIOEND_CHILD *dsc;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EventChildCommEnd, sizeof(EVENT_SIOEND_CHILD));
	dsc = GMEVENT_GetEventWork(event);
	GFL_STD_MemClear(dsc, sizeof(EVENT_SIOEND_CHILD));
	dsc->fieldmap = fieldmap;
	
	OS_TPrintf("子機：通信切断イベント起動\n");
	return event;
}

static GMEVENT_RESULT EventChildCommEnd(GMEVENT * event, int *seq, void*work)
{
  EVENT_SIOEND_CHILD *dsc = work;
	GAMESYS_WORK  * gsys = GMEVENT_GetGameSysWork(event);
	FIELDMAP_WORK * fieldmap = dsc->fieldmap;
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	
	switch (*seq) {
	case 0:
    GameCommSys_ExitReq(game_comm); //通信終了リクエスト
	  if(GAMEDATA_GetIntrudeReverseArea(GAMESYSTEM_GetGameData(gsys)) == TRUE){
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(dsc->fieldmap);
      dsc->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      dsc->msgWin = FLDMSGWIN_AddTalkWin( msgBG, dsc->msgData );
      FLDMSGWIN_Print( dsc->msgWin, 0, 0, msg_invasion_test06_01 );
    }
    (*seq)++;
    break;
  case 1:
    if( dsc->msgWin == NULL || FLDMSGWIN_CheckPrintTrans(dsc->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 2:
    dsc->wait++;
    if(dsc->msgWin == NULL || dsc->wait > 60){
      if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_NULL){
      #if 0
        GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
        int i;
        PLAYER_WORK *plwork;
        VecFx32 pos = {0,0,0};
        
        for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
          plwork = GAMEDATA_GetPlayerWork( gamedata , i );
          PLAYERWORK_setPosition(plwork, &pos);
        }
      #endif
        (*seq)++;
      }
    }
    break;
  case 3:
    if(dsc->msgWin != NULL){
      FLDMSGWIN_Delete( dsc->msgWin );
      GFL_MSG_Delete( dsc->msgData );
    }
    (*seq)++;
    break;
	default:
		return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//==================================================================
/**
 * ミッションターゲットにされたので表フィールドへ強制ワープ
 *
 * @param   gsys		
 * @param   fieldmap		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_MissionTargetWarp(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap )
{
  EVENT_MISSIONTARGET_WARP *emtw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EventMissionTargetWarp, sizeof(EVENT_MISSIONTARGET_WARP));
	emtw = GMEVENT_GetEventWork(event);
	return event;
}

//--------------------------------------------------------------
/**
 * ミッションターゲットにされたので表フィールドへ強制ワープ
 *
 * @param   event		
 * @param   seq		
 * @param   work		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventMissionTargetWarp(GMEVENT * event, int *seq, void*work)
{
	EVENT_MISSIONTARGET_WARP * emtw = work;
	GAMESYS_WORK  * gsys = GMEVENT_GetGameSysWork(event);
	FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  enum{
    SEQ_INIT,
    SEQ_INIT_PRINT_WAIT,
    SEQ_DISGUISE_INIT,
    SEQ_DISGUISE_MAIN,
    SEQ_MAP_WARP,
    SEQ_MAP_WARP_FINISH,
  };
	
	switch (*seq) {
	case SEQ_INIT:
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( fieldmap ) );
	  {
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldmap);
      emtw->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
      emtw->msgStream = FLDMSGWIN_STREAM_AddTalkWin(msgBG, emtw->msgData );

      FLDMSGWIN_STREAM_PrintStart(emtw->msgStream, 0, 0, plc_target_warp);
      (*seq)++;
    }
	  break;
  case SEQ_INIT_PRINT_WAIT:
    if( FLDMSGWIN_STREAM_Print(emtw->msgStream) == TRUE){
      FLDMSGWIN_STREAM_Delete(emtw->msgStream);
      GFL_MSG_Delete( emtw->msgData );
      (*seq)++;
  	}
  	break;

  case SEQ_DISGUISE_INIT:  //変装をしていれば元に戻す
    {
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
      
      if(FIELDMAP_IsReady(fieldWork) == TRUE){
        FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        
        if(FIELD_PLAYER_CheckIllegalOBJCode( fld_player ) == FALSE){
          IntrudeEvent_Sub_DisguiseEffectSetup(&emtw->iedw, gsys, fieldWork, DISGUISE_NO_NULL, 0,0);
          (*seq)++;
        }
        else{
          *seq = SEQ_MAP_WARP;
        }
      }
    }
    break;
  case SEQ_DISGUISE_MAIN:
    if(IntrudeEvent_Sub_DisguiseEffectMain(&emtw->iedw, NULL) == TRUE){
      (*seq)++;
    }
    break;

	case SEQ_MAP_WARP:
  	GMEVENT_CallEvent( event, EVENT_ChangeMapFromPalace(gsys) );
    (*seq)++;
    break;
	case SEQ_MAP_WARP_FINISH:
	  if(fieldmap != NULL && FIELDMAP_IsReady(fieldmap) == TRUE){
  	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(fieldmap));
	  	return GMEVENT_RES_FINISH;
	  }
	  break;
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
 * パレス島の中心付近にいるか、外側にいるかを判定
 *
 * @param   player_pos		
 *
 * @retval  BOOL		    TRUE:中心の外側　FALSE:中心
 *
 * 連結マップの生成タイミングに使用
 * 隣接したマップが見えないタイミングで連結する
 */
//--------------------------------------------------------------
static BOOL _PlayerPosCheck_PalaceCenter(const VecFx32 *player_pos)
{
  if(player_pos->x > 0x308000 || player_pos->x < 0xe8000){
    return TRUE;  //パレス島の中心から外れている
  }
  return FALSE;   //
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

  if(GameCommSys_GetLastStatus(game_comm) != GAME_COMM_LAST_STATUS_NULL){
    return;
  }
  if(GameCommSys_CheckSystemWaiting(game_comm) == TRUE){
    return;
  }
  if(NetErr_App_CheckError() != NET_ERR_CHECK_NONE){
    return;
  }
  if((GFL_UI_CheckCoverOff() == TRUE && GFL_UI_CheckLowBatt() == TRUE)){
    return;
  }
  if(GAMESYSTEM_IsBatt10Sleep(gameSys) == TRUE){
    return;
  }

  if(GAMESYSTEM_IsEventExists(gameSys) == TRUE){
    //橋の上で切断イベントが起動しているとイベント中にも関わらず通信が起動して
    //いつまでも切断と判定できなかったりする
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
      _InvasionCommBoot(gameSys); //チュートリアルの時は常時通信が起動されないのでここで起動される
    }
    break;
  case GAME_COMM_NO_INVASION:
    //自分のパレス島にいて接続している相手もいない場合は通信終了
    if(_PlayerPosCheck_PalaceBridge(&player_pos) == FALSE
        && GFL_NET_SystemGetConnectNum() <= 1){
      INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(game_comm);
      if(intcomm->comm_status != INTRUDE_COMM_STATUS_BOOT_CHILD){ //子として親を探しに行っている場合は通信終了にはまだいかない
        GameCommSys_ExitReq(game_comm);
        OS_TPrintf("パレス通信終了\n");
      }
    }
    break;
  case GAME_COMM_NO_FIELD_BEACON_SEARCH:
    //自分のパレス島にいて橋の上にいるならビーコンサーチをOFFし、パレス通信へ
    if(_PlayerPosCheck_PalaceBridge(&player_pos) == TRUE){
      GameCommSys_ExitReqCallback(game_comm, _ExitCallback_toInvasion, gameSys);
      OS_TPrintf("常時通信終了：パレス通信へ\n");
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * 常時通信終了コールバック：侵入通信を起動
 *
 * @param   pWork		GAMESYS_WORK
 * @param   exit_pileup   TRUE:多重に終了リクエストが発生している
 */
//--------------------------------------------------------------
static void _ExitCallback_toInvasion(void *pWork, BOOL exit_pileup)
{
  GAMESYS_WORK *gsys = pWork;
  if(exit_pileup == FALSE){
    _InvasionCommBoot(gsys);
  }
}

//--------------------------------------------------------------
/**
 * 侵入通信を親として起動
 *
 * @param   gsys		
 *
 * 単純に通信を起動する部分を共通化目的で関数にしただけなので、
 * 起動してよいかのチェックなどは外側でやる事
 */
//--------------------------------------------------------------
static void _InvasionCommBoot(GAMESYS_WORK *gsys)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  FIELD_INVALID_PARENT_WORK *invalid_parent;
  
  if(NetErr_App_CheckError() || GAMESYSTEM_IsBatt10Sleep(gsys) == TRUE
      || (GFL_UI_CheckCoverOff() == TRUE && GFL_UI_CheckLowBatt() == TRUE)){
    return;
  }
  
  invalid_parent = GFL_HEAP_AllocClearMemory(
      GFL_HEAP_LOWID(HEAPID_APP_CONTROL), sizeof(FIELD_INVALID_PARENT_WORK));
  invalid_parent->my_invasion = TRUE;
  invalid_parent->gsys = gsys;
  invalid_parent->game_comm = game_comm;
  GameCommSys_Boot(game_comm, GAME_COMM_NO_INVASION, invalid_parent);

  OS_TPrintf("パレス通信起動：親\n");
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
  fx32 palace_area_calc_x;
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
  palace_area_calc_x = pos.x;
  
  now_area = intcomm->intrude_status_mine.palace_area;
  new_area = now_area;
  
  left_end = 0;
  right_end = PALACE_MAP_LEN * (intcomm->connect_map_count + 1);  //intcomm->member_num;
  
  //マップループチェック
  if(pos.x < left_end){// && player_dir == DIR_LEFT){
    new_pos.x = right_end - (left_end - pos.x);
    warp = TRUE;
    //OS_TPrintf("left warp now_pos.x = %d, new_pos.x = %d, left_end=%d, right_end=%d\n", pos.x>>FX32_SHIFT, new_pos.x>>FX32_SHIFT, left_end>>FX32_SHIFT, right_end>>FX32_SHIFT);
  }
  else if(pos.x > right_end){// && player_dir == DIR_RIGHT){
    new_pos.x = left_end + (pos.x - right_end);
    warp = TRUE;
    //OS_TPrintf("right warp now_pos.x = %d, new_pos.x = %d, left_end=%d, right_end=%d\n", pos.x>>FX32_SHIFT, new_pos.x>>FX32_SHIFT, left_end>>FX32_SHIFT, right_end>>FX32_SHIFT);
  }
  
  if(warp == TRUE){
    palace_area_calc_x = new_pos.x;
    if(GAMESYSTEM_IsEventExists(gameSys) == TRUE){
      //イベント起動中はまだワープさせない(バリアが見えなくなるので)
      new_pos = pos;
      warp = FALSE;
    }
  }
  
  //現在の座標からパレスエリアを判定
  new_area = palace_area_calc_x / PALACE_MAP_LEN;

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
    Intrude_SetMinePalaceArea(intcomm, new_area);
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
  //パレスマップに入った時点で1つは自動で連結しているが、
  //この処理を通る場合はIntrudeField_ConnectMapに連結数を任せる為、
  //0初期化する
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
  
  if(intcomm == NULL || fieldWork == NULL || FIELDMAP_GetFieldG3Dmapper(fieldWork) == NULL || 
      ZONEDATA_IsPalace(FIELDMAP_GetZoneID(fieldWork)) == FALSE){
    return;
  }
  
  use_num = intcomm->member_num - 1;
  if(use_num <= 0){
    use_num = 1;  //白黒マップを見せる為、最低1つは連結する
  }
  if(intcomm->palace_map_not_connect == TRUE){  //ロックがかかっている場合は強制的に1個
    use_num = 1;
  }
  
  if(intcomm->connect_map_count < use_num){
    MAP_MATRIX *mmatrix;
    
    //隣接するマップが見えている場合は連結処理はしない
    {
      VecFx32 player_pos;
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      if(fld_player == NULL){
        return; //IsReadyで上でチェックしているので通常ありえない
      }
      FIELD_PLAYER_GetPos( fld_player, &player_pos );
      player_pos.x %= PALACE_MAP_LEN;
      if(_PlayerPosCheck_PalaceCenter(&player_pos) == TRUE){
        return;
      }
    }

    mmatrix = MAP_MATRIX_Create( GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
    MAP_MATRIX_Init(mmatrix,
        NARC_map_matrix_palace02_mat_bin, ZONE_ID_PALACE01, GFL_HEAP_LOWID( HEAPID_FIELDMAP ) );

    do{
      OS_TPrintf("--- Map連結 %d ----\n", intcomm->connect_map_count + 1);
      FLDMAPPER_Connect( fieldWork, FIELDMAP_GetFieldG3Dmapper(fieldWork), mmatrix );
      intcomm->connect_map_count++;
    }while(intcomm->connect_map_count < use_num);

    MAP_MATRIX_Delete( mmatrix );
  }
}

//==================================================================
/**
 * マップ指定数連結
 *
 * @param   fieldWork		
 * @param   connect_num 連結する数
 */
//==================================================================
void IntrudeField_ConnectMapNum(FIELDMAP_WORK *fieldWork, int connect_num)
{
  MAP_MATRIX *mmatrix;
  
  if(connect_num >= FIELD_COMM_MEMBER_MAX){
    GF_ASSERT(0);
    connect_num = FIELD_COMM_MEMBER_MAX - 1;
  }
  
  mmatrix = MAP_MATRIX_Create( HEAPID_FIELDMAP );
  MAP_MATRIX_Init(mmatrix,
      NARC_map_matrix_palace02_mat_bin, ZONE_ID_PALACE01, GFL_HEAP_LOWID( HEAPID_FIELDMAP ));
  
  for( ; connect_num > 0; connect_num--){
    OS_TPrintf("--- Map連結 One ----\n");
    FLDMAPPER_Connect( fieldWork, FIELDMAP_GetFieldG3Dmapper( fieldWork ), mmatrix );
  }

  MAP_MATRIX_Delete( mmatrix );
}

//==================================================================
/**
 * 自機を変装させる(OBJコードの変更)
 *
 * @param   intcomm		         NULLを渡した場合は通常の姿に戻します
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
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  int obj_code;
  int sex;
  MYSTATUS *myst;
  VecFx32 offset_pos = {0,0,0};
  
  if(GAMESYSTEM_CheckFieldMapWork( gsys ) == FALSE){
    return;
  }
  
  myst = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gsys));
  sex = MyStatus_GetMySex(myst);
  
  if(intcomm != NULL){
    if(disguise_code == DISGUISE_NO_NORMAL 
        && GAMEDATA_GetIntrudeMyID(gamedata) == intcomm->intrude_status_mine.palace_area){
      disguise_code = DISGUISE_NO_NULL; //自分のパレスにいる場合はNORMALはNULLにする
    }
    
    if(disguise_code == DISGUISE_NO_NULL){
      disguise_type = sex == PM_MALE ? TALK_TYPE_MAN : TALK_TYPE_WOMAN;
      disguise_sex = sex;
    }

    intcomm->intrude_status_mine.disguise_no = disguise_code;
    intcomm->intrude_status_mine.disguise_type = disguise_type;
    intcomm->intrude_status_mine.disguise_sex = disguise_sex;
    intcomm->send_status = TRUE;

    obj_code = Intrude_GetObjCode(&intcomm->intrude_status_mine, myst);
  }
  else{
    disguise_code = DISGUISE_NO_NULL;
    obj_code = sex == PM_MALE ? HERO : HEROINE;
  }

  fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  FIELD_PLAYER_ChangeOBJCode( fld_player, obj_code );
  //連れ歩きポケモンはdraw_offsetをずらしているので変身後は初期化する
  MMDL_SetVectorDrawOffsetPos( FIELD_PLAYER_GetMMdl(fld_player), &offset_pos );
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
        && intcomm->intrude_status_mine.palace_area == mdata->target_info.net_id
		    && MISSION_GetAchieveAnswer(intcomm, &intcomm->mission) == MISSION_ACHIEVE_NULL){
      const MISSION_TYPEDATA_ITEM *d_item = (void*)mdata->cdata.data;
      PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gsys );
      ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
    	const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( pcActor );
    	MMDL_GRIDPOS grid_pos;
    	
      MMDL_GetGridPos( fmmdl, &grid_pos );
      if(IntrudeSecretItemReverseZoneTbl[d_item->secret_pos_tblno] == zone_id
          && IntrudeSecretItemPosDataTbl[d_item->secret_pos_tblno].grid_x == grid_pos.gx
          && IntrudeSecretItemPosDataTbl[d_item->secret_pos_tblno].grid_y == grid_pos.gy
          && IntrudeSecretItemPosDataTbl[d_item->secret_pos_tblno].grid_z == grid_pos.gz){
        return EVENT_Intrude_SecretItemArrivalEvent(gsys, intcomm, mdata);
      }
    }
  }
  
  return NULL;
}

//==================================================================
/**
 * ゾーンIDと一致するパレス設定データへのポインタを取得する
 *
 * @param   zone_id		
 *
 * @retval  const PALACE_ZONE_SETTING *		一致するデータが無い場合はNULL
 */
//==================================================================
const PALACE_ZONE_SETTING * IntrudeField_GetZoneSettingData(u16 zone_id)
{
  int i;
  
  for(i = 0; i < NELEMS(PalaceZoneSetting); i++){
    if(PalaceZoneSetting[i].zone_id == zone_id){
      return &PalaceZoneSetting[i];
    }
  }
  return NULL;
}

//==================================================================
/**
 * パレスタウンへワープする為の座標取得
 *
 * @param   zone_id		ワープ先ゾーンID
 * @param   vec		    ワープ先座標代入先
 *
 * @retval  BOOL		  TRUE:正常終了　FALSE:パレスに関連するゾーンIDではない為、データ未取得
 */
//==================================================================
BOOL IntrudeField_GetPalaceTownZoneID(u16 zone_id, VecFx32 *vec)
{
  if(ZONEDATA_IsPalace(zone_id) != TRUE){
    const PALACE_ZONE_SETTING *zonesetting = IntrudeField_GetZoneSettingData(zone_id);
    if(zonesetting == NULL){
      GF_ASSERT_MSG(0, "zone_id = %d", zone_id);
      return FALSE;
    }
    VEC_Set(vec, GRID_SIZE_FX32(zonesetting->warp_grid_x), 
      GRID_SIZE_FX32(zonesetting->warp_grid_y), GRID_SIZE_FX32(zonesetting->warp_grid_z));
  }
  else{
    *vec = PalaceWarpPos;
  }
  return TRUE;
}

//==================================================================
/**
 * モノリスステータスを初期化する
 *
 * @param   monost		
 */
//==================================================================
void IntrudeField_MonolithStatus_Init(MONOLITH_STATUS *monost)
{
  GFL_STD_MemClear(monost, sizeof(MONOLITH_STATUS));
}

//==================================================================
/**
 * モノリスステータスにデータが入っているかを確かめる
 *
 * @param   monost		
 * @retval  BOOL		TRUE:入っている　FALSE:入っていない
 */
//==================================================================
BOOL IntrudeField_MonolithStatus_CheckOcc(const MONOLITH_STATUS *monost)
{
  return monost->occ;
}

//--------------------------------------------------------------
// 通信相手のパレスにいる爺さんや4姉妹の動作モデルヘッダー
//--------------------------------------------------------------
static const MMDL_HEADER data_MMdlHeader =
{
	100,	          ///<識別ID  fldmmdl_code.h
	                //  スクリプト上でX番の動作モデルをアニメ、といった判別に使用。
	                //  evファイル上で被らなければよい。
	OLDMAN1,      	///<表示するOBJコード fldmmdl_code.h
	MV_DOWN,	      ///<動作コード    
	EV_TYPE_NORMAL,	///<イベントタイプ
	0,	            ///<イベントフラグ  タイプ毎に使用されるフラグ。外側から取得する
	SCRID_PALACE01_OLDMAN1_01,///<イベントID 話しかけた時に起動するスクリプトID(*.ev の _EVENT_DATAの番号)
	DIR_DOWN,	      ///<指定方向  グリッド移動ならばDIR系 ノングリッドの場合は違う値
	0,	            ///<指定パラメタ 0  タイプ毎に使用されるパラメタ。外側から取得する
	0,            	///<指定パラメタ 1
	0,	            ///<指定パラメタ 2
	MOVE_LIMIT_NOT,	///<X方向移動制限
	MOVE_LIMIT_NOT,	///<Z方向移動制限
  MMDL_HEADER_POSTYPE_GRID,
  {0},            ///<ポジションバッファ (グリッドマップの場合はここはMMDL_HEADER_GRIDPOS)
};


//--------------------------------------------------------------
/**
 * 動作モデルをAdd
 *
 * @param   fieldWork		
 * @param   grid_x		グリッド座標X
 * @param   grid_z		グリッド座標Z
 * @param   fx_y		  Y座標(fx32)
 * @param   objcode		  OBJCODE
 * @param   event_id		イベントID
 * @param   id		      識別ID
 */
//--------------------------------------------------------------
static void IntrudeField_AddMMdl(FIELDMAP_WORK *fieldWork, u16 grid_x, u16 grid_z, fx32 fx_y, u16 objcode, u16 event_id, u16 id)
{
  MMDL_HEADER head;
  MMDL_HEADER_GRIDPOS *grid_pos;
  MMDLSYS *mmdl_sys;
  ZONEID zone_id;
  
  mmdl_sys = FIELDMAP_GetMMdlSys( fieldWork );
  zone_id = FIELDMAP_GetZoneID( fieldWork );
  
  head = data_MMdlHeader;
  head.id = id;
  head.obj_code = objcode;
  head.event_id = event_id;
  grid_pos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
  
  grid_pos->gx = grid_x;
  grid_pos->gz = grid_z;
  grid_pos->y = fx_y;
  
  MMDLSYS_AddMMdl(mmdl_sys, &head, zone_id);
}

//==================================================================
/**
 * 通信相手のパレスにいる爺さんや4姉妹の動作モデルをまとめて全てAdd
 *
 * @param   fieldWork		
 */
//==================================================================
void IntrudeField_PalaceMMdlAllAdd(FIELDMAP_WORK *fieldWork)
{
  int i, comm_no;
  static const struct{
    u16 objcode;
    u16 grid_x;
    u16 grid_z;
    u16 event_id;
    u16 obj_id;
  }PalaceMmdlData[] = {
    {OLDMAN1, 29, 30, SCRID_PALACE01_OLDMAN1_02, OBJID_PALACE_OLDMAN1},
//    {GIRL4, 14, 29, SCRID_PALACE01_GIRL4_01},
//    {GIRL4, 48, 29, SCRID_PALACE01_GIRL4_02},
  };
  
  for(comm_no = 1; comm_no < FIELD_COMM_MEMBER_MAX; comm_no++){
    for(i = 0; i < NELEMS(PalaceMmdlData); i++){
      IntrudeField_AddMMdl(fieldWork, 
        PalaceMmdlData[i].grid_x + PALACE_MAP_LEN_GRID * comm_no, PalaceMmdlData[i].grid_z, 
        0x2001f, PalaceMmdlData[i].objcode, PalaceMmdlData[i].event_id, 
        PalaceMmdlData[i].obj_id + comm_no-1);
    }
  }
}

//==================================================================
/**
 * 侵入先の通信相手が切断状態になっていないかチェックする
 *
 * @param   gsys		
 * @param   zone_id		現在地
 *
 * @retval  BOOL		TRUE:切断状態　FALSE:接続されている
 * 
 * 通信エラーの時もTRUEが返ります
 */
//==================================================================
BOOL IntrudeField_CheckIntrudeShutdown(GAMESYS_WORK *gsys, u16 zone_id)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
    return FALSE; //表フィールドにいるから関係ない
  }
  
  if(intcomm != NULL){
    return FALSE;  //intcommがあるので通信は確立している
  }
  
  //裏フィールドにいてintcommがない
  
  if(GAMESYSTEM_IsBatt10Sleep(gsys) == TRUE){ //スリープによる切断
    return TRUE;
  }
  
  // 1.自分のパレス or シンボルマップに来ただけ
  // 2.通信エラーによりintcommが既に解放済み
  if(ZONEDATA_IsPalace(zone_id) == TRUE){
    if(GameCommSys_GetLastStatus(game_comm) != GAME_COMM_LAST_STATUS_NULL){
      return TRUE;
    }
    return FALSE;
  }
  else if(ZONEDATA_IsBingo(zone_id) == TRUE){
    if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION
        || (GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_NULL
        && GameCommSys_GetLastCommNo(game_comm) == GAME_COMM_NO_INVASION 
        && GameCommSys_GetLastStatus(game_comm) != GAME_COMM_LAST_STATUS_NULL)){
      return TRUE;
    }
    return FALSE;
  }
  else{
    //intcommが無いのにパレスでもシンボルマップでもない裏フィールドにいるなら強制退出
    return TRUE;
  }
}

//==================================================================
/**
 * 通信相手がチュートリアル or 自分がミッションターゲットにされている、かを調べる
 *
 * @param   intcomm		
 *
 * @retval  BOOL		  どちらか片方だけでも満たせばTRUE
 */
//==================================================================
BOOL IntrudeField_Check_Tutorial_OR_TargetMine(INTRUDE_COMM_SYS_PTR intcomm)
{
  if(intcomm->member_is_tutorial == TRUE || MISSION_CheckTargetIsMine(intcomm) == TRUE){
    return TRUE;
  }
  return FALSE;
}


#ifdef PM_DEBUG
GMEVENT * DEBUG_EVENT_PalaceBarrierMove(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_PLAYER *fld_player, u16 dir)
{
  return EVENT_PalaceBarrierMove( gsys, fieldWork, fld_player, dir );
}

#endif

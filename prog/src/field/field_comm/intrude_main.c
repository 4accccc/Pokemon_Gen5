//==============================================================================
/**
 * @file    intrude_main.c
 * @brief   侵入メイン：常駐に配置
 * @author  matsuda
 * @date    2009.09.03(木)
 *
 * 通信が起動中、常に実行され続けるメインです
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "savedata/mystatus.h"
#include "system/net_err.h"
#include "net/network_define.h"
#include "field/intrude_comm.h"
#include "intrude_comm_command.h"
#include "net_app/union/comm_player.h"
#include "intrude_types.h"
#include "intrude_comm_command.h"
#include "field/field_player.h"
#include "intrude_main.h"
#include "fieldmap/zone_id.h"
#include "field/zonedata.h"
#include "intrude_field.h"
#include "field/field_const.h"
#include "intrude_mission.h"
#include "poke_tool/monsno_def.h"
#include "field/field_status_local.h"  // for FIELD_STATUS_
#include "intrude_work.h"
#include "savedata/symbol_save_notwifi.h"
#include "savedata/intrude_save.h"
#include "item/itemsym.h"


//==============================================================================
//  定数定義
//==============================================================================
///プロフィール要求リクエストを再度かけるまでのウェイト
#define INTRUDE_PROFILE_REQ_RETRY_WAIT    (30)

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void Intrude_CheckFieldProcAction(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_CheckProfileReq(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_CheckLeavePlayer(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_CheckTalkAnswerNG(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_CheckWfbcReq(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_CheckSymbolReq(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_CheckMonolithStatusReq(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_ConvertPlayerPos(INTRUDE_COMM_SYS_PTR intcomm, ZONEID mine_zone_id, fx32 mine_x, INTRUDE_STATUS *target);
static int Intrude_GetPalaceOffsetNo(const INTRUDE_COMM_SYS_PTR intcomm, int palace_area);
static void _SendBufferCreate_SymbolData(INTRUDE_COMM_SYS_PTR intcomm,const SYMBOL_DATA_REQ *p_sdr);
static void Intrude_UpdatePlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, NetID net_id);
static void Intrude_CancelCheckTalkedEventReserve(INTRUDE_COMM_SYS_PTR intcomm);

//==============================================================================
//  データ
//==============================================================================
///街の座標データ   ※テーブル数の増減をしたら必ずPALACE_TOWN_DATA_MAXの数も変えること！
const PALACE_TOWN_DATA PalaceTownData[] = {
  {
    ZONE_ID_C07, 
    ZONE_ID_PLC07, 
    9*8, 7*8,
  },
  {
    ZONE_ID_C08, 
    ZONE_ID_PLC08, 
    0x10*8, 7*8,
  },
  {
    ZONE_ID_T03, 
    ZONE_ID_PLT03, 
    0x17*8, 7*8,
  },
  {
    ZONE_ID_C06, 
    ZONE_ID_PLC06, 
    5*8, 0xb*8+4,
  },
  {
    ZONE_ID_T04, 
    ZONE_ID_PLT04, 
    0x1b*8, 0xb*8+4,
  },
  {
    ZONE_ID_C05, 
    ZONE_ID_PLC05, 
    9*8, 0x10*8,
  },
  {
    ZONE_ID_C04, 
    ZONE_ID_PLC04, 
    0x10*8, 0x10*8,
  },
  {
    ZONE_ID_BC10,
    ZONE_ID_PLC10, 
    0x17*8, 0x10*8,
  },
};
SDK_COMPILER_ASSERT(NELEMS(PalaceTownData) == PALACE_TOWN_DATA_MAX);

//==============================================================================
//  外部データ
//==============================================================================
#include "mission_ng_check.cdat"


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 侵入メインループ
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_Main(INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);

  //参加人数が変わっているなら人数を送信(親機専用)
  if(GFL_NET_IsParentMachine() == TRUE){
    int now_member = GFL_NET_GetConnectNum();
    if(now_member > intcomm->member_num){
      intcomm->member_num = now_member;
      intcomm->member_send_req = TRUE;
      GAMEDATA_SetIntrudeNum(gamedata, intcomm->member_num);
    }
  }
  if(intcomm->member_send_req == TRUE){
    if(IntrudeSend_MemberNum(intcomm, intcomm->member_num) == TRUE){
      intcomm->member_send_req = FALSE;
    }
  }

  //フィールドステータスのPROCアクションを反映
  Intrude_CheckFieldProcAction(intcomm);
  
  //話しかけられたイベントの予約キャンセルを監視
  Intrude_CancelCheckTalkedEventReserve(intcomm);

  //プロフィール要求リクエストを受けているなら送信
  if(intcomm->profile_req == TRUE){
    Intrude_SetSendProfileBuffer(intcomm);  //送信バッファに現在のデータをセット
    IntrudeSend_Profile(intcomm);           //送信
  }
  //通信が確立されているメンバーでプロフィールを持っていないメンバーがいるならリクエストをかける
  Intrude_CheckProfileReq(intcomm);
  //離脱者チェック
  Intrude_CheckLeavePlayer(intcomm);
  
  //話しかけお断りの返事が貯まっているなら返事を送信する
  Intrude_CheckTalkAnswerNG(intcomm);
  //占拠結果送信
  if(intcomm->send_occupy_result_send_req == TRUE){
    if(IntrudeSend_OccupyResult(intcomm) == TRUE){
      intcomm->send_occupy_result_send_req = FALSE;
    }
  }
  //ミッション更新処理
  MISSION_Update(intcomm, &intcomm->mission);
  //占拠情報送信リクエストがあれば送信
  if(intcomm->send_occupy == TRUE){
    if(IntrudeSend_OccupyInfo(intcomm) == TRUE){
      intcomm->send_occupy = FALSE;
    }
  }
  //WFBC送信リクエストがあれば送信
  Intrude_CheckWfbcReq(intcomm);
  //シンボルエンカウント要求
  Intrude_CheckSymbolReq(intcomm);
  //モノリスステータス要求
  Intrude_CheckMonolithStatusReq(intcomm);
  
  //プレイヤーステータスステータス更新
  if(Intrude_SetSendStatus(intcomm) == TRUE){
    intcomm->send_status = TRUE;
  }

  //プレイヤーステータス送信
  if(intcomm->send_status == TRUE && intcomm->send_my_position_stop == FALSE){
    if(IntrudeSend_PlayerStatus(intcomm, &intcomm->intrude_status_mine) == TRUE){
      intcomm->send_status = FALSE;
    }
  }
  
  //セーブ関連のデータ更新処理
  if(GAMEDATA_GetIsSave(gamedata) == FALSE){
    if(intcomm->recv_secret_item_flag == TRUE){
      INTRUDE_SAVE_WORK *intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork(gamedata) );
      if(Intrude_CheckNG_Item(intcomm->recv_secret_item.item) == FALSE){
        intcomm->recv_secret_item.item = ITEM_KIZUGUSURI;
      }
      ISC_SAVE_SetItem(intsave, &intcomm->recv_secret_item);
      intcomm->recv_secret_item_flag = FALSE;
    }
  }
}

//--------------------------------------------------------------
/**
 * フィールドステータスのPROCアクションの変更を反映する
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void Intrude_CheckFieldProcAction(INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  FIELD_STATUS * fldstatus = GAMEDATA_GetFieldStatus(gamedata);
  PROC_ACTION proc_action = FIELD_STATUS_GetProcAction( fldstatus );

  switch(proc_action){
  case PROC_ACTION_FIELD:
    if(intcomm->intrude_status_mine.action_status == INTRUDE_ACTION_BATTLE){
      Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_FIELD);
    }
    break;
  case PROC_ACTION_BATTLE:
    if(intcomm->intrude_status_mine.action_status == INTRUDE_ACTION_FIELD){
      Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_BATTLE);
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * 通信が確立されているメンバーでプロフィールを持っていないメンバーがいるならリクエストをかける
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void Intrude_CheckProfileReq(INTRUDE_COMM_SYS_PTR intcomm)
{
  int net_id;
  
  if(intcomm->profile_req_wait > 0){
    intcomm->profile_req_wait--;
    return;
  }
  
  for(net_id = 0; net_id < intcomm->member_num; net_id++){
    if(GFL_NET_IsConnectMember(net_id) == TRUE 
        && (intcomm->recv_profile & (1 << net_id)) == 0){
      OS_TPrintf("%d番のプロフィールが無い\n", net_id);
      IntrudeSend_ProfileReq();
      intcomm->profile_req_wait = INTRUDE_PROFILE_REQ_RETRY_WAIT;
      return;
    }
  }
}

//--------------------------------------------------------------
/**
 * 離脱者がいればプロフィール削除命令を送信
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void Intrude_CheckLeavePlayer(INTRUDE_COMM_SYS_PTR intcomm)
{
  int net_id;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  for(net_id = 0; net_id < intcomm->member_num; net_id++){
    if(GFL_NET_IsConnectMember(net_id) == FALSE 
        && (intcomm->recv_profile & (1 << net_id))){
      IntrudeSend_DeleteProfile(intcomm, net_id);
    }
  }
}

//--------------------------------------------------------------
/**
 * 話しかけNGの返事が発生していれば送信を行う
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void Intrude_CheckTalkAnswerNG(INTRUDE_COMM_SYS_PTR intcomm)
{
  int net_id;
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intcomm->answer_talk_ng[net_id] != TALK_RAND_NULL){
      if(GFL_NET_IsConnectMember(net_id) == TRUE){
        if(IntrudeSend_TalkAnswer(
            intcomm, net_id, INTRUDE_TALK_STATUS_NG, intcomm->answer_talk_ng[net_id]) == TRUE){
          intcomm->answer_talk_ng[net_id] = TALK_RAND_NULL;
        }
      }
      else{
        intcomm->answer_talk_ng[net_id] = TALK_RAND_NULL;
      }
    }
  }

  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intcomm->send_talk_rand_disagreement[net_id] != TALK_RAND_NULL){
      if(GFL_NET_IsConnectMember(net_id) == TRUE){
        if(IntrudeSend_TalkRandDisagreement(
            intcomm, net_id, intcomm->send_talk_rand_disagreement[net_id]) == TRUE){
          intcomm->send_talk_rand_disagreement[net_id] = TALK_RAND_NULL;
        }
      }
      else{
        intcomm->send_talk_rand_disagreement[net_id] = TALK_RAND_NULL;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * WFBC送信リクエストがあれば送信
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void Intrude_CheckWfbcReq(INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  FIELD_WFBC_COMM_NPC_ANS npc_ans;
  FIELD_WFBC_COMM_NPC_REQ npc_req;
  NetID net_id;
  
  if(intcomm->wfbc_req != 0){
    if(IntrudeSend_Wfbc(intcomm, intcomm->wfbc_req, GAMEDATA_GetMyWFBCCoreData(gamedata)) == TRUE){
      intcomm->wfbc_req = 0;
    }
  }
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(FIELD_WFBC_COMM_DATA_GetSendCommAnsData( 
        &intcomm->wfbc_comm_data, net_id, &npc_ans ) == TRUE){
      if(IntrudeSend_WfbcNpcAns(&npc_ans, net_id) == TRUE){
        FIELD_WFBC_COMM_DATA_ClearSendCommAnsData(&intcomm->wfbc_comm_data, net_id);
      }
    }
  }
  
  if(FIELD_WFBC_COMM_DATA_GetSendCommReqData(&intcomm->wfbc_comm_data, &npc_req, &net_id) == TRUE){
    if(IntrudeSend_WfbcNpcReq(&npc_req, net_id) == TRUE){
      FIELD_WFBC_COMM_DATA_ClearSendCommReqData(&intcomm->wfbc_comm_data);
    }
  }
}

//--------------------------------------------------------------
/**
 * シンボルデータ要求リクエストがあれば送信
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void Intrude_CheckSymbolReq(INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  FIELD_WFBC_COMM_NPC_ANS npc_ans;
  FIELD_WFBC_COMM_NPC_REQ npc_req;
  NetID net_id;
  
  if(intcomm->wfbc_req != 0){
    if(IntrudeSend_Wfbc(intcomm, intcomm->wfbc_req, GAMEDATA_GetMyWFBCCoreData(gamedata)) == TRUE){
      intcomm->wfbc_req = 0;
    }
  }
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intcomm->req_symbol_data[net_id].occ == TRUE){
      _SendBufferCreate_SymbolData(intcomm, &intcomm->req_symbol_data[net_id]);
      if(IntrudeSend_SymbolData(intcomm, net_id) == TRUE){
        intcomm->req_symbol_data[net_id].occ = FALSE;
      }
    }
  }
  
  if(intcomm->send_symbol_change.occ == TRUE){
    if(IntrudeSend_SymbolDataChange(&intcomm->send_symbol_change) == TRUE){
      intcomm->send_symbol_change.occ = FALSE;
    }
  }
}

//--------------------------------------------------------------
/**
 * 送信バッファ作成：シンボルエンカウントデータ
 *
 * @param   intcomm		
 * @param   p_sdr		  要求されているデータ
 */
//--------------------------------------------------------------
static void _SendBufferCreate_SymbolData(INTRUDE_COMM_SYS_PTR intcomm,const SYMBOL_DATA_REQ *p_sdr)
{
  INTRUDE_SYMBOL_WORK *sendbuf = &intcomm->intrude_send_symbol;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);
  u8 occ_num;
  
  SymbolSave_GetMapIDSymbolPokemon(
    symbol_save, sendbuf->spoke_array, SYMBOL_MAP_STOCK_MAX, p_sdr->symbol_map_id, &occ_num);
  
  sendbuf->num = occ_num;
  sendbuf->map_level_small = SymbolSave_GetMapLevelSmall(symbol_save);
  sendbuf->map_level_large = SymbolSave_GetMapLevelLarge(symbol_save);
  sendbuf->net_id = GFL_NET_SystemGetCurrentID();
  sendbuf->symbol_map_id = p_sdr->symbol_map_id;
}

//--------------------------------------------------------------
/**
 * モノリスステータス要求リクエストがあれば送信
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void Intrude_CheckMonolithStatusReq(INTRUDE_COMM_SYS_PTR intcomm)
{
  if(intcomm->monost_req > 0){
    if(IntrudeSend_MonolithStatus(intcomm, intcomm->monost_req) == TRUE){
      intcomm->monost_req = FALSE;
    }
  }
}

//==============================================================================
//  アクセス関数
//==============================================================================
//==================================================================
/**
 * アクションステータスをセット
 *
 * @param   intcomm		
 * @param   action		
 */
//==================================================================
void Intrude_SetActionStatus(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_ACTION action)
{
  intcomm->intrude_status_mine.action_status = action;
  intcomm->send_status = TRUE;
  OS_TPrintf("set intrude action status = %d\n", action);
}

//==================================================================
/**
 * プロフィール送信バッファに現在のデータをセット
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_SetSendProfileBuffer(INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  MYSTATUS *myst;
  OCCUPY_INFO *occupy;
  
  myst = GAMEDATA_GetMyStatus(gamedata);
  GFL_STD_MemCopy(myst, &intcomm->my_profile.mystatus, MyStatus_GetWorkSize());
  
  occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  GFL_STD_MemCopy(occupy, &intcomm->my_profile.occupy, sizeof(OCCUPY_INFO));
  
  //侵入ステータスをセット
  Intrude_SetSendStatus(intcomm);
  //プロフィール送信時でも自分のパレス番号が入っていない場合はセット
  if(intcomm->intrude_status_mine.palace_area == PALACE_AREA_NO_NULL){
    Intrude_SetMinePalaceArea(intcomm, GFL_NET_SystemGetCurrentID());
  }
  intcomm->my_profile.status = intcomm->intrude_status_mine;
}

//==================================================================
/**
 * プロフィールデータを受信バッファにセット
 *
 * @param   intcomm		
 * @param   net_id		
 * @param   profile		
 */
//==================================================================
void Intrude_SetProfile(INTRUDE_COMM_SYS_PTR intcomm, int net_id, const INTRUDE_PROFILE *profile)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  MYSTATUS *myst;
  OCCUPY_INFO *occupy;
  BOOL new_player = FALSE;
  
  myst = GAMEDATA_GetMyStatusPlayer(gamedata, net_id);
  if((intcomm->recv_profile & (1 << net_id)) == 0
      || MyStatus_GetID(myst) != MyStatus_GetID(&profile->mystatus)){
    new_player = TRUE;
  }
  MyStatus_Copy(&profile->mystatus, myst);

  occupy = GAMEDATA_GetOccupyInfo(gamedata, net_id);
  GFL_STD_MemCopy(&profile->occupy, occupy, sizeof(OCCUPY_INFO));
  
  GAMEDATA_SetIntrudeSeasonID(gamedata, net_id, profile->status.season);
  
  Intrude_SetPlayerStatus(intcomm, net_id, &profile->status, TRUE);
  
  intcomm->recv_profile |= 1 << net_id;
  OS_TPrintf("プロフィール受信　net_id=%d, recv_bit=%d\n", net_id, intcomm->recv_profile);
  
  if(new_player == TRUE && net_id != GFL_NET_SystemGetCurrentID()){
    GameCommInfo_MessageEntry_PalaceConnect(intcomm->game_comm, net_id);
  }
}

//==================================================================
/**
 * ステータスデータを受信バッファにセット
 *
 * @param   intcomm		
 * @param   net_id		
 * @param   sta		
 * @param   first_status      TRUE:初めてセットするステータス　FALSE:2回目以降
 */
//==================================================================
void Intrude_SetPlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, int net_id, const INTRUDE_STATUS *sta, BOOL first_status)
{
  INTRUDE_STATUS *target_status;
  BOOL vanish;
  
  target_status = &intcomm->intrude_status[net_id];
  //vanishは受信側で制御するので上書きしない。但し送信側から非表示設定が来ている時は受け取る
  vanish = target_status->player_pack.vanish;
  GFL_STD_MemCopy(sta, target_status, sizeof(INTRUDE_STATUS));
  target_status->player_pack.vanish = vanish;
  target_status->force_vanish = sta->player_pack.vanish;
  if(sta->player_pack.vanish == TRUE){
    target_status->player_pack.vanish = TRUE;
  }

  //プレイヤーステータスにデータセット　※game_commに持たせているのを変えるかも
  GameCommStatus_SetPlayerStatus(intcomm->game_comm, net_id, target_status->zone_id,
    target_status->palace_area, target_status->pm_version, first_status);

  //座標変換
  if(net_id != GFL_NET_SystemGetCurrentID()){
    intcomm->backup_player_pack[net_id] = target_status->player_pack;
    Intrude_ConvertPlayerPos(intcomm, intcomm->intrude_status_mine.zone_id, 
      intcomm->intrude_status_mine.player_pack.pos.x, target_status);
  }
  
  intcomm->player_status_update |= 1 << net_id;
}

//==================================================================
/**
 * プレイヤーステータスを通信アクターへ反映
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_UpdatePlayerStatusAll(INTRUDE_COMM_SYS_PTR intcomm)
{
  NetID net_id;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  
  if(GAMEDATA_GetIsSave(gamedata) == TRUE){
    return; //MatrixIDのチェックや通信プレイヤーのAddなどでカードアクセスする為
  }
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intcomm->player_status_update & (1 << net_id)){
      Intrude_UpdatePlayerStatus(intcomm, net_id);
      intcomm->player_status_update ^= 1 << net_id;
    }
  }
}

//==================================================================
/**
 * プレイヤーステータスを通信アクターへ反映
 *
 * @param   intcomm		
 * @param   net_id		
 */
//==================================================================
static void Intrude_UpdatePlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, NetID net_id)
{
  INTRUDE_STATUS *target_status;
  
  target_status = &intcomm->intrude_status[net_id];

  //侵入エリアの違いによるアクター表示・非表示設定
  if(net_id != GFL_NET_SystemGetCurrentID()){
    INTRUDE_STATUS *mine_st = &intcomm->intrude_status_mine;
    
    if(target_status->force_vanish == TRUE){
      //強制的に非表示
      target_status->player_pack.vanish = TRUE;
    }
    else if(target_status->player_pack.player_form != PLAYER_MOVE_FORM_NORMAL
        && target_status->player_pack.player_form != PLAYER_MOVE_FORM_CYCLE){
      //表示できない形態ならば非表示
      target_status->player_pack.vanish = TRUE;
    }
    else if(ZONEDATA_IsPalace(target_status->zone_id) == TRUE && ZONEDATA_IsPalace(mine_st->zone_id) == TRUE){
      //お互いパレス島に居るなら表示
      target_status->player_pack.vanish = FALSE;
    }
    else if(target_status->palace_area == mine_st->palace_area){
      //同じROM内にいる
      if(ZONEDATA_IsBingo(target_status->zone_id) == TRUE && ZONEDATA_IsBingo(mine_st->zone_id) == TRUE){
        //二人ともシンボルマップにいる
        if(target_status->symbol_mapid == mine_st->symbol_mapid){
          target_status->player_pack.vanish = FALSE;
        }
        else{
          target_status->player_pack.vanish = TRUE;
        }
      }
      else{ //同じROM内にいてシンボルマップ以外
        if(ZONEDATA_IsIntrudeSameMatrixID(mine_st->zone_id, target_status->zone_id) == TRUE){
          target_status->player_pack.vanish = FALSE;
        }
        else{
          target_status->player_pack.vanish = TRUE;
        }
      }
    }
    else{
      //違うROMにいる
      target_status->player_pack.vanish = TRUE;
    }
  }

  //変装によるOBJCODEが変わっているチェック
  if(CommPlayer_CheckOcc(intcomm->cps, net_id) == TRUE){
    u16 obj_code = Intrude_GetObjCode(target_status, Intrude_GetMyStatus(GameCommSys_GetGameData(intcomm->game_comm), net_id));
    if(obj_code != CommPlayer_GetObjCode(intcomm->cps, net_id)){
      CommPlayer_Del(intcomm->cps, net_id);
      CommPlayer_Add(intcomm->cps, net_id, obj_code, &target_status->player_pack);
    }
  }
}

//==================================================================
/**
 * ゾーンIDに一致する侵入タウンを検索する
 *
 * @param   zone_id		サーチするゾーンID
 * @param   result		結果代入先
 *
 * @retval  BOOL		TRUE:侵入タウンがあった。　FALSE:無かった
 */
//==================================================================
BOOL Intrude_SearchPalaceTown(ZONEID zone_id, PALACE_TOWN_RESULT *result)
{
  int i;
  
  result->zone_id = zone_id;
  for(i = 0; i < PALACE_TOWN_DATA_MAX; i++){
    if(PalaceTownData[i].front_zone_id == zone_id){
      result->front_reverse = PALACE_TOWN_SIDE_FRONT;
    }
    else if(PalaceTownData[i].reverse_zone_id == zone_id){
      result->front_reverse = PALACE_TOWN_SIDE_REVERSE;
    }
    else{
      continue;
    }
    result->tblno = i;
    return TRUE;
  }
  result->tblno = PALACE_TOWN_TBLNO_NULL;
  return FALSE;
}

//==================================================================
/**
 * 同じゾーンにいるか判定(表裏ゾーン判定も行う)
 *
 * @param   result_a		
 * @param   result_b		
 *
 * @retval  CHECKSAME		
 */
//==================================================================
CHECKSAME Intrude_CheckSameZoneID(const PALACE_TOWN_RESULT *result_a, const PALACE_TOWN_RESULT *result_b)
{
  int town_ret_a, town_ret_b;
  
  if(result_a->zone_id == result_b->zone_id){
    return CHECKSAME_SAME;
  }
  
  if(result_a->front_reverse != result_b->front_reverse){
    if(result_a->tblno == result_b->tblno){
      return CHECKSAME_SAME_REVERSE;
    }
  }
  return CHECKSAME_NOT;
}

//--------------------------------------------------------------
/**
 * 指定パレスエリアが左端から何個目のパレスなのかを取得する
 *
 * @param   intcomm		
 * @param   palace_area		パレスエリア
 *
 * @retval  int		左から何個目のパレスか
 */
//--------------------------------------------------------------
static int Intrude_GetPalaceOffsetNo(const INTRUDE_COMM_SYS_PTR intcomm, int palace_area)
{
  int start_area, offset_area;
  
  if(palace_area >= intcomm->member_num){
    GF_ASSERT_MSG(0, "palace_area = %d, member_num = %d\n", palace_area, intcomm->member_num);
    return 0;
  }
  
  start_area = GAMEDATA_GetIntrudeMyID(GameCommSys_GetGameData(intcomm->game_comm));
  offset_area = palace_area - start_area;
  if(offset_area < 0){
    offset_area += intcomm->member_num;
  }
  return offset_area;
}

//==================================================================
/**
 * 通信プレイヤーの座標をこちらの画面に合わせて変換する
 *
 * @param   mine		  
 * @param   target		
 */
//==================================================================
static void Intrude_ConvertPlayerPos(INTRUDE_COMM_SYS_PTR intcomm, ZONEID mine_zone_id, fx32 mine_x, INTRUDE_STATUS *target)
{
  PALACE_TOWN_RESULT result_a, result_b;
  BOOL search_a, search_b;
  CHECKSAME ret;
  
  Intrude_SearchPalaceTown(mine_zone_id, &result_a);
  Intrude_SearchPalaceTown(target->zone_id, &result_b);
  
  ret = Intrude_CheckSameZoneID(&result_a, &result_b);
  switch(ret){
  case CHECKSAME_SAME:
    if(ZONEDATA_IsPalace(mine_zone_id) == FALSE){
      return; //同じゾーンにいるので変換の必要無し
    }
    //双方ともパレスにいるのでパレスエリアに合わせた座標変換
    {
      fx32 offset_x, map_len;
      int area_offset;
      
      map_len = PALACE_MAP_LEN;
      offset_x = target->player_pack.pos.x % map_len;
      area_offset = Intrude_GetPalaceOffsetNo(intcomm, target->palace_area);
      if(area_offset == intcomm->member_num - 1 && offset_x > PALACE_MAP_LEN/2 
          && mine_x < PALACE_MAP_LEN/2){
        //相手が右端のエリアの右寄りにいて、自機が左端の左寄りにいるなら、相手の座標を
        //自機の左側にある回り込みマップ上の座標にする
        target->player_pack.pos.x = -map_len + offset_x;
      }
      else if(area_offset == 0 && offset_x < PALACE_MAP_LEN/2 
          && mine_x > (PALACE_MAP_LEN*intcomm->member_num - PALACE_MAP_LEN/2)){
        //相手が左端のエリアの左寄りにいて、自機が右端の右寄りにいるなら、相手の座標を
        //自機の右側にある回り込みマップ上の座標にする
        target->player_pack.pos.x = intcomm->member_num * map_len + offset_x;
      }
      else{
        target->player_pack.pos.x = area_offset * map_len + offset_x;
      }
    }
    break;
  case CHECKSAME_SAME_REVERSE:  //表裏ゾーンにいるのでこのままでよい
  case CHECKSAME_NOT:
    return; //違うゾーンにいるのでこのままでよい
  }
}

//==================================================================
/**
 * バックアップ座標を元に通信プレイヤーの座標を再計算
 *
 * @param   intcomm		
 * @param   mine_x		自機の座標X
 */
//==================================================================
void Intrude_PlayerPosRecalc(INTRUDE_COMM_SYS_PTR intcomm, fx32 mine_x)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  int i;
  int my_netid = GAMEDATA_GetIntrudeMyID(gamedata);
  const INTRUDE_STATUS *mine = &intcomm->intrude_status_mine;
  
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if((intcomm->recv_profile & (1 << i)) && i != my_netid){
      INTRUDE_STATUS *target = &intcomm->intrude_status[i];
      target->player_pack = intcomm->backup_player_pack[i];
      Intrude_ConvertPlayerPos(intcomm, mine->zone_id, mine_x, target);
    }
  }
}

//==================================================================
/**
 * 侵入ステータス送信バッファを更新
 *
 * @param   intcomm		
 *
 * @retval  BOOL		TRUE:変更あり　FALSE:変更なし
 */
//==================================================================
BOOL Intrude_SetSendStatus(INTRUDE_COMM_SYS_PTR intcomm)
{
  BOOL send_req = FALSE;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  PLAYER_WORK *plWork = GAMEDATA_GetMyPlayerWork( gamedata );
  ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
  INTRUDE_STATUS *ist = &intcomm->intrude_status_mine;
  BOOL detect_fold;
  
  send_req = CommPlayer_Mine_DataUpdate(intcomm->cps, &ist->player_pack);
  if(ist->zone_id != zone_id){
    ist->zone_id = zone_id;
    send_req = TRUE;
  }
  
  if(ist->symbol_mapid != GAMEDATA_GetSymbolMapID(gamedata)){
    ist->symbol_mapid = GAMEDATA_GetSymbolMapID(gamedata);
    send_req = TRUE;
  }
  
  detect_fold = PAD_DetectFold();
  if(ist->detect_fold != detect_fold){
    ist->detect_fold = detect_fold;
    send_req = TRUE;
  }
  
  return send_req;
}

//==================================================================
/**
 * 会話の認証データを初期化する
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_InitTalkCertification(INTRUDE_COMM_SYS_PTR intcomm)
{
  GFL_STD_MemClear(&intcomm->recv_certifi, sizeof(INTRUDE_TALK_CERTIFICATION));
}

//==================================================================
/**
 * 受信データから会話の認証データを取得する
 *
 * @param   intcomm		
 *
 * @retval  INTRUDE_TALK_CERTIFICATION *		受信した会話の認証データ(NULL時は未受信)
 */
//==================================================================
INTRUDE_TALK_CERTIFICATION * Intrude_GetTalkCertification(INTRUDE_COMM_SYS_PTR intcomm)
{
  if(intcomm->recv_certifi.occ == TRUE){
    return &intcomm->recv_certifi;
  }
  return NULL;
}

//==================================================================
/**
 * 会話構造体初期化
 *
 * @param   intcomm		
 * @param   talk_netid		クリアデータとして初期化ならばINTRUDE_NETID_NULL
 * @param   talk_netid		話しかけた時の初期化ならば話しかけた相手のNetID
 * @param   talk_netid		話しかけられた時の初期化ならば話しかけてきた相手のNetID
 */
//==================================================================
void Intrude_InitTalkWork(INTRUDE_COMM_SYS_PTR intcomm, int talk_netid)
{
  GFL_STD_MemClear(&intcomm->talk, sizeof(INTRUDE_TALK));
  intcomm->talk.talk_netid = talk_netid;
  intcomm->talk.answer_talk_netid = INTRUDE_NETID_NULL;
  intcomm->talk.talk_rand = TALK_RAND_NULL;
  
  Intrude_TalkRandClose(intcomm);
}

//==================================================================
/**
 * 話しかけが成立しないようにランダムコードを初期化する
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_TalkRandClose(INTRUDE_COMM_SYS_PTR intcomm)
{
  intcomm->now_talk_rand++;
  intcomm->talk.now_talk_rand = intcomm->now_talk_rand;
}


//==================================================================
/**
 * 会話エントリー
 *
 * @param   intcomm		
 * @param   net_id		エントリー者のNetID
 * @retval  TRUE:話が出来る。　FALSE:話が出来ない
 */
//==================================================================
BOOL Intrude_SetTalkReq(INTRUDE_COMM_SYS_PTR intcomm, int net_id, u8 talk_rand)
{
  if((intcomm->recv_profile & (1 << net_id)) == 0){
    OS_TPrintf("話しかけられたけど、まだプロフィールを受信していないので断る\n");
    intcomm->answer_talk_ng[net_id] = talk_rand;
    return FALSE;
  }

  GF_ASSERT(talk_rand != TALK_RAND_NULL);

  switch(intcomm->intrude_status_mine.action_status){
  case INTRUDE_ACTION_FIELD:
    if(intcomm->talk.talk_netid == INTRUDE_NETID_NULL && intcomm->talk.talk_rand == TALK_RAND_NULL
        && GAMESYSTEM_IsEventExists(intcomm->gsys) == FALSE){
      intcomm->talk.talk_netid = net_id;
      intcomm->talk.talk_status = INTRUDE_TALK_STATUS_OK;
      intcomm->talk.talk_rand = talk_rand;
      intcomm->talk.now_talk_rand = talk_rand;
      Intrude_SetTalkedEventReserve(intcomm);
      OS_TPrintf("talk_ok\n");
      return TRUE;
    }
    else{
      intcomm->answer_talk_ng[net_id] = talk_rand;
      OS_TPrintf("talk_ng field\n");
    }
    break;
  default:
    intcomm->answer_talk_ng[net_id] = talk_rand;
    OS_TPrintf("talk_ng\n");
    break;
  }
  
  return FALSE;
}

//==================================================================
/**
 * 話しかけられたイベントを予約
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_SetTalkedEventReserve(INTRUDE_COMM_SYS_PTR intcomm)
{
  intcomm->talked_event_reserve = TRUE;
}

//==================================================================
/**
 * 話しかけられたイベントの予約をクリア
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_ClearTalkedEventReserve(INTRUDE_COMM_SYS_PTR intcomm)
{
  intcomm->talked_event_reserve = FALSE;
}

//==================================================================
/**
 * 話しかけれたイベントが予約中に他のイベントが起動した場合は、
 * 話しかけられたイベントの予約をキャンセルする
 *
 * @param   intcomm		
 */
//==================================================================
static void Intrude_CancelCheckTalkedEventReserve(INTRUDE_COMM_SYS_PTR intcomm)
{
  u32 talk_netid;
  
  if(Intrude_CheckTalkedTo(intcomm, &talk_netid) == TRUE){
    if(GAMESYSTEM_IsEventExists(intcomm->gsys) == TRUE){
      if(intcomm->answer_talk_ng[intcomm->talk.talk_netid] == TALK_RAND_NULL){
        intcomm->answer_talk_ng[intcomm->talk.talk_netid] = intcomm->talk.talk_rand;
      }
      Intrude_InitTalkWork(intcomm, INTRUDE_NETID_NULL);
      Intrude_ClearTalkedEventReserve(intcomm);
    }
  }
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
BOOL Intrude_CheckTalkedTo(INTRUDE_COMM_SYS_PTR intcomm, u32 *hit_netid)
{
  s16 check_gx, check_gy, check_gz;
  u32 out_index;
  
  if(intcomm == NULL || intcomm->cps == NULL){
    return FALSE;
  }
  
  //自分から話しかけていないのに値がNULL以外なのは話しかけられたから
  if(intcomm->talk.talk_netid != INTRUDE_NETID_NULL && intcomm->talked_event_reserve == TRUE){
    *hit_netid = intcomm->talk.talk_netid;
    return TRUE;
  }
  
  return FALSE;
}

//==================================================================
/**
 * 会話の返事受信データセット
 *
 * @param   intcomm		
 * @param   net_id		    返事をしてきた人のNetID
 * @param   talk_status		会話ステータス
 */
//==================================================================
void Intrude_SetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int net_id, INTRUDE_TALK_STATUS talk_status, u8 talk_rand)
{
  //キャンセルはいつでも受け入れる
  if(talk_status != INTRUDE_TALK_STATUS_CANCEL){
    GF_ASSERT(intcomm->talk.answer_talk_status == INTRUDE_TALK_STATUS_NULL);
  }
  
  if(intcomm->talk.talk_netid == net_id && intcomm->talk.talk_rand == talk_rand){
    intcomm->talk.answer_talk_netid = net_id;
    intcomm->talk.answer_talk_status = talk_status;
  }
  else{
    OS_TPrintf("answer 不一致 my %d, %d, recv %d, %d\n", intcomm->talk.talk_netid, intcomm->talk.talk_rand, net_id, talk_rand);
  }
}

//==================================================================
/**
 * 会話の返事受信データセット(キャンセル専用)
 *
 * @param   intcomm		
 * @param   net_id		
 */
//==================================================================
void Intrude_SetTalkCancel(INTRUDE_COMM_SYS_PTR intcomm, int net_id, u8 talk_rand)
{
  Intrude_SetTalkAnswer(intcomm, net_id, INTRUDE_TALK_STATUS_CANCEL, talk_rand);
}

//==================================================================
/**
 * 会話の返事受信データを取得
 *
 * @param   intcomm		
 * @param   net_id		    返事をしてきた人のNetID
 * @param   talk_status		会話ステータス
 *
 * INTRUDE_TALK_STATUS_NULL以外のデータが入っていた場合は
 * バッファをNULLでクリアしてから戻り値を返します
 */
//==================================================================
INTRUDE_TALK_STATUS Intrude_GetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm)
{
  INTRUDE_TALK_STATUS ret;
  
  if(intcomm->talk.answer_talk_netid != INTRUDE_NETID_NULL){
    ret = intcomm->talk.answer_talk_status;
    if(ret != INTRUDE_TALK_STATUS_NULL){
      intcomm->talk.answer_talk_status = INTRUDE_TALK_STATUS_NULL;
      return ret;
    }
  }
  return INTRUDE_TALK_STATUS_NULL;
}

//==================================================================
/**
 * 会話の返事受信データをクリア
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_ClearTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm)
{
  intcomm->talk.answer_talk_status = INTRUDE_TALK_STATUS_NULL;
}



//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 侵入時のノーマル変装時のOBJCODEを取得
 *
 * @param   myst
 *
 * @retval  u16		OBJCODE
 */
//==================================================================
void Intrude_GetNormalDisguiseObjCode(const MYSTATUS *myst, u16 *objcode, u8 *disguise_type, u8 *disguise_sex)
{
  static const u16 NormalDisguise_Male[] = {
    BOY1,
    BOY2,
    BOY3,
    BOY4,
  };
  static const u16 NormalDisguise_Female[] = {
    GIRL1,
    GIRL2,
    GIRL3,
    GIRL4,
  };
  int tbl_no;
  u32 trainer_id, sex;
  
  trainer_id = MyStatus_GetID(myst);
  sex = MyStatus_GetMySex(myst);
  tbl_no = ((trainer_id >> 16) + (trainer_id & 0xffff)) % NELEMS(NormalDisguise_Male);
  if(sex == PM_MALE){
    *objcode = NormalDisguise_Male[tbl_no];
    *disguise_type = TALK_TYPE_MAN;
    *disguise_sex = PM_MALE;
  }
  else{
    *objcode = NormalDisguise_Female[tbl_no];
    *disguise_type = TALK_TYPE_WOMAN;
    *disguise_sex = PM_FEMALE;
  }
}

//==================================================================
/**
 * 侵入ステータスから表示するOBJコードを取得
 *
 * @param   sta		
 * @param   myst		
 *
 * @retval  u16		OBJコード
 */
//==================================================================
u16 Intrude_GetObjCode(const INTRUDE_STATUS *sta, const MYSTATUS *myst)
{
  u16 obj_code;
  u8 disguise_type, disguise_sex;

  if(sta->disguise_no == DISGUISE_NO_NULL){
    if(sta->player_pack.player_form == PLAYER_MOVE_FORM_CYCLE){
      obj_code = (MyStatus_GetMySex(myst) == PM_MALE) ? CYCLEHERO : CYCLEHEROINE;
    }
    else{
      obj_code = (MyStatus_GetMySex(myst) == PM_MALE) ? HERO : HEROINE;
    }
  }
  else if(sta->disguise_no == DISGUISE_NO_NORMAL){
    Intrude_GetNormalDisguiseObjCode(myst, &obj_code, &disguise_type, &disguise_sex);
  }
  else{
    obj_code = sta->disguise_no;
    if(Intrude_CheckNG_OBJID(obj_code) == FALSE){
      GF_ASSERT_MSG(0, "obj_code = %d", obj_code);
      obj_code = (MyStatus_GetMySex(myst) == PM_MALE) ? BOY1 : GIRL1;
    }
  }
  
  //念のため
  if((obj_code >= OBJCODEEND_BBD && obj_code < OBJCODESTART_TPOKE) 
      || obj_code >= OBJCODEEND_TPOKE){
    obj_code = BOY1;
    GF_ASSERT(0);
  }
  
  return obj_code;
}

//--------------------------------------------------------------
/**
 * 自分以外のプレイヤーが存在しているか調べる
 *
 * @param   none		
 *
 * @retval  BOOL		TRUE:存在している。　FALSE:誰もいない
 */
//--------------------------------------------------------------
BOOL Intrude_OtherPlayerExistence(void)
{
  if(NetErr_App_CheckError()){
    return FALSE;
  }
  if(GFL_NET_GetConnectNum() > 1){
    return TRUE;
  }
  OS_TPrintf("誰も居なくなった %d\n", GFL_NET_GetConnectNum());
  return FALSE;
}

//==================================================================
/**
 * 自分のモノリスステータスを作成する
 *
 * @param   gamedata		
 * @param   monost		  代入先
 */
//==================================================================
void Intrude_MyMonolithStatusSet(GAMEDATA *gamedata, MONOLITH_STATUS *monost)
{
  INTRUDE_SAVE_WORK *intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork(gamedata) );
  
  GFL_STD_MemClear(monost, sizeof(MONOLITH_STATUS));

  monost->clear_mission_count = ISC_SAVE_GetMissionClearCount(intsave);
  monost->palace_sojourn_time = ISC_SAVE_PalaceSojournParam(
    intsave, GAMEDATA_GetPlayTimeWork(gamedata), SOJOURN_TIME_CALC_GET);
  ISC_SAVE_GetDistributionGPower_Array(
    intsave, monost->gpower_distribution_bit, INTRUDE_SAVE_DISTRIBUTION_BIT_WORK_MAX);

  monost->occ = TRUE;
}

//==================================================================
/**
 * 占拠結果を受信しているか確認
 *
 * @param   intcomm		
 *
 * @retval  BOOL		TRUE：受信している　FALSE：受信していない
 */
//==================================================================
BOOL Intrude_CheckRecvOccupyResult(INTRUDE_COMM_SYS_PTR intcomm)
{
  return intcomm->recv_occupy_result.occ;
}

//==================================================================
/**
 * OBJIDが侵入で変装できるOBJIDかをチェックする(不正チェックに使用)
 *
 * @param   obj_id		
 *
 * @retval  BOOL		TRUE:表示OK　FALSE:不正なOBJID
 */
//==================================================================
BOOL Intrude_CheckNG_OBJID(u16 obj_id)
{
  int i;
  
  for(i = 0; i < NELEMS(IntrudeOBJID_List); i++){
    if(IntrudeOBJID_List[i] == obj_id){
      return TRUE;
    }
  }
  return FALSE;
}

//==================================================================
/**
 * アイテムが侵入のミッションで売買、入手出来るアイテムかをチェックする(不正チェックに使用)
 *
 * @param   item_no		
 *
 * @retval  BOOL		TRUE:表示OK　FALSE:不正なOBJID
 */
//==================================================================
BOOL Intrude_CheckNG_Item(u16 item_no)
{
  int i;
  
  for(i = 0; i < NELEMS(IntrudeItem_List); i++){
    if(IntrudeItem_List[i] == item_no){
      return TRUE;
    }
  }
  return FALSE;
}

//==================================================================
/**
 * タイムアウトによる切断の有効・無効を設定する
 *
 * @param   intcomm		
 * @param   stop		  TRUE:タイムアウトによる切断を行わない
 *              		  FALSE:タイムアウトによる切断をする
 */
//==================================================================
void Intrude_SetIsBattleFlag(INTRUDE_COMM_SYS_PTR intcomm, BOOL is_battle)
{
  intcomm->battle_proc_call = is_battle;
  OS_TPrintf("is_battle = %d\n", is_battle);
}

//==================================================================
/**
 * 自分が今いるパレスエリアをセットする時に行う処理をまとめたもの
 *
 * @param   intcomm		
 */
//==================================================================
void Intrude_SetMinePalaceArea(INTRUDE_COMM_SYS_PTR intcomm, int palace_area)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intcomm->gsys);
  
  intcomm->intrude_status_mine.palace_area = palace_area;
  GAMEDATA_SetIntrudePalaceArea(gamedata, palace_area);
  FIELD_WFBC_COMM_DATA_SetIntrudeNetID(&intcomm->wfbc_comm_data, palace_area);
}

//==================================================================
/**
 * 送信用の自分座標更新処理を停止するフラグをセット
 *
 * @param   intcomm		
 * @param   stop		  TRUE:更新停止　FALSE:更新する
 */
//==================================================================
void Intrude_SetSendMyPositionStopFlag(INTRUDE_COMM_SYS_PTR intcomm, BOOL stop)
{
  intcomm->send_my_position_stop = stop;
}

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
#include "palace_sys.h"
#include "intrude_main.h"
#include "bingo_system.h"


//==============================================================================
//  定数定義
//==============================================================================
///プロフィール要求リクエストを再度かけるまでのウェイト
#define INTRUDE_PROFILE_REQ_RETRY_WAIT    (30)

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void Intrude_CheckProfileReq(INTRUDE_COMM_SYS_PTR intcomm);
static void Intrude_CheckTalkAnswerNG(INTRUDE_COMM_SYS_PTR intcomm);



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
  //プロフィール要求リクエストを受けているなら送信
  if(intcomm->profile_req == TRUE){
    IntrudeSend_Profile(intcomm);
  }
  //通信が確立されているメンバーでプロフィールを持っていないメンバーがいるならリクエストをかける
  Intrude_CheckProfileReq(intcomm);
  
  //話しかけお断りの返事が貯まっているなら返事を送信する
  Intrude_CheckTalkAnswerNG(intcomm);
  //ビンゴの送信リクエストがあれば送信
  Bingo_SendUpdate(intcomm, Bingo_GetBingoSystemWork(intcomm));
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
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(GFL_NET_IsConnectMember(net_id) == TRUE 
        && (intcomm->recv_profile & (1 << net_id)) == 0){
      IntrudeSend_ProfileReq();
      intcomm->profile_req_wait = INTRUDE_PROFILE_REQ_RETRY_WAIT;
      return;
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
    if(intcomm->answer_talk_ng_bit & (1 << net_id)){
      if(GFL_NET_IsConnectMember(net_id) == TRUE){
        if(IntrudeSend_TalkAnswer(intcomm, net_id, INTRUDE_TALK_STATUS_NG) == TRUE){
          intcomm->answer_talk_ng_bit ^= 1 << net_id;
        }
      }
      else{
        intcomm->answer_talk_ng_bit ^= 1 << net_id;
      }
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
}

//==================================================================
/**
 * 会話構造体初期化
 *
 * @param   intcomm		
 * @param   talk_netid		
 */
//==================================================================
void Intrude_InitTalkWork(INTRUDE_COMM_SYS_PTR intcomm, int talk_netid)
{
  GFL_STD_MemClear(&intcomm->talk, sizeof(INTRUDE_TALK));
  intcomm->talk.talk_netid = talk_netid;
  intcomm->talk.answer_talk_netid = INTRUDE_NETID_NULL;
}

//==================================================================
/**
 * 会話エントリー
 *
 * @param   intcomm		
 * @param   net_id		エントリー者のNetID
 */
//==================================================================
void Intrude_SetTalkReq(INTRUDE_COMM_SYS_PTR intcomm, int net_id)
{
  if((intcomm->recv_profile & (1 << net_id)) == 0){
    OS_TPrintf("話しかけられたけど、まだプロフィールを受信していないので断る\n");
    intcomm->answer_talk_ng_bit |= 1 << net_id;
    return;
  }

  switch(intcomm->intrude_status_mine.action_status){
  case INTRUDE_ACTION_FIELD:
    if(intcomm->talk.talk_netid == INTRUDE_NETID_NULL){
      intcomm->talk.talk_netid = net_id;
      intcomm->talk.talk_status = INTRUDE_TALK_STATUS_OK;
    }
    else{
      intcomm->answer_talk_ng_bit |= 1 << net_id;
    }
    break;
  default:
    intcomm->answer_talk_ng_bit |= 1 << net_id;
    break;
  }
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
void Intrude_SetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int net_id, INTRUDE_TALK_STATUS talk_status)
{
  if(intcomm->talk.answer_talk_netid == INTRUDE_NETID_NULL && intcomm->talk.talk_netid == net_id){
    intcomm->talk.answer_talk_netid = net_id;
    intcomm->talk.answer_talk_status = talk_status;
  }
}

//==================================================================
/**
 * 会話の返事受信データを取得
 *
 * @param   intcomm		
 * @param   net_id		    返事をしてきた人のNetID
 * @param   talk_status		会話ステータス
 */
//==================================================================
INTRUDE_TALK_STATUS Intrude_GetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm)
{
  if(intcomm->talk.answer_talk_netid != INTRUDE_NETID_NULL){
    return intcomm->talk.answer_talk_status;
  }
  return INTRUDE_TALK_STATUS_NULL;
}


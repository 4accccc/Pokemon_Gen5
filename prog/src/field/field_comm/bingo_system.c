//==============================================================================
/**
 * @file    bingo_system.c
 * @brief   ビンゴシステム制御
 * @author  matsuda
 * @date    2009.10.10(土)
 * 
 * データアクセス系が通信コマンドからも呼ばれる為、intrude_comm_command.cと同じオーバーレイに
 * 配置させる必要があります。
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
#include "intrude_main.h"
#include "bingo_system.h"


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void Bingo_Parent_UpdateIntrusionData(BINGO_SYSTEM *bingo);



//==================================================================
/**
 * 侵入システムワークからビンゴシステムワークのポインタを取得する
 *
 * @param   intcomm		
 *
 * @retval  BINGO_SYSTEM *		
 */
//==================================================================
BINGO_SYSTEM * Bingo_GetBingoSystemWork(INTRUDE_COMM_SYS_PTR intcomm)
{
  return &intcomm->bingo;
}

//==================================================================
/**
 * ビンゴシステムワークを初期化
 *
 * @param   bingo		
 */
//==================================================================
void Bingo_InitBingoSystem(BINGO_SYSTEM *bingo)
{
  GFL_STD_MemClear(bingo, sizeof(BINGO_SYSTEM));
  Bingo_Clear_BingoBattleBeforeBuffer(bingo);
  Bingo_Clear_BingoIntrusionBeforeBuffer(bingo);
}

//==================================================================
/**
 * ビンゴバトル開始前にしなければいけないバッファの初期化
 *
 * @param   bingo		
 */
//==================================================================
void Bingo_Clear_BingoBattleBeforeBuffer(BINGO_SYSTEM *bingo)
{
  int i;

  GF_ASSERT(bingo->intrusion_netid_bit == 0);
  
  GFL_STD_MemClear(&bingo->intrusion_param, sizeof(BINGO_INTRUSION_PARAM));
  bingo->intrusion_param.occ = TRUE;  //※check　まだビンゴバトルそのものが無いので仮
  
  bingo->intrusion_netid_bit = 0;
  
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    bingo->parent.intrusion_player[i] = INTRUDE_NETID_NULL;
    bingo->parent.intrusion_player_answer[i] = INTRUDE_NETID_NULL;
  }
}

//==================================================================
/**
 * 乱入要求開始前にしなければいけないバッファの初期化
 *
 * @param   bingo		
 */
//==================================================================
void Bingo_Clear_BingoIntrusionBeforeBuffer(BINGO_SYSTEM *bingo)
{
  bingo->intrusion_recv_answer = BINGO_INTRUSION_ANSWER_WAIT;
  GFL_STD_MemClear(&bingo->intrusion_param, sizeof(BINGO_INTRUSION_PARAM));
}

//==================================================================
/**
 * ビンゴの送信データがあれば送信を行う
 *
 * @param   bingo		
 */
//==================================================================
void Bingo_SendUpdate(INTRUDE_COMM_SYS_PTR intcomm, BINGO_SYSTEM *bingo)
{
  int net_id;
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    //乱入許可の返事
    if(bingo->intrusion_answer_ok_netid_bit & (1 << net_id)){
      if(IntrudeSend_BingoIntrusionAnswer(net_id, BINGO_INTRUSION_ANSWER_OK) == TRUE){
        bingo->intrusion_answer_ok_netid_bit ^= 1 << net_id;
      }
    }
    else if(bingo->intrusion_answer_ng_netid_bit & (1 << net_id)){
      if(IntrudeSend_BingoIntrusionAnswer(net_id, BINGO_INTRUSION_ANSWER_NG) == TRUE){
        bingo->intrusion_answer_ng_netid_bit ^= 1 << net_id;
      }
    }
    
    //乱入パラメータ要求があった場合、送信
    if(bingo->param_req_bit & (1 << net_id)){
      if(IntrudeSend_BingoBattleIntrusionParam(bingo, net_id) == TRUE){
        bingo->param_req_bit ^= 1 << net_id;
      }
    }
  }

  //侵入システムとのParentワークを介した乱入許可NG、データ更新処理
  if(intcomm->intrude_status_mine.action_status == INTRUDE_ACTION_BINGO_BATTLE){
    Bingo_Parent_UpdateIntrusionData(bingo);
  }
}

//==================================================================
/**
 * ビンゴ乱入希望リクエストを受け付ける
 *
 * @param   intcomm		
 * @param   bingo		
 * @param   net_id		乱入希望者のNetID
 */
//==================================================================
void Bingo_Req_IntrusionPlayer(INTRUDE_COMM_SYS_PTR intcomm, BINGO_SYSTEM *bingo, int net_id)
{
  GF_ASSERT((bingo->intrusion_netid_bit & (1 << net_id)) == 0);
  
  if(intcomm->intrude_status_mine.action_status == INTRUDE_ACTION_BINGO_BATTLE){
    bingo->intrusion_netid_bit |= 1 << net_id;
    Bingo_Parent_EntryIntusion(bingo, net_id);
  }
  else{
    bingo->intrusion_answer_ng_netid_bit |= 1 << net_id;
    OS_TPrintf("乱入NG net_id = %d\n", net_id);
  }
}

//--------------------------------------------------------------
/**
 * ビンゴバトル側から乱入希望の返事が代入されていれば送信データに登録します
 *
 * @param   bingo		
 */
//--------------------------------------------------------------
static void Bingo_Parent_UpdateIntrusionData(BINGO_SYSTEM *bingo)
{
  int i, net_id;
  
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    net_id = bingo->parent.intrusion_player[i];
    if(net_id != INTRUDE_NETID_NULL && (bingo->intrusion_netid_bit & (1 << net_id))){
      if(bingo->parent.intrusion_player_answer[i] == BINGO_INTRUSION_ANSWER_OK){
        bingo->intrusion_answer_ok_netid_bit |= 1 << net_id;
        bingo->intrusion_netid_bit ^= 1 << net_id;
      }
      else if(bingo->parent.intrusion_player_answer[i] == BINGO_INTRUSION_ANSWER_NG){
        bingo->intrusion_answer_ng_netid_bit |= 1 << net_id;
        bingo->intrusion_netid_bit ^= 1 << net_id;
      }
      //※check　デバッグ用 ビンゴバトルが出来るまでの仮想乱入許可NGやり取り
      else{
        bingo->parent.intrusion_player_answer[i] = GFUser_GetPublicRand(BINGO_INTRUSION_ANSWER_NG + 1);
      }
    }
  }
}

//==================================================================
/**
 * ビンゴバトルのParentワークへの乱入要求エントリー
 *
 * @param   bingo		  
 * @param   net_id		エントリー者のネットID
 */
//==================================================================
void Bingo_Parent_EntryIntusion(BINGO_SYSTEM *bingo, int net_id)
{
  int i, set_pos = INTRUDE_NETID_NULL;
  
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    //設計的にありえないはずだけど念のため、既に登録済みでないか確認
    if(bingo->parent.intrusion_player[i] == net_id){
      //GF_ASSERT(0);
      return; //既に登録されている
    }
    else if(set_pos == INTRUDE_NETID_NULL 
        && bingo->parent.intrusion_player[i] == INTRUDE_NETID_NULL){
      set_pos = i;
    }
  }
  
  if(set_pos == INTRUDE_NETID_NULL){
    GF_ASSERT(0);
    return;
  }
  bingo->parent.intrusion_player[set_pos] = net_id;
  OS_TPrintf("乱入候補リストに登録 net_id = %d, %d番目\n", net_id, set_pos);
}

//==================================================================
/**
 * ビンゴ乱入許可要求の返事受信データを取得
 *
 * @param   intcomm		
 * @param   net_id		    返事をしてきた人のNetID
 * @param   answer		    BINGO_INTRUSION_ANSWER
 */
//==================================================================
BINGO_INTRUSION_ANSWER Bingo_GetBingoIntrusionAnswer(INTRUDE_COMM_SYS_PTR intcomm)
{
  BINGO_SYSTEM *bingo = Bingo_GetBingoSystemWork(intcomm);
  
  return bingo->intrusion_recv_answer;
}

//==================================================================
/**
 * ビンゴ乱入パラメータが有効になっているか調べる
 * @param   intcomm		
 * @retval  BOOL		TRUE:有効
 */
//==================================================================
BOOL Bingo_GetBingoIntrusionParam(INTRUDE_COMM_SYS_PTR intcomm)
{
  BINGO_SYSTEM *bingo = Bingo_GetBingoSystemWork(intcomm);

  return bingo->intrusion_param.occ;
}

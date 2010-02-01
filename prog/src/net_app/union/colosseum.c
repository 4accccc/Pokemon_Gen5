//==============================================================================
/**
 * @file    colosseum.c
 * @brief   コロシアム:システムパラメータアクセス系：常駐に配置(通信受信データをセットするため)
 * @author  matsuda
 * @date    2009.07.17(金)
 */
//==============================================================================
#include <gflib.h>
#include "net_app/union/colosseum.h"
#include "net_app/union/colosseum_types.h"
#include "field\fieldmap_proc.h"
#include "app\trainer_card.h"
#include "colosseum_types.h"
#include "system/main.h"
#include "comm_player.h"
#include "field\field_player.h"
#include "field\fieldmap.h"
#include "colosseum_comm_command.h"
#include "battle/btl_net.h"


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * コロシアムシステムワーク作成
 *
 * @param   fieldWork		
 * @param   myst		
 *
 * @retval  COLOSSEUM_SYSTEM_PTR		
 */
//==================================================================
COLOSSEUM_SYSTEM_PTR Colosseum_InitSystem(GAMEDATA *game_data, GAMESYS_WORK *gsys, MYSTATUS *myst, BOOL intrude)
{
  COLOSSEUM_SYSTEM_PTR clsys;
  int i, my_net_id;
  COLOSSEUM_BASIC_STATUS *my_basic;
  
  my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  
  //メモリ確保とシステム生成
  clsys = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(COLOSSEUM_SYSTEM));
  clsys->cps = CommPlayer_Init(COLOSSEUM_MEMBER_MAX, gsys, HEAPID_UNION);
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    clsys->recvbuf.tr_card[i] = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(TR_CARD_DATA));
    clsys->recvbuf.pokeparty[i] = PokeParty_AllocPartyWork(HEAPID_UNION);
  }
  
  //初期値セット
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    clsys->parentsys.stand_position[i] = COLOSSEUM_STANDING_POSITION_NULL;
    clsys->parentsys.answer_stand_position[i] = COLOSSEUM_STANDING_POSITION_NULL;
  }
  clsys->mine.stand_position = COLOSSEUM_STANDING_POSITION_NULL;
  clsys->mine.answer_stand_position = COLOSSEUM_STANDING_POSITION_NULL;
  PokeParty_Copy( //自分の手持ちポケモンをセットしておく
    GAMEDATA_GetMyPokemon(game_data), clsys->recvbuf.pokeparty[my_net_id]);
  
  //自分の基本情報エリアにデータをセットする
  my_basic = &clsys->basic_status[my_net_id];
  MyStatus_Copy(myst, &my_basic->myst);
  OS_GetMacAddress(my_basic->mac_address);
  my_basic->occ = TRUE;
  my_basic->force_entry = (intrude == TRUE) ? FALSE : TRUE;
  my_basic->battle_server_version = BTL_NET_SERVER_VERSION;
  
  //自分のトレーナーカード情報セット
  TRAINERCARD_GetSelfData(clsys->recvbuf.tr_card[my_net_id], game_data, TRUE);
  clsys->recvbuf.tr_card_occ[my_net_id] = TRUE;
  
  return clsys;
}

//==================================================================
/**
 * コロシアムシステムワーク破棄
 *
 * @param   clsys		
 */
//==================================================================
void Colosseum_ExitSystem(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i;
  
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    GFL_HEAP_FreeMemory(clsys->recvbuf.tr_card[i]);
    GFL_HEAP_FreeMemory(clsys->recvbuf.pokeparty[i]);
  }
  CommPlayer_Exit(clsys->cps);
  GFL_HEAP_FreeMemory(clsys);
}

//==================================================================
/**
 * comm_readyフラグをセットする
 *
 * @param   clsys		
 * @param   flag		
 */
//==================================================================
void Colosseum_CommReadySet(COLOSSEUM_SYSTEM_PTR clsys, BOOL flag)
{
  clsys->comm_ready = flag;
}

//==================================================================
/**
 * 自分の立ち位置をセット
 *
 * @param   clsys		          
 * @param   stand_position		立ち位置
 */
//==================================================================
void Colosseum_Mine_SetStandingPostion(COLOSSEUM_SYSTEM_PTR clsys, int stand_position)
{
  clsys->mine.stand_position = stand_position;
  clsys->mine.answer_stand_position = COLOSSEUM_STANDING_POSITION_NULL;
}

//==================================================================
/**
 * 自分の立ち位置を取得
 *
 * @param   clsys		          
 * @retval	立ち位置
 */
//==================================================================
u8 Colosseum_Mine_GetStandingPostion(COLOSSEUM_SYSTEM_PTR clsys)
{
  return clsys->mine.stand_position;
}

//==================================================================
/**
 * 立ち位置使用許可の結果を代入
 *
 * @param   clsys		
 * @param   result		TRUE:使用OK。　FALSE:使用不可
 */
//==================================================================
void Colosseum_Mine_SetAnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys, int result)
{
  clsys->mine.answer_stand_position = result;
}

//==================================================================
/**
 * 立ち位置使用許可の結果を代入
 *
 * @param   clsys		
 * @param   result		TRUE:使用OK。　FALSE:使用不可
 */
//==================================================================
u8 Colosseum_Mine_GetAnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys)
{
  return clsys->mine.answer_stand_position;
}

//==================================================================
/**
 * 親機専用命令：子から送られてきた立ち位置情報をセット
 *
 * @param   clsys		
 * @param   net_id		        送り主のNetID
 * @param   stand_position		立ち位置情報
 *
 * @retval  BOOL		TRUE:正常セット。　FALSE:立ち位置が競合している。セット失敗
 *          返事用の送信バッファへのデータセットも同時に行う
 */
//==================================================================
BOOL Colosseum_Parent_SetStandingPosition(COLOSSEUM_SYSTEM_PTR clsys, int net_id, u8 stand_position)
{
  int i;
  BOOL result;
  
  if(stand_position == COLOSSEUM_STANDING_POSITION_NULL){
    clsys->parentsys.stand_position[net_id] = stand_position;
    result = TRUE;
  }
  else{
    for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
      if(clsys->parentsys.stand_position[i] == stand_position){
        OS_TPrintf("立ち位置バッティング！ 先=%d, 後=%d\n", i, net_id);
        result = FALSE;
        break;
      }
    }
    if(i == COLOSSEUM_MEMBER_MAX){
      clsys->parentsys.stand_position[net_id] = stand_position;
      result = TRUE;
    }
  }

  clsys->parentsys.answer_stand_position[net_id] = result;
  return result;
}

//==================================================================
/**
 * 親機専用命令：立ち位置許可の返事データがあれば送信を行う
 *
 * @param   clsys		
 */
//==================================================================
void Colosseum_Parent_SendAnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(clsys->parentsys.answer_stand_position[i] != COLOSSEUM_STANDING_POSITION_NULL){
      if(ColosseumSend_AnswerStandingPosition(clsys, i, clsys->parentsys.answer_stand_position[i]) == TRUE){
        clsys->parentsys.answer_stand_position[i] = COLOSSEUM_STANDING_POSITION_NULL;
        OS_TPrintf("立ち位置返事送信：成功：送信先net_id=%d\n", i);
      }
      else{
        OS_TPrintf("立ち位置返事送信：失敗：送信先net_id=%d\n", i);
      }
    }
  }
}

//==================================================================
/**
 * 親機専用命令：戦闘準備キャンセルの要求があれば送信を行う
 *               全員の戦闘準備が完了しているのであれば、それの送信を行う
 *
 * @param   clsys		
 */
//==================================================================
void Colosseum_Parent_BattleReadyAnswer(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i, ready_count;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  ready_count = 0;
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(clsys->parentsys.battle_ready[i] == TRUE){
      ready_count++;
    }
  }
  if(ready_count == GFL_NET_GetConnectNum()){
    if(ColosseumSend_AllBattleReady() == TRUE){
      for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){  //送信成功したなら今までの受信バッファはクリア
        clsys->parentsys.battle_ready[i] = FALSE;
        clsys->parentsys.battle_ready_cancel[i] = FALSE;
      }
    }
  }
  else{ //既に全員準備OKの場合は今更キャンセルさせない
    for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
      if(clsys->parentsys.battle_ready_cancel[i] == TRUE){
        if(ColosseumSend_BattleReadyCancelOK(i) == TRUE){
          clsys->parentsys.battle_ready_cancel[i] = FALSE;
        }
      }
    }
  }
}

//==================================================================
/**
 * 通信プレイヤーの座標パッケージをセット
 *
 * @param   clsys		
 * @param   net_id		対象プレイヤーのnetID
 * @param   pack		  座標パッケージ
 */
//==================================================================
void Colosseum_SetCommPlayerPos(COLOSSEUM_SYSTEM_PTR clsys, int net_id, const COMM_PLAYER_PACKAGE *pack)
{
  clsys->recvbuf.comm_player_pack[net_id] = *pack;
  clsys->recvbuf.comm_player_pack_update[net_id] = TRUE;
}

//==================================================================
/**
 * 通信プレイヤーの座標パッケージを取得
 *
 * @param   clsys		
 * @param   net_id		対象プレイヤーのnetID
 * @param   dest		  座標代入先
 *
 * @retval  BOOL		TRUE:更新あり　FALSE:更新なし
 * 
 * この関数で取得すると更新フラグがクリアされる事に注意
 */
//==================================================================
BOOL Colosseum_GetCommPlayerPos(COLOSSEUM_SYSTEM_PTR clsys, int net_id, COMM_PLAYER_PACKAGE *dest)
{
  BOOL update_flag;
  
  *dest = clsys->recvbuf.comm_player_pack[net_id];
  update_flag = clsys->recvbuf.comm_player_pack_update[net_id];
  clsys->recvbuf.comm_player_pack_update[net_id] = FALSE;
  return update_flag;
}

//==================================================================
/**
 * ポケモンリスト選択完了人数を取得する
 *
 * @param   clsys		
 *
 * @retval  u8		  選択完了人数(自分含む)
 */
//==================================================================
u8 ColosseumTool_ReceiveCheck_PokeListSeletedNum(COLOSSEUM_SYSTEM_PTR clsys)
{
  return clsys->recvbuf.pokelist_selected_num;
}

//==================================================================
/**
 * 受信バッファのクリア：ポケモンリスト選択完了人数
 *
 * @param   clsys		
 */
//==================================================================
void ColosseumTool_Clear_ReceivePokeListSelected(COLOSSEUM_SYSTEM_PTR clsys)
{
  clsys->recvbuf.pokelist_selected_num = 0;
}

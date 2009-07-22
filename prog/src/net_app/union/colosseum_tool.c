//==============================================================================
/**
 * @file    colosseum_tool.c
 * @brief   コロシアムツール類：オーバーレイに配置
 * @author  matsuda
 * @date    2009.07.18(土)
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
#include "colosseum_tool.h"


//==============================================================================
//  データ
//==============================================================================
///1vs1の時の立ち位置座標
static const struct{
  s16 x;
  s16 z;
}ColosseumStandingPos_1vs1[] = {
  {72, 88},   //左
  {136, 88},  //右
};

///マルチの時の立ち位置座標
static const struct{
  s16 x;
  s16 z;
}ColosseumStandingPos_multi[] = {
  {72, 72},       //左上
  {136, 72},      //右上
  {72, 104},      //左下
  {136, 104},     //右下
};



//==================================================================
/**
 * 全員分の基本情報が受け取れているか調べる
 *
 * @param   clsys		
 *
 * @retval  BOOL		TRUE:全員分受信している
 */
//==================================================================
BOOL ColosseumTool_AllReceiveCheck_BasicStatus(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i, count;
  
  count = 0;
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(clsys->basic_status[i].occ == TRUE){
      count++;
    }
  }
  if(count >= GFL_NET_GetConnectNum()){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 全員分のトレーナーカードが受け取れているか調べる
 *
 * @param   clsys		
 *
 * @retval  BOOL		TRUE:全員分受信している
 */
//==================================================================
BOOL ColosseumTool_AllReceiveCheck_TrainerCard(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i, count;
  
  count = 0;
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(clsys->tr_card_occ[i] == TRUE){
      count++;
    }
  }
  if(count >= GFL_NET_GetConnectNum()){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 立ち位置座標にいるかチェック
 *
 * @param   fieldWork		
 * @param   entry_num		参加人数
 * @param   stand_pos		結果代入先(立ち位置にいる場合、何番目の立ち位置かが返る)
 *
 * @retval  TRUE:立ち位置にいる。　FALSE:立ち位置にいない
 */
//==================================================================
BOOL ColosseumTool_CheckStandingPosition(FIELD_MAIN_WORK *fieldWork, int entry_num, int *stand_pos)
{
  VecFx32 pos;
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  int i;
  
  *stand_pos = 0;
  
  FIELD_PLAYER_GetPos(player, &pos);
  pos.x >>= FX32_SHIFT;
  pos.z >>= FX32_SHIFT;
  if(entry_num == 2){
    for(i = 0; i < NELEMS(ColosseumStandingPos_1vs1); i++){
      if(pos.x == ColosseumStandingPos_1vs1[i].x && pos.z == ColosseumStandingPos_1vs1[i].z){
        *stand_pos = i;
        return TRUE;
      }
    }
  }
  else{
    for(i = 0; i < NELEMS(ColosseumStandingPos_multi); i++){
      if(pos.x == ColosseumStandingPos_multi[i].x && pos.z == ColosseumStandingPos_multi[i].z){
        *stand_pos = i;
        return TRUE;
      }
    }
  }
  
  return FALSE;
}

//==================================================================
/**
 * 通信プレイヤーの座標更新
 *
 * @param   clsys		    
 */
//==================================================================
void ColosseumTool_CommPlayerUpdate(COLOSSEUM_SYSTEM_PTR clsys)
{
  int net_id, my_net_id;
  COMM_PLAYER_PACKAGE pospack;
  
  my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  for(net_id = 0; net_id < COLOSSEUM_MEMBER_MAX; net_id++){
    if(my_net_id == net_id){
      continue;
    }
    if(Colosseum_GetCommPlayerPos(clsys, net_id, &pospack) == TRUE){
      if(CommPlayer_CheckOcc(clsys->cps, net_id) == FALSE){
        CommPlayer_Add(clsys->cps, net_id, clsys->basic_status[net_id].sex, &pospack);
      }
      else{
        CommPlayer_SetParam(clsys->cps, net_id, &pospack);
      }
    }
  }
}


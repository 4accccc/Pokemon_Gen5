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
#include "fieldmap/zone_id.h"


//==============================================================================
//  構造体定義
//==============================================================================
///s16のポイント型
typedef struct{
  s16 x;
  s16 z;
}POINT_S16;

//==============================================================================
//  データ
//==============================================================================
//--------------------------------------------------------------
//  立ち位置座標
//--------------------------------------------------------------
///1vs1の時の立ち位置座標(グリッド座標)
static const POINT_S16 ColosseumStandingPos_1vs1[] = {
  {4, 5},   //左
  {8, 5},  //右
};

///マルチの時の立ち位置座標(グリッド座標)
static const POINT_S16 ColosseumStandingPos_multi[] = {
  {4, 4},     //左上
  {8, 4},     //右上
  {4, 6},     //左下
  {8, 6},     //右下
};

//--------------------------------------------------------------
//  出口座標
//--------------------------------------------------------------
///1vs1の時の立ち位置座標
static const POINT_S16 ColosseumWayOutPos_1vs1[] = {
  {88, 168},
  {104, 168},
  {120, 168},
};

///マルチの時の立ち位置座標
static const POINT_S16 ColosseumWayOutPos_multi[] = {
  {88, 168},
  {104, 168},
  {120, 168},
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
    if(clsys->recvbuf.tr_card_occ[i] == TRUE){
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
 * 全員分のPOKEPARTYが受け取れているか調べる
 *
 * @param   clsys		
 *
 * @retval  BOOL		TRUE:全員分受信している
 */
//==================================================================
BOOL ColosseumTool_AllReceiveCheck_Pokeparty(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i, count;
  
  count = 0;
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(clsys->recvbuf.pokeparty_occ[i] == TRUE){
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
 * 受信バッファのクリア：POKEPARTY
 *
 * @param   clsys		
 * @param   except_for_mine   TRUE:自分のデータはクリアしない(受信フラグは落とす)
 */
//==================================================================
void ColosseumTool_Clear_ReceivePokeParty(COLOSSEUM_SYSTEM_PTR clsys, BOOL except_for_mine)
{
  int i, my_net_id;
  
  my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(except_for_mine == FALSE || my_net_id != i){
      PokeParty_InitWork(clsys->recvbuf.pokeparty[i]);
    }
    clsys->recvbuf.pokeparty_occ[i] = FALSE;
  }
}

//==================================================================
/**
 * 全員分の立ち位置が受け取れているか調べる
 *
 * @param   clsys		
 *
 * @retval  BOOL		TRUE:OK
 */
//==================================================================
BOOL ColosseumTool_ReceiveCheck_StandingPos(COLOSSEUM_SYSTEM_PTR clsys)
{
  if(clsys->recvbuf.stand_position_occ == TRUE){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 受信バッファのクリア：全員分の立ち位置受信
 *
 * @param   clsys		
 */
//==================================================================
void ColosseumTool_Clear_ReceiveStandingPos(COLOSSEUM_SYSTEM_PTR clsys)
{
  clsys->recvbuf.stand_position_occ = FALSE;
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
BOOL ColosseumTool_CheckStandingPosition(FIELDMAP_WORK *fieldWork, int entry_num, int *stand_pos)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  int i;
  s16 grid_x, grid_z;
  MMDL *player_mmdl;
  
  *stand_pos = 0;
  
  player_mmdl = FIELD_PLAYER_GetMMdl( player );
  grid_x = MMDL_GetGridPosX(player_mmdl);
  grid_z = MMDL_GetGridPosZ(player_mmdl);

  if(entry_num == 2){
    for(i = 0; i < NELEMS(ColosseumStandingPos_1vs1); i++){
      if(grid_x == ColosseumStandingPos_1vs1[i].x && grid_z == ColosseumStandingPos_1vs1[i].z){
        *stand_pos = i;
        return TRUE;
      }
    }
  }
  else{
    for(i = 0; i < NELEMS(ColosseumStandingPos_multi); i++){
      if(grid_x == ColosseumStandingPos_multi[i].x && grid_z == ColosseumStandingPos_multi[i].z){
        *stand_pos = i;
        return TRUE;
      }
    }
  }
  
  return FALSE;
}

//==================================================================
/**
 * 全員分の「退出します」が受け取れているか調べる
 *
 * @param   clsys		
 *
 * @retval  BOOL		TRUE:全員分受信している
 */
//==================================================================
BOOL ColosseumTool_AllReceiveCheck_Leave(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i, count;
  
  count = 0;
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(clsys->recvbuf.leave[i] == TRUE){
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
 * 誰かから「退出します」フラグが来ていないか調べる
 *
 * @param   clsys		
 *
 * @retval  BOOL		TRUE:フラグが来ている
 */
//==================================================================
BOOL ColosseumTool_ReceiveCheck_Leave(COLOSSEUM_SYSTEM_PTR clsys)
{
  int i;
  for(i = 0; i < COLOSSEUM_MEMBER_MAX; i++){
    if(clsys->recvbuf.leave[i] == TRUE){
      return TRUE;
    }
  }
  return FALSE;
}

//==================================================================
/**
 * コロシアムの出口座標にいるかチェック
 *
 * @param   fieldWork		
 *
 * @retval  BOOL		TRUE:出口座標にいる
 */
//==================================================================
BOOL ColosseumTool_CheckWayOut(FIELDMAP_WORK *fieldWork)
{
  VecFx32 pos;
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  const POINT_S16 *postbl;
  int i, tbl_max;
  
  FIELD_PLAYER_GetPos(player, &pos);
  pos.x >>= FX32_SHIFT;
  pos.z >>= FX32_SHIFT;

  if(FIELDMAP_GetZoneID(fieldWork) == ZONE_ID_CLOSSEUM){
    postbl = ColosseumWayOutPos_1vs1;
    tbl_max = NELEMS(ColosseumWayOutPos_1vs1);
  }
  else{
    postbl = ColosseumWayOutPos_multi;
    tbl_max = NELEMS(ColosseumWayOutPos_multi);
  }
  
  for(i = 0; i < tbl_max; i++){
    if(pos.x == postbl[i].x && pos.z == postbl[i].z){
      return TRUE;
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
        u16 obj_code = (clsys->basic_status[net_id].sex == PM_MALE) ? HERO : HEROINE;
        CommPlayer_Add(clsys->cps, net_id, obj_code, &pospack);
      }
      else{
        CommPlayer_SetParam(clsys->cps, net_id, &pospack);
      }
    }
  }
}

#if 0 //※check
void ColosseumTool_ActorSetup(COLOSSEUM_SYSTEM_PTR clsys)
{
  clsys->clunit = GFL_CLACT_UNIT_Create( 6, 10, HEAPID_WORLD);
}

void ColosseumTool_ActorExit(COLOSSEUM_SYSTEM_PTR clsys)
{
  if(clsys->clunit != NULL){
    GFL_CLACT_UNIT_Delete(clsys->clunit);
    clsys->clunit = NULL;
  }
}

void ColosseumTool_PokeIconAdd(COLOSSEUM_SYSTEM_PTR clsys, POKEPARTY *party)
{
  GF_ASSERT(clsys->clunit != NULL);
  
  //リソース登録
  for(i = 0; i < TEMOTI_POKEMAX; i++){
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(party, i);
    BOOL backup_fast;
    u32 data_idx;
    
    backup_fast = PP_FastModeOn( pp );
    
    monsno = PP_Get( pp, ID_PARA_monsno, NULL);
    monsno = PP_Get( pp, ID_PARA_form_no, NULL);
    monsno = PP_Get( pp, ID_PARA_monsno_egg, NULL);
    data_idx = POKEICON_GetCgxArcIndexByMonsNumber( u32 mons, u32 form_no, BOOL egg );
    
    PP_FastModeOff( pp, backup_fast );
    
  {//リソース登録
    ARCHANDLE *handle;

    handle = GFL_ARC_OpenDataHandle(ARCID_PALACE, heap_id);
    
  	number->cgr_id = GFL_CLGRP_CGR_Register( 
  	  handle, NARC_palace_debug_areano_NCGR, FALSE, CLSYS_DRAW_MAIN, heap_id );
  	number->pltt_id = GFL_CLGRP_PLTT_RegisterEx( 
  	  handle, NARC_palace_debug_areano_NCLR, CLSYS_DRAW_MAIN, 15*32, 0, 1, heap_id );
  	number->cell_id = GFL_CLGRP_CELLANIM_Register( 
  	  handle, NARC_palace_debug_areano_NCER, NARC_palace_debug_areano_NANR, heap_id );
  	
  	GFL_ARC_CloseDataHandle(handle);
  }
  
  {//アクター生成
    static const GFL_CLWK_DATA clwkdata = {
      16, 40, 0, 1, 0,
    };
    
    number->clact = GFL_CLACT_WK_Create(
      palace->clunit, number->cgr_id, number->pltt_id, number->cell_id, 
      &clwkdata, CLSYS_DEFREND_MAIN, heap_id);
    GFL_CLACT_WK_SetAutoAnmFlag(number->clact, TRUE); //オートアニメON
    GFL_CLACT_WK_SetDrawEnable(number->clact, FALSE);
  }
  
}
#endif

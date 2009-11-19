//==============================================================================
/**
 * @file    intrude_work.c
 * @brief   侵入システムワークに外部から触る為の関数群
 * @author  matsuda
 * @date    2009.10.10(土)
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
#include "bingo_types.h"
#include "intrude_work.h"


//--------------------------------------------------------------
/**
 * 侵入システムワークを取得する
 *
 * @param   game_comm		
 *
 * @retval  INTRUDE_COMM_SYS_PTR		侵入システムワークへのポインタ
 * @retval  INTRUDE_COMM_SYS_PTR    侵入システムが起動していない場合はNULL
 */
//--------------------------------------------------------------
static INTRUDE_COMM_SYS_PTR _GetIntrudeCommSys(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(game_comm);
  
  if(intcomm == NULL || GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION){
    GF_ASSERT(0);
    return NULL;
  }
  return intcomm;
}

//==================================================================
/**
 * アクションステータスを設定
 *
 * @param   game_comm		
 * @param   action		
 */
//==================================================================
void IntrudeWork_SetActionStatus(GAME_COMM_SYS_PTR game_comm, INTRUDE_ACTION action)
{
  INTRUDE_COMM_SYS_PTR intcomm = _GetIntrudeCommSys(game_comm);
  
  if(intcomm == NULL){
    return;
  }
  
  Intrude_SetActionStatus(intcomm, action);
  intcomm->send_status = TRUE;
}

//==================================================================
/**
 * ワープする街番号を設定
 *
 * @param   game_comm		
 * @param   town_tblno		
 */
//==================================================================
void Intrude_SetWarpTown(GAME_COMM_SYS_PTR game_comm, int town_tblno)
{
  INTRUDE_COMM_SYS_PTR intcomm = _GetIntrudeCommSys(game_comm);
  
  if(intcomm == NULL){
    return;
  }
  
  intcomm->warp_town_tblno = town_tblno;
}

//==================================================================
/**
 * ワープ先の街番号を取得
 *
 * @param   game_comm		
 *
 * @retval  int		街番号(PALACE_TOWN_DATA_NULLの場合は無効)
 */
//==================================================================
int Intrude_GetWarpTown(GAME_COMM_SYS_PTR game_comm)
{
  INTRUDE_COMM_SYS_PTR intcomm = _GetIntrudeCommSys(game_comm);
  
  if(intcomm == NULL){
    return PALACE_TOWN_DATA_NULL;
  }
  return intcomm->warp_town_tblno;
}

//==================================================================
/**
 * 相手指定のタイミングコマンドが受信できているか確認します
 *
 * @param   intcomm		
 * @param   timing_no		    同期番号
 * @param   target_netid		同期相手のNetID
 *
 * @retval  BOOL		TRUE:同期取り完了。　FALSE:同期待ち
 */
//==================================================================
BOOL Intrude_GetTargetTimingNo(INTRUDE_COMM_SYS_PTR intcomm, u8 timing_no, u8 target_netid)
{
  u8 recv_timing_no = intcomm->recv_target_timing_no;
  
  if(timing_no == recv_timing_no && target_netid == intcomm->recv_target_timing_netid){
    intcomm->recv_target_timing_no = 0;
    return TRUE;
  }
  return FALSE;
}

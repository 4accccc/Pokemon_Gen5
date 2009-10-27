//==============================================================================
/**
 * @file    intrude_mission.c
 * @brief   ミッション処理    受信データのセットなどを行う為、常駐に配置
 * @author  matsuda
 * @date    2009.10.26(月)
 */
//==============================================================================
#include <gflib.h>
#include "intrude_types.h"
#include "intrude_mission.h"
#include "intrude_comm_command.h"
#include "msg/msg_invasion.h"


//==================================================================
/**
 * ミッションデータ初期化
 *
 * @param   mission		ミッションデータへのポインタ
 */
//==================================================================
void MISSION_Init(MISSION_DATA *mission)
{
  mission->mission_no = MISSION_NO_NULL;
}

//==================================================================
/**
 * ミッションリクエストデータを受け取り、ミッションを発動する
 *
 * @param   mission		      ミッションデータへのポインタ
 * @param   req		          リクエストデータへのポインタ
 * @param   accept_netid		ミッション受注者のNetID
 *
 * @retval  BOOL		TRUE:ミッション発動。　FALSE:発動できない
 */
//==================================================================
BOOL MISSION_SetEntry(INTRUDE_COMM_SYS_PTR intcomm, MISSION_DATA *mission, const MISSION_REQ *req, int accept_netid)
{
  int i, target_no, connect_num;
  
  if(mission->mission_no != MISSION_NO_NULL){
    return FALSE;
  }
  mission->mission_no = GFUser_GetPublicRand0(MISSION_NO_MAX);
  mission->accept_netid = accept_netid;
  mission->monolith_type = req->monolith_type;
  
  connect_num = GFL_NET_GetConnectNum();
  target_no = GFUser_GetPublicRand0(connect_num - 1); // -1 = 受注者分
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(i != accept_netid && intcomm->recv_profile & (1 << i)){
      if(target_no == 0){
        mission->target_netid = i;
        break;
      }
    }
  }
  if(i == FIELD_COMM_MEMBER_MAX){
    GF_ASSERT_MSG(0, "connect_num = %d, accept_netid = %d, recv_profile = %d\n", 
      connect_num, accept_netid, intcomm->recv_profile);
    mission->target_netid = 0;
  }
  
  return TRUE;
}

//==================================================================
/**
 * ミッションデータをセット
 *
 * @param   mission		代入先
 * @param   src		    セットするデータ
 */
//==================================================================
void MISSION_SetMissionData(MISSION_DATA *mission, const MISSION_DATA *src)
{
  if(mission->mission_no != MISSION_NO_NULL){
    return;
  }
  
  *mission = *src;
  OS_TPrintf("mission受信 mission_no = %d\n", src->mission_no);
  //不正チェック
#if 0 //もう少ししっかりとデータの取り扱いが決まってから有効にする
  if(mission->mission_no >= MISSION_NO_MAX || mission->monolith_type >= MONOLITH_TYPE_MAX){
    return;
  }
#endif
}

//==================================================================
/**
 * ミッションデータ送信リクエストが発生していれば送信を行う
 *
 * @param   intcomm		
 * @param   mission		
 */
//==================================================================
void MISSION_SendUpdate(INTRUDE_COMM_SYS_PTR intcomm, MISSION_DATA *mission)
{
  if(intcomm->mission_send_req == TRUE){
    if(IntrudeSend_MissonData(intcomm, mission) == TRUE){
      intcomm->mission_send_req = FALSE;
    }
  }
}

//==================================================================
/**
 * ミッションデータ受信済みかチェック
 *
 * @param   mission		
 *
 * @retval  BOOL		TRUE:受信済み　FALSE:受信していない
 */
//==================================================================
BOOL MISSION_RecvCheck(const MISSION_DATA *mission)
{
  if(mission->mission_no == MISSION_NO_NULL){
    return FALSE;
  }
  return TRUE;
}

//==================================================================
/**
 * ミッションデータから対応したミッションメッセージIDを取得する
 *
 * @param   mission		
 *
 * @retval  u16		メッセージID
 */
//==================================================================
u16 MISSION_GetMissionMsgID(const MISSION_DATA *mission)
{
  u16 msg_id;
  
  if(mission->mission_no == MISSION_NO_NULL || mission->mission_no >= MISSION_NO_MAX){
    return msg_invasion_mission000;
  }
  msg_id = mission->mission_no * 2 + mission->monolith_type;
  return msg_invasion_mission001 + msg_id;
}

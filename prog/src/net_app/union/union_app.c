//==============================================================================
/**
 * @file    union_app.c
 * @brief   ユニオンルームでのミニゲーム制御系：union_appオーバーレイに配置
 * @author  matsuda
 * @date    2010.01.06(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "union_types.h"
#include "union_local.h"
#include "savedata/mystatus.h"
#include "net_app/union/union_main.h"
#include "system/net_err.h"
#include "net/network_define.h"
#include "union_comm_command.h"
#include "union_msg.h"
#include "comm_player.h"
#include "union_subproc.h"
#include "union_chat.h"
#include "net_app/union_app.h"
#include "union_app_local.h"



//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _SendUpdate_LeavePlayer(UNION_APP_PTR uniapp);
static void _SendUpdate_BasicStatus(UNION_APP_PTR uniapp);
static void _SendUpdate_Mystatus(UNION_APP_PTR uniapp);
static void _Update_IntrudeReadyCallback(UNION_APP_PTR uniapp);
static void _Update_LeaveCallback(UNION_APP_PTR uniapp);




//==============================================================================
//
//  システム側で使用
//
//==============================================================================
//==================================================================
/**
 * ユニオンアプリ制御ワークをAllocします
 *
 * @param   heap_id		
 * @param   member_max		参加最大数
 *
 * @retval  UNION_APP_PTR		
 */
//==================================================================
UNION_APP_PTR UnionAppSystem_AllocAppWork(HEAPID heap_id, u8 member_max, const MYSTATUS *myst)
{
  UNION_APP_PTR uniapp;
  int i;
  
  GF_ASSERT(member_max <= UNION_APP_MEMBER_MAX);
  
  uniapp = GFL_HEAP_AllocClearMemory(heap_id, sizeof(struct _UNION_APP_WORK));
  OS_TPrintf("aaa size uniapp = %d\n", sizeof(struct _UNION_APP_WORK)); //※check
  for(i = 0; i < UNION_APP_MEMBER_MAX; i++){
    uniapp->mystatus[i] = MyStatus_AllocWork(heap_id);
  }
  
  if(GFL_NET_IsParentMachine() == TRUE){
    uniapp->basic_status.member_bit = 3;  //最初は二人通信から始まる (NetID 0と1)
  }
  uniapp->basic_status.member_max = member_max;
  
  //自分のMYSTATUSセット
  UnionAppSystem_SetMystatus(uniapp, 
    GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()), myst);
  
  Union_App_Parent_EntryBlock(uniapp);
  Union_App_Parent_LeaveBlock(uniapp);
  
  return uniapp;
}

//==================================================================
/**
 * ユニオンアプリ制御ワークを解放します
 *
 * @param   uniapp		
 */
//==================================================================
void UnionAppSystem_FreeAppWork(UNION_APP_PTR uniapp)
{
  int i;
  
  for(i = 0; i < UNION_APP_MEMBER_MAX; i++){
    GFL_HEAP_FreeMemory(uniapp->mystatus[i]);
  }
  GFL_HEAP_FreeMemory(uniapp);
}

//==================================================================
/**
 * 乱入希望者登録
 *
 * @param   uniapp		
 * @param   net_id		乱入希望者のNetID
 * @param   myst		  乱入希望者のMYSTATUS
 *
 * @retval  BOOL		TRUE:乱入OK　FALSE:乱入NG
 */
//==================================================================
BOOL UnionAppSystem_SetEntryUser(UNION_APP_PTR uniapp, NetID net_id, const MYSTATUS *myst)
{
  if(uniapp->entry_block == FALSE){
    u32 member_num = MATH_CountPopulation((u32)(uniapp->basic_status.member_bit));
    u32 entry_num = MATH_CountPopulation((u32)(uniapp->entry_reserve_bit));
    if(member_num + entry_num < uniapp->basic_status.member_max){
      uniapp->entry_reserve_bit |= 1 << net_id;
      //MyStatus_Copy(myst, uniapp->mystatus[net_id]);
      return TRUE;
    }
  }
  return FALSE;
}

//==================================================================
/**
 * ユニオンアプリ制御更新処理
 *
 * @param   uniapp		
 * @param   unisys		
 */
//==================================================================
void UnionAppSystem_Update(UNION_APP_PTR uniapp, UNION_SYSTEM_PTR unisys)
{
  if(uniapp->shutdown == TRUE){
    return;
  }
  if(uniapp->shutdown_req == TRUE){
    UnionComm_Req_Shutdown(unisys);
    uniapp->shutdown_req = FALSE;
    uniapp->shutdown_exe = TRUE;
    return;
  }
  if(uniapp->shutdown_exe == TRUE){
    if(UnionComm_Check_ShutdownRestarts(unisys) == FALSE){
      uniapp->shutdown_exe = FALSE;
      uniapp->shutdown = TRUE;
    }
    return;
  }
  
  
  if(GFL_NET_IsParentMachine() == TRUE){
    _SendUpdate_BasicStatus(uniapp);
    _SendUpdate_LeavePlayer(uniapp);
  }
  _SendUpdate_Mystatus(uniapp);
  _Update_IntrudeReadyCallback(uniapp);
  _Update_LeaveCallback(uniapp);
}

//--------------------------------------------------------------
/**
 * 離脱者がいれば離脱した事を他のメンバーへ送信する
 *
 * @param   uniapp		
 */
//--------------------------------------------------------------
static void _SendUpdate_LeavePlayer(UNION_APP_PTR uniapp)
{
  u32 now_member_bit = uniapp->basic_status.member_bit;
  int net_id;
  
  //退出者のNetIDを調査
  for(net_id = 0; net_id < UNION_APP_MEMBER_MAX; net_id++){
    if((uniapp->basic_status.member_bit & (1 << net_id))
        && GFL_NET_IsConnectMember(net_id) == FALSE){
      now_member_bit ^= 1 << net_id;
    }
  }

  //残っているメンバーへ向けて退出者のNetIDを送信する
  if(now_member_bit != uniapp->basic_status.member_bit){
    for(net_id = 0; net_id < UNION_APP_MEMBER_MAX; net_id++){
      if((uniapp->basic_status.member_bit & (1 << net_id))
          && (now_member_bit & (1 << net_id)) == 0){
        if(UnionSend_MinigameLeaveChild(now_member_bit, net_id) == TRUE){
          uniapp->basic_status.member_bit ^= 1 << net_id;
        }
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 基本情報要求リクエストが発生していれば送信
 *
 * @param   uniapp		
 */
//--------------------------------------------------------------
static void _SendUpdate_BasicStatus(UNION_APP_PTR uniapp)
{
  int i;
  
  if(uniapp->recv_basic_status_req_bit > 0){
    if(UnionSend_MinigameBasicStatus(
        &uniapp->basic_status, uniapp->recv_basic_status_req_bit) == TRUE){
      uniapp->recv_basic_status_req_bit = 0;
    }
  }
}

//--------------------------------------------------------------
/**
 * MYSTATUS要求リクエストが発生していれば送信
 *
 * @param   uniapp		
 */
//--------------------------------------------------------------
static void _SendUpdate_Mystatus(UNION_APP_PTR uniapp)
{
  int i;
  
  if(uniapp->recv_mystatus_req_bit > 0){
    if(UnionSend_MinigameMystatus(uniapp->recv_mystatus_req_bit,
        uniapp->mystatus[GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())]) == TRUE){
      uniapp->recv_mystatus_req_bit = 0;
    }
  }
}

//--------------------------------------------------------------
/**
 * 乱入コールバック
 *
 * @param   uniapp		
 */
//--------------------------------------------------------------
static void _Update_IntrudeReadyCallback(UNION_APP_PTR uniapp)
{
  int net_id;
  
  if(uniapp->recv_intrude_ready_bit > 0 && uniapp->entry_callback != NULL){
    for(net_id = 0; net_id < UNION_APP_MEMBER_MAX; net_id++){
      if(uniapp->recv_intrude_ready_bit & (1 << net_id)){
        if((uniapp->recv_mystatus_bit & (1 << net_id)) 
            && uniapp->entry_reserve_bit & (1 << net_id)){
          OS_TPrintf("乱入CallBack NetID=%d\n", net_id);
          uniapp->entry_callback(net_id, uniapp->mystatus[net_id], uniapp->userwork);
          //予約状態から正規メンバーへと移す
          uniapp->entry_reserve_bit ^= 1 << net_id;
          uniapp->basic_status.member_bit |= 1 << net_id;
        }
        uniapp->recv_intrude_ready_bit ^= 1 << net_id;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 離脱コールバック
 *
 * @param   uniapp		
 */
//--------------------------------------------------------------
static void _Update_LeaveCallback(UNION_APP_PTR uniapp)
{
  int net_id;
  
  if(uniapp->recv_leave_bit > 0 && uniapp->leave_callback != NULL){
    for(net_id = 0; net_id < UNION_APP_MEMBER_MAX; net_id++){
      if(uniapp->recv_leave_bit & (1 << net_id)){
        if((uniapp->recv_mystatus_bit & (1 << net_id)) 
            && uniapp->basic_status.member_bit & (1 << net_id)){
          OS_TPrintf("離脱CallBack NetID=%d\n", net_id);
          uniapp->leave_callback(net_id, uniapp->mystatus[net_id], uniapp->userwork);
          //正規メンバーのbitを落とす
          uniapp->basic_status.member_bit ^= 1 << net_id;
          uniapp->recv_mystatus_bit ^= 1 << net_id;
        }
        uniapp->recv_leave_bit ^= 1 << net_id;
      }
    }
  }
}

//==================================================================
/**
 * 基本情報要求リクエストを登録
 *
 * @param   uniapp		
 * @param   net_id		要求相手のNetID
 */
//==================================================================
void UnionAppSystem_ReqBasicStatus(UNION_APP_PTR uniapp, NetID net_id)
{
  uniapp->recv_basic_status_req_bit |= 1 << net_id;
}

//==================================================================
/**
 * 基本情報をセット
 *
 * @param   uniapp		
 * @param   basic_status		
 */
//==================================================================
void UnionAppSystem_SetBasicStatus(UNION_APP_PTR uniapp, const UNION_APP_BASIC *basic_status)
{
  GFL_STD_MemCopy(basic_status, &uniapp->basic_status, sizeof(UNION_APP_BASIC_STATUS));
}

//==================================================================
/**
 * MYSTATUS要求リクエストを登録
 *
 * @param   uniapp		
 * @param   net_id		要求相手のNetID
 */
//==================================================================
void UnionAppSystem_ReqMystatus(UNION_APP_PTR uniapp, NetID net_id)
{
  uniapp->recv_mystatus_req_bit |= 1 << net_id;
}

//==================================================================
/**
 * MYSTATUSをセット
 *
 * @param   uniapp		
 * @param   net_id		
 * @param   myst		
 */
//==================================================================
void UnionAppSystem_SetMystatus(UNION_APP_PTR uniapp, NetID net_id, const MYSTATUS *myst)
{
  MyStatus_Copy(myst, uniapp->mystatus[net_id]);
  uniapp->recv_mystatus_bit |= 1 << net_id;
}

//==================================================================
/**
 * 基本情報を受け取っているか調べる
 *
 * @param   uniapp		
 *
 * @retval  BOOL		TRUE:受け取っている。　FALSE:受け取っていない
 */
//==================================================================
BOOL UnionAppSystem_CheckBasicStatus(UNION_APP_PTR uniapp)
{
  if(uniapp->basic_status.member_bit > 0){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 乱入待ちではなく正式にゲームに参加しているユーザー全員分のMYSTATUSを受け取っているか調べる
 *
 * @param   uniapp		
 *
 * @retval  BOOL		TRUE:全員分受け取っている
 */
//==================================================================
BOOL UnionAppSystem_CheckMystatus(UNION_APP_PTR uniapp)
{
  int net_id;
  
  for(net_id = 0; net_id < UNION_APP_MEMBER_MAX; net_id++){
    if(uniapp->basic_status.member_bit & (1 << net_id)){
      if(GFL_NET_IsConnectMember(net_id) == TRUE){  //通信が切れているものは無視する
        if((uniapp->recv_mystatus_bit & (1 << net_id)) == 0){
          return FALSE; //一台でもMYSTATUSが届いていないものがあった
        }
      }
    }
  }
  
  return TRUE;
}

//==================================================================
/**
 * 乱入準備完了宣言を受信
 *
 * @param   uniapp		
 * @param   net_id		乱入者のNetID
 */
//==================================================================
void UnionAppSystem_SetIntrudeReady(UNION_APP_PTR uniapp, NetID net_id)
{
  if((uniapp->recv_mystatus_bit & (1 << net_id)) && (uniapp->basic_status.member_bit & (1 << net_id))){
    uniapp->recv_intrude_ready_bit |= 1 << net_id;
  }
  else{
    GF_ASSERT_MSG(0, "recv_mystbit=%d, member_bit=%d, net_id=%d\n", 
      uniapp->recv_mystatus_req_bit, uniapp->basic_status.member_bit, net_id);
  }
}

//==================================================================
/**
 * 子機が離脱しましたを受信
 *
 * @param   uniapp		
 * @param   net_id		離脱した子機のNetID
 */
//==================================================================
void UnionAppSystem_SetLeaveChild(UNION_APP_PTR uniapp, NetID net_id)
{
  if((uniapp->recv_mystatus_bit & (1 << net_id)) && (uniapp->basic_status.member_bit & (1 << net_id))){
    uniapp->recv_leave_bit |= 1 << net_id;
  }
  else{
    GF_ASSERT_MSG(0, "recv_mystbit=%d, member_bit=%d, net_id=%d\n", 
      uniapp->recv_mystatus_req_bit, uniapp->basic_status.member_bit, net_id);
  }
}

//==================================================================
/**
 * 基本情報構造体のサイズを取得する
 *
 * @retval  u32		
 */
//==================================================================
u32 UnionAppSystem_GetBasicSize(void)
{
  return sizeof(UNION_APP_BASIC_STATUS);
}



//==============================================================================
//
//  アプリ側で使用
//
//==============================================================================
//==================================================================
/**
 * 乱入、退出時のコールバック関数を登録
 *
 * @param   uniapp		        ユニオンアプリ制御ワークへのポインタ
 * @param   entry_callback		乱入コールバック
 * @param   eave_callback		  退出コールバック
 * @param   userwork		      コールバック関数で引数として渡されるワークのポインタ
 */
//==================================================================
void Union_App_SetCallback(UNION_APP_PTR uniapp, UNION_APP_CALLBACK_ENTRY_FUNC entry_callback, UNION_APP_CALLBACK_LEAVE_FUNC leave_callback, void *userwork)
{
  uniapp->entry_callback = entry_callback;
  uniapp->leave_callback = leave_callback;
  uniapp->userwork = userwork;
}

//==================================================================
/**
 * 乱入を禁止する　※親機専用命令
 *
 * @param   uniapp		
 *
 * @retval  BOOL		  TRUE:禁止にした。　FALSE:禁止に出来ない(既に乱入希望者がいる)
 */
//==================================================================
BOOL Union_App_Parent_EntryBlock(UNION_APP_PTR uniapp)
{
  if(uniapp->entry_reserve_bit == 0){
    GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(), FALSE);
    uniapp->entry_block = TRUE;
    return TRUE;
  }
  
  return FALSE;
}

//==================================================================
/**
 * 乱入禁止をリセットする　※親機専用命令
 *
 * @param   uniapp		
 */
//==================================================================
void Union_App_Parent_ResetEntryBlock(UNION_APP_PTR uniapp)
{
  GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(), TRUE);
  uniapp->entry_block = FALSE;
}

//==================================================================
/**
 * 退出を禁止する(Union_App_ReqLeaveの結果が常にNGになります)　※親機専用命令
 *
 * @param   uniapp		
 */
//==================================================================
void Union_App_Parent_LeaveBlock(UNION_APP_PTR uniapp)
{
  uniapp->leave_block = TRUE;
}

//==================================================================
/**
 * 退出禁止をリセットします　※親機専用命令
 *
 * @param   uniapp		
 */
//==================================================================
void Union_App_Parent_ResetLeaveBlock(UNION_APP_PTR uniapp)
{
  uniapp->leave_block = FALSE;
}

//==================================================================
/**
 * 親機に途中退出の許可をリクエストします
 *
 * @param   uniapp		
 *
 * この関数実行後、Union_App_GetLeaveResultで結果の取得を行ってください
 */
//==================================================================
void Union_App_ReqLeave(UNION_APP_PTR uniapp)
{
  uniapp->send_leave_req = TRUE;
  uniapp->recv_leave_req_result = UNION_APP_LEAVE_NULL;
}

//==================================================================
/**
 * 途中退出リクエストの親機からの返事を取得します
 *
 * @param   uniapp		
 *
 * @retval  UNION_APP_LEAVE		親機からの返事
 */
//==================================================================
UNION_APP_LEAVE Union_App_GetLeaveResult(UNION_APP_PTR uniapp)
{
  return uniapp->recv_leave_req_result;
}

//==================================================================
/**
 * 通信切断します
 *
 * @param   uniapp		
 */
//==================================================================
void Union_App_Shutdown(UNION_APP_PTR uniapp)
{
  uniapp->shutdown_req = TRUE;
}

//==================================================================
/**
 * 通信切断の完了待ち
 *
 * @param   uniapp		
 *
 * @retval  BOOL		TRUE:通信切断完了　FALSE:未完了
 */
//==================================================================
BOOL Union_App_WaitShutdown(UNION_APP_PTR uniapp)
{
  return uniapp->shutdown;
}

//==================================================================
/**
 * 指定NetIDのMYSTATUSを取得します
 *
 * @param   net_id		取得したい相手のNetID
 *
 * @retval  const MYSTATUS *		MYSTATUSへのポインタ(NetIDのユーザーが退出している場合はNULL)
 */
//==================================================================
const MYSTATUS * Union_App_GetMystatus(UNION_APP_PTR uniapp, NetID net_id)
{
  GF_ASSERT_MSG(uniapp->recv_mystatus_bit & (1 << net_id), 
    "net_id=%d, recvmystbit=%d, member_bit=%d\n", 
    net_id, uniapp->recv_mystatus_bit, uniapp->basic_status.member_bit);
  
  return uniapp->mystatus[net_id];
}

//==================================================================
/**
 * 接続しているプレイヤーのNetIDをbit単位で取得します
 *    ※GFL_NET_SendDataExBitでそのまま使える形です
 *
 * @param   uniapp		
 *
 * @retval  u32		接続プレイヤーNetID(bit単位)
 *
 * NetID:0番 NetID:2番 のプレイヤーだけが繋がっている場合は
 *    return (1 << 0) | (1 << 2);
 * といった値が返ります
 */
//==================================================================
u32 Union_App_GetMemberNetBit(UNION_APP_PTR uniapp)
{
  return uniapp->basic_status.member_bit;
}


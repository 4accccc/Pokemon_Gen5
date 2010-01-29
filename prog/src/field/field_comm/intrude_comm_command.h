//==============================================================================
/**
 * @file    intrude_comm_command.h
 * @brief   侵入：通信コマンド
 * @author  matsuda
 * @date    2009.09.03(木)
 */
//==============================================================================
#pragma once

#include <gflib.h>
#include "net/network_define.h"
#include "intrude_types.h"
#include "field\field_wfbc_data.h"


//==============================================================================
//  定数定義
//==============================================================================

///ユニオン受信コマンド番号　　※Union_CommPacketTblと並びを同じにしておくこと！！
enum INTRUDE_CMD{
  INTRUDE_CMD_START = GFL_NET_CMD_PALACE,
  
  INTRUDE_CMD_SHUTDOWN = INTRUDE_CMD_START,   ///<切断
  INTRUDE_CMD_MEMBER_NUM,                     ///<侵入参加人数
  INTRUDE_CMD_PROFILE_REQ,                    ///<プロフィール要求
  INTRUDE_CMD_PROFILE,                        ///<プロフィールデータ
  INTRUDE_CMD_DELETE_PROFILE,                 ///<離脱者のプロフィール削除
  INTRUDE_CMD_PLAYER_STATUS,                  ///<侵入ステータス(現在情報)
  INTRUDE_CMD_TALK,                           ///<話しかける
  INTRUDE_CMD_TALK_ANSWER,                    ///<話しかけられたので自分の状況を返す
  INTRUDE_CMD_TALK_CANCEL,                    ///<話しかけをキャンセル
  INTRUDE_CMD_BINGO_INTRUSION,                ///<ビンゴバトル乱入許可要求
  INTRUDE_CMD_BINGO_INTRUSION_ANSWER,         ///<ビンゴバトル乱入許可要求の返事
  INTRUDE_CMD_REQ_BINGO_INTRUSION_PARAM,      ///<ビンゴバトル乱入用のパラメータ要求
  INTRUDE_CMD_BINGO_INTRUSION_PARAM,          ///<ビンゴバトル乱入用のパラメータ
  INTRUDE_CMD_MISSION_LIST_REQ,               ///<ミッション選択候補リスト要求リクエスト
  INTRUDE_CMD_MISSION_LIST,                   ///<ミッション選択候補リスト
  INTRUDE_CMD_MISSION_ORDER_CONFIRM,          ///<ミッション受注します
  INTRUDE_CMD_MISSION_ENTRY_ANSWER,           ///<「ミッション受注します」の返事
  INTRUDE_CMD_MISSION_REQ,                    ///<ミッション要求リクエスト
  INTRUDE_CMD_MISSION_DATA,                   ///<ミッションデータ
  INTRUDE_CMD_MISSION_START,                  ///<ミッション開始
  INTRUDE_CMD_MISSION_ACHIEVE,                ///<ミッション達成報告
  INTRUDE_CMD_MISSION_ACHIEVE_ANSWER,         ///<ミッション達成報告の返事
  INTRUDE_CMD_MISSION_RESULT,                 ///<ミッション結果
  INTRUDE_CMD_OCCUPY_INFO,                    ///<占拠情報
  INTRUDE_CMD_TARGET_TIMING,                  ///<相手指定のタイミングコマンド
  INTRUDE_CMD_PLAYER_SUPPORT,                 ///<通信相手をサポート
  INTRUDE_CMD_WFBC_REQ,                       ///<WFBCパラメータ要求
  INTRUDE_CMD_WFBC,                           ///<WFBCパラメータ
  INTRUDE_CMD_WFBC_NPC_ANS,                   ///<WFBCパラメータ
  INTRUDE_CMD_WFBC_NPC_REQ,                   ///<WFBCパラメータ
  
  INTRUDE_CMD_MAX,
  INTRUDE_CMD_NUM = INTRUDE_CMD_MAX - INTRUDE_CMD_START,
};


//==============================================================================
//  外部データ
//==============================================================================
extern const NetRecvFuncTable Intrude_CommPacketTbl[];


//==============================================================================
//  外部関数宣言
//==============================================================================
extern BOOL IntrudeSend_Shutdown(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL IntrudeSend_MemberNum(INTRUDE_COMM_SYS_PTR intcomm, u8 member_num);
extern BOOL IntrudeSend_ProfileReq(void);
extern BOOL IntrudeSend_Profile(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL IntrudeSend_DeleteProfile(INTRUDE_COMM_SYS_PTR intcomm, int leave_netid);
extern BOOL IntrudeSend_PlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_STATUS *send_status);
extern BOOL IntrudeSend_Talk(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id);
extern BOOL IntrudeSend_TalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id, INTRUDE_TALK_STATUS answer);
extern BOOL IntrudeSend_TalkCancel(int send_net_id);
extern BOOL IntrudeSend_BingoIntrusion(int send_net_id);
extern BOOL IntrudeSend_BingoIntrusionAnswer(int send_net_id, BINGO_INTRUSION_ANSWER answer);
extern BOOL IntrudeSend_ReqBingoBattleIntrusionParam(int send_net_id);
extern BOOL IntrudeSend_BingoBattleIntrusionParam(BINGO_SYSTEM *bingo, int send_net_id);
extern BOOL IntrudeSend_MissionListReq(INTRUDE_COMM_SYS_PTR intcomm, u32 palace_area);
extern BOOL IntrudeSend_MissionList(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission, int palace_area);
extern BOOL IntrudeSend_MissionOrderConfirm(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_DATA *mdata);
extern BOOL IntrudeSend_MissionEntryAnswer(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_ENTRY_ANSWER *entry_answer, int send_netid);
extern BOOL IntrudeSend_MissionReq(INTRUDE_COMM_SYS_PTR intcomm, int monolith_type, u16 zone_id);
extern BOOL IntrudeSend_MissionData(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission);
extern BOOL IntrudeSend_MissionStart(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission);
extern BOOL IntrudeSend_MissionAchieve(
  INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission);
extern BOOL IntrudeSend_MissionAchieveAnswer(INTRUDE_COMM_SYS_PTR intcomm, 
  MISSION_ACHIEVE achieve, int send_netid);
extern BOOL IntrudeSend_MissionResult(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission);
extern BOOL IntrudeSend_OccupyInfo(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL IntrudeSend_TargetTiming(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid, u8 timing_no);
extern BOOL IntrudeSend_PlayerSupport(
  INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid, SUPPORT_TYPE support_type);
extern BOOL IntrudeSend_WfbcReq(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid);
extern BOOL IntrudeSend_Wfbc(INTRUDE_COMM_SYS_PTR intcomm, u32 send_netid_bit, const FIELD_WFBC_CORE *wfbc_core);
extern BOOL IntrudeSend_WfbcNpcAns(const FIELD_WFBC_COMM_NPC_ANS *npc_ans, NetID send_netid);
extern BOOL IntrudeSend_WfbcNpcReq(const FIELD_WFBC_COMM_NPC_REQ *npc_req, NetID send_netid);


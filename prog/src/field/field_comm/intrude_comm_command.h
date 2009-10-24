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


//==============================================================================
//  定数定義
//==============================================================================

///ユニオン受信コマンド番号　　※Union_CommPacketTblと並びを同じにしておくこと！！
enum INTRUDE_CMD{
  INTRUDE_CMD_START = GFL_NET_CMD_FIELD,
  
  INTRUDE_CMD_SHUTDOWN = INTRUDE_CMD_START,   ///<切断
  INTRUDE_CMD_MEMBER_NUM,                     ///<侵入参加人数
  INTRUDE_CMD_PROFILE_REQ,                    ///<プロフィール要求
  INTRUDE_CMD_PROFILE,                        ///<プロフィールデータ
  INTRUDE_CMD_PLAYER_STATUS,                  ///<侵入ステータス(現在情報)
  INTRUDE_CMD_TALK,                           ///<話しかける
  INTRUDE_CMD_TALK_ANSWER,                    ///<話しかけられたので自分の状況を返す
  INTRUDE_CMD_BINGO_INTRUSION,                ///<ビンゴバトル乱入許可要求
  INTRUDE_CMD_BINGO_INTRUSION_ANSWER,         ///<ビンゴバトル乱入許可要求の返事
  INTRUDE_CMD_REQ_BINGO_INTRUSION_PARAM,      ///<ビンゴバトル乱入用のパラメータ要求
  INTRUDE_CMD_BINGO_INTRUSION_PARAM,          ///<ビンゴバトル乱入用のパラメータ
  
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
extern BOOL IntrudeSend_Shutdown(void);
extern BOOL IntrudeSend_MemberNum(INTRUDE_COMM_SYS_PTR intcomm, u8 member_num);
extern BOOL IntrudeSend_ProfileReq(void);
extern BOOL IntrudeSend_Profile(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL IntrudeSend_PlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_STATUS *send_status);
extern BOOL IntrudeSend_Talk(int send_net_id);
extern BOOL IntrudeSend_TalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id, INTRUDE_TALK_STATUS answer);
extern BOOL IntrudeSend_BingoIntrusion(int send_net_id);
extern BOOL IntrudeSend_BingoIntrusionAnswer(int send_net_id, BINGO_INTRUSION_ANSWER answer);
extern BOOL IntrudeSend_ReqBingoBattleIntrusionParam(int send_net_id);
extern BOOL IntrudeSend_BingoBattleIntrusionParam(BINGO_SYSTEM *bingo, int send_net_id);


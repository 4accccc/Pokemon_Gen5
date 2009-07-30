//==============================================================================
/**
 * @file    colosseum_comm_command.h
 * @brief   コロシアム：通信コマンド
 * @author  matsuda
 * @date    2009.07.14(火)
 */
//==============================================================================
#pragma once

#include <gflib.h>
#include "net/network_define.h"
#include "colosseum.h"
#include "net_app/union/union_main.h"
#include "poke_tool/pokeparty.h"


//==============================================================================
//  定数定義
//==============================================================================

///ユニオン受信コマンド番号　　※Colosseum_CommPacketTblと並びを同じにしておくこと！！
enum COLOSSEUM_CMD{
  COLOSSEUM_CMD_START = GFL_NET_CMD_COLOSSEUM,
  
  COLOSSEUM_CMD_SHUTDOWN = COLOSSEUM_CMD_START, ///<切断
  COLOSSEUM_CMD_BASIC_STATUS,               ///<基本情報
  COLOSSEUM_CMD_TRAINERCARD,                ///<トレーナーカード情報
  COLOSSEUM_CMD_POS_PACKAGE,                ///<座標パッケージ
  COLOSSEUM_CMD_STANDPOS_CONFIRM,           ///<親に自分の今の立ち位置を使っても問題ないか確認する
  COLOSSEUM_CMD_ANSWER_STANDPOS,            ///<立ち位置使用結果の返事
  COLOSSEUM_CMD_POKEPARTY,                  ///<POKEPARTY受信
  COLOSSEUM_CMD_STANDING_POS,               ///<全員分の立ち位置を受信
  COLOSSEUM_CMD_BATTLE_READY,               ///<戦闘準備完了
  COLOSSEUM_CMD_BATTLE_READY_CANCEL,        ///<戦闘準備キャンセル
  COLOSSEUM_CMD_BATTLE_READY_CANCEL_OK,     ///<戦闘準備キャンセルOK
  COLOSSEUM_CMD_ALL_BATTLE_READY,           ///<全員戦闘準備完了
  COLOSSEUM_CMD_LEAVE,                      ///<退出
  
  COLOSSEUM_CMD_MAX,
  COLOSSEUM_CMD_NUM = COLOSSEUM_CMD_MAX - COLOSSEUM_CMD_START,
};


//==============================================================================
//  外部データ
//==============================================================================
extern const NetRecvFuncTable Colosseum_CommPacketTbl[];


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void Colosseum_AddCommandTable(UNION_SYSTEM_PTR unisys);
extern void Colosseum_DelCommandTable(void);

extern BOOL ColosseumSend_Shutdown(COMM_PLAYER_PACKAGE *pos_package);
extern BOOL ColosseumSend_EntryStatus(COLOSSEUM_BASIC_STATUS *basic_status);
extern BOOL ColosseumSend_BasicStatus(COLOSSEUM_BASIC_STATUS *basic_status, BOOL parent_only);
extern BOOL ColosseumSend_TrainerCard(TR_CARD_DATA *send_card);
extern BOOL ColosseumSend_PosPackage(COMM_PLAYER_PACKAGE *pos_package);
extern BOOL ColosseumSend_StandingPositionConfirm(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL ColosseumSend_AnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys, int send_net_id, BOOL result);
extern BOOL ColosseumSend_Pokeparty(POKEPARTY *pokeparty);
extern BOOL ColosseumSend_StandingPos(u8 *standing_pos);
extern BOOL ColosseumSend_BattleReady(void);
extern BOOL ColosseumSend_BattleReadyCancel(void);
extern BOOL ColosseumSend_BattleReadyCancelOK(int send_netid);
extern BOOL ColosseumSend_AllBattleReady(void);
extern BOOL ColosseumSend_Leave(void);
extern BOOL ColosseumSend_EntryAnswer(int send_netid, COMM_ENTRY_ANSWER answer);


//==============================================================================
/**
 * @file    union_comm_command.h
 * @brief   ユニオン：通信コマンド
 * @author  matsuda
 * @date    2009.07.14(火)
 */
//==============================================================================
#pragma once

#include <gflib.h>
#include "net/network_define.h"
#include "colosseum_types.h"


//==============================================================================
//  定数定義
//==============================================================================

///ユニオン受信コマンド番号　　※Union_CommPacketTblと並びを同じにしておくこと！！
enum UNION_CMD{
  UNION_CMD_START = GFL_NET_CMD_UNION,
  
  UNION_CMD_SHUTDOWN = UNION_CMD_START,     ///<切断
  UNION_CMD_MAINMENU_LIST_RESULT,           ///<メインメニュー選択結果
  UNION_CMD_MAINMENU_LIST_RESULT_ANSWER,    ///<メインメニュー選択結果の返事
  UNION_CMD_MYSTATUS,                       ///<MYSTATUS
  UNION_CMD_TRAINERCARD_PARAM,              ///<トレーナーカード情報
  UNION_CMD_COLOSSEUM_ENTRY,                ///<コロシアムエントリー
  UNION_CMD_COLOSSEUM_ENTRY_ANSWER,         ///<コロシアムエントリー結果
  UNION_CMD_COLOSSEUM_ENTRY_ALL_READY,      ///<コロシアムエントリー全員揃った
  UNION_CMD_MINIGAME_ENTRY_REQ,             ///<ミニゲーム乱入希望
  UNION_CMD_MINIGAME_ENTRY_REQ_ANSWER_NG,   ///<ミニゲーム乱入希望の返事(NG)
  UNION_CMD_MINIGAME_ENTRY_REQ_ANSWER_OK,   ///<ミニゲーム乱入希望の返事(OK)
  UNION_CMD_MINIGAME_BASIC_STATUS_REQ,      ///<ミニゲーム基本情報要求
  UNION_CMD_MINIGAME_BASIC_STATUS,          ///<ミニゲーム基本情報
  UNION_CMD_MINIGAME_MYSTATUS_REQ,          ///<ミニゲーム用MYSTATUS要求
  UNION_CMD_MINIGAME_MYSTATUS,              ///<ミニゲーム用MYSTATUS
  UNION_CMD_MINIGAME_INTRUDE_READY,         ///<ミニゲーム乱入の準備完了宣言
  UNION_CMD_MINIGAME_LEAVE_CHILD,           ///<ミニゲームで子機の誰かが離脱しました

  UNION_CMD_MAX,
  UNION_CMD_NUM = UNION_CMD_MAX - UNION_CMD_START,
};


//==============================================================================
//  外部データ
//==============================================================================
extern const NetRecvFuncTable Union_CommPacketTbl[];


//==============================================================================
//  外部関数宣言
//==============================================================================
extern BOOL UnionSend_MainMenuListResult(u32 select_list);
extern BOOL UnionSend_MainMenuListResultAnswer(BOOL yes_no);
extern BOOL UnionSend_Mystatus(UNION_SYSTEM_PTR unisys);
extern BOOL UnionSend_TrainerCardParam(UNION_SYSTEM_PTR unisys);
extern BOOL UnionSend_ColosseumEntryStatus(COLOSSEUM_BASIC_STATUS *basic_status);
extern BOOL UnionSend_ColosseumEntryAnswer(int send_netid, COMM_ENTRY_ANSWER answer);
extern BOOL UnionSend_ColosseumEntryAllReady(void);
extern BOOL UnionSend_MinigameEntryReq(UNION_SYSTEM_PTR unisys);
extern BOOL UnionSend_MinigameEntryReqAnswerNG(u8 send_bit);
extern BOOL UnionSend_MinigameEntryReqAnswerOK(u8 send_bit);
extern BOOL UnionSend_MinigameBasicStatusReq(void);
extern BOOL UnionSend_MinigameBasicStatus(const UNION_APP_BASIC *app_basic, u8 send_bit);
extern BOOL UnionSend_MinigameMystatusReq(u8 send_bit);
extern BOOL UnionSend_MinigameMystatus(u8 send_bit, const MYSTATUS *myst);
extern BOOL UnionSend_MinigameIntrudeReady(u8 send_bit);
extern BOOL UnionSend_MinigameLeaveChild(u8 send_bit, NetID leave_netid);

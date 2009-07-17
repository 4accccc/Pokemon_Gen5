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


//==============================================================================
//  定数定義
//==============================================================================

///ユニオン受信コマンド番号　　※Colosseum_CommPacketTblと並びを同じにしておくこと！！
enum COLOSSEUM_CMD{
  COLOSSEUM_CMD_START = GFL_NET_CMD_COLOSSEUM,
  
  COLOSSEUM_CMD_SHUTDOWN = COLOSSEUM_CMD_START,     ///<切断
  COLOSSEUM_CMD_MAINMENU_LIST_RESULT,           ///<メインメニュー選択結果
  COLOSSEUM_CMD_MAINMENU_LIST_RESULT_ANSWER,    ///<メインメニュー選択結果の返事
  COLOSSEUM_CMD_TRAINERCARD_PARAM,              ///<トレーナーカード情報
  
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

extern BOOL ColosseumSend_MainMenuListResult(u32 select_list);
extern BOOL ColosseumSend_MainMenuListResultAnswer(BOOL yes_no);
extern BOOL ColosseumSend_TrainerCardParam(UNION_SYSTEM_PTR unisys);

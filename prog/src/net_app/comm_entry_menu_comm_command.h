//==============================================================================
/**
 * @file    comm_entry_menu_comm_command.c
 * @brief   参加募集メニュー制御：通信テーブル
 * @author  matsuda
 * @date    2010.01.11(月)
 */
//==============================================================================
#pragma once

#include <gflib.h>
#include "net/network_define.h"


//==============================================================================
//  定数定義
//==============================================================================

///ユニオン受信コマンド番号　　※CommEntryMenu_CommPacketTblと並びを同じにしておくこと！！
enum COLOSSEUM_CMD{
  ENTRYMENU_CMD_START = GFL_NET_CMD_COMM_ENTRY_MENU,
  
  ENTRYMENU_CMD_ENTRY = ENTRYMENU_CMD_START,      ///<親へエントリーを通知
  ENTRYMENU_CMD_ENTRY_OK,                         ///<エントリーOK
  ENTRYMENU_CMD_ENTRY_NG,                         ///<エントリーNG
  ENTRYMENU_CMD_GAME_START,                       ///<ゲーム開始を通知
  ENTRYMENU_CMD_GAME_CANCEL,                      ///<ゲームを中止します
  ENTRYMENU_CMD_MEMBER_INFO,                      ///<現在の参加者リスト

  ENTRYMENU_CMD_MAX,
  ENTRYMENU_CMD_NUM = ENTRYMENU_CMD_MAX - ENTRYMENU_CMD_START,
};


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void CommEntryMenu_AddCommandTable(COMM_ENTRY_MENU_PTR em);
extern void CommEntryMenu_DelCommandTable(void);

extern BOOL CemSend_Entry(const MYSTATUS *myst, BOOL force_entry);
extern BOOL CemSend_EntryOK(NetID send_id);
extern BOOL CemSend_EntryNG(NetID send_id);
extern BOOL CemSend_GameStart(void);
extern BOOL CemSend_GameCancel(void);
extern BOOL CemSend_MemberInfo(const ENTRYMENU_MEMBER_INFO *member_info, u8 send_bit);

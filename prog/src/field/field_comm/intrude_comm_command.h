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
  INTRUDE_CMD_PROFILE_REQ,                    ///<プロフィール要求
  INTRUDE_CMD_PROFILE,                        ///<プロフィールデータ
  INTRUDE_CMD_PLAYER_STATUS,                  ///<侵入ステータス(現在情報)
  
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
extern BOOL IntrudeSend_ProfileReq(void);
extern BOOL IntrudeSend_Profile(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL IntrudeSend_PlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, 
  GAMEDATA *gamedata, INTRUDE_STATUS *send_status, int palace_area, int mission_no);


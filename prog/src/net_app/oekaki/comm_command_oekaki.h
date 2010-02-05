//=============================================================================
/**
 * @file  comm_command_oekaki.h
 * @brief お絵かきボード通信コマンド定義ヘッダ
 *          
 * @author  Akito Mori
 * @date    2010.01.20
 */
//=============================================================================
#ifndef __COMM_COMMAND_OEKAKI_H__
#define __COMM_COMMAND_OEKAKI_H__

#include <gflib.h>

//#include "communication/comm_command.h"
//#include "../../field/comm_command_field.h"
#include "net/network_define.h"

// ５人接続通信の時にユーザーが子機として送信できる最大受信バイト数
#define COMM_SEND_5TH_PACKET_MAX  (10)

// ---------------------------------------------------
// 通信コマンド定義
// ---------------------------------------------------
enum{
  CO_OEKAKI_GRAPHICDATA=GFL_NET_CMD_PICTURE,
  CO_OEKAKI_LINEPOS,   
  CO_OEKAKI_LINEPOS_SERVER,   
  CO_OEKAKI_STOP,      
  CO_OEKAKI_RESTART,   
  CO_OEKAKI_END_CHILD, 
  CO_OEKAKI_END,     
  CO_OEKAKI_CHILD_JOIN,
  CO_OEKAKI_OUT_CONTROL,
};
extern void OEKAKIBOARD_CommandInitialize( void* pWk );



#endif

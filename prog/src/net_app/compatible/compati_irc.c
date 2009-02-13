//==============================================================================
/**
 * @file	compati_irc.c
 * @brief	赤外線で繋がっているか
 * @author	matsuda
 * @date	2009.02.13(金)
 */
//==============================================================================
#include <gflib.h>
#include "system\main.h"
#include "net\network_define.h"
#include "system/gfl_use.h"

#include "compati_types.h"
#include "compati_tool.h"
#include "compati_comm.h"
#include "compati_irc.h"


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void IRC_ReceiveCallback(u8 *data, u8 size, u8 command, u8 value);

static COMPATI_IRC_SYS *glb_ircsys = NULL;


//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   ircsys		
 */
//--------------------------------------------------------------
void CompatiIrc_Init(COMPATI_IRC_SYS *ircsys)
{
	glb_ircsys = ircsys;

	IRC_Init();
	IRC_SetRecvCallback(IRC_ReceiveCallback);
}

//--------------------------------------------------------------
/**
 * @brief   相性チェックゲーム中の赤外線接続確認
 *
 * @param   ircsys		
 *
 * @retval  TRUE:接続している。　FALSE:接続していない
 */
//--------------------------------------------------------------
BOOL CompatiIrc_Main(COMPATI_IRC_SYS *ircsys)
{
	BOOL irc_ret;
	
	IRC_Move();
	irc_ret = IRC_IsConnect();
	if(irc_ret == TRUE){
		//OS_TPrintf("赤外線接続されている\n");
		if(ircsys->shutdown_req == TRUE){
			//OS_TPrintf("赤外線shutdown\n");
			IRC_Shutdown();
			return irc_ret;
		}
	}
	else{
		//OS_TPrintf("赤外線繋がっていない\n");
		if(ircsys->shutdown_req == TRUE){
			//OS_TPrintf("shutdownがかかっている\n");
			return irc_ret;
		}
		else if(ircsys->seq == 100){
			//OS_TPrintf("赤外線seq＝０\n");
			ircsys->seq = 0;
		}
	}
	
	switch(ircsys->seq){
	case 0:
		ircsys->connect = FALSE;
		IRC_Init();
		IRC_SetRecvCallback(IRC_ReceiveCallback);
		ircsys->wait = GFUser_GetPublicRand(30);	//親子同時にリクエストを送らないようにランダム
		//OS_TPrintf("赤外線初期化\n");
		ircsys->seq++;
		break;
	case 1:
		if(ircsys->wait > 0){
			ircsys->wait--;
			break;
		}
		IRC_Connect();
		//OS_TPrintf("赤外線接続リクエスト\n");
		ircsys->seq++;
		break;
	case 2:
		if(IRC_IsConnect() == TRUE){
			if(IRC_IsSender() == FALSE){
				IRC_Send(NULL, 0, 8, 10);
			}
			//OS_TPrintf("赤外線繋がった\n");
			ircsys->seq = 100;
		}
		else{
			ircsys->wait++;
			if(ircsys->wait > 60*2){	//ある程度待っても接続出来ない場合はもう一度初期化から
				ircsys->wait = 0;
				ircsys->seq = 0;
				//OS_TPrintf("赤外線なかなか接続しないので最初から\n");
			}
			break;
		}
		break;
	default:	//接続中
		break;
	}
	
	return irc_ret;
}

//--------------------------------------------------------------
/**
 * @brief   赤外線切断リクエスト
 *
 * @param   ircsys		
 */
//--------------------------------------------------------------
void CompatiIRC_Shoutdown(COMPATI_IRC_SYS *ircsys)
{
	ircsys->shutdown_req = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   IRC受信時に呼ばれるコールバック
 *
 * @param   data		
 * @param   size		
 * @param   command		
 * @param   value		
 */
//--------------------------------------------------------------
static void IRC_ReceiveCallback(u8 *data, u8 size, u8 command, u8 value)
{
	//OS_TPrintf("赤外線受信 command = %d\n", command);

	//-- 赤外線ライブラリ内部で使用しているシステムコマンド --//
	if(command >= 0xf0){
		if(command == 0xf4){
			OS_TPrintf("IRC切断コマンドを受信\n");
		}
		return;	//赤外線ライブラリ内部で使用しているシステムコマンドの為、ここでは無視
	}
	
	IRC_Send(NULL, 0, 8, 0);	//通信継続の為、返事を返す
	glb_ircsys->connect = TRUE;
	glb_ircsys->seq = 100;
}

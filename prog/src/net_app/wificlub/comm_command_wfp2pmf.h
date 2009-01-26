//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		comm_command_wfp2pmf.h
 *	@brief		２～４人専用待合室	通信コマンド
 *	@author		tomoya takahashi
 *	@data		2007.05.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __COMM_COMMAND_WFP2PMF_H__
#define __COMM_COMMAND_WFP2PMF_H__

#include "net/network_define.h"

enum CommCommandTradeList_e {

	//-------------------------------------
	//　ゲーム用
	//=====================================
	// 同期用	
	CNM_WFP2PMF_RESULT = GFL_NET_CMD_WIFICLUB,	///< 通信KONG		親ー＞子
	CNM_WFP2PMF_START,						///< ゲーム開始		親ー＞子
	CNM_WFP2PMF_VCHAT,						///< VCHATデータ	親ー＞子

	//-------------------------------------
	//	
	//=====================================
	
	//------------------------------------------------ここまで
	CNM_COMMAND_MAX   // 終端--------------これは移動させないでください
};

extern const NetRecvFuncTable* WFP2PMF_CommCommandTclGet( void );
extern int WFP2PMF_CommCommandTblNumGet( void );

#endif		// __COMM_COMMAND_WFP2PMF_H__


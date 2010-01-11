#pragma once
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file	    comm_command_wfp2pmf.h
 *	@brief		wifiクラブ全般 事前送信
 *	@author		tomoya takahashi -> k.ohno
 *	@data		  2007.05.24 -> 2010.01.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "net/network_define.h"

enum CommCommandTradeList_e {

	//-------------------------------------
	//　ゲーム用
	//=====================================
	// 同期用	
	CNM_WFP2PMF_RESULT = GFL_NET_CMD_WIFICLUB,	///< 通信KONG		親ー＞子
	CNM_WFP2PMF_START,						///< ゲーム開始		親ー＞子
	CNM_WFP2PMF_VCHAT,						///< VCHATデータ	親ー＞子
  CNM_WFP2PMF_STATUS,   ///< WIFIステータスデータ
  CNM_WFP2PMF_MYSTATUS,   ///< MYステータスデータ
	//-------------------------------------
	//	
	//=====================================
	
	//------------------------------------------------ここまで
	CNM_COMMAND_MAX   // 終端--------------これは移動させないでください
};

extern const NetRecvFuncTable* WFP2PMF_CommCommandTclGet( void );
extern int WFP2PMF_CommCommandTblNumGet( void );



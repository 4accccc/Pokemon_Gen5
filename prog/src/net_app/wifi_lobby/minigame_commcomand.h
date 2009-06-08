//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		minigame_commcomand.h
 *	@brief		ミニゲームツール通信コマンド
 *	@author		tomoya takahashi
 *	@data		2007.11.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "net\network_define.h"
#include "net_old\comm_command.h"

enum CommCommandMNGM {

	//-------------------------------------
	//　ゲーム用
	//=====================================
	CNM_MNGM_RETRY_YES = CS_COMMAND_MAX,	//< 子ー＞親	RETRY YES
	CNM_MNGM_RETRY_NO,						//<	子ー＞親	RETRY　NO
	CNM_MNGM_RETRY_OK,						//< 親ー＞子	RETRY
	CNM_MNGM_RETRY_NG,						//< 
	CNM_MNGM_ENTRY_RAREGAME,				//<	親ー＞子	Rareゲームタイプ
	

	//-------------------------------------
	//	
	//=====================================
	
	//------------------------------------------------ここまで
	CNM_COMMAND_MAX   // 終端--------------これは移動させないでください
};

extern const CommPacketTbl* MNGM_CommCommandTclGet( void );
extern int MNGM_CommCommandTblNumGet( void );


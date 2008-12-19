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
#ifndef __MINIGAME_COMMCOMAND_H__
#define __MINIGAME_COMMCOMAND_H__

#include "net\network_define.h"

enum CommCommandMNGM {

	//-------------------------------------
	//　ゲーム用
	//=====================================
	CNM_MNGM_RETRY_YES = GFL_NET_CMD_MINIGAME_TOOL,	//< 子ー＞親	RETRY YES
	CNM_MNGM_RETRY_NO,						//<	子ー＞親	RETRY　NO
	CNM_MNGM_RETRY_OK,						//< 親ー＞子	RETRY
	CNM_MNGM_RETRY_NG,						//< 
	CNM_MNGM_ENTRY_RAREGAME,				//<	親ー＞子	Rareゲームタイプ

};

extern const NetRecvFuncTable* MNGM_CommCommandTclGet( void );
extern int MNGM_CommCommandTblNumGet( void );

#endif		// __MINIGAME_COMMCOMAND_H__


//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_aura.h
 *	@brief	赤外線ミニゲーム	オーラチェック
 *	@author	Toru=Nagihashi
 *	@date		2009.05.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/gamesystem.h"
#include "net_app/compatible_irc_sys.h"
#include "net_app/irc_compatible.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	進行状況
//=====================================
typedef enum {
	IRCAURA_RESULT_CLEAR,		//クリアした
	IRCAURA_RESULT_RETURN,	//戻りを選択
	IRCAURA_RESULT_RESULT,	//結果画面へ
} IRCAURA_RESULT;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	PROCに渡すパラメータ
//=====================================
typedef struct {
	GAMESYS_WORK				*p_gamesys;//[in]ゲームシステム
	COMPATIBLE_IRC_SYS	*p_irc;	//[in]赤外線システム
  COMPATIBLE_STATUS		*p_you_status;//[out]通信相手のステータス（バッファを作って渡してください）
	IRCAURA_RESULT			result;	//[out]PROCが戻ってきたときの内部進行状況
	u8									score;	//[out]オーラチェックで得た点数
  u8                  minus;  //[out]減点

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
	BOOL								is_only_play;	//[in]デバッグ時専用、一人プレイモード
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY
}IRC_AURA_PARAM;

//-------------------------------------
///	オーラチェック用プロックデータ
//=====================================
extern const GFL_PROC_DATA IrcAura_ProcData;


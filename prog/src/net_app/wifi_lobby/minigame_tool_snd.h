//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		minigame_tool_snd.h
 *	@brief		ミニゲームツール専用サウンド定義
 *	@author		tomoya takahashi
 *	@data		2007.10.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "sound/pm_sndsys.h"
#include "net_app/net_bugfix.h"

#if WB_FIX   //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


#define MNGM_SND_COUNT				( SEQ_SE_DP_DECIDE )		// ３・２・１
#define MNGM_SND_START				( SEQ_SE_DP_CON_016 )		// スタート

#define MNGM_SND_TIMEUP				( SEQ_SE_DP_PINPON )		// タイムアップ

//////////////////////////////////////////////////////////////////////////////
//以下を当ててください

#define MNGM_SND_TITLE				( SEQ_SE_PL_FCALL )			// 3匹の絵が表示される時
//#define MNGM_SND_BALLOON			( SEQ_SE_DP_BOX02 )			// 協力して割った風船を表示される音
#define MNGM_SND_BALLOON			( SEQ_SE_PL_BALLOON07 )		// 協力して割った風船を表示される音

//通信対戦と合わせている
#define MNGM_SND_BAR_IN				( SEQ_SE_PL_MNGM_IN )		// バー(タッチペン)がインする動き
#define MNGM_SND_BAR_OUT			( SEQ_SE_PL_MNGM_OUT )		// バー(タッチペン)がアウトする動き
#define MNGM_SND_BAR_FLASH			( SEQ_SE_PL_FVSDEMO06 )		// 1位のバーが点滅 
//SEQ_PL_WINMINI2(MEを当てることに変更予定 08.03.18)





#else //WB_FIX @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@





#define MNGM_SND_COUNT				( SEQ_SE_SELECT1 )		// ３・２・１
#define MNGM_SND_START				( SEQ_SE_SELECT1 )		// スタート

#define MNGM_SND_TIMEUP				( SEQ_SE_SELECT1 )		// タイムアップ

#define MNGM_SND_TITLE				( SEQ_SE_SELECT1 )			// 3匹の絵が表示される時
//#define MNGM_SND_BALLOON			( SEQ_SE_SELECT1 )			// 協力して割った風船を表示される音
#define MNGM_SND_BALLOON			( SEQ_SE_SELECT1 )		// 協力して割った風船を表示される音

//通信対戦と合わせている
#define MNGM_SND_BAR_IN				( SEQ_SE_SELECT1 )		// バー(タッチペン)がインする動き
#define MNGM_SND_BAR_OUT			( SEQ_SE_SELECT1 )		// バー(タッチペン)がアウトする動き
#define MNGM_SND_BAR_FLASH			( SEQ_SE_SELECT1 )		// 1位のバーが点滅 
//SEQ_PL_WINMINI2(MEを当てることに変更予定 08.03.18)

#endif  //WB_FIX


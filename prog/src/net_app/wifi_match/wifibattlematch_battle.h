//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_battle.h
 *	@brief  デモ～バトル～終了デモへのつなぎ(録画なし)
 *	@author	Toru=Nagihashi
 *	@date		2010.02.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "demo/comm_btl_demo.h"
#include "gamesystem/game_data.h"
#include "battle/battle.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	バトルプロセスの戻り値
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_BATTLELINK_RESULT_SUCCESS,          //正常に終了
  WIFIBATTLEMATCH_BATTLELINK_RESULT_DISCONNECT,       //切断された検知
  WIFIBATTLEMATCH_BATTLELINK_RESULT_EVIL,             //不正に狩猟
} WIFIBATTLEMATCH_BATTLELINK_RESULT;



//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
/// バトルプロセスの引数
//=====================================
typedef struct
{
  // [IN]
  GAMEDATA            *p_gamedata;
  BATTLE_SETUP_PARAM  *p_btl_setup_param;
  COMM_BTL_DEMO_PARAM *p_demo_param;

  // [out]
  WIFIBATTLEMATCH_BATTLELINK_RESULT result;

} WIFIBATTLEMATCH_BATTLELINK_PARAM;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	バトル＋デモつなぎ
//=====================================
extern const GFL_PROC_DATA	WifiBattleMatch_BattleLink_ProcData;

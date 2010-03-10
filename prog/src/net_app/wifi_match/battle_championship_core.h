//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		battle_championship_core.h
 *	@brief  バトル大会コア
 *	@author	Toru=Nagihashi
 *	@date		2010.03.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//ライブラリ
#include <gflib.h>
#include "net_app/wifi_match/wifibattlematch_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
/// 大会コアモード
//=====================================
typedef enum
{
  BATTLE_CHAMPIONSHIP_CORE_MODE_MAIN_MEMU, //メインメニューへいく
  BATTLE_CHAMPIONSHIP_CORE_MODE_WIFI_MENU, //WIFIメニューへいく
  BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_MENU, //LIVEメニューへいく
  BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_START, //ライブ大会フロー開始へいく（いらない？）
  BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_BTLEND, //ライブ大会フローバトル終了後へいく
  BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_RECEND, //ライブ大会フロー録画終了後へいく
  BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_MENU,   //ライブ大会フローメインメニューへいく
} BATTLE_CHAMPIONSHIP_CORE_MODE;

//-------------------------------------
///	大会コア戻り値
//=====================================
typedef enum
{
  BATTLE_CHAMPIONSHIP_CORE_RET_TITLE,   //タイトルへ行く
  BATTLE_CHAMPIONSHIP_CORE_RET_WIFICUP, //WIFI大会へ行く
  BATTLE_CHAMPIONSHIP_CORE_RET_LIVEBTL, //LIVE用バトルへ行く
  BATTLE_CHAMPIONSHIP_CORE_RET_LIVEREC, //LIVE用録画へ行く
  BATTLE_CHAMPIONSHIP_CORE_RET_LIVERECPLAY, //LIVE用録画再生へ行く

} BATTLE_CHAMPIONSHIP_CORE_RET;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	引数
//=====================================
typedef struct 
{
  GAMEDATA                      *p_gamedata;  //[in ]ゲームデータ
  BATTLE_CHAMPIONSHIP_CORE_MODE mode;         //[in ]モードどこから開始するか
  BATTLE_CHAMPIONSHIP_CORE_RET  ret;          //[out]戻り値どこへいくか

  //以下、[in ]常駐データ
  WIFIBATTLEMATCH_ENEMYDATA     *p_player_data;
  WIFIBATTLEMATCH_ENEMYDATA     *p_enemy_data;

} BATTLE_CHAMPIONSHIP_CORE_PARAM;


//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
FS_EXTERN_OVERLAY( battle_championship );
extern const GFL_PROC_DATA BATTLE_CHAMPIONSHIP_CORE_ProcData;

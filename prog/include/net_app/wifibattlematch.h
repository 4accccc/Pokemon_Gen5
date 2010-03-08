//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch.h
 *	@brief	バトルマッチ画面
 *	        （以前はWIFIだけだったがライブ大会とバトルメニューも包括したため、名前があっていません。）
 *	        ・ポケセンのWIFIカウンターからいくランダムマッチ
 *	        ・タイトルからいくバトル大会メニュー
 *	        ・バトル大会メニューからいくWIFI大会
 *	        ・バトル大会メニューからいくライブ大会
 *	@author	Toru=Nagihashi
 *	@date		2009.11.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "battle/battle.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	起動モード
//=====================================
typedef enum
{
	WIFIBATTLEMATCH_MODE_MAINMENU,  //タイトルから進む、メインメニュー
	WIFIBATTLEMATCH_MODE_RANDOM,    //ポケセンのWIFIカウンターからすすむ、ランダム対戦
} WIFIBATTLEMATCH_MODE;

//-------------------------------------
///	大会モード  内部で使用する値です
//=====================================
typedef enum
{
	WIFIBATTLEMATCH_TYPE_WIFICUP,
	WIFIBATTLEMATCH_TYPE_LIVECUP,
	WIFIBATTLEMATCH_TYPE_RNDRATE,
	WIFIBATTLEMATCH_TYPE_RNDFREE,

  WIFIBATTLEMATCH_TYPE_MAX
} WIFIBATTLEMATCH_TYPE;

//-------------------------------------
///	使用ポケモン  ランダム対戦のときのみ設定
//=====================================
typedef enum
{
	WIFIBATTLEMATCH_POKE_TEMOTI,      //手持ちポケ
	WIFIBATTLEMATCH_POKE_BTLBOX,      //バトルボックスポケ

} WIFIBATTLEMATCH_POKE;

//-------------------------------------
///	戦闘タイプ    ランダム対戦のときのみ設定
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_BTLRULE_SINGLE,
  WIFIBATTLEMATCH_BTLRULE_DOUBLE,
  WIFIBATTLEMATCH_BTLRULE_TRIPLE,
  WIFIBATTLEMATCH_BTLRULE_ROTATE,
  WIFIBATTLEMATCH_BTLRULE_SHOOTER,
} WIFIBATTLEMATCH_BTLRULE;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	PROCパラメータ
//    パラメータを上位PROCで持ち続けられない状況があるので、
//    その場合はis_auto_releaseをTRUEにして引数を渡すことで、
//    内部で解放する
//=====================================
typedef struct 
{
  GAMEDATA                  *p_game_data;   //[in ]ゲームデータ NULLの場合は内部で作成する（タイトルの場合ゲームデータがないため）
  WIFIBATTLEMATCH_BTLRULE   btl_rule;       //[in ]バトルルール
	WIFIBATTLEMATCH_MODE	    mode;           //[in ]起動モード
  WIFIBATTLEMATCH_POKE      poke;           //[in ]選択用パーティ（手持ちorバトルボックスのはず）
  BOOL                      is_auto_release;//[in ]このパラメータを内部解放するフラグ
} WIFIBATTLEMATCH_PARAM;


//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
FS_EXTERN_OVERLAY( wifibattlematch_sys );
extern const GFL_PROC_DATA	WifiBattleMatch_ProcData;

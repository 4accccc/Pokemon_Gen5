//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch.h
 *	@brief	WIFIのバトルマッチ画面
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
///	種類
//=====================================
typedef enum
{
	WIFIBATTLEMATCH_MODE_RANDOM,  //ランダム対戦
	WIFIBATTLEMATCH_MODE_WIFI,    //WIFI大会
	WIFIBATTLEMATCH_MODE_LIVE,    //ライブ大会

} WIFIBATTLEMATCH_MODE;

//-------------------------------------
///	使用ポケモン
//=====================================
typedef enum
{
	WIFIBATTLEMATCH_POKE_TEMOTI,      //手持ちポケ
	WIFIBATTLEMATCH_POKE_BTLBOX,      //バトルボックスポケ

} WIFIBATTLEMATCH_POKE;

//-------------------------------------
///	戦闘タイプ
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
//    下記は上位PROCで持ち続けられない状況があるので、
//    その場合はis_auto_releaseをTRUEにして引数を渡すことで、
//    内部で解放する
//=====================================
typedef struct 
{
  GAMEDATA              *p_game_data; //[in ]ゲームデータ NULLの場合は内部で作成する
  WIFIBATTLEMATCH_BTLRULE   btl_rule;     //[in ]バトルルール
	WIFIBATTLEMATCH_MODE	mode;         //[in ]起動モード
  WIFIBATTLEMATCH_POKE  poke;         //[in ]選択用パーティ（手持ちorバトルボックスのはず）
  BOOL                  is_auto_release;//[in]アドレスを内部解放するフラグ
} WIFIBATTLEMATCH_PARAM;


//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
FS_EXTERN_OVERLAY( wifibattlematch_sys );
extern const GFL_PROC_DATA	WifiBattleMaptch_ProcData;

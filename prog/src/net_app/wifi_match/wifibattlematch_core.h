//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_core.h
 *	@brief	WIFIのバトルマッチコア画面
 *	@author	Toru=Nagihashi
 *	@date		2009.11.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "net_app/wifibattlematch.h"
#include "battle/battle.h"
#include "wifibattlematch_data.h"
#include "savedata/rndmatch_savedata.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	終了コード
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_CORE_RESULT_FINISH,       //アプリ終了
  WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE,  //バトルへ
  WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN,  //シャットダウンエラーのため、ログインからやり直し
} WIFIBATTLEMATCH_CORE_RESULT;

//-------------------------------------
///	起動モード
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_CORE_MODE_START,      //開始
  WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE,  //バトル終了
} WIFIBATTLEMATCH_CORE_MODE;

//-------------------------------------
///	終了時モード
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_CORE_RETMODE_RATE,  //レーティングモード
  WIFIBATTLEMATCH_CORE_RETMODE_FREE,  //フリーモード
} WIFIBATTLEMATCH_CORE_RETMODE;

//=============================================================================
/**
 *					構造体
*/
//=============================================================================
//-------------------------------------
///	コアプロセスの引数
//=====================================
typedef struct
{
  WIFIBATTLEMATCH_CORE_RETMODE retmode;     //[in/out]前回終了時のモード
  WIFIBATTLEMATCH_CORE_RESULT result;       //[out]終了コード
  WIFIBATTLEMATCH_PARAM       *p_param;     //[in ]外部引数
  WIFIBATTLEMATCH_CORE_MODE   mode;         //[in ]起動モード
  BtlResult                   btl_result;   //[in ]バトル結果
  RNDMATCH_DATA               *p_rndmatch;  //[in ]セーブデータ

  //以下、[in ]常駐データ置き場
  WIFIBATTLEMATCH_ENEMYDATA   *p_player_data;
  WIFIBATTLEMATCH_ENEMYDATA   *p_enemy_data;

} WIFIBATTLEMATCH_CORE_PARAM;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
FS_EXTERN_OVERLAY( wifibattlematch_core );
extern const GFL_PROC_DATA	WifiBattleMaptchCore_ProcData;

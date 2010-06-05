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
#include "net/dreamworld_netdata.h"
#include "wifibattlematch_net.h"

#include <dwc.h>
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
  WIFIBATTLEMATCH_CORE_RESULT_FINISH,           //アプリ終了
  WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE,      //バトルへ
  WIFIBATTLEMATCH_CORE_RESULT_NEXT_REC,         //録画
  WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN,   //シャットダウンエラーのため、ログインからやり直し
  WIFIBATTLEMATCH_CORE_RESULT_END_WIFICUP,           //WIFI大会終了
} WIFIBATTLEMATCH_CORE_RESULT;

//-------------------------------------
///	起動モード
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_CORE_MODE_START,          //開始
  WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE,      //バトル終了
  WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE_ERR,  //バトル終了ただしエラー
  WIFIBATTLEMATCH_CORE_MODE_ENDREC,         //録画終了
} WIFIBATTLEMATCH_CORE_MODE;

//-------------------------------------
///	終了時モード
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_CORE_RETMODE_RATE,  //レーティングモード
  WIFIBATTLEMATCH_CORE_RETMODE_FREE,  //フリーモード
  WIFIBATTLEMATCH_CORE_RETMODE_WIFI,  //WIFI大会
  WIFIBATTLEMATCH_CORE_RETMODE_NONE,  //決めずに抜けた
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
  RNDMATCH_DATA               *p_rndmatch;  //[in ]セーブデータ

  //以下、[in ]常駐データ置き場
  WIFIBATTLEMATCH_ENEMYDATA   *p_player_data;
  WIFIBATTLEMATCH_ENEMYDATA   *p_enemy_data;
  DWCSvlResult                *p_svl_result;
  u32                         *p_server_time;
  WIFIBATTLEMATCH_RECORD_DATA *p_record_data;
  WIFIBATTLEMATCH_RECV_DATA   *p_recv_data;
  const BATTLEMATCH_BATTLE_SCORE  *cp_btl_score;
  DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA *p_gpf_data;
  WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA       *p_wifi_sake_data;
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA        *p_rnd_sake_data;
  WIFIBATTLEMATCH_NET_DATA    *p_net_data;
  BOOL                        *p_is_dirty_name;
} WIFIBATTLEMATCH_CORE_PARAM;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
FS_EXTERN_OVERLAY( wifibattlematch_core );
extern const GFL_PROC_DATA	WifiBattleMatchCore_ProcData;

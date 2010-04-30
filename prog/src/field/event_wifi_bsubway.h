//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_wifi_bsubway.h
 *	@brief  Wifiバトルサブウェイ  イベント
 *	@author	tomoya takahashi
 *	@date		2010.02.19
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "net_app/wifi_bsubway.h"



FS_EXTERN_OVERLAY(event_wifibsubway);


//-------------------------------------
///	GMEVENT_CreateOverlayEventCall関数用コールバック関数
//
//  void* work には EV_WIFIBATTLEMATCH_PARAM*を渡す。
//=====================================
//-------------------------------------
///	パラメータ
//=====================================
typedef struct {
  
  WIFI_BSUBWAY_MODE mode;
  u16* ret_wk;
  
} EV_WIFIBSUBWAY_PARAM;


//-----------------------------------------------------------------------------
/**
 *  バトルサブウェイ  WiFi　Dataアップロード　ダウンロード
 *
 *　mode 
      WIFI_BSUBWAY_MODE_SCORE_UPLOAD,         // 前回の記録をアップロード
      WIFI_BSUBWAY_MODE_GAMEDATA_DOWNLOAD,    // ゲーム情報をダウンロード
      WIFI_BSUBWAY_MODE_SUCCESSDATA_DOWNLOAD, // 歴代情報をダウンロード
 *
 */
//-----------------------------------------------------------------------------
extern GMEVENT * WIFI_BSUBWAY_EVENT_CallStart(
    GAMESYS_WORK *gsys, void* p_work );



#ifdef _cplusplus
}	// extern "C"{
#endif




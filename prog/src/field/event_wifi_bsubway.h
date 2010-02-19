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
extern GMEVENT * WIFI_BSUBWAY_EVENT_Start(
    GAMESYS_WORK *gsys, WIFI_BSUBWAY_MODE mode );


#ifdef _cplusplus
}	// extern "C"{
#endif




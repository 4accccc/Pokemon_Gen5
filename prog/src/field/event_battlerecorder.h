//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_battlerecorder.h
 *	@brief  バトルレコーダーイベント
 *	@author	Toru=Nagihashi
 *	@date		2009.12.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>

#include "field/fieldmap.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "net_app/battle_recorder.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
FS_EXTERN_OVERLAY(event_btlrecorder);


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	パラメータ
//=====================================
typedef struct {
  FIELDMAP_WORK* fieldmap;
  BR_MODE mode;
} EV_WIFIBATTLERECORDER_PARAM;

//=============================================================================
/**
 *					外部公開
 *  GMEVENT_CreateOverlayEventCall用
*/
//=============================================================================
extern GMEVENT* EVENT_CallWifiBattleRecorder( GAMESYS_WORK * gsys, void* work );

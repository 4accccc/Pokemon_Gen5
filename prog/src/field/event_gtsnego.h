//============================================================================================
/**
 * @file	  event_gtsnego.h
 * @brief	  イベント：GTSネゴシエーション
 * @author	ohno
 * @date	  2009.11.15
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "net_app/gtsnego.h"

FS_EXTERN_OVERLAY(event_gtsnego);

//-----------------------------------------------------------------------------
/**
 *    オーバーレイ管理＋イベント起動
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	GMEVENT_CreateOverlayEventCall関数用コールバック関数
//
//  void* work には FIELDMAP_WORK*を渡す。
//=====================================
extern GMEVENT* EVENT_CallGTSNego( GAMESYS_WORK* gsys, void* work );


#if PM_DEBUG

extern GMEVENT* EVENT_GTSNegoChangeDebug(GAMESYS_WORK * gsys, void * work);
#endif

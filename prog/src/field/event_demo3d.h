//============================================================================================
/**
 * @file  event_demo3d.h
 * @brief Demo3Dアプリ呼び出しイベント
 * @date  2010.04.07
 * @author  iwasawa GAME FREAK inc.
 */
//============================================================================================

#pragma once

#include "gamesystem/gamesystem.h"
#include "demo/demo3d_demoid.h"

//------------------------------------------------------------------
/*
 *  @brief  デモコールイベント起動
 *
 *  @param  gsys  GAMESYS_WORK*
 *  @param  demo_id デモID DEMO3D_ID_C_CRUISER他
 *  @param  scene_id  シーンID
 *  @param  use_rtc 時間をRTCから取るならTRUE,EV-TIMEから取るならFALSE
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_CallDemo3D( GAMESYS_WORK* gsys, u16 demo_id, u16 scene_id, BOOL use_rtc );


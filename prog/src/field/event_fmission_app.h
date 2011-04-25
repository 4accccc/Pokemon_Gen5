//============================================================================================
/**
 * @file    event_fmission_app.h
 * @brief	  イベント：フェスミッション詳細画面呼び出しイベント
 * @author  iwasawa	
 * @date	  2011.04.25 
 */
//============================================================================================

#pragma once

#include "app/fmission_app.h"

FS_EXTERN_OVERLAY( notwifi_event_fmission_app );

//------------------------------------------------------------------
/*
 *  @param  gsys
 *  @param  FMISSION_APP_MODE*
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_FMissionApp(GAMESYS_WORK * gsys, void* work );


/**
 *	@file		event_cgearget.h
 *	@brief  CGEARƒQƒbƒg
 *	@author	k.ohno
 *	@date		2010.02.22
 */
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"



//-----------------------------------------------------------------------------
/**
 * CGEAR‹N“®
 */
//-----------------------------------------------------------------------------
extern GMEVENT * CGEARGET_EVENT_Start( GAMESYS_WORK *gsys );

#if PM_DEBUG
extern void CGEARGET_EVENT_Change(GAMESYS_WORK * gsys,GMEVENT * event);
#endif

#ifdef _cplusplus
}	// extern "C"{
#endif




//======================================================================
/*
 * @file	evt_lock.h
 * @brief	イベントロック
 * @author saito
 */
//======================================================================

#pragma once

#include <gflib.h>

#include "savedata/misc.h"
#include "savedata/mystatus.h"

#include "evt_lock_def.h"

#define EVT_MAX (8)

extern void EVTLOCK_SetEvtLock( MISC * misc, const int inNo, MYSTATUS *my );
extern BOOL EVTLOCK_CheckEvtLock( MISC * misc, const int inNo, MYSTATUS *my );
#ifdef PM_DEBUG
extern void EVTLOCK_ClearEvtLock( MISC * misc, const int inNo, MYSTATUS *my );
#endif


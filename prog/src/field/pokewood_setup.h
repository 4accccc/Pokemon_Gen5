//============================================================================================
/**
 * @file  pokewood_setup.h
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.07.04
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "app/pkwd_count.h"
#include "app/pkwd_book.h"
#include "app/pkwd_selectscreen.h"
#include "app/pkwd_briefing.h"
#include "app/pkwd_record_monitor.h"

//=============================================================================
//    ŠO•”ŒöŠJ
//=============================================================================
extern PKWD_COUNT_PARAM * PKWD_COUNT_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work );
extern void PKWD_COUNT_PARAM_Delete( PKWD_COUNT_PARAM * pkwd_param );

#ifdef  PM_DEBUG
extern void PKWD_COUNT_PARAM_Setup_Debug( GAMESYS_WORK * gsys, PKWD_COUNT_PARAM * pkwd_param );
#endif

extern PKWD_BOOK_PARAM * PKWD_BOOK_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work );
extern void PKWD_BOOK_PARAM_Delete( PKWD_BOOK_PARAM * pkwd_param );


extern PKWD_SELECT_SCREEN_PARAM * PKWD_SELECT_SCREEN_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, PKWD_SELECT_MODE mode, u16 * res_work );
extern void PKWD_SELECT_SCREEN_PARAM_Delete( PKWD_SELECT_SCREEN_PARAM * param );

extern PKWD_BRIEFING_PARAM * PKWD_BRIEFING_PARAM_Create( HEAPID heapID, GAMESYS_WORK *gsys, int scenario_no );
extern void PKWD_BRIEFING_PARAM_Delete( PKWD_BRIEFING_PARAM * param );

extern PKWD_RECORD_MONITOR_PARAM * PKWD_RECORD_MONITOR_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys );
extern void PKWD_RECORD_MONITOR_PARAM_Delete( PKWD_RECORD_MONITOR_PARAM * pkwd_param );


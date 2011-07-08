//============================================================================================
/**
 * @file  wbt_setup.h
 * @brief
 * @author  tamada GAME FREAK inc.
 * @date  2011.02.13
 *
 */
//============================================================================================
#pragma once

#include "field/wbt.h"

#include "app/wbt_list.h" //WBTLIST_PARAM
#include "app/win_record.h"		// WIN_RECORD_PARAM
#include "app/t_download.h"		// T_DOWNLOAD_PARAM


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------

//============================================================================================
//============================================================================================

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern WBTLIST_PARAM * WBTLIST_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys );
extern void WBTLIST_PARAM_Delete( WBTLIST_PARAM * wbtlist_param );

extern void WBTLIST_PARAM_Setup( GAMESYS_WORK * gsys, WBTSYS * wbtsys, WBTLIST_PARAM * wbtlist_param );
#ifdef  PM_DEBUG
extern void WBTLIST_PARAM_Setup_Debug(
    GAMESYS_WORK * gsys, WBTLIST_PARAM * wbtlist_param,
    WBTCUP wbtcup, WBTRULE rule,
    WBTSTAGE stage, const u16 * trid_table, int debug_value );
#endif

//------------------------------------------------------------------
//
//------------------------------------------------------------------
extern WIN_RECORD_PARAM * WIN_RECORD_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work );
extern void WIN_RECORD_PARAM_Delete( WIN_RECORD_PARAM * wr_param );
#ifdef  PM_DEBUG
extern void WIN_RECORD_PARAM_Setup_Debug( GAMESYS_WORK * gsys, WIN_RECORD_PARAM * wr_param );
#endif

//------------------------------------------------------------------
//
//------------------------------------------------------------------
extern T_DOWNLOAD_PARAM * T_DOWNLOAD_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work );
extern void T_DOWNLOAD_PARAM_Delete( T_DOWNLOAD_PARAM * td_param );
#ifdef PM_DEBUG
extern void T_DOWNLOAD_PARAM_Setup_Debug( GAMESYS_WORK * gsys, T_DOWNLOAD_PARAM * td_param );
#endif


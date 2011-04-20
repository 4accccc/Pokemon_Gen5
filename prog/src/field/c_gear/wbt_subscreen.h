//=============================================================================
/**
 * @file	  wbt_subscreen.h
 * @brief	  WBT専用下画面
 * @author	tamada@gamefreak.co.jp
 * @date	  2011.04.20
 *
 * src/field/c_gear/no_gear.cからコピペで作成開始
 */
//=============================================================================



#pragma once

#include <gflib.h>
#include "savedata/c_gear_data.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

FS_EXTERN_OVERLAY(wbt_subscreen);

typedef struct _WBTSUBSCRN_WORK  WBTSUBSCRN_WORK;

extern WBTSUBSCRN_WORK* WBTSUBSCRN_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys );
extern void WBTSUBSCRN_Main( WBTSUBSCRN_WORK* pWork,BOOL bAction );
extern void WBTSUBSCRN_ActionCallback( WBTSUBSCRN_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno);
extern void WBTSUBSCRN_Exit(WBTSUBSCRN_WORK* pWork);



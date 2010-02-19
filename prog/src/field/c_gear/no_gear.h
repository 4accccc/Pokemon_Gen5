//=============================================================================
/**
 * @file	  no_gear.h
 * @brief	  コミュニケーションギアが無い状態
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/09
 */
//=============================================================================



#pragma once

#include <gflib.h>
#include "savedata/c_gear_data.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

FS_EXTERN_OVERLAY(no_gear);

typedef struct _NO_GEAR_WORK  NO_GEAR_WORK;

extern NO_GEAR_WORK* NOGEAR_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys );
extern void NOGEAR_Main( NO_GEAR_WORK* pWork,BOOL bAction );
extern void NOGEAR_ActionCallback( NO_GEAR_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno);
extern void NOGEAR_Exit(NO_GEAR_WORK* pWork);



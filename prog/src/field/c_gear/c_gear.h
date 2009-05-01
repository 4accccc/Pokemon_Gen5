//=============================================================================
/**
 * @file	  c_gear.h
 * @brief	  コミュニケーションギア
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================



#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

typedef struct _C_GEAR_WORK  C_GEAR_WORK;

extern C_GEAR_WORK* CGEAR_Init(void);
extern void CGEAR_Main(C_GEAR_WORK* pWork);
extern void CGEAR_Exit(C_GEAR_WORK* pWork);

//extern const GFL_PROC_DATA CGEARProcData;



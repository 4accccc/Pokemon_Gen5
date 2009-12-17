//=============================================================================
/**
 * @file	  cg_wireless_menu.h
 * @brief	  C-GEARワイヤレスメニュー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/12/14
 */
//=============================================================================


#pragma once


#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"



typedef struct _CG_WIRELESS_MENU  CG_WIRELESS_MENU;

extern const GFL_PROC_DATA CG_WirelessMenuProcData;
extern const GFL_PROC_DATA CGEAR_SubMenuProcData;


enum CG_WIRELESS_RETURNMODE {
  CG_WIRELESS_RETURNMODE_PALACE,
  CG_WIRELESS_RETURNMODE_TV,
  CG_WIRELESS_RETURNMODE_NONE,
  CG_WIRELESS_RETURNMODE_MAX,
};


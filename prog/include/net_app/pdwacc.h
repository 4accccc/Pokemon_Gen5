//=============================================================================
/**
 * @file	  gsync.h
 * @bfief	  ゲームシンク関連のヘッダー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/15
 */
//=============================================================================




#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

typedef struct _PDWACC_WORK PDWACC_WORK;

typedef enum{
  PDWACC_GETACC,
  PDWACC_DISPPASS,
  
} PDWACC_TYPE_ENUM;


typedef struct {
  GAMEDATA  *gameData;
  DWCSvlResult* pSvl;
  HEAPID heapID;
  int type;
} PDWACC_PROCWORK;


extern const GFL_PROC_DATA PDW_ACC_ProcData;


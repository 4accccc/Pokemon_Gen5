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



typedef struct _GAMESYNC_MENU  GAMESYNC_MENU;
typedef struct _G_SYNC_WORK G_SYNC_WORK;

extern const GFL_PROC_DATA GameSyncMenuProcData;
extern const GFL_PROC_DATA G_SYNC_ProcData;

enum GAMESYNC_RETURNMODE {
  GAMESYNC_RETURNMODE_NONE,
  GAMESYNC_RETURNMODE_SYNC,
  GAMESYNC_RETURNMODE_UTIL,

  GAMESYNC_RETURNMODE_BOXJUMP,
  GAMESYNC_RETURNMODE_EXIT,


  
  GAMESYNC_RETURNMODE_MAX,
};


//=============================================================================
/**
 * @file	  dpc_check.h
 * @bfief	  デジタルプレーヤーカード検査
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  10/01/07
 */
//=============================================================================




#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"


typedef struct _DPC_CHECK_WORK DPC_CHECK_WORK;

extern const GFL_PROC_DATA DPC_CheckProcData;


typedef enum  {
  DPC_CHECK_RETURNMODE_OK,         // バトル可能
  DPC_CHECK_RETURNMODE_END,        // 期限切れ
  DPC_CHECK_RETURNMODE_FAILED,     // ダウンロード失敗
  DPC_CHECK_RETURNMODE_NOACCOUNT,  // 大好きクラブサーバに登録が無い

  DPC_CHECK_RETURNMODE_MAX,
}DPC_CHECK_RETURNMODE;

typedef struct{
  DPC_CHECK_RETURNMODE mode;
  GAMEDATA* gameData;
  HEAPID heapID;
} DPC_CHECK_INITWORK;


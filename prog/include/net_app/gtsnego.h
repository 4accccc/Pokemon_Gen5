//=============================================================================
/**
 * @file	  gtsnego.h
 * @bfief	  GTSネゴシエーション関連のヘッダー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/15
 */
//=============================================================================


#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#define EVENT_GTSNEGO_RECONNECT_NUM (100)


//親ワーク
typedef struct {
  int selectLV;
  int selectType;
} EVENT_GTSNEGO_USER_DATA;

typedef struct {
  GAMEDATA * gamedata;
  EVENT_GTSNEGO_USER_DATA aUser[2];
  MYSTATUS* pStatus[2];
  u32 profileID[EVENT_GTSNEGO_RECONNECT_NUM];
  int count;
} EVENT_GTSNEGO_WORK;

typedef struct _GTSNEGO_WORK  GTSNEGO_WORK;

extern const GFL_PROC_DATA GtsNego_ProcData;

FS_EXTERN_OVERLAY(gts_negotiate);








//=============================================================================
/**
 * @file	  wifi_login.h
 * @bfief	  WIFI ログイン関連のヘッダー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/17
 */
//=============================================================================


#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

typedef struct _WIFILOGIN_WORK  WIFILOGIN_WORK;

extern const GFL_PROC_DATA WiFiLogin_ProcData;

FS_EXTERN_OVERLAY(wifi_login);


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

//-------------------------------------
///	WIFILOGIN終了
//=====================================
typedef enum
{
  WIFILOGIN_RESULT_LOGIN, //ログインした
  WIFILOGIN_RESULT_CANCEL,//キャンセルした
} WIFILOGIN_RESULT;


//-------------------------------------
///	WIFILOGINに渡す引数
//=====================================
typedef struct 
{
  GAMEDATA      *gamedata;  //[in ]ゲームデータ
  int bDreamWorld;          //[in ]ドリームワールドの時は背景変更
  WIFILOGIN_RESULT  result; //[out]終了方法
} WIFILOGIN_PARAM;


typedef struct _WIFILOGIN_WORK  WIFILOGIN_WORK;

extern const GFL_PROC_DATA WiFiLogin_ProcData;

FS_EXTERN_OVERLAY(wifi_login);


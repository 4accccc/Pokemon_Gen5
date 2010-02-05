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
///	WIFILOGIN表示方法 文字やはいいいえが表示されるディスプレイ
//=====================================
typedef enum
{
  WIFILOGIN_DISPLAY_DOWN, //下画面にテキストと選択肢を表示
  WIFILOGIN_DISPLAY_UP,   //上画面にテキストと選択肢を表示
} WIFILOGIN_DISPLAY;

//-------------------------------------
///	WIFILOGIN表示方法 
//=====================================
typedef enum
{
  WIFILOGIN_BG_NORMAL,      //通常用
  WIFILOGIN_BG_DREAM_WORLD, //ドリームワールド用
} WIFILOGIN_BG;

//-------------------------------------
///	WIFILOGINに渡す引数
//=====================================
typedef struct 
{
  GAMEDATA      *gamedata;    //[in ]ゲームデータ
  WIFILOGIN_BG  bg;           //[in ]使用する背景
  WIFILOGIN_DISPLAY display;  //[in ]どちらの画面を使うか
  WIFILOGIN_RESULT  result;   //[out]終了方法
} WIFILOGIN_PARAM;


typedef struct _WIFILOGIN_WORK  WIFILOGIN_WORK;

extern const GFL_PROC_DATA WiFiLogin_ProcData;

FS_EXTERN_OVERLAY(wifi_login);


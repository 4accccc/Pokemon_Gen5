//============================================================================================
/**
 * @file	event_wificlub.h
 * @brief	イベント：WIFIクラブ
 * @author	ohno
 * @date	2009.03.31
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

#include "battle/battle.h"

#include "pm_define.h"
#include "net/network_define.h"

#include "gamesystem/btl_setup.h"

#include "net_app/wificlub/wifi_p2pmatch.h"
#include "net_app/pokemontrade.h"
#include "net_app/wificlub/wifi_p2pmatch_local.h"

#include "net/dwc_rap.h"
#include "net/dwc_raputil.h"
#include "net/dwc_rapcommon.h"
#include "savedata/wifilist.h"
#include "system/main.h"
#include "sound/pm_sndsys.h"
#include "sound/pm_wb_voice.h"

#include "field/event_wificlub.h"

#include "net_app/comm_tvt_sys.h"  //TVトランシーバ
//#include "wifi_status.h"
#include "net_app/wifi_login.h"
#ifdef BUGFIX_AF_SYSTEM175_101021
#include "net_app/wifi_logout.h"
#endif

#include "app/pokelist.h"
#include "include/demo/comm_btl_demo.h"
#include "field/event_battle_call.h"

//リスト⇔ステータスのプロック
#include "net_app/wificlub/wifi_p2p_subproc.h"


//typedef struct _EVENT_WIFICLUB_WORK EVENT_WIFICLUB_WORK;


FS_EXTERN_OVERLAY(event_wificlub);

typedef struct {
  GMEVENT * event;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
//  SAVE_CONTROL_WORK *ctrl;
  void* pWork;
  BOOL isEndProc;
  BOOL bFieldEnd;
  int selectType;

  GFL_PROCSYS* localProc;
  WIFICLUB_BATTLE_SUBPROC_PARAM subProcParam;
  COMM_TVT_INIT_WORK aTVT;
  WIFIP2PMATCH_PROC_PARAM* pMatchParam;
  POKEMONTRADE_PARAM aPokeTr;
  GAMEDATA* pGameData;
  WIFI_LIST* pWifiList;
//  GAMESYS_WORK * gsys;
  WIFILOGIN_PARAM     login;
#ifdef BUGFIX_AF_SYSTEM175_101021
  WIFILOGOUT_PARAM     logout;
#endif
  BATTLE_SETUP_PARAM* para;
  COMM_BTL_DEMO_PARAM demo_prm;
  COMM_BATTLE_CALL_PROC_PARAM prm;
  POKEPARTY* pPokeParty;   //バトルに渡すPokeParty
  u16* ret;
  u16 soundNo;
  u8 bSingle;
  u8 dummy;

} EVENT_WIFICLUB_WORK;




//-------------------------------------
///	GMEVENT_CreateOverlayEventCall関数用コールバック関数
//
//  void* work には EV_WIFICLUB_PARAM*を渡す。
//=====================================
//-------------------------------------
///	パラメータ
//=====================================
typedef struct {
  FIELDMAP_WORK * fieldmap;
  BOOL bFieldEnd;
} EV_WIFICLUB_PARAM;
extern GMEVENT* EVENT_CallWiFiClub( GAMESYS_WORK * gsys, void * work );


//============================================================================================
/**
 * @file	wifi_p2pmatch.h
 * @bfief	WIFI P2P接続のマッチング
 * @author	k.ohno
 * @date	06.04.07
 */
//============================================================================================
#pragma once

#include <gflib.h>
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "wifi_p2pmatch_def.h"

//============================================================================================
//	定義
//============================================================================================

//------------------------------------------------------
/**
 * WIFIともだちリストGFL_PROCパラメータ
 */
//------------------------------------------------------
typedef struct {
  void* pMatch;
  GAMEDATA* pGameData;
  SAVE_CONTROL_WORK*  pSaveData;
  int seq;        // どこに分岐してほしいかが入っている
  int targetID;   // 対戦 交換する人が誰なのかが入っている
  u8 vchatMain;  // VCHATをONOFFするメインフラグ 内部は相手によって変化する為
  u8 btalk;    // 話しかけたのか、掲示板なのかで戻った際の分岐がある
}WIFIP2PMATCH_PROC_PARAM;

extern const GFL_PROC_DATA WifiP2PMatchProcData;

// ポインタ参照だけできるレコードコーナーワーク構造体
typedef struct _WIFIP2PMATCH_WORK WIFIP2PMATCH_WORK;	

// 呼び出すためのイベント
extern void EventCmd_P2PMatchProc(GMEVENT * event);
// 世界交換前に呼び出すためのイベント
extern void EventCmd_DPWInitProc(GMEVENT * event);
//
extern void EventCmd_DPWInitProc2(GMEVENT * event, u16* ret);


//------------------------------------------------------------------
/*
   @title WIFIクラブイベントメモリを確保
 */
//------------------------------------------------------------------
extern void* WIFICLUB_CreateWork(void);




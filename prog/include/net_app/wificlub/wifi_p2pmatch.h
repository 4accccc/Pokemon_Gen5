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

#include "savedata/wifilist.h"
#include "savedata/regulation.h"

//============================================================================================
//	定義
//============================================================================================

typedef struct{
  u8 shooterSelect;   //自分で選択したシューター
  u8 battleModeSelect; //自分で選択したバトルモード
  u8 battleRuleSelect; //自分で選択したバトルルール
  u8 dummy;
} BATTLEBOARD_MODE;


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
  POKEPARTY* pPokeParty[2];   //お互いのPartyを受信
  REGULATION* pRegulation;    //ROMから読み込み
  BATTLEBOARD_MODE battleBoard;  //ばとるレギュレーション選択モード 自分の
  u8 matchno[WIFILIST_FRIEND_MAX];   //前回マッチングした時のno
  u8 VCTOn[2];   ///<CNM_WFP2PMF_STATUS  
  u8 vchatMain;  // VCHATをONOFFするメインフラグ 内部は相手によって変化する為
  u8 friendNo;  //対戦している人の番号
  u8 shooter;         //戦うようのシューター
  u8 bTalk;   //話したか募集か
  u8 dummy2; 
  u8 dummy3; 
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




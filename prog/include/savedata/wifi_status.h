#pragma once
//============================================================================================
/**
 * @file	  wifi_status.h
 * @brief	  WIFIに流れる情報の管理
            セーブするデータではないが mystatus構造体の内包のためここにある
 * @author	k.ohno
 * @date	  2009.6.6
 */
//============================================================================================
#include "savedata/mystatus.h"

typedef struct _WIFI_STATUS WIFI_STATUS;


//---------------WIFISTATUSデータ
typedef enum{
  WIFI_STATUS_NONE,   // 何も無い	NONEのときは出現もしません
  WIFI_STATUS_VCT,      // VCT中
  WIFI_STATUS_SBATTLE50,      // シングル対戦中
  WIFI_STATUS_SBATTLE100,      // シングル対戦中
  WIFI_STATUS_SBATTLE_FREE,      // シングル対戦中
  WIFI_STATUS_DBATTLE50,      // ダブル対戦中
  WIFI_STATUS_DBATTLE100,      // ダブル対戦中
  WIFI_STATUS_DBATTLE_FREE,      // ダブル対戦中
  WIFI_STATUS_TRADE,          // 交換中
  WIFI_STATUS_TVT,          // TVトランシーバ
  WIFI_STATUS_SBATTLE50_WAIT,   // シングルLv50対戦募集中
  WIFI_STATUS_SBATTLE100_WAIT,   // シングルLv100対戦募集中
  WIFI_STATUS_SBATTLE_FREE_WAIT,   // シングルFree対戦募集中
  WIFI_STATUS_DBATTLE50_WAIT,   // ダブルLv50対戦募集中
  WIFI_STATUS_DBATTLE100_WAIT,   // ダブルLv100対戦募集中
  WIFI_STATUS_DBATTLE_FREE_WAIT,   // ダブルFree対戦募集中
  WIFI_STATUS_TRADE_WAIT,    // 交換募集中
  WIFI_STATUS_TVT_WAIT,    // 交換募集中
  WIFI_STATUS_LOGIN_WAIT,    // 待機中　ログイン直後はこれ
  
  WIFI_STATUS_DP_UNK,        // DPのUNKNOWN
  
  // プラチナで追加
  WIFI_STATUS_FRONTIER,          // フロンティア中
  WIFI_STATUS_FRONTIER_WAIT,    // フロンティア募集中

  WIFI_STATUS_PLAY_OTHER,	// WiFiクラブに以外で遊び中
  WIFI_STATUS_UNKNOWN,   // 新たに作ったらこの番号以上になる

} WIFI_STATUS_e;



extern int WIFI_STATUS_GetSize(void);
extern MYSTATUS* WIFI_STATUS_GetMyStatus(WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetMyStatus(WIFI_STATUS* pStatus, const MYSTATUS* pMy);
extern u8 WIFI_STATUS_GetWifiMode(const WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetWifiMode( WIFI_STATUS* pStatus, u8 mode);
extern u8 WIFI_STATUS_GetVChatStatus(const WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetVChatStatus(WIFI_STATUS* pStatus,u8 vct);

extern void WIFI_STATUS_SetMonsNo(WIFI_STATUS* pStatus,int index,u16 no);
extern void WIFI_STATUS_SetItemNo(WIFI_STATUS* pStatus,int index,u16 no);
extern u16 WIFI_STATUS_GetMonsNo(WIFI_STATUS* pStatus,int index);
extern u16 WIFI_STATUS_GetItemNo(WIFI_STATUS* pStatus,int index);
extern void WIFI_STATUS_SetMyNation(WIFI_STATUS* pStatus,u8 no);
extern void WIFI_STATUS_SetMyArea(WIFI_STATUS* pStatus,u8 no);

extern u8 WIFI_STATUS_GetActive(const WIFI_STATUS* pStatus);
extern BOOL WIFI_STATUS_IsVChatMac(const WIFI_STATUS* pMyStatus, const WIFI_STATUS* pFriendStatus);
extern void WIFI_STATUS_SetVChatMac(WIFI_STATUS* pStatus, const WIFI_STATUS* pFriendStatus);
extern void WIFI_STATUS_ResetVChatMac(WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetMyMac(WIFI_STATUS* pStatus);



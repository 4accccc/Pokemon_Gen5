//==============================================================================
/**
 * @file    game_comm.h
 * @brief   ゲーム通信管理のヘッダ
 * @author  matsuda
 * @date    2009.05.12(火)
 */
//==============================================================================
#pragma once

//==============================================================================
//  スクリプトと共有する定数定義
//==============================================================================
///実行しているゲーム種類
#define GAME_COMM_NO_NULL                 (0)   ///<何も起動していない状態
#define GAME_COMM_NO_FIELD_BEACON_SEARCH  (1)   ///<フィールド上でビーコンサーチ
#define GAME_COMM_NO_INVASION             (2)   ///<侵入
#define GAME_COMM_NO_UNION                (3)   ///<ユニオンルーム
#define GAME_COMM_NO_DEBUG_SCANONLY       (4)   ///<デバッグ負荷用の常時スキャン
#define GAME_COMM_NO_MAX                  (5)

//=========================================================================
//アセンブラでincludeされている場合は、下の宣言を無視できるようにifndefで囲んである
#ifndef	__ASM_NO_DEF_

#include "gamesystem/game_data.h"

typedef int GAME_COMM_NO; ///GAME_COMM_NO_XXXの定義値を返す型

//==============================================================================
//  定数定義
//==============================================================================
///実行している通信モード
typedef enum{
  GAME_COMM_STATUS_NULL,              ///<何もしていない
  
  GAME_COMM_STATUS_IRC,               ///<赤外線通信
  GAME_COMM_STATUS_WIRELESS,          ///<ワイヤレス通信 パレス
  GAME_COMM_STATUS_WIRELESS_TR,       ///<ワイヤレス通信 トランシーバー
  GAME_COMM_STATUS_WIRELESS_UN,       ///<ワイヤレス通信 ユニオン
  GAME_COMM_STATUS_WIRELESS_FU,       ///<ワイヤレス通信 不思議
  GAME_COMM_STATUS_WIFI,              ///<Wi-Fi通信 登録済み
  GAME_COMM_STATUS_WIFI_ZONE,              ///<Wi-Fi通信 任天堂ゾーン等任天堂公式
  GAME_COMM_STATUS_WIFI_FREE,              ///<Wi-Fi通信 鍵が無い
  GAME_COMM_STATUS_WIFI_LOCK,              ///<Wi-Fi通信 鍵がある
  GAME_COMM_STATUS_MAX
}GAME_COMM_STATUS;

///実行している通信モード BIT版 
typedef enum{
  GAME_COMM_STATUS_BIT_IRC          = 1<<GAME_COMM_STATUS_IRC,///<赤外線通信
  GAME_COMM_STATUS_BIT_WIRELESS     = 1<<GAME_COMM_STATUS_WIRELESS,///<ワイヤレス通信 パレス
  GAME_COMM_STATUS_BIT_WIRELESS_TR  = 1<<GAME_COMM_STATUS_WIRELESS_TR, ///<ワイヤレス通信 トランシーバー
  GAME_COMM_STATUS_BIT_WIRELESS_UN  = 1<<GAME_COMM_STATUS_WIRELESS_UN,///<ワイヤレス通信 ユニオン
  GAME_COMM_STATUS_BIT_WIRELESS_FU  = 1<<GAME_COMM_STATUS_WIRELESS_FU,///<ワイヤレス通信 不思議
  GAME_COMM_STATUS_BIT_WIFI         = 1<<GAME_COMM_STATUS_WIFI,///<Wi-Fi通信 登録済み
  GAME_COMM_STATUS_BIT_WIFI_ZONE    = 1<<GAME_COMM_STATUS_WIFI_ZONE,///<Wi-Fi通信 任天堂ゾーン等任天堂公式
  GAME_COMM_STATUS_BIT_WIFI_FREE    = 1<<GAME_COMM_STATUS_WIFI_FREE,///<Wi-Fi通信 鍵が無い
  GAME_COMM_STATUS_BIT_WIFI_LOCK    = 1<<GAME_COMM_STATUS_WIFI_LOCK, ///<Wi-Fi通信 鍵がある


  GAME_COMM_SBIT_WIFI_ALL           = (GAME_COMM_STATUS_BIT_WIFI | GAME_COMM_STATUS_BIT_WIFI_ZONE | GAME_COMM_STATUS_BIT_WIFI_FREE | GAME_COMM_STATUS_BIT_WIFI_LOCK),
  GAME_COMM_SBIT_WIRELESS_ALL = (GAME_COMM_STATUS_BIT_WIRELESS|GAME_COMM_STATUS_BIT_WIRELESS_TR|GAME_COMM_STATUS_BIT_WIRELESS_UN|GAME_COMM_STATUS_BIT_WIRELESS_FU),
  GAME_COMM_SBIT_IRC_ALL =GAME_COMM_STATUS_BIT_IRC,
}GAME_COMM_STATUS_BIT;





///インフォメーションメッセージ内のwordset最大数
#define INFO_WORDSET_MAX      (4)

//==============================================================================
//  型定義
//==============================================================================
///ゲーム通信管理ワークの不定形ポインタ
typedef struct _GAME_COMM_SYS * GAME_COMM_SYS_PTR;


//==============================================================================
//  構造体定義
//==============================================================================
///インフォメーションメッセージ構造体(キューを取得する時、この構造体に変換して取得する)
typedef struct{
  STRBUF *name[INFO_WORDSET_MAX];     ///<未使用の場合はNULLが入っています
  u8 wordset_no[INFO_WORDSET_MAX];
  u16 message_id;
  u8 padding[2];
}GAME_COMM_INFO_MESSAGE;


//==============================================================================
//  外部関数宣言
//==============================================================================
//--------------------------------------------------------------
//  システム使用    ※通常ユーザーは使用しない
//--------------------------------------------------------------
extern GAME_COMM_SYS_PTR GameCommSys_Alloc(HEAPID heap_id, GAMEDATA *gamedata);
extern void GameCommSys_Free(GAME_COMM_SYS_PTR gcsp);
extern void GameCommSys_Main(GAME_COMM_SYS_PTR gcsp);
extern void GameCommSys_Callback_FieldCreate(GAME_COMM_SYS_PTR gcsp, void *fieldWork);
extern void GameCommSys_Callback_FieldDelete(GAME_COMM_SYS_PTR gcsp, void *fieldWork);

//--------------------------------------------------------------
//  ユーザー使用
//--------------------------------------------------------------
extern void GameCommSys_Boot(GAME_COMM_SYS_PTR gcsp, GAME_COMM_NO game_comm_no, void *parent_work);
extern void GameCommSys_ExitReq(GAME_COMM_SYS_PTR gcsp);
extern void GameCommSys_ChangeReq(GAME_COMM_SYS_PTR gcsp, GAME_COMM_NO game_comm_no, void *parent_work);
extern GAME_COMM_STATUS GameCommSys_GetCommStatus(GAME_COMM_SYS_PTR gcsp);
extern GAME_COMM_NO GameCommSys_BootCheck(GAME_COMM_SYS_PTR gcsp);
extern GAME_COMM_NO GameCommSys_GetCommGameNo(GAME_COMM_SYS_PTR gcsp);
extern BOOL GameCommSys_CheckSystemWaiting(GAME_COMM_SYS_PTR gcsp);
extern void *GameCommSys_GetAppWork(GAME_COMM_SYS_PTR gcsp);
extern GAMEDATA * GameCommSys_GetGameData(GAME_COMM_SYS_PTR gcsp);
extern void GameCommStatus_SetPlayerStatus(GAME_COMM_SYS_PTR gcsp, int comm_net_id, ZONEID zone_id, u8 invasion_netid);
extern u8 GameCommStatus_GetPlayerStatus_InvasionNetID(GAME_COMM_SYS_PTR gcsp, int comm_net_id);
extern BOOL GameCommInfo_GetMessage(GAME_COMM_SYS_PTR gcsp, GAME_COMM_INFO_MESSAGE *dest_msg);
extern void GameCommInfo_MessageEntry_Mission(GAME_COMM_SYS_PTR gcsp, int accept_netid);


#endif  //__ASM_NO_DEF_

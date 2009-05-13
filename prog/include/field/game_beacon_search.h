//==============================================================================
/**
 * @file    game_beacon_search.h
 * @brief   
 * @author  matsuda
 * @date    2009.05.12(火)
 */
//==============================================================================
#pragma once


//==============================================================================
//  型定義
//==============================================================================
///ゲーム通信管理ワークの不定形ポインタ
typedef struct _GAME_BEACON_SYS * GAME_BEACON_SYS_PTR;


//==============================================================================
//  構造体定義
//==============================================================================
///送受信するビーコンデータ
typedef struct
{
  GameServiceID gsid; ///<Game Service ID
  u8 member_num;      ///<現在の参加人数
  u8 member_max;      ///<最大人数
  u8 padding[2];
}GBS_BEACON;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void * GameBeacon_Init(void);
extern BOOL GameBeacon_InitWait(void *pWork);
extern void GameBeacon_Exit(void *pWork);
extern BOOL GameBeacon_ExitWait(void *pWork);
extern void GameBeacon_Update(void *pWork);

extern int GameBeacon_GetBeaconSize(void *pWork);


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
//  定数定義
//==============================================================================
typedef enum{
  GAME_COMM_NO_NULL,                  ///<何も起動していない状態
  
  GAME_COMM_NO_FIELD_BEACON_SEARCH,   ///<フィールド上でビーコンサーチ
  GAME_COMM_NO_INVASION,              ///<侵入
}GAME_COMM_NO;


//==============================================================================
//  型定義
//==============================================================================
///ゲーム通信管理ワークの不定形ポインタ
typedef struct _GAME_COMM_SYS * GAME_COMM_SYS_PTR;


//==============================================================================
//  外部関数宣言
//==============================================================================
//--------------------------------------------------------------
//  システム使用    ※通常ユーザーは使用しない
//--------------------------------------------------------------
extern GAME_COMM_SYS_PTR GameCommSys_Alloc(HEAPID heap_id);
extern void GameCommSys_Free(GAME_COMM_SYS_PTR gcsp);
extern void GameCommSys_Main(GAME_COMM_SYS_PTR gcsp);

//--------------------------------------------------------------
//  ユーザー使用
//--------------------------------------------------------------
extern void GameCommSys_Boot(GAME_COMM_SYS_PTR gcsp, GAME_COMM_NO comm_game_no);
extern void GameCommSys_ExitReq(GAME_COMM_SYS_PTR gcsp);
extern void GameCommSys_ChangeReq(GAME_COMM_SYS_PTR gcsp, GAME_COMM_NO comm_game_no);
extern GAME_COMM_NO GameCommSys_BootCheck(GAME_COMM_SYS_PTR gcsp);
extern GAME_COMM_NO GameCommSys_GetCommGameNo(GAME_COMM_SYS_PTR gcsp);
extern BOOL GameCommSys_CheckSystemWaiting(GAME_COMM_SYS_PTR gcsp);
extern void *GameCommSys_GetAppWork(GAME_COMM_SYS_PTR gcsp);



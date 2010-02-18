//======================================================================
/**
 * @file	mus_comm_func.h
 * @brief	ミュージカル通信機能
 * @author	ariizumi
 * @data	09/05/29
 *
 * モジュール名：MUS_COMM_FUNC
 */
//======================================================================
#pragma once

#include "musical/musical_define.h"
#include "musical/musical_local.h"
#include "musical/musical_comm_field.h"
#include "gamesystem/game_comm.h"
#include "savedata/save_control.h"
#include "savedata/mystatus.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//タイミングコマンド
#define MUS_COMM_TIMMING_DRESSUP_WAIT (10)
#define MUS_COMM_TIMMING_START_ACTING (20)
#define MUS_COMM_TIMMING_WAIT_FITTING (30)
//デバッグ
#define MUS_COMM_TIMMING_START_SCRIPT (200)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//通信モード
typedef enum
{
  MCM_NONE,
  MCM_PARENT,
  MCM_CHILD,
  MUS_COMM_MODE_MAX
}MUS_COMM_MODE;


//ゲーム状態
typedef enum
{
  MCGS_NONE = 0,
//  MCGS_LOBBY,
  MCGS_DRESSUP = 1,
  MCGS_WAIT_DRESSUP = 2,
  MCGS_ACTING = 3,
  
}MUS_COMM_GAME_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  STRCODE name[BUFLEN_PERSON_NAME+EOM_SIZE];
}MUS_COMM_BEACON;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern void MUS_COMM_InitMusical( MUS_COMM_WORK* work , MYSTATUS *myStatus ,  GAME_COMM_SYS_PTR gameComm , const HEAPID heapId );
extern void MUS_COMM_ExitMusical( MUS_COMM_WORK* work );

extern MUS_COMM_WORK* MUS_COMM_CreateWork( HEAPID heapId , GAME_COMM_SYS_PTR gameComm , SAVE_CONTROL_WORK *saveCtrl , MUSICAL_DISTRIBUTE_DATA *distData );
extern void MUS_COMM_DeleteWork( MUS_COMM_WORK* work );

extern void MUS_COMM_InitComm( MUS_COMM_WORK* work );
extern void MUS_COMM_ExitComm( MUS_COMM_WORK* work );
extern void MUS_COMM_UpdateComm( MUS_COMM_WORK* work );
extern const BOOL MUS_COMM_IsInitComm( MUS_COMM_WORK* work );

extern void MUS_COMM_StartParent( MUS_COMM_WORK* work );
extern void MUS_COMM_StartChild( MUS_COMM_WORK* work );
extern const BOOL MUS_COMM_StartGame( MUS_COMM_WORK* work );
extern const BOOL MUS_COMM_IsStartGame( MUS_COMM_WORK* work );

//ゲーム状態の設定
extern const BOOL MUS_COMM_SetCommGameState( MUS_COMM_WORK *work , MUS_COMM_GAME_STATE state );

extern const BOOL MUS_COMM_Send_MusPokeData( MUS_COMM_WORK *work , MUSICAL_POKE_PARAM *musPoke );
extern const BOOL MUS_COMM_Send_AllMusPokeData( MUS_COMM_WORK *work  );
extern const BOOL MUS_COMM_CheckAllPostPokeData( MUS_COMM_WORK *work );

extern void MUS_COMM_Start_SendStrmData( MUS_COMM_WORK *work );
extern const BOOL MUS_COMM_Send_StrmData( MUS_COMM_WORK *work , const u8 idx );
extern const BOOL MUS_COMM_CheckFinishSendStrm( MUS_COMM_WORK *work );

extern const BOOL MUS_COMM_Send_ProgramSize( MUS_COMM_WORK *work );
extern const BOOL MUS_COMM_Send_ProgramData( MUS_COMM_WORK *work );
extern const BOOL MUS_COMM_Send_MessageSize( MUS_COMM_WORK *work );
extern const BOOL MUS_COMM_Send_MessageData( MUS_COMM_WORK *work );
extern const BOOL MUS_COMM_Send_ScriptSize( MUS_COMM_WORK *work );
extern const BOOL MUS_COMM_Send_ScriptData( MUS_COMM_WORK *work );

//ミュージカルの役者IDXでデータを取得
extern MUSICAL_POKE_PARAM* MUS_COMM_GetMusPokeParam( MUS_COMM_WORK *work , const u8 musicalIdx );

extern void MUS_COMM_SendTimingCommand( MUS_COMM_WORK *work , const u8 no );
extern const BOOL MUS_COMM_CheckTimingCommand( MUS_COMM_WORK *work , const u8 no );

extern MYSTATUS *MUS_COMM_GetPlayerMyStatus( MUS_COMM_WORK* work , u8 idx );
extern const BOOL MUS_COMM_IsRefreshUserData( MUS_COMM_WORK *work );
extern void MUS_COMM_ResetRefreshUserData( MUS_COMM_WORK *work );

extern const BOOL MUS_COMM_Send_MusicalIndex( MUS_COMM_WORK* work );
extern const BOOL MUS_COMM_Send_UseButtonFlg( MUS_COMM_WORK* work , u8 pos );

extern u8 MUS_COMM_GetSelfMusicalIndex( MUS_COMM_WORK* work );
extern u8 MUS_COMM_GetMusicalIndex( MUS_COMM_WORK* work , const u8 idx);
extern GAME_COMM_SYS_PTR MUS_COMM_GetGameComm( MUS_COMM_WORK* work );
extern MUS_COMM_MODE MUS_COMM_GetMode( MUS_COMM_WORK* work );
extern const BOOL MUS_COMM_IsPostAllMyStatus( MUS_COMM_WORK* work );
extern void MUS_COMM_StartSendProgram( MUS_COMM_WORK* work , MUSICAL_DISTRIBUTE_DATA *distData );

extern u8 MUS_COMM_GetUseButtonPos( MUS_COMM_WORK* work , const u8 musIdx );
extern void MUS_COMM_ResetUseButtonPos( MUS_COMM_WORK* work , const u8 musIdx );
extern u8 MUS_COMM_GetUseButtonAttention( MUS_COMM_WORK* work );
extern void MUS_COMM_ResetUseButtonAttention( MUS_COMM_WORK* work );

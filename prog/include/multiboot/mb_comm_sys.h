//======================================================================
/**
 * @file	mb_comm_sys.c
 * @brief	マルチブート 通信システム
 * @author	ariizumi
 * @data	09/11/13
 *
 * モジュール名：MB_COMM
 */
//======================================================================
#pragma once

#include "poke_tool/poke_tool.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  //親→子
  MCFT_POST_POKE,
  MCFT_PERMIT_START_SAVE,
  MCFT_PERMIT_FIRST_SAVE,
  MCFT_PERMIT_SECOND_SAVE,
  MCFT_PERMIT_FINISH_SAVE,

  //子→親
  MCFT_CHILD_STATE = 10,
  MCFT_READY_START_SAVE,
  MCFT_FINISH_FIRST_SAVE,
  MCFT_FINISH_SECOND_SAVE,
  MCFT_FINISH_SAVE,
  
  MCFT_MAX,
}MB_COMM_FLG_TYPE;

typedef enum
{
  MCCS_NONE,
  MCCS_CONNECT,
  MCCS_SELECT_BOX,
  MCCS_WAIT_GAME_DATA,
  MCCS_CAP_GAME,
  MCCS_SEND_POKE,
  MCCS_NEXT_GAME,

  MCCS_END_GAME,
  MCCS_CANCEL_BOX,

}MB_COMM_CHILD_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _MB_COMM_WORK MB_COMM_WORK;

typedef struct
{
  int  msgSpeed;
  u16  highScore;
}MB_COMM_INIT_DATA;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//通信確立前
extern MB_COMM_WORK* MB_COMM_CreateSystem( const HEAPID heapId );
extern void MB_COMM_DeleteSystem( MB_COMM_WORK* commWork );
extern void MB_COMM_UpdateSystem( MB_COMM_WORK* commWork );
extern void MB_COMM_InitComm( MB_COMM_WORK* commWork );
extern void MB_COMM_ExitComm( MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_IsInitComm( MB_COMM_WORK* commWork);
extern const BOOL MB_COMM_IsFinishComm( MB_COMM_WORK* commWork );

extern void MB_COMM_InitParent( MB_COMM_WORK* commWork );
extern void MB_COMM_InitChild( MB_COMM_WORK* commWork , u8 *macAddress );

extern void MB_COMM_ReqDisconnect( MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_IsDisconnect( MB_COMM_WORK* commWork );

//通信確立後
extern MB_COMM_INIT_DATA* MB_COMM_GetInitData( MB_COMM_WORK* commWork );

//チェック系
extern const BOOL MB_COMM_IsSendEnable( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_IsPostInitData( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_IsPostGameData( const MB_COMM_WORK* commWork );

extern void MB_COMM_SetChildState( MB_COMM_WORK* commWork , MB_COMM_CHILD_STATE state );
extern const MB_COMM_CHILD_STATE MB_COMM_GetChildState( const MB_COMM_WORK* commWork );

extern void MB_COMM_ResetFlag( MB_COMM_WORK* commWork );

extern const BOOL MB_COMM_GetIsReadyChildStartSave( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_GetIsFinishChildFirstSave( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_GetIsFinishChildSecondSave( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_GetIsFinishChildSave( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_GetIsPermitStartSave( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_GetIsPermitFirstSave( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_GetIsPermitSecondSave( const MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_GetIsPermitFinishSave( const MB_COMM_WORK* commWork );
extern const u8 MB_COMM_GetSaveWaitTime( const MB_COMM_WORK* commWork );

extern void MB_COMM_InitSendGameData( MB_COMM_WORK* commWork , void* gameData , u32 size );
extern void MB_COMM_ClearSendPokeData( MB_COMM_WORK* commWork );
extern void MB_COMM_AddSendPokeData( MB_COMM_WORK* commWork , const POKEMON_PASO_PARAM *ppp );
extern void MB_COMM_SetScore( MB_COMM_WORK* commWork , const u16 score );

extern const BOOL MB_COMM_IsPostPoke( MB_COMM_WORK* commWork );
extern const u8 MB_COMM_GetPostPokeNum( MB_COMM_WORK* commWork );
extern const POKEMON_PASO_PARAM* MB_COMM_GetPostPokeData( MB_COMM_WORK* commWork , const u8 idx );
extern const u16 MB_COMM_GetScore( MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_IsPost_PostPoke( MB_COMM_WORK* commWork );

//送信系
extern const BOOL MB_COMM_Send_Flag( MB_COMM_WORK *commWork , const MB_COMM_FLG_TYPE type , const u32 value );
//送信終わるまでデータの保持を
extern const BOOL MB_COMM_Send_InitData( MB_COMM_WORK *commWork , MB_COMM_INIT_DATA *initData );
extern const BOOL MB_COMM_Send_GameData( MB_COMM_WORK *commWork , void *gameData , const u32 size );
extern const BOOL MB_COMM_Send_PokeData( MB_COMM_WORK *commWork );


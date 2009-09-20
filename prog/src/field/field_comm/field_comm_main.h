//======================================================================
/**
 * @file  field_comm_main.c
 * @brief フィールド通信の処理の外と中をつなぐ関数とか群
 * @author  ariizumi
 * @data  08/11/11
 */
//======================================================================

#ifndef FIELD_COMM_MAIN__
#define FIELD_COMM_MAIN__

#include "field_comm_def.h"
#include "../fieldmap.h"
#include "../field_player.h"
#include "gamesystem/gamesystem.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================
///フィールド通信タイプ
typedef enum{
  FIELD_COMM_TYPE_NORMAL,   ///<ノーマル
  FIELD_COMM_TYPE_PALACE,   ///<パレス
}FIELD_COMM_TYPE;

//======================================================================
//  typedef struct
//======================================================================

//======================================================================
//  proto
//======================================================================
extern FIELD_COMM_MAIN* FIELD_COMM_MAIN_InitSystem( HEAPID heapID , HEAPID commHeapID, GAME_COMM_SYS_PTR game_comm );
extern void FIELD_COMM_MAIN_TermSystem( FIELDMAP_WORK *fieldWork, FIELD_COMM_MAIN *commSys );
extern void FIELD_COMM_MAIN_UpdateCommSystem( FIELDMAP_WORK *fieldWork , GAMESYS_WORK *gameSys , FIELD_PLAYER *pcActor , FIELD_COMM_MAIN *commSys );

extern void FIELD_COMM_MAIN_SetCommType(FIELD_COMM_MAIN *commSys, FIELD_COMM_TYPE comm_type);
extern FIELD_COMM_TYPE FIELD_COMM_MAIN_GetCommType(FIELD_COMM_MAIN *commSys);
extern void FIELD_COMM_MAIN_SetCommActor(FIELD_COMM_MAIN *commSys, MMDLSYS *fmmdlsys);

extern const BOOL FIELD_COMM_MAIN_CanTalk( FIELD_COMM_MAIN *commSys );
extern const BOOL FIELD_COMM_MAIN_CheckReserveTalk( FIELD_COMM_MAIN *commSys );
extern void FIELD_COMM_MAIN_StartTalk( FIELD_COMM_MAIN *commSys );
extern void FIELD_COMM_MAIN_StartTalkPartner( FIELD_COMM_MAIN *commSys );

extern void FIELD_COMM_MAIN_InitStartCommMenu( FIELD_COMM_MAIN *commSys, FLDMSGBG *fldMsgBG );
extern void FIELD_COMM_MAIN_TermStartCommMenu( FIELD_COMM_MAIN *commSys );
extern const BOOL  FIELD_COMM_MAIN_LoopStartCommMenu( FIELD_COMM_MAIN *commSys, GAMESYS_WORK *gsys );

extern void FIELD_COMM_MAIN_InitStartInvasionMenu( FIELD_COMM_MAIN *commSys, FLDMSGBG *fldMsgBG );
extern void FIELD_COMM_MAIN_TermStartInvasionMenu( FIELD_COMM_MAIN *commSys );
extern const BOOL FIELD_COMM_MAIN_LoopStartInvasionMenu( GAMESYS_WORK *gsys, FIELD_COMM_MAIN *commSys );

extern void FIELD_COMM_MAIN_Disconnect( FIELDMAP_WORK *fieldWork , FIELD_COMM_MAIN *commSys );
extern BOOL FIELD_COMM_MAIN_DisconnectWait( FIELDMAP_WORK *fieldWork, FIELD_COMM_MAIN *commSys );

extern COMM_FIELD_SYS_PTR FIELD_COMM_MAIN_GetCommFieldSysPtr( FIELD_COMM_MAIN *commSys );
extern const int FIELD_COMM_MAIN_GetWorkSize(void);
extern FIELD_COMM_MENU** FIELD_COMM_MAIN_GetCommMenuWork( FIELD_COMM_MAIN *commSys );
extern const HEAPID FIELD_COMM_MAIN_GetHeapID( FIELD_COMM_MAIN *commSys );

extern COMM_FIELD_SYS_PTR FIELD_COMM_MAIN_CommFieldSysAlloc(HEAPID commHeapID, GAME_COMM_SYS_PTR game_comm);
extern void FIELD_COMM_MAIN_CommFieldSysFree(COMM_FIELD_SYS_PTR comm_field);
extern void FIELD_COMM_MAIN_CommFieldMapInit(COMM_FIELD_SYS_PTR comm_field);

//--------------------------------------------------------------
//  デバッグ用
//--------------------------------------------------------------
extern GMEVENT * DEBUG_PalaceJamp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor);
extern GMEVENT * DEBUG_PalaceTreeMapWarp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor, FIELD_COMM_MAIN *commSys);

#endif //FIELD_COMM_MAIN__


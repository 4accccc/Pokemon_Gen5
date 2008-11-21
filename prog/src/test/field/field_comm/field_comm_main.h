//======================================================================
/**
 * @file	field_comm_main.c
 * @brief	フィールド通信の処理の外と中をつなぐ関数とか群
 * @author	ariizumi
 * @data	08/11/11
 */
//======================================================================

#ifndef FIELD_COMM_MAIN__
#define FIELD_COMM_MAIN__

#include "../field_main.h"
#include "../field_player.h"
#include "gamesystem/gamesystem.h"

//======================================================================
//	define
//======================================================================
#define FIELD_COMM_MEMBER_MAX (4)
#define FCM_BGPLANE_YESNO_WINDOW (GFL_BG_FRAME1_M)	//使用フレーム(DEBUG_BGFRAME_MENU 
#define FCM_BGPLANE_MSG_WINDOW (GFL_BG_FRAME2_M)	//使用フレーム(DEBUG_BGFRAME_MENU 

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct _FIELD_COMM_MAIN FIELD_COMM_MAIN;

//======================================================================
//	proto
//======================================================================
extern FIELD_COMM_MAIN* FIELD_COMM_MAIN_InitSystem( HEAPID heapID , HEAPID commHeapID );
extern void	FIELD_COMM_MAIN_TermSystem( FIELD_COMM_MAIN *commSys , BOOL isTermAll );
extern void	FIELD_COMM_MAIN_UpdateCommSystem( FIELD_MAIN_WORK *fieldWork , GAMESYS_WORK *gameSys , PC_ACTCONT *pcActor , FIELD_COMM_MAIN *commSys );

extern const BOOL	FIELD_COMM_MAIN_CanTalk( FIELD_COMM_MAIN *commSys );
extern const BOOL	FIELD_COMM_MAIN_CheckReserveTalk( FIELD_COMM_MAIN *commSys );
extern void	FIELD_COMM_MAIN_StartTalk( FIELD_COMM_MAIN *commSys );
extern void	FIELD_COMM_MAIN_StartTalkPartner( FIELD_COMM_MAIN *commSys );

extern void	FIELD_COMM_MAIN_InitStartCommMenu( FIELD_COMM_MAIN *commSys );
extern void	FIELD_COMM_MAIN_TermStartCommMenu( FIELD_COMM_MAIN *commSys );
extern const BOOL	FIELD_COMM_MAIN_LoopStartCommMenu( FIELD_COMM_MAIN *commSys );

extern void	FIELD_COMM_MAIN_InitStartInvasionMenu( FIELD_COMM_MAIN *commSys );
extern void	FIELD_COMM_MAIN_TermStartInvasionMenu( FIELD_COMM_MAIN *commSys );
extern const BOOL	FIELD_COMM_MAIN_LoopStartInvasionMenu( FIELD_COMM_MAIN *commSys );




#endif //FIELD_COMM_MAIN__


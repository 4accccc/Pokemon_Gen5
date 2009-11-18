//======================================================================
/**
 * @file	mb_util_msg.h
 * @brief	マルチブート・メッセージ表示
 * @author	ariizumi
 * @data	09/11/16
 *
 * モジュール名：MB_MSG
 */
//======================================================================
#pragma once

#include "app/app_taskmenu.h"
#include "print/wordset.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MB_MSG_PLT_MAIN_TASKMENU (12)  //2本
#define MB_MSG_PLT_MAIN_MSGWIN (14)
#define MB_MSG_PLT_MAIN_FONT   (15)

#define MB_MSG_MSGWIN_CGX    (1)
#define MB_MSG_MSGWIN_TOP    (8)
#define MB_MSG_MSGWIN_LEFT   (3)
#define MB_MSG_MSGWIN_HEIGHT (8)
#define MB_MSG_MSGWIN_WIDTH  (26)

#define MB_MSG_YESNO_X ( 32-APP_TASKMENU_PLATE_WIDTH_YN_WIN )
#define MB_MSG_YESNO_Y ( 24-APP_TASKMENU_PLATE_HEIGHT*2 )
#define MB_MSG_YESNO_COLOR (APP_TASKMENU_ITEM_MSGCOLOR)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _MB_MSG_WORK MB_MSG_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern MB_MSG_WORK* MB_MSG_MessageInit( HEAPID heapId , const u8 frame );
extern void MB_MSG_MessageTerm( MB_MSG_WORK *work );
extern void MB_MSG_MessageMain( MB_MSG_WORK *work );
extern void MB_MSG_MessageDisp( MB_MSG_WORK *work , const u16 msgId );
extern void MB_MSG_MessageDispNoWait( MB_MSG_WORK *msgWork , const u16 msgId );
extern void MB_MSG_MessageHide( MB_MSG_WORK *work );
extern void MB_MSG_MessageCreateWordset( MB_MSG_WORK *work );
extern void MB_MSG_MessageDeleteWordset( MB_MSG_WORK *work );
extern void MB_MSG_MessageWordsetName( MB_MSG_WORK *msgWork , const u32 bufId , MYSTATUS *myStatus);
extern void MB_MSG_DispYesNo( MB_MSG_WORK *work );

extern GFL_MSGDATA* MB_MSG_GetMsgHandle( MB_MSG_WORK *msgWork );
extern WORDSET* MB_MSG_GetWordSet( MB_MSG_WORK *msgWork );


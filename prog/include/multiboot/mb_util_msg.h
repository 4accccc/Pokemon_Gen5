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
#include "system/bmp_winframe.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MB_MSG_PLT_MAIN_TASKMENU (10)  //2本
#define MB_MSG_PLT_MAIN_TALKWIN (12)
#define MB_MSG_PLT_MAIN_MSGWIN (13)
#define MB_MSG_PLT_MAIN_FONT   (14)

#define MB_MSG_MSGWIN_CGX    (1)
#define MB_MSG_TALKWIN_CGX   (MB_MSG_MSGWIN_CGX+MENU_WIN_CGX_SIZ)
#define MB_MSG_MSGWIN_TOP    (8)
#define MB_MSG_MSGWIN_LEFT   (3)
#define MB_MSG_MSGWIN_HEIGHT (8)
#define MB_MSG_MSGWIN_WIDTH  (26)

#define MB_MSG_MSGWIN_SEL_TOP    (21)
#define MB_MSG_MSGWIN_SEL_LEFT   (1)
#define MB_MSG_MSGWIN_SEL_HEIGHT (2)
#define MB_MSG_MSGWIN_SEL_WIDTH  (30)

#define MB_MSG_MSGWIN_SEL_L_TOP    (19) //下用
#define MB_MSG_MSGWIN_SEL_L_TOP_U  (1)  //上用
#define MB_MSG_MSGWIN_SEL_L_LEFT   (1)
#define MB_MSG_MSGWIN_SEL_L_HEIGHT (4)
#define MB_MSG_MSGWIN_SEL_L_WIDTH  (30)

#define MB_MSG_YESNO_X ( 32-APP_TASKMENU_PLATE_WIDTH_YN_WIN )
#define MB_MSG_YESNO_Y_DOWN ( 24-APP_TASKMENU_PLATE_HEIGHT*2 )
#define MB_MSG_YESNO_Y_UP ( 6 )
#define MB_MSG_YESNO_COLOR (APP_TASKMENU_ITEM_MSGCOLOR)
//選択画面用
#define MB_MSG_YESNO_SEL_X ( 24 )
#define MB_MSG_YESNO_SEL_Y ( 12 )

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MMWT_NONE ,
  MMWT_NORMAL,  //画面中央
  MMWT_1LINE,   //画面下１行
  MMWT_2LINE,   //画面下２行

  MMWT_2LINE_UP,   //画面上２行

  MMWT_2LINE_TALK,   //画面下２行(会話WIN
}MB_MSG_WIN_TYPE;

typedef enum
{
  MMYR_NONE,
  MMYR_RET1,  //上のほう
  MMYR_RET2,  //下のほう
}MB_MSG_YESNO_RET;

typedef enum
{
  MMYT_UP,
  MMYT_MID,
  MMYT_DOWN,
  
}MB_MSG_YESNO_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _MB_MSG_WORK MB_MSG_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern MB_MSG_WORK* MB_MSG_MessageInit( HEAPID heapId , const u8 frame , const u8 selFrame , const u32 datId , const BOOL useTalkWin );
extern void MB_MSG_MessageTerm( MB_MSG_WORK *work );
extern void MB_MSG_MessageMain( MB_MSG_WORK *work );
extern void MB_MSG_MessageCreateWindow( MB_MSG_WORK *msgWork , MB_MSG_WIN_TYPE type );
extern void MB_MSG_MessageDisp( MB_MSG_WORK *work , const u16 msgId , const int msgSpeed );
extern void MB_MSG_MessageDispNoWait( MB_MSG_WORK *msgWork , const u16 msgId );
extern void MB_MSG_MessageHide( MB_MSG_WORK *work );
extern void MB_MSG_MessageCreateWordset( MB_MSG_WORK *work );
extern void MB_MSG_MessageDeleteWordset( MB_MSG_WORK *work );
extern void MB_MSG_MessageWordsetName( MB_MSG_WORK *msgWork , const u32 bufId , MYSTATUS *myStatus);
extern void MB_MSG_DispYesNo( MB_MSG_WORK *work , const MB_MSG_YESNO_TYPE type );
extern void MB_MSG_ClearYesNo( MB_MSG_WORK *msgWork );
extern const MB_MSG_YESNO_RET MB_MSG_UpdateYesNo( MB_MSG_WORK *work );

extern GFL_MSGDATA* MB_MSG_GetMsgHandle( MB_MSG_WORK *msgWork );
extern WORDSET* MB_MSG_GetWordSet( MB_MSG_WORK *msgWork );
extern GFL_FONT* MB_MSG_GetFont( MB_MSG_WORK *msgWork );
extern PRINT_QUE* MB_MSG_GetPrintQue( MB_MSG_WORK *msgWork );
extern const BOOL MB_MSG_CheckPrintQueIsFinish( MB_MSG_WORK *msgWork );
extern const BOOL MB_MSG_CheckPrintStreamIsFinish( MB_MSG_WORK *msgWork );
extern void MB_MSG_SetDispKeyCursor( MB_MSG_WORK *msgWork , const BOOL flg );



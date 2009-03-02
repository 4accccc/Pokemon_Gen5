//=============================================================================
/**
 * @file	ircbattlemenu.h
 * @bfief	赤外線通信から無線に変わってバトルを行うメニュー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	09/02/19
 */
//=============================================================================



#ifndef IRCBATTLEMENU_H__
#define IRCBATTLEMENU_H__

#include <gflib.h>
//#include "../field_comm_def.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

typedef struct _IRC_BATTLE_MENU  IRC_BATTLE_MENU;

//extern int IRCBATTLE_MENU_GetWorkSize(void);
//extern void IRCBATTLE_MENU_InitWork( const HEAPID heapID , GAMESYS_WORK *gameSys , FIELD_MAIN_WORK *fieldWork , GMEVENT *event , IRC_BATTLE_MENU *pWork );
//extern GMEVENT_RESULT IRCBATTLE_MENU_Main( GMEVENT *event , int *seq , void *work );

extern const GFL_PROC_DATA IrcBattleMenuProcData;


#endif //IRCBATTLEMENU_H__


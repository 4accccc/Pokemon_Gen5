
#pragma once

#include <gflib.h>
#include "ui/touchbar.h"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK



typedef struct _GSYNC_MESSAGE_WORK  GSYNC_MESSAGE_WORK;
typedef struct _GSYNC_DISP_WORK  GSYNC_DISP_WORK;

#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント
#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット
#define _TOUCHBAR_PAL  (8)  //タッチバーのパレット

// はいいいえのタイプ
#define GSYNC_YESNOTYPE_INFO (0)
#define GSYNC_YESNOTYPE_SYS  (1)


typedef enum
{
  _ARROW_LEVEL_U,
  _ARROW_LEVEL_D,
  _ARROW_MY_U,
  _ARROW_MY_D,
  _ARROW_FRIEND_U,
  _ARROW_FRIEND_D,
} _ARROW_TYPE;


//→座標
#define _ARROW_LEVEL_XU  (12)
#define _ARROW_LEVEL_YU  (5)
#define _ARROW_LEVEL_XD  (30)
#define _ARROW_LEVEL_YD  (5)

#define _ARROW_MY_XU  (15)
#define _ARROW_MY_YU  (12)
#define _ARROW_MY_XD  (30)
#define _ARROW_MY_YD  (12)

#define _ARROW_FRIEND_XU  (15)
#define _ARROW_FRIEND_YU  (17)
#define _ARROW_FRIEND_XD  (30)
#define _ARROW_FRIEND_YD  (17)


extern GSYNC_MESSAGE_WORK* GSYNC_MESSAGE_Init(HEAPID id,int msg_dat);
extern void GSYNC_MESSAGE_Main(GSYNC_MESSAGE_WORK* pWork);
extern void GSYNC_MESSAGE_End(GSYNC_MESSAGE_WORK* pWork);
extern void GSYNC_MESSAGE_InfoMessageDisp(GSYNC_MESSAGE_WORK* pWork,int msgid);
extern BOOL GSYNC_MESSAGE_InfoMessageEndCheck(GSYNC_MESSAGE_WORK* pWork);
extern void GSYNC_MESSAGE_InfoMessageEnd(GSYNC_MESSAGE_WORK* pWork);
extern APP_TASKMENU_WORK* GSYNC_MESSAGE_YesNoStart(GSYNC_MESSAGE_WORK* pWork,int type);
extern void GSYNC_MESSAGE_ButtonWindowCreate(int num,int* pMsgBuff,GSYNC_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void GSYNC_MESSAGE_ButtonWindowDelete(GSYNC_MESSAGE_WORK* pWork);
extern void GSYNC_MESSAGE_ButtonWindowMain(GSYNC_MESSAGE_WORK* pWork);

extern void GSYNC_MESSAGE_SystemMessageDisp(GSYNC_MESSAGE_WORK* pWork,int msgid);
extern void GSYNC_MESSAGE_SystemMessageEnd(GSYNC_MESSAGE_WORK* pWork);
extern void GSYNC_MESSAGE_ErrorMessageDisp(GSYNC_MESSAGE_WORK* pWork,int msgid,int no);

extern void GSYNC_MESSAGE_DispAnyoneOrFriend(GSYNC_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void GSYNC_MESSAGE_DispLevel(GSYNC_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void GSYNC_MESSAGE_DispClear(GSYNC_MESSAGE_WORK* pWork);


extern void GSYNC_MESSAGE_DispLevelChange(GSYNC_MESSAGE_WORK* pWork,int no);
extern void GSYNC_MESSAGE_DispMyChange(GSYNC_MESSAGE_WORK* pWork,int no);
extern void GSYNC_MESSAGE_DispFriendChange(GSYNC_MESSAGE_WORK* pWork,int no);

extern APP_TASKMENU_WORK* GSYNC_MESSAGE_SearchButtonStart(GSYNC_MESSAGE_WORK* pWork);


extern GSYNC_DISP_WORK* GSYNC_DISP_Init(HEAPID id);
extern void GSYNC_DISP_Main(GSYNC_DISP_WORK* pWork);
extern void GSYNC_DISP_End(GSYNC_DISP_WORK* pWork);
extern void GSYNC_DISP_SettingSubBgControl(GSYNC_DISP_WORK* pWork);
extern void GSYNC_DISP_HandInit(GSYNC_DISP_WORK* pWork);
extern TOUCHBAR_WORK* GSYNC_DISP_GetTouchWork(GSYNC_DISP_WORK* pWork);
extern void GSYNC_DISP_PokemonIconCreate(GSYNC_DISP_WORK* pWork, POKEMON_PASO_PARAM* ppp, int draw);
extern void GSYNC_DISP_PokemonIconMove(GSYNC_DISP_WORK* pWork);
extern void GSYNC_DISP_PokemonIconJump(GSYNC_DISP_WORK* pWork);

extern void GSYNC_DISP_ObjInit(GSYNC_DISP_WORK* pWork,int no);
extern void GSYNC_DISP_ObjChange(GSYNC_DISP_WORK* pWork,int no,int no2);
extern void GSYNC_DISP_ObjEnd(GSYNC_DISP_WORK* pWork,int no);
extern void GSYNC_DISP_DreamSmokeBgStart(GSYNC_DISP_WORK* pWork);
extern void GSYNC_DISP_BlendSmokeStart(GSYNC_DISP_WORK* pWork,BOOL bFadein);
extern void GSYNC_DISP_SetPerfomance(GSYNC_DISP_WORK* pWork,int percent);


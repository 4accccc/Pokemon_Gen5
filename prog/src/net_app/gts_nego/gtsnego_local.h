
#pragma once

#include <gflib.h>
#include "ui/touchbar.h"


#define _SE_CUR	(SEQ_SE_SELECT1)         //カーソル移動。上下左右キー
#define _SE_DECIDE	(SEQ_SE_DECIDE1)     //決定音
#define _SE_CANCEL	(SEQ_SE_CANCEL1)     //キャンセル音


typedef struct _GTSNEGO_MESSAGE_WORK  GTSNEGO_MESSAGE_WORK;
typedef struct _GTSNEGO_DISP_WORK  GTSNEGO_DISP_WORK;

#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント
#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット
#define _TOUCHBAR_PAL  (8)  //タッチバーのパレット

// はいいいえのタイプ
#define GTSNEGO_YESNOTYPE_INFO (0)
#define GTSNEGO_YESNOTYPE_SYS  (1)




// スクロールするパネルのカーソルの管理
typedef struct{
  int listmax;
  int curpos;
  int oamlistpos;
} SCROLLPANELCURSOR;


typedef enum
{
  _ARROW_LEVEL_U,
  _ARROW_LEVEL_D,
  _ARROW_MY_U,
  _ARROW_MY_D,
  _ARROW_FRIEND_U,
  _ARROW_FRIEND_D,
} _ARROW_TYPE;

typedef enum
{
  _CROSSCUR_TYPE_MAINUP,
  _CROSSCUR_TYPE_MAINDOWN,
  _CROSSCUR_TYPE_ANY1,
  _CROSSCUR_TYPE_ANY2,
  _CROSSCUR_TYPE_ANY3,
  _CROSSCUR_TYPE_ANY4,
  _CROSSCUR_TYPE_FRIEND1,
  _CROSSCUR_TYPE_FRIEND2,
  _CROSSCUR_TYPE_FRIEND3,
  _CROSSCUR_TYPE_FRIEND4,
  _CROSSCUR_TYPE_NONE,
} CROSSCUR_TYPE;


typedef enum
{
  PANEL_NONESCROLL_,
  PANEL_UPSCROLL_,
  PANEL_DOWNSCROLL_,
} PANEL_SCROLL_TYPE_;



//→座標
#define _ARROW_LEVEL_XU  (12)
#define _ARROW_LEVEL_YU  (7)
#define _ARROW_LEVEL_XD  (30)
#define _ARROW_LEVEL_YD  (7)

#define _ARROW_MY_XU  (15)
#define _ARROW_MY_YU  (12)
#define _ARROW_MY_XD  (30)
#define _ARROW_MY_YD  (12)

#define _ARROW_FRIEND_XU  (15)
#define _ARROW_FRIEND_YU  (17)
#define _ARROW_FRIEND_XD  (30)
#define _ARROW_FRIEND_YD  (17)


extern GTSNEGO_MESSAGE_WORK* GTSNEGO_MESSAGE_Init(HEAPID id,int msg_dat);
extern void GTSNEGO_MESSAGE_Main(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_End(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_InfoMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid);
extern BOOL GTSNEGO_MESSAGE_InfoMessageEndCheck(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_InfoMessageEnd(GTSNEGO_MESSAGE_WORK* pWork);
extern APP_TASKMENU_WORK* GTSNEGO_MESSAGE_YesNoStart(GTSNEGO_MESSAGE_WORK* pWork,int type);
extern void GTSNEGO_MESSAGE_ButtonWindowCreate(int num,int* pMsgBuff,GTSNEGO_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void GTSNEGO_MESSAGE_ButtonWindowDelete(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_ButtonWindowMain(GTSNEGO_MESSAGE_WORK* pWork);

extern void GTSNEGO_MESSAGE_SystemMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid);
extern void GTSNEGO_MESSAGE_SystemMessageEnd(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_ErrorMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid,int no);

extern void GTSNEGO_MESSAGE_DispAnyoneOrFriend(GTSNEGO_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void GTSNEGO_MESSAGE_DispLevel(GTSNEGO_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void GTSNEGO_MESSAGE_DispClear(GTSNEGO_MESSAGE_WORK* pWork);


extern void GTSNEGO_MESSAGE_DispLevelChange(GTSNEGO_MESSAGE_WORK* pWork,int no);
extern void GTSNEGO_MESSAGE_DispMyChange(GTSNEGO_MESSAGE_WORK* pWork,int no);
extern void GTSNEGO_MESSAGE_DispFriendChange(GTSNEGO_MESSAGE_WORK* pWork,int no);
extern void GTSNEGO_MESSAGE_DeleteDispLevel(GTSNEGO_MESSAGE_WORK* pWork);

extern APP_TASKMENU_WIN_WORK* GTSNEGO_MESSAGE_SearchButtonStart(GTSNEGO_MESSAGE_WORK* pWork,int msgno);


extern GTSNEGO_DISP_WORK* GTSNEGO_DISP_Init(HEAPID id, GAMEDATA* pGameData);
extern void GTSNEGO_DISP_Main(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_End(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_SettingSubBgControl(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_LevelInputInit(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_LevelInputFree(GTSNEGO_DISP_WORK* pWork);
extern TOUCHBAR_WORK* GTSNEGO_DISP_GetTouchWork(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_FriendSelectInit(GTSNEGO_DISP_WORK* pWork, GTSNEGO_MESSAGE_WORK* pMessageWork);
extern void GTSNEGO_DISP_FriendSelectFree(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_CrossIconDisp(GTSNEGO_DISP_WORK* pWork,APP_TASKMENU_WIN_WORK* pAppWin , CROSSCUR_TYPE type);
extern BOOL GTSNEGO_DISP_FriendListDownChk(GTSNEGO_DISP_WORK* pWork, SCROLLPANELCURSOR* pCur);
extern BOOL GTSNEGO_DISP_FriendListUpChk(GTSNEGO_DISP_WORK* pWork, SCROLLPANELCURSOR* pCur);
extern void GTSNEGO_DISP_ScrollReset(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_PanelScrollStart(GTSNEGO_DISP_WORK* pWork,int scrollType);
extern int GTSNEGO_DISP_PanelScrollMain(GTSNEGO_DISP_WORK* pWork,int* EndTrg);
extern void GTSNEGO_DISP_UnionListDisp(GTSNEGO_DISP_WORK* pWork,MYSTATUS* pMy, int index);
extern void GTSNEGO_DISP_UnionListUp(GTSNEGO_DISP_WORK* pWork,MYSTATUS* pMy);
extern void GTSNEGO_DISP_UnionListDown(GTSNEGO_DISP_WORK* pWork,MYSTATUS* pMy);



//extern void GTSNEGO_MESSAGE_PlateDisp(GTSNEGO_MESSAGE_WORK* pWork, MYSTATUS* pMyStatus, int index );
extern void GTSNEGO_MESSAGE_FriendListPlateDisp(GTSNEGO_MESSAGE_WORK* pWork,GAMEDATA* pGameData);
extern void GTSNEGO_MESSAGE_FriendListDownStart(GTSNEGO_MESSAGE_WORK* pWork,GAMEDATA* pGameData, int no);
extern void GTSNEGO_MESSAGE_FriendListDownEnd(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_FriendListUpStart(GTSNEGO_MESSAGE_WORK* pWork,GAMEDATA* pGameData, int no);
extern void GTSNEGO_MESSAGE_FriendListUpEnd(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_AppMenuClose(APP_TASKMENU_WORK* pAppTask);
extern APP_TASKMENU_WORK* GTSNEGO_MESSAGE_MatchOrReturnStart(GTSNEGO_MESSAGE_WORK* pWork,int type);

extern MYSTATUS* GTSNEGO_GetMyStatus( GAMEDATA* pGameData, int index);



#define SCROLL_HEIGHT_SINGLE (48)   ///１パネルの高さ
#define SCROLL_HEIGHT_DEFAULT (24+SCROLL_HEIGHT_SINGLE*1) 
#define SCROLL_PANEL_NUM (6)   ///パネルの数
#define GTSNEGO_WINDOW_MAXNUM (SCROLL_PANEL_NUM)   //ウインドウのパターン数

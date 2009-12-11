
#pragma once

#include <gflib.h>
#include "ui/touchbar.h"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK



typedef struct _PDWACC_MESSAGE_WORK  PDWACC_MESSAGE_WORK;
typedef struct _PDWACC_DISP_WORK  PDWACC_DISP_WORK;

#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント
#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット
#define _TOUCHBAR_PAL  (8)  //タッチバーのパレット

// はいいいえのタイプ
#define PDWACC_YESNOTYPE_INFO (0)
#define PDWACC_YESNOTYPE_SYS  (1)


extern PDWACC_MESSAGE_WORK* PDWACC_MESSAGE_Init(HEAPID id,int msg_dat);
extern void PDWACC_MESSAGE_Main(PDWACC_MESSAGE_WORK* pWork);
extern void PDWACC_MESSAGE_End(PDWACC_MESSAGE_WORK* pWork);
extern void PDWACC_MESSAGE_InfoMessageDisp(PDWACC_MESSAGE_WORK* pWork,int msgid);
extern BOOL PDWACC_MESSAGE_InfoMessageEndCheck(PDWACC_MESSAGE_WORK* pWork);
extern void PDWACC_MESSAGE_InfoMessageEnd(PDWACC_MESSAGE_WORK* pWork);
extern APP_TASKMENU_WORK* PDWACC_MESSAGE_YesNoStart(PDWACC_MESSAGE_WORK* pWork,int type);
extern void PDWACC_MESSAGE_ButtonWindowCreate(int num,int* pMsgBuff,PDWACC_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void PDWACC_MESSAGE_ButtonWindowDelete(PDWACC_MESSAGE_WORK* pWork);
extern void PDWACC_MESSAGE_ButtonWindowMain(PDWACC_MESSAGE_WORK* pWork);

extern void PDWACC_MESSAGE_SystemMessageDisp(PDWACC_MESSAGE_WORK* pWork,int msgid);
extern void PDWACC_MESSAGE_SystemMessageEnd(PDWACC_MESSAGE_WORK* pWork);
extern void PDWACC_MESSAGE_ErrorMessageDisp(PDWACC_MESSAGE_WORK* pWork,int msgid,int no);

extern void PDWACC_MESSAGE_DispAnyoneOrFriend(PDWACC_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void PDWACC_MESSAGE_DispLevel(PDWACC_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void PDWACC_MESSAGE_DispClear(PDWACC_MESSAGE_WORK* pWork);


extern void PDWACC_MESSAGE_DispLevelChange(PDWACC_MESSAGE_WORK* pWork,int no);
extern void PDWACC_MESSAGE_DispMyChange(PDWACC_MESSAGE_WORK* pWork,int no);
extern void PDWACC_MESSAGE_DispFriendChange(PDWACC_MESSAGE_WORK* pWork,int no);

extern void PDWACC_MESSAGE_NoMessageDisp(PDWACC_MESSAGE_WORK* pWork,u64 code);
extern BOOL PDWACC_MESSAGE_NoMessageEndCheck(PDWACC_MESSAGE_WORK* pWork);
extern void PDWACC_MESSAGE_NoMessageEnd(PDWACC_MESSAGE_WORK* pWork);




extern APP_TASKMENU_WORK* PDWACC_MESSAGE_SearchButtonStart(PDWACC_MESSAGE_WORK* pWork);


extern PDWACC_DISP_WORK* PDWACC_DISP_Init(HEAPID id);
extern void PDWACC_DISP_Main(PDWACC_DISP_WORK* pWork);
extern void PDWACC_DISP_End(PDWACC_DISP_WORK* pWork);
extern void PDWACC_DISP_SettingSubBgControl(PDWACC_DISP_WORK* pWork);
extern void PDWACC_DISP_HandInit(PDWACC_DISP_WORK* pWork);
extern TOUCHBAR_WORK* PDWACC_DISP_GetTouchWork(PDWACC_DISP_WORK* pWork);
extern void PDWACC_DISP_PokemonIconCreate(PDWACC_DISP_WORK* pWork, POKEMON_PASO_PARAM* ppp, int draw);
extern void PDWACC_DISP_PokemonIconMove(PDWACC_DISP_WORK* pWork);
extern void PDWACC_DISP_PokemonIconJump(PDWACC_DISP_WORK* pWork);

extern void PDWACC_DISP_ObjInit(PDWACC_DISP_WORK* pWork,int no);
extern void PDWACC_DISP_ObjChange(PDWACC_DISP_WORK* pWork,int no,int no2);
extern void PDWACC_DISP_ObjEnd(PDWACC_DISP_WORK* pWork,int no);
extern void PDWACC_DISP_DreamSmokeBgStart(PDWACC_DISP_WORK* pWork);
extern void PDWACC_DISP_BlendSmokeStart(PDWACC_DISP_WORK* pWork,BOOL bFadein);


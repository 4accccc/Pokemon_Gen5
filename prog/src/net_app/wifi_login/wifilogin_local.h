
#pragma once

#include <gflib.h>



typedef struct _WIFILOGIN_MESSAGE_WORK  WIFILOGIN_MESSAGE_WORK;
typedef struct _WIFILOGIN_DISP_WORK  WIFILOGIN_DISP_WORK;

#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント

// はいいいえのタイプ
#define WIFILOGIN_YESNOTYPE_INFO (0)
#define WIFILOGIN_YESNOTYPE_SYS  (1)


extern WIFILOGIN_MESSAGE_WORK* WIFILOGIN_MESSAGE_Init(HEAPID id,int msg_dat);
extern void WIFILOGIN_MESSAGE_Main(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_End(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_InfoMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid);
extern BOOL WIFILOGIN_MESSAGE_InfoMessageEndCheck(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_InfoMessageEnd(WIFILOGIN_MESSAGE_WORK* pWork);
extern APP_TASKMENU_WORK* WIFILOGIN_MESSAGE_YesNoStart(WIFILOGIN_MESSAGE_WORK* pWork,int type);
extern void WIFILOGIN_MESSAGE_ButtonWindowCreate(int num,int* pMsgBuff,WIFILOGIN_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork);
extern void WIFILOGIN_MESSAGE_ButtonWindowDelete(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_ButtonWindowMain(WIFILOGIN_MESSAGE_WORK* pWork);

extern void WIFILOGIN_MESSAGE_SystemMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid);
extern void WIFILOGIN_MESSAGE_SystemMessageEnd(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_ErrorMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid,int no);


extern WIFILOGIN_DISP_WORK* WIFILOGIN_DISP_Init(HEAPID id);
extern void WIFILOGIN_DISP_Main(WIFILOGIN_DISP_WORK* pWork);
extern void WIFILOGIN_DISP_End(WIFILOGIN_DISP_WORK* pWork);
extern void WIFILOGIN_DISP_SettingSubBgControl(WIFILOGIN_DISP_WORK* pWork);


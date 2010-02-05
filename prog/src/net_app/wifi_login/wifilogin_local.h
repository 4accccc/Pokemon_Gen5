
#pragma once

#include <gflib.h>
#include "net_app/wifi_login.h"


typedef struct _WIFILOGIN_MESSAGE_WORK  WIFILOGIN_MESSAGE_WORK;
typedef struct _WIFILOGIN_DISP_WORK WIFILOGIN_DISP_WORK;
typedef struct _WIFILOGIN_YESNO_WORK WIFILOGIN_YESNO_WORK;

#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント

// はいいいえのタイプ
#define WIFILOGIN_YESNOTYPE_INFO (0)
#define WIFILOGIN_YESNOTYPE_SYS  (1)


//メッセージシステム
extern WIFILOGIN_MESSAGE_WORK* WIFILOGIN_MESSAGE_Init(HEAPID id,int msg_dat, WIFILOGIN_DISPLAY display);
extern void WIFILOGIN_MESSAGE_Main(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_End(WIFILOGIN_MESSAGE_WORK* pWork);

//メッセージ描画
extern void WIFILOGIN_MESSAGE_InfoMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid);
extern BOOL WIFILOGIN_MESSAGE_InfoMessageEndCheck(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_InfoMessageEnd(WIFILOGIN_MESSAGE_WORK* pWork);


//はい、いいえ選択
extern WIFILOGIN_YESNO_WORK* WIFILOGIN_MESSAGE_YesNoStart(WIFILOGIN_MESSAGE_WORK* pWork,int type);
extern void WIFILOGIN_MESSAGE_YesNoEnd( WIFILOGIN_YESNO_WORK* pWork );
extern void WIFILOGIN_MESSAGE_YesNoUpdate( WIFILOGIN_YESNO_WORK* pWork );
extern BOOL WIFILOGIN_MESSAGE_YesNoIsFinish( const WIFILOGIN_YESNO_WORK* pWork );
extern u8 WIFILOGIN_MESSAGE_YesNoGetCursorPos( const WIFILOGIN_YESNO_WORK* pWork );


//システムメッセージ描画
extern void WIFILOGIN_MESSAGE_SystemMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid);
extern void WIFILOGIN_MESSAGE_SystemMessageEnd(WIFILOGIN_MESSAGE_WORK* pWork);
extern void WIFILOGIN_MESSAGE_ErrorMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid,int no);

//画面構築
extern WIFILOGIN_DISP_WORK* WIFILOGIN_DISP_Init(HEAPID id, WIFILOGIN_BG bg, WIFILOGIN_DISPLAY display );
extern void WIFILOGIN_DISP_Main(WIFILOGIN_DISP_WORK* pWork);
extern void WIFILOGIN_DISP_End(WIFILOGIN_DISP_WORK* pWork);
extern void WIFILOGIN_DISP_SettingSubBgControl(WIFILOGIN_DISP_WORK* pWork);
extern void WIFILOGIN_DISP_StartSmoke(WIFILOGIN_DISP_WORK* pWork);



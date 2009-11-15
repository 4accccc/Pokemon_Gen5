
#pragma once

#include <gflib.h>



typedef struct _GTSNEGO_MESSAGE_WORK  GTSNEGO_MESSAGE_WORK;
typedef struct _GTSNEGO_DISP_WORK  GTSNEGO_DISP_WORK;



extern GTSNEGO_MESSAGE_WORK* GTSNEGO_MESSAGE_Init(HEAPID id,int msgid);
extern void GTSNEGO_MESSAGE_Main(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_End(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_InfoMessageDisp(GTSNEGO_MESSAGE_WORK* pWork);
extern BOOL GTSNEGO_MESSAGE_InfoMessageEndCheck(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_InfoMessageEnd(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_YesNoStart(GTSNEGO_MESSAGE_WORK* pWork);
extern void GTSNEGO_MESSAGE_ButtonWindowCreate(int num,int* pMsgBuff,GTSNEGO_MESSAGE_WORK* pWork);


extern GTSNEGO_DISP_WORK* GTSNEGO_DISP_Init(HEAPID id);
extern void GTSNEGO_DISP_Main(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_End(GTSNEGO_DISP_WORK* pWork);
extern void GTSNEGO_DISP_SettingSubBgControl(GTSNEGO_DISP_WORK* pWork);

